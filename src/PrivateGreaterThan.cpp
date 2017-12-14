//
// Created by riku on 10/4/17.
//

#include "GreaterThan.hpp"
#include <HElib/FHE.h>
#include <NTL/ZZ_pX.h>
static void encodeOnDegree(NTL::ZZX *poly, long degree, const FHEcontext &context) {
    if (!poly)
        return;
    long m = context.zMStar.getM();
    long phiM = phi_N(m);
    poly->SetLength(phiM);
    while (degree < 0) 
        degree += phiM;
    degree %= phiM; 
    NTL::SetCoeff(*poly, degree, 1);
}

/// Create a testing vector: 1 + X + X^2 + ... + X^{N-1}.
/// N = phiN(M).
static NTL::ZZX create_test_v(FHEcontext const& context) {
    NTL::ZZX test_v;
    long m = context.zMStar.getM();
    long phiM = phi_N(m);
    if (phiM != (m >> 1))
        std::cerr << "WARNING! m should be power of 2" << std::endl;
    test_v.SetLength(phiM);
    for (long i = 0; i < phiM; i++)
        NTL::SetCoeff(test_v, i, 1);
    return test_v;
}
/// Generate a random polynomial from the plaintext
static NTL::ZZX generate_random(FHEcontext const& context) {
    /// (riku) Should backup the current modulus.
    NTL::zz_pBak backup; backup.save();
    NTL::zz_p::init(context.alMod.getPPowR());
    NTL::zz_pX poly;
    NTL::random(poly, context.zMStar.getPhiM());
    backup.restore();
    return NTL::conv<NTL::ZZX>(poly);
}

static void check_auxiliary(FHEPubKey const& pk) {
    long M = pk.getContext().zMStar.getM();
    assert(pk.haveKeySWmatrix(1, M - 1, 0, 0) && "Call setup_auxiliary_for_greater_than.");
}

void setup_auxiliary_for_greater_than(FHESecKey *sk) {
    if (!sk)
        return;
    long M = sk->getContext().zMStar.getM();
    long phiM = phi_N(M);
    if (phiM != (M >> 1))
        std::cerr << "Warning! M should be power of 2" << std::endl;
    /// We use F(X) --> F(X^{m - 1}) automorph when doing the private greater than.
    sk->GenKeySWmatrix(1, M - 1, 0, 0);
    sk->setKeySwitchMap();
}
/// Privately comparing two encrypted values (in a proper form).
/// The return value is determined by GreaterThanArgs.
Ctxt greater_than(Ctxt const& ctx_a, Ctxt const& ctx_b,
                  GreaterThanArgs const& args,
                  FHEcontext const& context) {
    check_auxiliary(ctx_a.getPubKey()); //sanity check
    Ctxt b_copy(ctx_b);
    smart_negate_degree(&b_copy, context); // X^{-b}
    b_copy.multiplyBy(ctx_a); // X^a * X^{-b}

    b_copy.multByConstant((args.mu1 - args.one_half) * args.test_v);
    NTL::ZZX r;
    if (args.randomized) {
        r = generate_random(context);
        NTL::SetCoeff(r, 0, args.one_half); // Set the constant term 1/2
    } else {
        NTL::SetCoeff(r, 0, args.one_half); // Set the constant term 1/2
    }
    b_copy.addConstant(r);
    return b_copy;
}

static NTL::ZZX prepare_Xb(long b,
                           GreaterThanArgs const& args,
                           FHEcontext const& context) {
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

Ctxt greater_than(Ctxt const& ctx_a, long b,
                  GreaterThanArgs const& args,
                  FHEcontext const& context) {
    check_auxiliary(ctx_a.getPubKey()); //sanity check
    NTL::ZZX Xb = prepare_Xb(b, args, context);

    Ctxt result(ctx_a);
    result.multByConstant(Xb);

    NTL::ZZX r;
    if (args.randomized) {
        r = generate_random(context);
        NTL::SetCoeff(r, 0, args.one_half); // Set the constant term 1/2
    } else {
        NTL::SetCoeff(r, 0, args.one_half); // Set the constant term 1/2
    }
    result.addConstant(r);
    return result;
}

/// If greater then returns mu_0, else returns mu_1.
Ctxt greater_than(Ctxt const& ctx_a, Ctxt const& ctx_b, FHEcontext const& context) {
    GreaterThanArgs args = create_greater_than_args(0L, 1L, context);
    return greater_than(ctx_a, ctx_b, args, context);
}

Ctxt greater_than(Ctxt const& ctx_a, long b, FHEcontext const& context) {
    GreaterThanArgs args = create_greater_than_args(0L, 1L, context);
    return greater_than(ctx_a, b, args, context);
}

Ctxt equality_test(Ctxt const& ctx_a, Ctxt const& ctx_b, FHEcontext const& context, bool rnd) {
    check_auxiliary(ctx_a.getPubKey()); //sanity check
    NTL::ZZX test_v = create_test_v(context);
    NTL::SetCoeff(test_v, 0, 0L);

    Ctxt a_minus_b(ctx_b);
    smart_negate_degree(&a_minus_b, context);
    a_minus_b.multiplyBy(ctx_a); // X^{a - b}
    
    Ctxt helper(a_minus_b);
    helper.multByConstant(test_v);
    
    auto gt_args = create_greater_than_args(2L, 0L, context);
    a_minus_b.multByConstant((gt_args.mu1 - gt_args.one_half) * gt_args.test_v);
    a_minus_b.addConstant(NTL::to_ZZ(gt_args.one_half));
    
    helper += a_minus_b;
    if (rnd) {
        NTL::ZZX r = generate_random(context);
        NTL::SetCoeff(r, 0, 0L); // Set the constant term as zero
        helper.addConstant(r);
    }
    return helper;
}

GreaterThanArgs create_greater_than_args(long mu0, long mu1,
                                         FHEcontext const& context) {
    if (context.zMStar.getPhiM() != (context.zMStar.getM() >> 1))
        std::cerr << "Warning! the parameter m should be power of 2.";
    GreaterThanArgs args;
    args.mu0 = mu0;
    args.mu1 = mu1;
    long ptxt_space = context.alMod.getPPowR();
    args.one_half = (NTL::InvMod(2L, ptxt_space) * (mu0 + mu1)) % ptxt_space;
    args.test_v = create_test_v(context);
    args.randomized = true;
    return args;
}

void encrypt_in_degree(Ctxt &ctx, long val, FHEPubKey const& key) {
    FHEcontext const& context = key.getContext();
    NTL::ZZX poly;
    encodeOnDegree(&poly, val, context);
    key.Encrypt(ctx, poly);
}

void encrypt_in_degree(Ctxt &ctx, long val, FHESecKey const& key) {
    FHEcontext const& context = key.getContext();
    NTL::ZZX poly;
    encodeOnDegree(&poly, val, context);
    key.Encrypt(ctx, poly);
}

Ctxt encrypt_in_degree(long val, FHEPubKey const& key) {
    Ctxt cipher(key);
    FHEcontext const& context = key.getContext();
    NTL::ZZX poly;
    encodeOnDegree(&poly, val, context);
    key.Encrypt(cipher, poly);
    return cipher;
}

Ctxt encrypt_in_degree(long val, FHESecKey const& key) {
    Ctxt cipher(key);
    FHEcontext const& context = key.getContext();
    NTL::ZZX poly;
    encodeOnDegree(&poly, val, context);
    key.Encrypt(cipher, poly);
    return cipher;
}
/// F(X^a) --> F(X^{-a}) then apply the keyswtiching.
void smart_negate_degree(Ctxt *ctx, FHEcontext const& context) {
    if (!ctx)
        return;
    long M = context.zMStar.getM();
    ctx->smartAutomorph(M - 1);
}

Ctxt count_less_than(Ctxt const& ctx_a, 
                     std::vector<Ctxt> const& ctx_b_vec, 
                     FHEcontext const& context) {
    Ctxt sum_b(ctx_a.getPubKey());
    for (auto const& b : ctx_b_vec)
        sum_b += b;
    /// [X^b] --> [X^-b]
    smart_negate_degree(&sum_b, context);
    /// return 1 for greater, o.w. return 0
    GreaterThanArgs gt_args = create_greater_than_args(1, 0, context);
    sum_b.multiplyBy(ctx_a);
    NTL::ZZX T = (gt_args.mu1 - gt_args.one_half) * gt_args.test_v;
    sum_b.multByConstant(T);
    long n = ctx_b_vec.size() * gt_args.one_half;
    sum_b.addConstant(NTL::to_ZZ(n));
    return sum_b;
}
