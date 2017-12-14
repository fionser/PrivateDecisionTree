#ifndef PRIVATE_DECISION_TREE_GREATER_THAN_HPP
#define PRIVATE_DECISION_TREE_GREATER_THAN_HPP
#include <NTL/ZZX.h>
#include <vector>
/// Arguments for private greater than.
/// Return mu0 if greater, otherwise return mu1
struct GreaterThanArgs {
    long mu0;
    long mu1;
    long one_half; // (mu0 + mu1) / 2
    bool randomized; // Add randomized poly. False for debug.
    NTL::ZZX test_v; // a polynomial in the form of 1 + X + X^2 + ...
    long gt() const {
        return mu0;
    }
    long ngt() const {
        return mu1;
    }
};

class FHEcontext; // From HElib
class FHESecKey; // From HElib
class FHEPubKey; // From HElib
class Ctxt; // From HElib
/// Create a GreaterThanArgs for the private greater than.
/// Return (a cipher of) mu0 if the A > B, otherwise return mu1.
GreaterThanArgs create_greater_than_args(long mu0, long mu1, FHEcontext const& context);

/// Encrypt the value into the degree of the polynomial.
Ctxt encrypt_in_degree(long value, FHEPubKey const& key);
Ctxt encrypt_in_degree(long value, FHESecKey const& key);
void encrypt_in_degree(Ctxt &ctx, long val, FHEPubKey const& key);
void encrypt_in_degree(Ctxt &ctx, long val, FHESecKey const& key);

/// Add the necessary key switching matrix into the key.
/// This method should be called before calling the private greater than.
void setup_auxiliary_for_greater_than(FHESecKey *sk);

/// Privately comparing two encrypted values (in a proper form).
/// The return value is determined by GreaterThanArgs.
Ctxt greater_than(Ctxt const&a, Ctxt const &b, GreaterThanArgs const& args, FHEcontext const& context);
Ctxt greater_than(Ctxt const&a, long b, GreaterThanArgs const& args, FHEcontext const& context);

/// Privately comparing two encrypted values (in a proper form).
/// Return a cipher that encrypts 0 if the value of ctx_a is greater than the value of ctx_b.
/// Otherwise, return a cipher that encrypts 1.
Ctxt greater_than(Ctxt const& ctx_a, Ctxt const& ctx_b, FHEcontext const& context);
Ctxt greater_than(Ctxt const& ctx_a, long b, FHEcontext const& context);

/// Privately accounting how many values in ctx_b_vec is less than the value encrypted in ctxt_a.
/// That is to return a ciphertext that encrypts this cardinality |{i| b_i < a}| in its 0-th coefficient.
Ctxt count_less_than(Ctxt const& ctxt_a, std::vector<Ctxt> const& ctx_b_vec, FHEcontext const& context);

/// Privately comparing two encrypted values.
/// Return a cipher of 0 if the two values are equal, otherwise return a cipher of 1.
Ctxt equality_test(Ctxt const& ctx_a, Ctxt const& ctx_b, FHEcontext const& context, bool randomized = true);

/// E(X^a) --> E(X^{-a})
void smart_negate_degree(Ctxt *ctx, FHEcontext const& context);
#endif // PRIVATE_GREATER_THAN_GREATER_THAN_HPP
