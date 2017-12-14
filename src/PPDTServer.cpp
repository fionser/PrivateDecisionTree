#include "network/PPDT.hpp"
#include "GreaterThan.hpp"
#include "util/literal.hpp"
#include "util/Timer.hpp"

#include <HElib/FHE.h>
#include <HElib/FHEContext.h>
#include <HElib/EncryptedArray.h>

#include <vector>
#include <iostream>
#include <memory>

struct Tree {
    // static std::atomic<size_t> counter;
    /// WARN: not thread safe
    Tree() : feature_index(-1), id(-1), left(nullptr), right(nullptr) {
        //id = Tree::counter.fetch_add(1);
    }

    long feature_index;
    long id;
    struct Tree *left;
    struct Tree *right;

    bool is_leaf() const {
        return !this->left and !this->right;
    }

    void free_tree(Tree *root) {
        if (!root) 
            return;
        if (root->is_leaf()) {
            delete root;
        } else {
            free_tree(root->left);
            free_tree(root->right);
        }
    }

    void print() {
        print(this, "");
    }

    void print(Tree *root, std::string path) {
        if (!root)
            return;
        if (root->is_leaf()) {
            std::cout << path << " " << root->id << std::endl;
        } else {
            path = path + " " + std::to_string(root->id);
            print(root->right, path);
            print(root->left, path);
        }
    }
};
// std::atomic<size_t> Tree::counter(0);

/// create X^{-b} mod X^m + 1
static NTL::ZZX prepare_Xb(long b, GreaterThanArgs const& args, FHEcontext const& context) {
	auto T(args.test_v);
    long m = context.zMStar.getPhiM();
    /// only works for X^N + 1 ring
    assert(context.zMStar.getM() == (m << 1));
    assert(b >= 0 && b < m);
    T *= (args.ngt() - args.one_half);
    if (b == 0)
        return T;
    for (long i = 0; i < b; i++) {
        auto coeff = NTL::coeff(T, m - 1 - i);
        NTL::SetCoeff(T, m - 1 - i, -coeff);
    }
    return T;
}

static void sample_random_poly(NTL::ZZX &poly, const long p) {
    const long deg = NTL::deg(poly);
    for (long i = 1; i <= deg; i++)
        NTL::SetCoeff(poly, i, NTL::RandomBnd(p));
}

struct PathNode_t {
    long feature_index;
    long id;
    const Tree *node;
    PathNode_t() : feature_index(-1), id(-1), node(nullptr) {
    }
};
using Path_t = std::vector<PathNode_t>;

struct PPDTServer::Imp {
    using ctx_ptr_t = std::unique_ptr<Ctxt>;
    Imp() {}

    ~Imp() { root->free_tree(root); delete root;}

    bool load(std::string const& file) {
        std::ifstream fd(file);
        if (!fd.is_open())
            return false;
        bool ok = true;
        ok &= load_threshold(fd);
        ok &= load_mapping(fd);
        ok &= load_path(fd);
        ok &= build_tree_from_path();
        fd.close();
        return ok;
    }

	bool recevie_features(std::vector<Ctxt> &features, 
						  FHEPubKey const& evk,
						  std::istream &conn) const {
        int32_t num;
        conn >> num;
        features.resize(num, evk);
        for (size_t i = 0; i < num; i++) {
            conn >> features[i];
        }
        return true;
    }

    bool recevie_evk(FHEPubKey &evk, std::istream &conn) const {
        conn >> evk;
        return true;
    }

    void compare_all_internal_nodes(Tree *root, 
									std::vector<Ctxt> const& features,
									FHEcontext const& context) {
        if (root->is_leaf())
            return;
        compare_all_internal_nodes(root->right, features, context);
        compare_all_internal_nodes(root->left, features, context);
        auto index = root->feature_index;
        auto id = root->id;
		NTL::ZZX threshold = prepare_Xb(thresholds_.at(id),
										gt_args_,
										context);
 		assert(greater_than_.find(root->id) == greater_than_.end());
        ctx_ptr_t f(new Ctxt(features.at(index)));
        f->multByConstant(threshold);
        greater_than_.insert(std::pair<int, ctx_ptr_t>(id, std::move(f)));
    }
    /// Walks along the path, and sum up the comparison results,
    void sum_along_path(ctx_ptr_t &result, Path_t const& path) const {
        if (path.empty())
            return ;
        size_t depth = path.size();
        for (size_t i = 0; i < depth; i++) {
            const Tree *node = path.at(i).node;
            assert(node);
            assert(node->feature_index == path.at(i).feature_index);
            if (node->is_leaf()) {
                assert(i + 1 == depth);
                break;
            }
            auto kv = greater_than_.find(node->id);
            assert(kv != greater_than_.end());
            result->addCtxt(*(kv->second));
        }
    }

    long count_left_nodes(Path_t const& path) const {
        if (path.empty())
            return 0;
        long left_node = 0;
        size_t depth = path.size();
        for (size_t i = 0; i < depth; i++) {
            const Tree *node = path.at(i).node;
            assert(node);
            assert(node->feature_index == path.at(i).feature_index);
            if (node->is_leaf()) {
                assert(i + 1 == depth);
                break;
            }
            if (node->right->feature_index != path.at(i + 1).feature_index)
                left_node += 1;
        }
        return left_node;
    }

    void sum_up_paths(FHEPubKey const& evk) {
        const size_t paths_cnt = paths_.size();
        const long phim = evk.getContext().zMStar.getPhiM();
        const long p = evk.getContext().zMStar.getP();

        summations_.resize(paths_cnt);
        labeled_.resize(paths_cnt);
//#pragma omp parallel
        for (size_t i = 0; i < paths_cnt; i++) {
            summations_[i].reset(new Ctxt(evk));
            sum_along_path(summations_[i], paths_[i]);
            long left_nodes_cnt = count_left_nodes(paths_[i]);
            long depth = (paths_[i].size() - 1);
            long modification = gt_args_.one_half * depth + left_nodes_cnt;
            NTL::ZZX random;
            random.SetLength(phim);
            sample_random_poly(random, p);
            NTL::SetCoeff(random, 0, modification);
            summations_[i]->addConstant(random);
            /// duplicate summations_[i]
            labeled_[i].reset(new Ctxt(*summations_[i])); 
        }
    }

    long random_non_zero(const long p) const {
        long ret = 0;
        do {
            ret = NTL::RandomBnd(p);
        } while (ret == 0);
        return ret;
    }

    void randomize(const long p) {
        const size_t paths_cnt = paths_.size();

#pragma omp parallel
        for (size_t i = 0; i < paths_cnt; i++) {
            NTL::ZZX non_zero_random(0, 1);
            long label = i; // TODO(riku) to use the true label
            NTL::SetCoeff(non_zero_random, 0, random_non_zero(p));
            labeled_[i]->multByConstant(non_zero_random);
            labeled_[i]->addConstant(NTL::to_ZZX(i));
            /// use two independent rands.
            NTL::SetCoeff(non_zero_random, 0, random_non_zero(p)); 
            summations_[i]->multByConstant(non_zero_random);
            /// mod down to lowest level to reduce communication cost
            labeled_[i]->modDownToLevel(1);
            summations_[i]->modDownToLevel(1);
        }
    }

    void response_result(tcp::iostream &conn) const {
        assert(labeled_.size() == summations_.size());
        int32_t num = labeled_.size();
        conn << (num << 1);
        for (size_t i = 0; i < labeled_.size(); i++) {
            conn << (*summations_[i]);
            conn << (*labeled_[i]);
        }
    }

    void run(tcp::iostream &conn) {
        double end2end_time;
        Timer *end2end = new Timer(&end2end_time);
        FHEcontext context = receive_context(conn);
        /// return 0 for greater, 1 other wise.
		gt_args_ = create_greater_than_args(0L, 1L, context);
        FHEPubKey evk(context);
        if (!recevie_evk(evk, conn)) {
            std::cerr << "Error happned when to recevie evaluation key\n";
            return;
        }

        std::vector<Ctxt> features;
        if (!recevie_features(features, evk, conn)) {
            std::cerr << "Error happned when to recevie features\n";
            return;
        }

        double evl_time;
        do {
            Timer timer(&evl_time);
            compare_all_internal_nodes(root, features, context);
            sum_up_paths(evk);  
            randomize(context.zMStar.getP());
            response_result(conn);
        } while(0);
        delete end2end;
        std::cout << "EVAL ALL" << std::endl;
        printf("%.3f %.3f\n", evl_time, end2end_time);
    }

    /// threshold format: i1,i2,i3, ...
    bool load_threshold(std::istream &fd) {
        std::string line;
        std::getline(fd, line, '\n');
        auto fields = util::split_by(line, ',');
        if (fields.empty())
            return false;
        thresholds_.resize(fields.size());
        bool ok = true;
        std::transform(fields.cbegin(), fields.cend(), thresholds_.begin(),
                       [&ok](const std::string &field) -> long {
                           auto f = util::trim(field);
                           size_t pos;
                           long val = std::stol(f, &pos, 10);
                           if (pos != f.size())
                               ok = false;
                           return val;  
                       });
        return ok;
    }

    bool load_mapping(std::istream &fd) {
        std::string line;
        std::getline(fd, line, '\n');
        auto fields = util::split_by(line, ',');
        if (fields.empty())
            return false;
        id_2_feature_index_.clear();
        bool ok = true;
        for (const auto &field : fields) {
            auto pair = util::split_by(field, ':');
            if (pair.size() != 2)
                return false;
            long id = std::stol(pair[0], nullptr, 10);
            long feature_index = std::stol(pair[1], nullptr, 10);
            id_2_feature_index_.insert({id, feature_index});
        }
        return ok;
    }
    /// path format: [id1, id2, id3, ...]
    bool load_path(std::istream &fd) {
        paths_.clear();
        for (std::string line; std::getline(fd, line); ) {
            size_t len = line.size();
            if (len == 0 or line[0] != '[' or line[len - 1] != ']') {
                std::cerr << "An invalid path data: " << line << std::endl;
                return false;
            }

            std::string raw_data = line.substr(1, len - 2);
            auto fields = util::split_by(raw_data, ',');
            Path_t path;
            if (!parse_path(path, fields)) {
                std::cerr << "Can not parse this path data: " << line << std::endl;
                return false;
            } else {
                paths_.push_back(path);
            }
        }
        return true;
    }

    bool parse_path(Path_t &path, std::vector<std::string> const& fields) const {
        for (auto field : fields) {
            field = util::trim(field);
            size_t pos;
            long id = std::stol(field, &pos, 10);
            if (pos != field.size()) {
                std::cerr << "Invalid field " << field << "\n";
                return false;
            }
            if (id < 0) {
                std::cerr << "Invalid id " << id << "\n";
                return false;
            }
            PathNode_t pn;
            pn.id = id;
            auto kv = id_2_feature_index_.find(id);
            assert(kv != id_2_feature_index_.end());
            pn.feature_index = kv->second;
            if (pn.feature_index < 0)
                pn.feature_index = -1;
            path.push_back(pn);
        }
        return true;
    }

    bool build_tree_from_path() {
        root = new Tree();
        for (auto &path : paths_) {
            insert_path_to_tree(root, 0, path);
        }
        // root->print();
        return true;
    }

    void insert_path_to_tree(Tree *tree, size_t pos, Path_t &path) {
        if (pos >= path.size())
            return ;
        if (!tree)
            return ;
        bool has_next = (pos + 1) < path.size();
        path.at(pos).node = tree;
        if (tree->id == -1) {
            tree->id = path.at(pos).id;
            tree->feature_index = path.at(pos).feature_index;
            if (has_next)
                tree->right = new Tree();
            insert_path_to_tree(tree->right, pos + 1, path);
        } else {
            assert(tree->id == path.at(pos).id);
            if (tree->right) {
                if (has_next and tree->right->id == path.at(pos + 1).id) {
                    insert_path_to_tree(tree->right, pos + 1, path);
                } else {
                    if (!tree->left and has_next)
                        tree->left = new Tree();
                    insert_path_to_tree(tree->left, pos + 1, path);
                }
            } else {
                if (has_next)
                     tree->right = new Tree();
                insert_path_to_tree(tree->right, pos + 1, path);
            }
        }
    }

    std::vector<long> thresholds_;
    std::map<long, long> id_2_feature_index_;
    std::vector<Path_t> paths_;
    std::map<size_t, ctx_ptr_t> greater_than_;
    std::vector<ctx_ptr_t> summations_, labeled_;
    GreaterThanArgs gt_args_;
    Tree *root;
};

bool PPDTServer::load(std::string const& file) {
    if (!imp_)
        imp_.reset(new PPDTServer::Imp());
    return imp_->load(file);
}

void PPDTServer::run(tcp::iostream &conn) {
    if (imp_) 
        imp_->run(conn);
    else
        std::cerr << "call PPDTServer::load first" << std::endl;
}
