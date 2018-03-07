#include "GreaterThan.hpp"
#include "util/Timer.hpp"
#include "HElib/FHE.h"
#include <cmath>
std::pair<double, double> mean_std(std::vector<double> const& vals)
{
    size_t sze = vals.size();
    if (sze <= 1)
        return {0., 0.};

    double mean = 0.;
    for (double v : vals) mean += v;

    double std = 0.;
    mean /= sze;
    for (double v : vals)
        std += (v - mean) * (v - mean);
    std = std::sqrt(std / (sze - 1));
    return {mean, std};
}

int main(int argc, char *argv[]) {
    ArgMapping amap;
	long m = 8192;
	long p = 113; // small domain
    long L = 4;
    amap.arg("m", m, "m");
    amap.arg("p", p, "p");
    amap.arg("L", L, "L");
    amap.parse(argc, argv);


	FHEcontext context(m, p, 1);
	buildModChain(context, L);
	std::cout << "kappa " << context.securityLevel() << std::endl;

	FHESecKey sk(context);
	sk.GenSecKey(64);
	setup_auxiliary_for_greater_than(&sk);
	FHEPubKey pk = sk;
    std::vector<double> times[3];
    for (long _it = 0; _it < 100; _it++) {
        Timer timer(nullptr);
        auto start_ = Timer::Clock::now();
        Ctxt enc_a = encrypt_in_degree(127, pk);
        Ctxt enc_b = encrypt_in_degree(48, pk);
        auto end_ = Timer::Clock::now();
        times[0].push_back(timer.as_millsecond(end_ - start_));

        auto gt_args = create_greater_than_args(0, 1, context);

        start_ = Timer::Clock::now();
        auto ans = greater_than(enc_a, enc_b, context);
        end_ = Timer::Clock::now();
        times[1].push_back(timer.as_millsecond(end_ - start_));

        NTL::ZZX ply;
        start_ = Timer::Clock::now();
        sk.Decrypt(ply, ans);
        end_ = Timer::Clock::now();
        times[2].push_back(timer.as_millsecond(end_ - start_));
    }

    auto ms = mean_std(times[0]);
    std::cout << ms.first << " " << ms.second << "\n";
    ms = mean_std(times[1]);
    std::cout << ms.first << " " << ms.second << "\n";
    ms = mean_std(times[2]);
    std::cout << ms.first << " " << ms.second << "\n";
	return 0;
}
