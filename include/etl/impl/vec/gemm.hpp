//=======================================================================
// Copyright (c) 2014-2017 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "etl/impl/vec/gemm_blis.hpp" // BLIS-Like optimized kernel

// Allocations to row major
#include "etl/impl/vec/gemm_rr_to_r.hpp"
#include "etl/impl/vec/gemm_cr_to_r.hpp"
#include "etl/impl/vec/gemm_rc_to_r.hpp"

// Allocations to column major
#include "etl/impl/vec/gemm_cc_to_c.hpp"
#include "etl/impl/vec/gemm_cr_to_c.hpp"
#include "etl/impl/vec/gemm_rc_to_c.hpp"

// The idea of the GEMM kernels is largely inspired by the kernels in Blaze by
// Klaus Igleberg

namespace etl {

namespace impl {

namespace vec {

/*!
 * \brief Optimized version of GEMM for row major version
 *
 * \param a The lhs matrix
 * \param b The rhs matrix
 * \param c The result matrix
 */
template <typename A, typename B, typename C, cpp_enable_iff(all_row_major<A, B, C> && all_homogeneous<A, B, C> && all_vectorizable<vector_mode, A, B, C>)>
void gemm(A&& a, B&& b, C&& c) {
    a.ensure_cpu_up_to_date();
    b.ensure_cpu_up_to_date();

    const size_t M = etl::rows(a);
    const size_t N = etl::columns(b);
    const size_t K = etl::columns(a);

    gemm_rr_to_r(a.memory_start(), b.memory_start(), c.memory_start(), M, N, K);

    c.invalidate_gpu();
}

/*!
 * \brief Unoptimized version of GEMM for column major version
 * \param a The lhs matrix
 * \param b The rhs matrix
 * \param c The result matrix
 */
template <typename A, typename B, typename C, cpp_enable_iff(all_column_major<A, B, C> && all_homogeneous<A, B, C> && all_vectorizable<vector_mode, A, B, C>)>
void gemm(A&& a, B&& b, C&& c) {
    a.ensure_cpu_up_to_date();
    b.ensure_cpu_up_to_date();

    const size_t M = etl::rows(a);
    const size_t N = etl::columns(b);
    const size_t K = etl::columns(a);

    gemm_cc_to_c(a.memory_start(), b.memory_start(), c.memory_start(), M, N, K);

    c.invalidate_gpu();
}

/*!
 * \brief Optimized version of GEMM for C = trans(A) * B where all matrices are
 * stored in row-major order.
 *
 * \param a The lhs matrix (row major)
 * \param b The rhs matrix (transposed row major)
 * \param c The result matrix (row major)
 */
template <typename A, typename B, typename C, cpp_enable_iff(all_row_major<B, C> && is_column_major<A> && all_homogeneous<A, B, C> && all_vectorizable<vector_mode, A, B, C>)>
void gemm(A&& a, B&& b, C&& c) {
    a.ensure_cpu_up_to_date();
    b.ensure_cpu_up_to_date();

    const size_t M = etl::rows(a);
    const size_t N = etl::columns(b);
    const size_t K = etl::columns(a);

    gemm_cr_to_r(a.memory_start(), b.memory_start(), c.memory_start(), M, N, K);

    c.invalidate_gpu();
}

/*!
 * \brief Optimized version of GEMM for C = trans(A) * B where all matrices are
 * stored in row-major order.
 *
 * \param a The lhs matrix (row major)
 * \param b The rhs matrix (transposed row major)
 * \param c The result matrix (row major)
 */
template <typename A, typename B, typename C, cpp_enable_iff(all_row_major<A, C> && is_column_major<B> && all_homogeneous<A, B, C> && all_vectorizable<vector_mode, A, B, C>)>
void gemm(A&& a, B&& b, C&& c) {
    a.ensure_cpu_up_to_date();
    b.ensure_cpu_up_to_date();

    const size_t M = etl::rows(a);
    const size_t N = etl::columns(b);
    const size_t K = etl::columns(a);

    gemm_rc_to_r(a.memory_start(), b.memory_start(), c.memory_start(), M, N, K);

    c.invalidate_gpu();
}

/*!
 * \brief Optimized version of GEMM for C = trans(A) * B where all matrices are
 * stored in row-major order.
 *
 * \param a The lhs matrix (row major)
 * \param b The rhs matrix (transposed row major)
 * \param c The result matrix (row major)
 */
template <typename A, typename B, typename C, cpp_enable_iff(is_row_major<C> && all_column_major<A, B> && all_homogeneous<A, B, C> && all_vectorizable<vector_mode, A, B, C>)>
void gemm(A&& a, B&& b, C&& c) {
    const size_t M = etl::rows(a);
    const size_t N = etl::columns(b);
    const size_t K = etl::columns(a);

    if(etl::size(a) < etl::size(b)){
        auto t_a = force_temporary_opp(a);

        t_a.ensure_cpu_up_to_date();
        b.ensure_cpu_up_to_date();

        gemm_rc_to_r(t_a.memory_start(), b.memory_start(), c.memory_start(), M, N, K);
    } else {
        auto t_b = force_temporary_opp(b);

        t_b.ensure_cpu_up_to_date();
        a.ensure_cpu_up_to_date();

        gemm_cr_to_r(a.memory_start(), t_b.memory_start(), c.memory_start(), M, N, K);
    }

    c.invalidate_gpu();
}

/*!
 * \brief Optimized version of GEMM for C = trans(A) * B where all matrices are
 * stored in row-major order.
 *
 * \param a The lhs matrix (row major)
 * \param b The rhs matrix (transposed row major)
 * \param c The result matrix (row major)
 */
template <typename A, typename B, typename C, cpp_enable_iff(is_row_major<A> && all_column_major<B, C> && all_homogeneous<A, B, C> && all_vectorizable<vector_mode, A, B, C>)>
void gemm(A&& a, B&& b, C&& c) {
    a.ensure_cpu_up_to_date();
    b.ensure_cpu_up_to_date();

    const size_t M = etl::rows(a);
    const size_t N = etl::columns(b);
    const size_t K = etl::columns(a);

    gemm_rc_to_c(a.memory_start(), b.memory_start(), c.memory_start(), M, N, K);

    c.invalidate_gpu();
}

/*!
 * \brief Optimized version of GEMM for C = trans(A) * B where all matrices are
 * stored in row-major order.
 *
 * \param a The lhs matrix (row major)
 * \param b The rhs matrix (transposed row major)
 * \param c The result matrix (row major)
 */
template <typename A, typename B, typename C, cpp_enable_iff(is_row_major<B> && all_column_major<A, C> && all_homogeneous<A, B, C> && all_vectorizable<vector_mode, A, B, C>)>
void gemm(A&& a, B&& b, C&& c) {
    a.ensure_cpu_up_to_date();
    b.ensure_cpu_up_to_date();

    const size_t M = etl::rows(a);
    const size_t N = etl::columns(b);
    const size_t K = etl::columns(a);

    gemm_cr_to_c(a.memory_start(), b.memory_start(), c.memory_start(), M, N, K);

    c.invalidate_gpu();
}

/*!
 * \brief Optimized version of GEMM for C = trans(A) * B where all matrices are
 * stored in row-major order.
 *
 * \param a The lhs matrix (row major)
 * \param b The rhs matrix (transposed row major)
 * \param c The result matrix (row major)
 */
template <typename A, typename B, typename C, cpp_enable_iff(is_column_major<C> && all_row_major<A, B> && all_homogeneous<A, B, C> && all_vectorizable<vector_mode, A, B, C>)>
void gemm(A&& a, B&& b, C&& c) {
    const size_t M = etl::rows(a);
    const size_t N = etl::columns(b);
    const size_t K = etl::columns(a);

    if(etl::size(a) < etl::size(b)){
        auto t_a = force_temporary_opp(a);

        t_a.ensure_cpu_up_to_date();
        b.ensure_cpu_up_to_date();

        gemm_cr_to_c(t_a.memory_start(), b.memory_start(), c.memory_start(), M, N, K);
    } else {
        auto t_b = force_temporary_opp(b);

        t_b.ensure_cpu_up_to_date();
        a.ensure_cpu_up_to_date();

        gemm_rc_to_c(a.memory_start(), t_b.memory_start(), c.memory_start(), M, N, K);
    }

    c.invalidate_gpu();
}

/*!
 * \brief Optimized version of GEMM for C = trans(A) * B where all matrices are
 * stored in row-major order.
 *
 * \param a The lhs matrix (row major)
 * \param b The rhs matrix (transposed row major)
 * \param c The result matrix (row major)
 */
template <typename A, typename B, typename C, cpp_enable_iff(all_row_major<A, B, C> && all_homogeneous<A, B, C> && all_vectorizable<vector_mode, A, B, C>)>
void gemm_tn(A&& a, B&& b, C&& c) {
    a.ensure_cpu_up_to_date();
    b.ensure_cpu_up_to_date();

    const size_t M = etl::columns(a); // rows(trans(A)) = rows(C)
    const size_t N = etl::columns(b); // columns (B) = columns(C)
    const size_t K = etl::rows(a);    // columns(trans(A)) = rows(B)

    gemm_cr_to_r(a.memory_start(), b.memory_start(), c.memory_start(), M, N, K);

    c.invalidate_gpu();
}

/*!
 * \brief Optimized version of GEMM for C = trans(A) * B where all matrices are
 * stored in column-major order.
 *
 * \param a The lhs matrix (row major)
 * \param b The rhs matrix (transposed row major)
 * \param c The result matrix (row major)
 */
template <typename A, typename B, typename C, cpp_enable_iff(all_column_major<A, B, C> && all_homogeneous<A, B, C> && all_vectorizable<vector_mode, A, B, C>)>
void gemm_tn(A&& a, B&& b, C&& c) {
    a.ensure_cpu_up_to_date();
    b.ensure_cpu_up_to_date();

    const size_t M = etl::columns(a); // rows(trans(A)) = rows(C)
    const size_t N = etl::columns(b); // columns (B) = columns(C)
    const size_t K = etl::rows(a);    // columns(trans(A)) = rows(B)

    gemm_rc_to_c(a.memory_start(), b.memory_start(), c.memory_start(), M, N, K);

    c.invalidate_gpu();
}

/*!
 * \brief Optimized version of GEMM for C = A * trans(B) where all matrices are
 * stored in row-major order.
 *
 * \param a The lhs matrix (row major)
 * \param b The rhs matrix (transposed row major)
 * \param c The result matrix (row major)
 */
template <typename A, typename B, typename C, cpp_enable_iff(all_row_major<A, B, C> && all_homogeneous<A, B, C> && all_vectorizable<vector_mode, A, B, C>)>
void gemm_nt(A&& a, B&& b, C&& c) {
    a.ensure_cpu_up_to_date();
    b.ensure_cpu_up_to_date();

    const size_t M = etl::rows(a);
    const size_t N = etl::rows(b);
    const size_t K = etl::columns(a);

    gemm_rc_to_r(a.memory_start(), b.memory_start(), c.memory_start(), M, N, K);

    c.invalidate_gpu();
}

/*!
 * \brief Optimized version of GEMM for C = A * trans(B) where all matrices are
 * stored in colum-major order.
 *
 * \param a The lhs matrix (row major)
 * \param b The rhs matrix (transposed row major)
 * \param c The result matrix (row major)
 */
template <typename A, typename B, typename C, cpp_enable_iff(all_column_major<A, B, C> && all_homogeneous<A, B, C> && all_vectorizable<vector_mode, A, B, C>)>
void gemm_nt(A&& a, B&& b, C&& c) {
    a.ensure_cpu_up_to_date();
    b.ensure_cpu_up_to_date();

    const size_t M = etl::rows(a);
    const size_t N = etl::rows(b);
    const size_t K = etl::columns(a);

    gemm_cr_to_c(a.memory_start(), b.memory_start(), c.memory_start(), M, N, K);

    c.invalidate_gpu();
}

// Fallback functions for heterogeneous types
// CPP17: Replace with a if constexpr in the base functions ?

/*!
 * \brief GEMM with heterogeneous types
 *
 * \param a The lhs matrix
 * \param b The rhs matrix
 * \param c The result matrix
 */
template <typename A, typename B, typename C, cpp_enable_iff(!all_homogeneous<A, B, C> || !all_vectorizable<vector_mode, A, B, C>)>
void gemm(A&& a, B&& b, C&& c) {
    cpp_unused(a);
    cpp_unused(b);
    cpp_unused(c);

    cpp_unreachable("Invalid operation called vec::gemm with heterogeneous types");
}

/*!
 * \brief GEMM with heterogeneous types
 *
 * \param a The lhs matrix (row major)
 * \param b The rhs matrix (transposed row major)
 * \param c The result matrix (row major)
 */
template <typename A, typename B, typename C, cpp_enable_iff(!all_homogeneous<A, B, C> || !all_vectorizable<vector_mode, A, B, C>)>
void gemm_nt(A&& a, B&& b, C&& c) {
    cpp_unused(a);
    cpp_unused(b);
    cpp_unused(c);

    cpp_unreachable("Invalid operation called vec::gemm_nt with heterogeneous types");
}

/*!
 * \brief GEMM with heterogeneous types
 *
 * \param a The lhs matrix (row major)
 * \param b The rhs matrix (transposed row major)
 * \param c The result matrix (row major)
 */
template <typename A, typename B, typename C, cpp_enable_iff(!all_homogeneous<A, B, C> || !all_vectorizable<vector_mode, A, B, C>)>
void gemm_tn(A&& a, B&& b, C&& c) {
    cpp_unused(a);
    cpp_unused(b);
    cpp_unused(c);

    cpp_unreachable("Invalid operation called vec::gemm_tn with heterogeneous types");
}

} //end of namespace vec
} //end of namespace impl
} //end of namespace etl
