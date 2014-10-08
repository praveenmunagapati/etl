//=======================================================================
// Copyright (c) 2014 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef ETL_FAST_OP_HPP
#define ETL_FAST_OP_HPP

#include <random>
#include <functional>
#include <ctime>

#include "math.hpp"

namespace etl {

using random_engine = std::mt19937_64;

template<typename T>
struct scalar {
    const T value;

    explicit constexpr scalar(T v) : value(v) {}

    constexpr const T operator[](std::size_t) const {
        return value;
    }

    constexpr const T operator()(std::size_t) const {
        return value;
    }

    constexpr const T operator()(std::size_t, std::size_t) const {
        return value;
    }
};

template<typename T>
struct hflip_transformer {
    using sub_type = T;

    const T& sub;

    explicit hflip_transformer(const T& vec) : sub(vec) {}

    typename T::value_type operator[](std::size_t i) const {
        return sub[size(sub) - 1 - i];
    }

    typename T::value_type operator()(std::size_t i) const {
        return sub(size(sub) - 1 - i);
    }

    typename T::value_type operator()(std::size_t i, std::size_t j) const {
        return sub(i, columns(sub) - 1 - j);
    }
};

template<typename T>
struct vflip_transformer {
    using sub_type = T;

    const T& sub;

    explicit vflip_transformer(const T& vec) : sub(vec) {}

    typename T::value_type operator[](std::size_t i) const {
        return sub[i];
    }

    typename T::value_type operator()(std::size_t i) const {
        return sub(i);
    }

    typename T::value_type operator()(std::size_t i, std::size_t j) const {
        return sub(rows(sub) - 1 - i, j);
    }
};

template<typename T>
struct fflip_transformer {
    using sub_type = T;

    const T& sub;

    explicit fflip_transformer(const T& vec) : sub(vec) {}

    typename T::value_type operator[](std::size_t i) const {
        return sub[i];
    }

    typename T::value_type operator()(std::size_t i) const {
        return sub(i);
    }

    typename T::value_type operator()(std::size_t i, std::size_t j) const {
        return sub(rows(sub) - 1 - i, columns(sub) - 1 - j);
    }
};

template<typename T>
struct transpose_transformer {
    using sub_type = T;

    const T& sub;

    explicit transpose_transformer(const T& vec) : sub(vec) {}

    typename T::value_type operator[](std::size_t i) const {
        return sub[i];
    }

    typename T::value_type operator()(std::size_t i) const {
        return sub(i);
    }

    typename T::value_type operator()(std::size_t i, std::size_t j) const {
        return sub(j, i);
    }
};

template<typename T, std::size_t D>
struct dim_view {
    static_assert(D > 0 || D < 3, "Invalid dimension");

    using sub_type = T;
    using value_type = typename T::value_type;

    T& sub;
    const std::size_t i;

    using return_type = typename std::conditional<
        cpp::and_u<
            std::is_lvalue_reference<decltype(sub(0,0))>::value,
            cpp::not_u<std::is_const<T>::value>::value
        >::value,
        value_type&,
        value_type>::type;

    using const_return_type = typename std::conditional<
        std::is_lvalue_reference<decltype(sub(0,0))>::value,
        const value_type&,
        value_type>::type;

    dim_view(T& sub, std::size_t i) : sub(sub), i(i) {}

    const_return_type operator[](std::size_t j) const {
        if(D == 1){
            return sub(i, j);
        } else if(D == 2){
            return sub(j, i);
        }
    }

    return_type operator[](std::size_t j){
        if(D == 1){
            return sub(i, j);
        } else if(D == 2){
            return sub(j, i);
        }
    }

    const_return_type operator()(std::size_t j) const {
        if(D == 1){
            return sub(i, j);
        } else if(D == 2){
            return sub(j, i);
        }
    }

    return_type operator()(std::size_t j){
        if(D == 1){
            return sub(i, j);
        } else if(D == 2){
            return sub(j, i);
        }
    }
};

template<typename T>
struct sub_view {
    using parent_type = T;
    using value_type = typename T::value_type;

    T& parent;
    const std::size_t i;

    using return_type = typename std::conditional<
        cpp::and_u<
            std::is_lvalue_reference<decltype(parent[0])>::value,
            cpp::not_u<std::is_const<T>::value>::value
        >::value,
        value_type&,
        value_type>::type;

    using const_return_type = typename std::conditional<
        std::is_lvalue_reference<decltype(parent[0])>::value,
        const value_type&,
        value_type>::type;

    sub_view(T& parent, std::size_t i) : parent(parent), i(i) {}

    const_return_type operator[](std::size_t j) const {
        return parent[i * subsize(parent) + j];
    }

    template<typename... S>
    const_return_type operator()(S... args) const {
        return parent(i, static_cast<size_t>(args)...);
    }

    return_type operator[](std::size_t j){
        return parent[i * subsize(parent) + j];
    }

    template<typename... S>
    return_type operator()(S... args){
        return parent(i, static_cast<size_t>(args)...);
    }
};

template<typename T, std::size_t Rows, std::size_t Columns>
struct fast_matrix_view {
    static_assert(Rows > 0 && Columns > 0 , "Invalid dimensions");

    using sub_tyoe = T;
    using value_type = typename T::value_type;

    T& sub;

    using return_type = typename std::conditional<
        cpp::and_u<
            std::is_lvalue_reference<decltype(sub(0))>::value,
            cpp::not_u<std::is_const<T>::value>::value
        >::value,
        value_type&,
        value_type>::type;

    using const_return_type = typename std::conditional<
        std::is_lvalue_reference<decltype(sub(0))>::value,
        const value_type&,
        value_type>::type;

    explicit fast_matrix_view(T& sub) : sub(sub) {}

    const_return_type operator[](std::size_t j) const {
        return sub(j);
    }

    const_return_type operator()(std::size_t j) const {
        return sub(j);
    }

    const_return_type operator()(std::size_t i, std::size_t j) const {
        return sub(i * Columns + j);
    }

    return_type operator[](std::size_t j){
        return sub(j);
    }

    return_type operator()(std::size_t j){
        return sub(j);
    }

    return_type operator()(std::size_t i, std::size_t j){
        return sub(i * Columns + j);
    }
};

template<typename T>
struct dyn_matrix_view {
    using sub_type = T;
    using value_type = typename T::value_type;

    T& sub;
    std::size_t rows;
    std::size_t columns;

    using return_type = typename std::conditional<
        cpp::and_u<
            std::is_lvalue_reference<decltype(sub(0))>::value,
            cpp::not_u<std::is_const<T>::value>::value
        >::value,
        value_type&,
        value_type>::type;

    using const_return_type = typename std::conditional<
        std::is_lvalue_reference<decltype(sub(0))>::value,
        const value_type&,
        value_type>::type;

    dyn_matrix_view(T& sub, std::size_t rows, std::size_t columns) : sub(sub), rows(rows), columns(columns) {}

    const_return_type operator[](std::size_t j) const {
        return sub(j);
    }

    const_return_type operator()(std::size_t j) const {
        return sub(j);
    }

    const_return_type operator()(std::size_t i, std::size_t j) const {
        return sub(i * columns + j);
    }

    return_type operator[](std::size_t j){
        return sub(j);
    }

    return_type operator()(std::size_t j){
        return sub(j);
    }

    return_type operator()(std::size_t i, std::size_t j){
        return sub(i * columns + j);
    }
};

template<typename T>
struct plus_binary_op {
    static constexpr T apply(const T& lhs, const T& rhs){
        return lhs + rhs;
    }
};

template<typename T>
struct minus_binary_op {
    static constexpr T apply(const T& lhs, const T& rhs){
        return lhs - rhs;
    }
};

template<typename T>
struct mul_binary_op {
    static constexpr T apply(const T& lhs, const T& rhs){
        return lhs * rhs;
    }
};

template<typename T>
struct div_binary_op {
    static constexpr T apply(const T& lhs, const T& rhs){
        return lhs / rhs;
    }
};

template<typename T>
struct mod_binary_op {
    static constexpr T apply(const T& lhs, const T& rhs){
        return lhs % rhs;
    }
};

template<typename T>
struct abs_unary_op {
    static constexpr T apply(const T& x){
        return std::abs(x);
    }
};

template<typename T>
struct log_unary_op {
    static constexpr T apply(const T& x){
        return std::log(x);
    }
};

template<typename T>
struct exp_unary_op {
    static constexpr T apply(const T& x){
        return std::exp(x);
    }
};

template<typename T>
struct sign_unary_op {
    static constexpr T apply(const T& x){
        return sign(x);
    }
};

template<typename T>
struct sigmoid_unary_op {
    static constexpr T apply(const T& x){
        return logistic_sigmoid(x);
    }
};

template<typename T>
struct softplus_unary_op {
    static constexpr T apply(const T& x){
        return softplus(x);
    }
};

template<typename T>
struct minus_unary_op {
    static constexpr T apply(const T& x){
        return -x;
    }
};

template<typename T>
struct plus_unary_op {
    static constexpr T apply(const T& x){
        return +x;
    }
};

template<typename T>
struct bernoulli_unary_op {
    static T apply(const T& x){
        static random_engine rand_engine(std::time(nullptr));
        static std::uniform_real_distribution<double> normal_distribution(0.0, 1.0);
        static auto normal_generator = std::bind(normal_distribution, rand_engine);

        return x > normal_generator() ? 1.0 : 0.0;
    }
};

template<typename T>
struct uniform_noise_unary_op {
    static T apply(const T& x){
        static random_engine rand_engine(std::time(nullptr));
        static std::uniform_real_distribution<double> real_distribution(0.0, 1.0);
        static auto noise = std::bind(real_distribution, rand_engine);

        return x + noise();
    }
};

template<typename T>
struct normal_noise_unary_op {
    static T apply(const T& x){
        static random_engine rand_engine(std::time(nullptr));
        static std::normal_distribution<double> normal_distribution(0.0, 1.0);
        static auto noise = std::bind(normal_distribution, rand_engine);

        return x + noise();
    }
};

template<typename T>
struct logistic_noise_unary_op {
    static T apply(const T& x){
        static random_engine rand_engine(std::time(nullptr));

        std::normal_distribution<double> noise_distribution(0.0, logistic_sigmoid(x));
        auto noise = std::bind(noise_distribution, rand_engine);

        return x + noise();
    }
};

template<typename T, typename E>
struct ranged_noise_binary_op {
    static T apply(const T& x, E value){
        static random_engine rand_engine(std::time(nullptr));
        static std::normal_distribution<double> normal_distribution(0.0, 1.0);
        static auto noise = std::bind(normal_distribution, rand_engine);

        if(x == 0.0 || x == value){
            return x;
        } else {
            return x + noise();
        }
    }
};

template<typename T, typename E>
struct max_binary_op {
    static constexpr T apply(const T& x, E value){
        return std::max(x, value);
    }
};

template<typename T, typename E>
struct min_binary_op {
    static constexpr T apply(const T& x, E value){
        return std::min(x, value);
    }
};

template<typename T = double>
struct normal_generator_op {
    using value_type = T;

    random_engine rand_engine;
    std::uniform_real_distribution<value_type> normal_distribution;

    normal_generator_op() : rand_engine(std::time(nullptr)), normal_distribution(0.0, 1.0) {}

    value_type operator()(){
        return normal_distribution(rand_engine);
    }
};

} //end of namespace etl

#endif
