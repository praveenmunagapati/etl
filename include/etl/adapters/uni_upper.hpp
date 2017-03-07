//=======================================================================
// Copyright (c) 2014-2017 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

/*!
 * \file
 * \brief Contains uni upper triangular matrix implementation
 */

#pragma once

#include "etl/adapters/adapter.hpp"            // The adapter base class
#include "etl/adapters/uni_upper_exception.hpp"    // The exception
#include "etl/adapters/uni_upper_reference.hpp"    // The reference proxy

namespace etl {

/*!
 * \brief A uni upper triangular matrix adapter.
 *
 * This is only a prototype.
 */
template <typename Matrix>
struct uni_upper_matrix final : adapter<Matrix>, iterable<const uni_upper_matrix<Matrix>> {
    using matrix_t = Matrix;   ///< The adapted matrix type
    using expr_t   = matrix_t; ///< The wrapped expression type

    static_assert(etl_traits<matrix_t>::is_value, "Uni Upper triangular matrix only works with value classes");
    static_assert(etl_traits<matrix_t>::dimensions() == 2, "Uni Upper triangular matrix must be two-dimensional");
    static_assert(is_square_matrix<matrix_t>::value, "Uni Upper triangular matrix must be square");

    static constexpr std::size_t n_dimensions = etl_traits<matrix_t>::dimensions();  ///< The number of dimensions
    static constexpr order storage_order      = etl_traits<matrix_t>::storage_order; ///< The storage order
    static constexpr std::size_t alignment    = matrix_t::alignment;                 ///< The memory alignment

    using value_type        = value_t<matrix_t>;                 ///< The value type
    using memory_type       = value_type*;                       ///< The memory type
    using const_memory_type = const value_type*;                 ///< The const memory type

    using iterator       = typename matrix_t::const_iterator; ///< The type of const iterator
    using const_iterator = typename matrix_t::const_iterator; ///< The type of const iterator

    using base_type = adapter<Matrix>; ///< The base type

    /*!
     * \brief The vectorization type for V
     */
    template <typename V = default_vec>
    using vec_type       = typename V::template vec_type<value_type>;

    using base_type::matrix;

public:
    /*!
     * \brief Construct a new uni upper triangular matrix and fill it with zeros
     *
     * This constructor can only be used when the matrix is fast
     */
    uni_upper_matrix() noexcept : base_type() {
        // Fill the diagonal
        for(size_t i = 0; i < etl::dim<0>(matrix); ++i){
            matrix(i,i) = value_type(1);
        }
    }

    /*!
     * \brief Construct a new uni upper triangular matrix and fill it with zeros
     * \param dim The dimension of the matrix
     */
    explicit uni_upper_matrix(std::size_t dim) noexcept : base_type(dim) {
        // Fill the diagonal
        for(size_t i = 0; i < etl::dim<0>(matrix); ++i){
            matrix(i,i) = value_type(1);
        }
    }

    /*!
     * \brief Construct a uni_upper_matrix by copy
     * \param rhs The right-hand-side matrix
     */
    uni_upper_matrix(const uni_upper_matrix& rhs) = default;

    /*!
     * \brief Assign to the matrix by copy
     * \param rhs The right-hand-side matrix
     * \return a reference to the assigned matrix
     */
    uni_upper_matrix& operator=(const uni_upper_matrix& rhs) = default;

    /*!
     * \brief Construct a uni_upper_matrix by move
     * \param rhs The right-hand-side matrix
     */
    uni_upper_matrix(uni_upper_matrix&& rhs) = default;

    /*!
     * \brief Assign to the matrix by move
     * \param rhs The right-hand-side matrix
     * \return a reference to the assigned matrix
     */
    uni_upper_matrix& operator=(uni_upper_matrix&& rhs) = default;

    /*!
     * \brief Assign the values of the ETL expression to the uni upper triangular matrix
     * \param e The ETL expression to get the values from
     * \return a reference to the fast matrix
     */
    template <typename E, cpp_enable_if(std::is_convertible<value_t<E>, value_type>::value, is_etl_expr<E>::value)>
    uni_upper_matrix& operator=(E&& e) noexcept(false) {
        // Make sure the other matrix is uni upper triangular
        if(!is_uni_upper_triangular(e)){
            throw uni_upper_exception();
        }

        // Perform the real assign

        validate_assign(*this, e);
        e.assign_to(*this);

        return *this;
    }

    /*!
     * \brief Multiply each element by the right hand side scalar
     * \param rhs The right hand side scalar
     * \return a reference to the matrix
     */
    uni_upper_matrix& operator+=(const value_type& rhs) noexcept {
        detail::scalar_add::apply(*this, rhs);
        return *this;
    }

    /*!
     * \brief Multiply each element by the value of the elements in the right hand side expression
     * \param rhs The right hand side
     * \return a reference to the matrix
     */
    template<typename R, cpp_enable_if(is_etl_expr<R>::value)>
    uni_upper_matrix& operator+=(R&& rhs){
        // Make sure the other matrix is uni upper triangular
        if(!is_uni_upper_triangular(rhs)){
            throw uni_upper_exception();
        }

        validate_expression(*this, rhs);
        rhs.assign_add_to(*this);
        return *this;
    }

    /*!
     * \brief Multiply each element by the right hand side scalar
     * \param rhs The right hand side scalar
     * \return a reference to the matrix
     */
    uni_upper_matrix& operator-=(const value_type& rhs) noexcept {
        detail::scalar_sub::apply(*this, rhs);
        return *this;
    }

    /*!
     * \brief Multiply each element by the value of the elements in the right hand side expression
     * \param rhs The right hand side
     * \return a reference to the matrix
     */
    template<typename R, cpp_enable_if(is_etl_expr<R>::value)>
    uni_upper_matrix& operator-=(R&& rhs){
        // Make sure the other matrix is uni upper triangular
        if(!is_uni_upper_triangular(rhs)){
            throw uni_upper_exception();
        }

        validate_expression(*this, rhs);
        rhs.assign_sub_to(*this);
        return *this;
    }

    /*!
     * \brief Multiply each element by the right hand side scalar
     * \param rhs The right hand side scalar
     * \return a reference to the matrix
     */
    uni_upper_matrix& operator*=(const value_type& rhs) noexcept {
        detail::scalar_mul::apply(*this, rhs);
        return *this;
    }

    /*!
     * \brief Multiply each element by the value of the elements in the right hand side expression
     * \param rhs The right hand side
     * \return a reference to the matrix
     */
    template<typename R, cpp_enable_if(is_etl_expr<R>::value)>
    uni_upper_matrix& operator*=(R&& rhs) {
        // Make sure the other matrix is uni upper triangular
        if(!is_uni_upper_triangular(rhs)){
            throw uni_upper_exception();
        }

        validate_expression(*this, rhs);
        rhs.assign_mul_to(*this);
        return *this;
    }

    /*!
     * \brief Multiply each element by the right hand side scalar
     * \param rhs The right hand side scalar
     * \return a reference to the matrix
     */
    uni_upper_matrix& operator>>=(const value_type& rhs) noexcept {
        detail::scalar_mul::apply(*this, rhs);
        return *this;
    }

    /*!
     * \brief Multiply each element by the value of the elements in the right hand side expression
     * \param rhs The right hand side
     * \return a reference to the matrix
     */
    template<typename R, cpp_enable_if(is_etl_expr<R>::value)>
    uni_upper_matrix& operator>>=(R&& rhs) {
        // Make sure the other matrix is uni upper triangular
        if(!is_uni_upper_triangular(rhs)){
            throw uni_upper_exception();
        }

        validate_expression(*this, rhs);
        rhs.assign_mul_to(*this);
        return *this;
    }

    /*!
     * \brief Divide each element by the right hand side scalar
     * \param rhs The right hand side scalar
     * \return a reference to the matrix
     */
    uni_upper_matrix& operator/=(const value_type& rhs) noexcept {
        detail::scalar_div::apply(*this, rhs);
        return *this;
    }

    /*!
     * \brief Modulo each element by the value of the elements in the right hand side expression
     * \param rhs The right hand side
     * \return a reference to the matrix
     */
    template<typename R, cpp_enable_if(is_etl_expr<R>::value)>
    uni_upper_matrix& operator/=(R&& rhs) {
        // Make sure the other matrix is uni upper triangular
        if(!is_uni_upper_triangular(rhs)){
            throw uni_upper_exception();
        }

        validate_expression(*this, rhs);
        rhs.assign_div_to(*this);
        return *this;
    }

    /*!
     * \brief Modulo each element by the right hand side scalar
     * \param rhs The right hand side scalar
     * \return a reference to the matrix
     */
    uni_upper_matrix& operator%=(const value_type& rhs) noexcept {
        detail::scalar_mod::apply(*this, rhs);
        return *this;
    }

    /*!
     * \brief Modulo each element by the value of the elements in the right hand side expression
     * \param rhs The right hand side
     * \return a reference to the matrix
     */
    template<typename R, cpp_enable_if(is_etl_expr<R>::value)>
    uni_upper_matrix& operator%=(R&& rhs){
        // Make sure the other matrix is uni upper triangular
        if(!is_uni_upper_triangular(rhs)){
            throw uni_upper_exception();
        }

        validate_expression(*this, rhs);
        rhs.assign_mod_to(*this);
        return *this;
    }

    /*!
     * \brief Access the (i, j) element of the 2D matrix
     * \param i The index of the first dimension
     * \param j The index of the second dimension
     * \return a reference to the (i,j) element
     *
     * Accessing an element outside the matrix results in Undefined Behaviour.
     */
    uni_upper_detail::uni_upper_reference<matrix_t> operator()(std::size_t i, std::size_t j) noexcept {
        return {matrix, i, j};
    }

    using base_type::operator();
};

/*!
 * \brief Traits specialization for uni_upper_matrix
 */
template <typename Matrix>
struct etl_traits<uni_upper_matrix<Matrix>> : wrapper_traits<uni_upper_matrix<Matrix>> {};

} //end of namespace etl
