//=======================================================================
// Copyright (c) 2014-2017 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include "etl/expr/base_temporary_expr.hpp"

namespace etl {

/*!
 * \brief An upsample expression.
 * \tparam A The type of the expression to upsample
 */
template <typename A, size_t C1, size_t C2, size_t C3, typename Impl>
struct upsample_3d_expr : base_temporary_expr_un<upsample_3d_expr<A, C1, C2, C3, Impl>, A> {
    using value_type = value_t<A>;                            ///< The type of value of the expression
    using this_type  = upsample_3d_expr<A, C1, C2, C3, Impl>; ///< The type of this expression
    using base_type  = base_temporary_expr_un<this_type, A>;  ///< The base type
    using sub_traits = decay_traits<A>;                       ///< The traits of the sub type

    static constexpr auto storage_order = sub_traits::storage_order; ///< The sub storage order

    /*!
     * \brief Construct a new expression
     * \param a The sub expression
     */
    explicit upsample_3d_expr(A a) : base_type(a) {
        //Nothing else to init
    }

    // Assignment functions

    /*!
     * \brief Assign to a matrix of the same storage order
     * \param lhs The expression to which assign
     */
    template<typename L>
    void assign_to(L&& lhs)  const {
        static_assert(all_etl_expr<A, L>::value, "upsample_3d only supported for ETL expressions");
        static_assert(etl::dimensions<A>() == etl::dimensions<L>(), "upsample_3d must be applied on matrices of same dimensionality");

        auto& a = this->a();

        standard_evaluator::pre_assign_rhs(a);
        standard_evaluator::pre_assign_lhs(lhs);

        Impl::template apply<C1, C2, C3>(
            make_temporary(a),
            std::forward<L>(lhs));
    }

    /*!
     * \brief Add to the given left-hand-side expression
     * \param lhs The expression to which assign
     */
    template<typename L>
    void assign_add_to(L&& lhs)  const {
        std_add_evaluate(*this, lhs);
    }

    /*!
     * \brief Sub from the given left-hand-side expression
     * \param lhs The expression to which assign
     */
    template<typename L>
    void assign_sub_to(L&& lhs)  const {
        std_sub_evaluate(*this, lhs);
    }

    /*!
     * \brief Multiply the given left-hand-side expression
     * \param lhs The expression to which assign
     */
    template<typename L>
    void assign_mul_to(L&& lhs)  const {
        std_mul_evaluate(*this, lhs);
    }

    /*!
     * \brief Divide the given left-hand-side expression
     * \param lhs The expression to which assign
     */
    template<typename L>
    void assign_div_to(L&& lhs)  const {
        std_div_evaluate(*this, lhs);
    }

    /*!
     * \brief Modulo the given left-hand-side expression
     * \param lhs The expression to which assign
     */
    template<typename L>
    void assign_mod_to(L&& lhs)  const {
        std_mod_evaluate(*this, lhs);
    }
};

/*!
 * \brief Traits for a transpose expression
 * \tparam A The transposed sub type
 */
template <typename A, size_t C1, size_t C2, size_t C3, typename Impl>
struct etl_traits<etl::upsample_3d_expr<A, C1, C2, C3, Impl>> {
    using expr_t     = etl::upsample_3d_expr<A, C1, C2, C3, Impl>; ///< The expression type
    using sub_expr_t = std::decay_t<A>;                            ///< The sub expression type
    using sub_traits = etl_traits<sub_expr_t>;                     ///< The sub traits
    using value_type = value_t<A>;                                 ///< The value type of the expression

    static constexpr size_t D = sub_traits::dimensions(); ///< The number of dimensions of this expressions

    static constexpr bool is_etl                  = true;                      ///< Indicates if the type is an ETL expression
    static constexpr bool is_transformer          = false;                     ///< Indicates if the type is a transformer
    static constexpr bool is_view                 = false;                     ///< Indicates if the type is a view
    static constexpr bool is_magic_view           = false;                     ///< Indicates if the type is a magic view
    static constexpr bool is_fast                 = sub_traits::is_fast;       ///< Indicates if the expression is fast
    static constexpr bool is_linear               = true;                      ///< Indicates if the expression is linear
    static constexpr bool is_thread_safe          = true;                      ///< Indicates if the expression is thread safe
    static constexpr bool is_value                = false;                     ///< Indicates if the expression is of value type
    static constexpr bool is_direct               = true;                      ///< Indicates if the expression has direct memory access
    static constexpr bool is_generator            = false;                     ///< Indicates if the expression is a generator
    static constexpr bool is_padded               = false;                     ///< Indicates if the expression is padded
    static constexpr bool is_aligned              = true;                      ///< Indicates if the expression is padded
    static constexpr bool is_gpu                  = false;                     ///< Indicates if the expression can be done on GPU
    static constexpr bool needs_evaluator_visitor = true;                      ///< Indicates if the expression needs a evaluator visitor
    static constexpr order storage_order          = sub_traits::storage_order; ///< The expression's storage order

    /*!
     * \brief Indicates if the expression is vectorizable using the
     * given vector mode
     * \tparam V The vector mode
     */
    template <vector_mode_t V>
    using vectorizable = std::true_type;

    /*!
     * \brief Returns the DDth dimension of the expression
     * \return the DDth dimension of the expression
     */
    template <std::size_t DD>
    static constexpr std::size_t dim() {
        return DD == D - 3 ? decay_traits<A>::template dim<DD>() * C1
             : DD == D - 2 ? decay_traits<A>::template dim<DD>() * C2
             : DD == D - 1 ? decay_traits<A>::template dim<DD>() * C3
                           : decay_traits<A>::template dim<DD>();
    }

    /*!
     * \brief Returns the dth dimension of the expression
     * \param e The sub expression
     * \param d The dimension to get
     * \return the dth dimension of the expression
     */
    static std::size_t dim(const expr_t& e, std::size_t d) {
        if (d == D - 3) {
            return etl::dim(e._a, d) * C1;
        } else if (d == D - 2) {
            return etl::dim(e._a, d) * C2;
        } else if (d == D - 1){
            return etl::dim(e._a, d) * C3;
        } else {
            return etl::dim(e._a, d);
        }
    }

    /*!
     * \brief Returns the size of the expression
     * \param e The sub expression
     * \return the size of the expression
     */
    static std::size_t size(const expr_t& e) {
        size_t acc = 1;
        for (size_t i = 0; i < D; ++i) {
            acc *= dim(e, i);
        }
        return acc;
    }

    /*!
     * \brief Returns the multiplicative sum of the dimensions at the given indices
     * \return the multiplicative sum of the dimensions at the given indices
     */
    template <size_t... I>
    static constexpr size_t size_mul(const std::index_sequence<I...>& /*seq*/) {
        return mul_all<dim<I>()...>::value;
    }

    /*!
     * \brief Returns the size of the expression
     * \return the size of the expression
     */
    static constexpr std::size_t size() {
        return size_mul(std::make_index_sequence<D>());
    }

    /*!
     * \brief Returns the number of dimensions of the expression
     * \return the number of dimensions of the expression
     */
    static constexpr std::size_t dimensions() {
        return D;
    }
};

} //end of namespace etl
