#include "network/PPDT.hpp"
#include "network/net_io.hpp"
#include "util/Timer.hpp"
#include "GreaterThan.hpp"
#include <HElib/FHE.h>
#include <HElib/FHEContext.h>


struct PPDTClient::Imp {
    Imp() {}
    ~Imp() {}
    bool load(std::string const& file) {
        features_.resize(57);
        // fake features for debug
        for (long i = 0; i < features_.size(); i++)
            features_[i] = 10 * (i + 1);
        return true;
    }

    void send_context(FHEcontext const&context, 
                      std::ostream &conn) const {
        ::send_context(conn, context);
    }

    void send_evk(FHESecKey const& sk,
                  std::ostream &conn) const {
        FHEPubKey ek(sk);
        ek.makeSymmetric();
        conn << ek;
    }

    void encrypt_feature(FHESecKey const& sk) {
        Timer timer(&enc_time_);
        enc_features_.resize(features_.size(), sk);
        for (size_t i = 0; i < features_.size(); i++) {
            encrypt_in_degree(enc_features_[i], features_[i], sk);
        }
    }

    void send_encrypted_features(std::ostream &conn) const {
        int32_t num = enc_features_.size();
        conn << num << '\n';
        for (const auto &ctx : enc_features_)
            conn << ctx;
    }

    long wait_result(FHESecKey const& sk, 
                     std::istream &conn) {
        /// notice that this time include some network
        Timer timer(&dec_time_);
        int32_t num;
        conn >> num;
        Ctxt summation(sk), label(sk);
        bool need_dec = true;
        long prediction = -1;
        for (int32_t i = 0; i < num; i += 2) {
            conn >> summation;
            conn >> label;
            if (need_dec) {
                if (!label.isCorrect())
                    std::cerr << "Warn. The decryption might fail" << std::endl;
                NTL::ZZX poly;
                sk.Decrypt(poly, summation);
                if (NTL::coeff(poly, 0) == 0) {
                    sk.Decrypt(poly, label);
                    prediction = NTL::to_long(NTL::coeff(poly, 0));
                    need_dec = false;
                }
            } 
        }
        return prediction;
    }

    void run(tcp::iostream &conn) {
        long m = 4096 << 1;
        long p = 1031;
        long L = 3;
        FHEcontext context(m, p, 1);
        context.bitsPerLevel += 1;
        buildModChain(context, L);
        std::cout << "kappa " << context.securityLevel() << std::endl;
        send_context(context, conn);

        FHESecKey sk(context);
        sk.GenSecKey(64);
        setup_auxiliary_for_greater_than(&sk);
        long label;
        do {
            Timer timer(&end2end_time_);
            send_evk(sk, conn);
            encrypt_feature(sk);

            send_encrypted_features(conn);
            label = wait_result(sk, conn);
        } while(0);
        std::cout << "prediction label is " << label << std::endl;
        std::cout << "ENC DEC ALL\n" << std::endl;
        printf("%.3f %.3f %.3f\n", enc_time_, dec_time_, end2end_time_);
    }

    std::vector<long> features_;
    std::vector<Ctxt> enc_features_;
    double enc_time_, dec_time_, end2end_time_;
};

bool PPDTClient::load(std::string const& file) {
    if (!imp_)
        imp_.reset(new PPDTClient::Imp());
    return imp_->load(file);
}

void PPDTClient::run(tcp::iostream &conn) {
    if (imp_) 
        imp_->run(conn);
    else
        std::cerr << "call PPDTClient::load first" << std::endl;
}
