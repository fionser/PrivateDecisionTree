#include "GreaterThan.hpp"
#include "util/Timer.hpp"
#include "HElib/FHE.h"

int main() {
	long m = 8192;
	long p = 113; // small domain
	FHEcontext context(m, p, 1);
	buildModChain(context, 4);
	std::cout << "kappa " << context.securityLevel() << std::endl;
	
	FHESecKey sk(context);
	sk.GenSecKey(64);
	setup_auxiliary_for_greater_than(&sk);
	FHEPubKey pk = sk;
	
	Timer timer(nullptr);
	auto start_ = Timer::Clock::now();
	Ctxt enc_a = encrypt_in_degree(127, pk);
	Ctxt enc_b = encrypt_in_degree(48, pk);
	auto end_ = Timer::Clock::now();
	std::cout << "ENC " << timer.as_millsecond(end_ - start_) << std::endl;

	auto gt_args = create_greater_than_args(0, 1, context);

	start_ = Timer::Clock::now();
	auto ans = greater_than(enc_a, enc_b, context);
	end_ = Timer::Clock::now();
	std::cout << "EVAL " << timer.as_millsecond(end_ - start_) << std::endl;

	NTL::ZZX ply;
	start_ = Timer::Clock::now();
	sk.Decrypt(ply, ans);
	end_ = Timer::Clock::now();
	std::cout << "DEC " << timer.as_millsecond(end_ - start_) << std::endl;
	std::cout << ply[0] << std::endl;
	return 0;
}
