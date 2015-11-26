//=======================================================================
// Copyright (c) 2014-2015 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#pragma once

#include <array>     //To store the dimensions
#include <tuple>     //For TMP stuff
#include <algorithm> //For std::find_if
#include <iosfwd>    //For stream support

#include "cpp_utils/assert.hpp"
#include "cpp_utils/tmp.hpp"

#include "etl/traits_lite.hpp" //forward declaration of the traits
#include "etl/compat.hpp"      //To make it work with g++
#include "etl/dyn_base.hpp"    //The base class and utilities

namespace etl {

namespace sparse_detail {

template<typename M>
struct sparse_reference {
    using matrix_type        = M;
    using value_type         = typename matrix_type::value_type;
    using raw_pointer_type   = value_type*;
    using raw_reference_type = value_type&;

    matrix_type& matrix;
    std::size_t i;
    std::size_t j;
    std::size_t n;
    raw_pointer_type ptr;

    sparse_reference(matrix_type& matrix, std::size_t i, std::size_t j) : matrix(matrix), i(i), j(j) {
        n = matrix.find_n(i, j);
        matrix.unsafe_set_hint(i, j, n, matrix.get_hint(i, j, n));
        ptr = &matrix.unsafe_ref_hint(n);
    }

    ~sparse_reference(){
        //Update the value, possibly erasing it
        matrix.set_hint(i, j, n, *ptr);
    }

    sparse_reference& operator=(value_type rhs){
        get() = rhs;
        return *this;
    }

    sparse_reference& operator+=(value_type rhs){
        get() += rhs;
        return *this;
    }

    sparse_reference& operator-=(value_type rhs){
        get() -= rhs;
        return *this;
    }

    sparse_reference& operator*=(value_type rhs){
        get() *= rhs;
        return *this;
    }

    sparse_reference& operator/=(value_type rhs){
        get() /= rhs;
        return *this;
    }

    sparse_reference& operator%=(value_type rhs){
        get() %= rhs;
        return *this;
    }

    operator raw_reference_type(){
        return get();
    }

private:
    raw_reference_type get(){
        return *ptr;
    }
};

} //end of namespace sparse_detail

template <typename T, sparse_storage SS, std::size_t D>
struct sparse_matrix_impl;

//Implementation with COO format
template <typename T, std::size_t D>
struct sparse_matrix_impl <T, sparse_storage::COO, D> final : dyn_base<T, D> {
    static constexpr const std::size_t n_dimensions      = D;
    static constexpr const sparse_storage storage_format = sparse_storage::COO;
    static constexpr const order storage_order           = order::RowMajor;
    static constexpr const std::size_t alignment         = intrinsic_traits<T>::alignment;

    using base_type              = dyn_base<T, D>;
    using this_type              = sparse_matrix_impl<T, sparse_storage::COO, D>;
    using reference_type         = sparse_detail::sparse_reference<this_type>;
    using const_reference_type   = sparse_detail::sparse_reference<const this_type>;
    using value_type             = T;
    using dimension_storage_impl = std::array<std::size_t, n_dimensions>;
    using memory_type            = value_type*;
    using const_memory_type      = const value_type*;
    using index_type             = std::size_t;
    using index_memory_type      = index_type*;
    using iterator               = memory_type;
    using const_iterator         = const_memory_type;
    using vec_type               = intrinsic_type<T>;

    //template<typename M>
    //friend struct sparse_detail::sparse_reference<M>;
    friend struct sparse_detail::sparse_reference<this_type>;
    friend struct sparse_detail::sparse_reference<const this_type>;

private:
    using base_type::_size;
    using base_type::_dimensions;
    memory_type _memory;
    index_memory_type _row_index;
    index_memory_type _col_index;
    std::size_t nnz;

    using base_type::release;
    using base_type::allocate;
    using base_type::check_invariants;

    template<typename It>
    void build_from_iterable(const It& iterable){
        nnz =  0;
        for (auto v : iterable) {
            if(v != 0.0){
                ++nnz;
            }
        }

        if(nnz > 0){
            //Allocate space for the three arrays
            _memory    = allocate(nnz);
            _row_index = base_type::template allocate<index_type>(nnz);
            _col_index = base_type::template allocate<index_type>(nnz);

            auto it = iterable.begin();
            std::size_t n = 0;

            for(std::size_t i = 0; i < rows(); ++i){
                for(std::size_t j = 0; j < columns(); ++j){
                    if(*it != 0.0){
                        _memory[n] = *it;
                        _row_index[n] = i;
                        _col_index[n] = j;
                        ++n;
                    }

                    ++it;
                }
            }
        }
    }

    void reserve_hint(std::size_t hint){
        cpp_assert(hint < nnz + 1, "Invalid hint for reserve_hint");

        if(_memory){
            auto new_memory    = allocate(nnz + 1);
            auto new_row_index = base_type::template allocate<index_type>(nnz + 1);
            auto new_col_index = base_type::template allocate<index_type>(nnz + 1);

            if(hint == nnz){
                //Copy the elements
                std::copy_n(_memory, nnz, new_memory);
                std::copy_n(_row_index, nnz, new_row_index);
                std::copy_n(_col_index, nnz, new_col_index);
            } else {
                //Copy the elements before hint
                std::copy(_memory, _memory + hint, new_memory);
                std::copy(_row_index, _row_index + hint, new_row_index);
                std::copy(_col_index, _col_index + hint, new_col_index);

                //Copy the elements after hint
                std::copy(_memory + hint, _memory + nnz, new_memory + hint + 1);
                std::copy(_row_index + hint, _row_index + nnz, new_row_index + hint + 1);
                std::copy(_col_index + hint, _col_index + nnz, new_col_index + hint + 1);
            }

            release(_memory, nnz);
            release(_row_index, nnz);
            release(_col_index, nnz);

            _memory = new_memory;
            _col_index = new_col_index;
            _row_index = new_row_index;
        } else {
            cpp_assert(hint == 0, "Invalid hint for reserve_hint");

            _memory    = allocate(nnz + 1);
            _row_index = base_type::template allocate<index_type>(nnz + 1);
            _col_index = base_type::template allocate<index_type>(nnz + 1);
        }

        ++nnz;
    }

    void erase_hint(std::size_t n){
        cpp_assert(nnz > 0, "Invalid erase_hint call (no non-zero elements");

        if(nnz == 1){
            release(_memory, nnz);
            release(_row_index, nnz);
            release(_col_index, nnz);

            _memory    = nullptr;
            _row_index = nullptr;
            _col_index = nullptr;
        } else {
            auto new_memory    = allocate(nnz - 1);
            auto new_row_index = base_type::template allocate<index_type>(nnz - 1);
            auto new_col_index = base_type::template allocate<index_type>(nnz - 1);

            if(n == nnz - 1){
                std::copy_n(_memory, nnz - 1, new_memory);
                std::copy_n(_row_index, nnz - 1, new_row_index);
                std::copy_n(_col_index, nnz - 1, new_col_index);
            } else {
                std::copy(_memory, _memory + n, new_memory);
                std::copy(_row_index, _row_index + n, new_row_index);
                std::copy(_col_index, _col_index + n, new_col_index);

                std::copy(_memory + n + 1, _memory + nnz, new_memory + n);
                std::copy(_row_index + n + 1, _row_index + nnz, new_row_index + n);
                std::copy(_col_index + n + 1, _col_index + nnz, new_col_index + n);
            }

            release(_memory, nnz);
            release(_row_index, nnz);
            release(_col_index, nnz);

            _memory    = new_memory;
            _row_index = new_row_index;
            _col_index = new_col_index;
        }

        --nnz;
    }

    std::size_t find_n(std::size_t i, std::size_t j) const noexcept {
        for(std::size_t n = 0; n < nnz; ++n){
            //The value exists, modify it
            if(_row_index[n] == i && _col_index[n] == j){
                return n;
            }

            //The insertion point has been found
            if((_row_index[n] == i && _col_index[n] > j) || _row_index[n] > i){
                return n;
            }
        }

        return nnz;
    }

    void unsafe_set_hint(std::size_t i, std::size_t j, std::size_t n, value_type value){
        if(n < nnz){
            //The value exists, modify it
            if(_row_index[n] == i && _col_index[n] == j){
                _memory[n] = value;
                return;
            }
        }

        reserve_hint(n);

        _memory[n] = value;
        _row_index[n] = i;
        _col_index[n] = j;
    }

    template <bool B = n_dimensions == 2, cpp_enable_if(B)>
    value_type get_hint(std::size_t i, std::size_t j, std::size_t n) const noexcept {
        if (n < nnz) {
            if (_row_index[n] == i && _col_index[n] == j) {
                return _memory[n];
            }
        }

        return 0.0;
    }

    void set_hint(std::size_t i, std::size_t j, std::size_t n, value_type value){
        if(n < nnz){
            if(_row_index[n] == i && _col_index[n] == j){
                //At this point, there is already a value for (i,j)
                //If zero, we remove it, otherwise edit it
                if(value != value_type(0)){
                    unsafe_set_hint(i, j, n, value);
                } else {
                    erase_hint(n);
                }
            } else {
                //At this point, the value does not exist
                //We insert it if not zero
                if(value != value_type(0)){
                    unsafe_set_hint(i, j, n, value);
                }
            }
        } else {
            //At this point, the value does not exist
            //We insert it if not zero
            if(value != value_type(0)){
                unsafe_set_hint(i, j, n, value);
            }
        }
    }

    value_type& unsafe_ref_hint(std::size_t n) {
        return _memory[n];
    }

    const value_type& unsafe_ref_hint(std::size_t n) const {
        return _memory[n];
    }

public:
    using base_type::dim;
    using base_type::rows;
    using base_type::columns;

    // Construction

    //Default constructor (constructs an empty matrix)
    sparse_matrix_impl() noexcept : base_type(), _memory(nullptr), _row_index(nullptr), _col_index(nullptr), nnz(0) {
        //Nothing else to init
    }

    //Normal constructor with only sizes
    template <typename... S, cpp_enable_if(
                                 (sizeof...(S) == D),
                                 cpp::all_convertible_to<std::size_t, S...>::value,
                                 cpp::is_homogeneous<typename cpp::first_type<S...>::type, S...>::value)>
    explicit sparse_matrix_impl(S... sizes) noexcept : base_type(dyn_detail::size(sizes...), {{static_cast<std::size_t>(sizes)...}}),
                                                       _memory(nullptr), _row_index(nullptr), _col_index(nullptr), nnz(0) {
        //Nothing else to init
    }

    //Sizes followed by an initializer list
    template <typename... S, cpp_enable_if(dyn_detail::is_initializer_list_constructor<S...>::value)>
    explicit sparse_matrix_impl(S... sizes) noexcept : base_type(dyn_detail::size(std::make_index_sequence<(sizeof...(S)-1)>(), sizes...),
                                                              dyn_detail::sizes(std::make_index_sequence<(sizeof...(S)-1)>(), sizes...)){
        static_assert(sizeof...(S) == D + 1, "Invalid number of dimensions");

        auto list = cpp::last_value(sizes...);
        build_from_iterable(list);
    }

    //Sizes followed by a values_t
    template <typename S1, typename... S, cpp_enable_if(
                                              (sizeof...(S) == D),
                                              cpp::is_specialization_of<values_t, typename cpp::last_type<S1, S...>::type>::value)>
    explicit sparse_matrix_impl(S1 s1, S... sizes) noexcept : base_type(dyn_detail::size(std::make_index_sequence<(sizeof...(S))>(), s1, sizes...),
                                                                     dyn_detail::sizes(std::make_index_sequence<(sizeof...(S))>(), s1, sizes...)){
        auto list = cpp::last_value(sizes...).template list<value_type>();
        build_from_iterable(list);
    }

    template <typename E, cpp_enable_if(!std::is_same<std::decay_t<E>, sparse_matrix_impl<T, storage_format, D>>::value && std::is_convertible<value_t<E>, value_type>::value && is_etl_expr<E>::value)>
    sparse_matrix_impl& operator=(E&& e) noexcept {
        validate_assign(*this, e);

        assign_evaluate(e, *this);

        check_invariants();

        return *this;
    }

    /*!
     * \brief Returns the value at the given (i,j) position in the matrix.
     *
     * This function will never insert a new element in the matrix. It is suited when only reading the matrix and not neeeding references.
     *
     * \param i The row
     * \param j The column
     *
     * \return The value at the (i,j) position.
     */
    template <bool B = n_dimensions == 2, cpp_enable_if(B)>
    value_type get(std::size_t i, std::size_t j) noexcept {
        cpp_assert(i < dim(0), "Out of bounds");
        cpp_assert(j < dim(1), "Out of bounds");

        auto n = find_n(i, j);
        return get_hint(i, j, n);
    }

    template <bool B = n_dimensions == 2, cpp_enable_if(B)>
    reference_type operator()(std::size_t i, std::size_t j) noexcept {
        return {*this, i, j};
    }

    template <bool B = n_dimensions == 2, cpp_enable_if(B)>
    const_reference_type operator()(std::size_t i, std::size_t j) const noexcept {
        return {*this, i, j};
    }

    template <bool B = n_dimensions == 2, cpp_enable_if(B)>
    reference_type operator[](std::size_t n) noexcept {
        return {*this, n / columns(), n % columns()};
    }

    template <bool B = n_dimensions == 2, cpp_enable_if(B)>
    const_reference_type operator[](std::size_t n) const noexcept {
        return {*this, n / columns(), n % columns()};
    }

    template <bool B = n_dimensions == 2, cpp_enable_if(B)>
    value_type read_flat(std::size_t n) const noexcept {
        return get(n / columns(), n % columns());
    }

    /*!
     * \brief Returns the number of non zeros entries in the sparse matrix.
     *
     * This is a constant time O(1) operation.
     *
     * \return The number of non zeros entries in the sparse matrix.
     */
    std::size_t non_zeros() const noexcept {
        return nnz;
    }

    void unsafe_set(std::size_t i, std::size_t j, value_type value){
        cpp_assert(i < dim(0), "Out of bounds");
        cpp_assert(j < dim(1), "Out of bounds");

        auto n = find_n(i, j);

        unsafe_set_hint(i, j, n, value);
    }

    void set(std::size_t i, std::size_t j, value_type value){
        auto n = find_n(i, j);
        set_hint(i, j, n, value);
    }

    void erase(std::size_t i, std::size_t j) {
        auto n = find_n(i, j);

        if (n < nnz) {
            if (_row_index[n] == i && _col_index[n] == j) {
                erase_hint(n);
            }
        }
    }

    //Destructor

    ~sparse_matrix_impl() noexcept {
        if(_memory){
            release(_memory, nnz);
            release(_row_index, nnz);
            release(_col_index, nnz);
        }
    }
};

} //end of namespace etl
