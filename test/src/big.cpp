//=======================================================================
// Copyright (c) 2014-2016 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#include "test_light.hpp"
#include "cpp_utils/algorithm.hpp"

// These tests are made to test operations using enough operations
// to make sure thresholds are reached

TEMPLATE_TEST_CASE_2("big/add", "[big][add]", Z, double, float) {
    etl::dyn_matrix<Z> a(etl::parallel_threshold, 2UL);
    etl::dyn_matrix<Z> b(etl::parallel_threshold, 2UL);
    etl::dyn_matrix<Z> c(etl::parallel_threshold, 2UL);

    a = etl::uniform_generator(-1000.0, 5000.0);
    b = etl::uniform_generator(-1000.0, 5000.0);

    c = a + b;

    for (std::size_t i = 0; i < c.size(); ++i) {
        REQUIRE(c[i] == Approx(Z(a[i] + b[i])));
    }
}

TEMPLATE_TEST_CASE_2("big/sub", "[big][sub]", Z, double, float) {
    etl::dyn_matrix<Z> a(etl::parallel_threshold, 2UL);
    etl::dyn_matrix<Z> b(etl::parallel_threshold, 2UL);
    etl::dyn_matrix<Z> c(etl::parallel_threshold, 2UL);

    a = etl::uniform_generator(-1000.0, 5000.0);
    b = etl::uniform_generator(-1000.0, 5000.0);

    c = a - b;

    for (std::size_t i = 0; i < c.size(); ++i) {
        REQUIRE(c[i] == Approx(Z(a[i] - b[i])));
    }
}

TEMPLATE_TEST_CASE_2("big/mul", "[big][sub]", Z, double, float) {
    etl::dyn_matrix<Z> a(etl::parallel_threshold, 2UL);
    etl::dyn_matrix<Z> b(etl::parallel_threshold, 2UL);
    etl::dyn_matrix<Z> c(etl::parallel_threshold, 2UL);

    a = etl::uniform_generator(-1000.0, 5000.0);
    b = etl::uniform_generator(-1000.0, 5000.0);

    c = a >> b;

    for (std::size_t i = 0; i < c.size(); ++i) {
        REQUIRE(c[i] == Approx(Z(a[i] * b[i])));
    }
}

TEMPLATE_TEST_CASE_2("big/compound/add", "[big][add]", Z, double, float) {
    etl::dyn_matrix<Z> a(etl::parallel_threshold, 2UL);
    etl::dyn_matrix<Z> b(etl::parallel_threshold, 2UL);
    etl::dyn_matrix<Z> c(etl::parallel_threshold, 2UL);

    a = etl::uniform_generator(-1000.0, 5000.0);
    b = etl::uniform_generator(-1000.0, 5000.0);
    c = 120.0;

    c += a + b;

    for (std::size_t i = 0; i < c.size(); ++i) {
        REQUIRE(c[i] == Approx(Z(120.0) + a[i] + b[i]).epsilon(1e-1));
    }
}

TEMPLATE_TEST_CASE_2("big/compound/sub", "[big][add]", Z, double, float) {
    etl::dyn_matrix<Z> a(etl::parallel_threshold, 2UL);
    etl::dyn_matrix<Z> b(etl::parallel_threshold, 2UL);
    etl::dyn_matrix<Z> c(etl::parallel_threshold, 2UL);

    a = etl::uniform_generator(-1000.0, 5000.0);
    b = etl::uniform_generator(-1000.0, 5000.0);
    c = 1200.0;

    c -= a + b;

    for (std::size_t i = 0; i < c.size(); ++i) {
        REQUIRE(c[i] == Approx(Z(1200.0 - (a[i] + b[i]))));
    }
}

TEMPLATE_TEST_CASE_2("big/compound/mul", "[big][add]", Z, double, float) {
    etl::dyn_matrix<Z> a(etl::parallel_threshold, 2UL);
    etl::dyn_matrix<Z> b(etl::parallel_threshold, 2UL);
    etl::dyn_matrix<Z> c(etl::parallel_threshold, 2UL);

    a = etl::uniform_generator(-1000.0, 5000.0);
    b = etl::uniform_generator(-1000.0, 5000.0);
    c = 1200.0;

    c *= a + b;

    for (std::size_t i = 0; i < c.size(); ++i) {
        REQUIRE(c[i] == Approx(Z(1200.0 * (a[i] + b[i]))));
    }
}

TEMPLATE_TEST_CASE_2("big/compound/div", "[big][add]", Z, double, float) {
    etl::dyn_matrix<Z> a(etl::parallel_threshold, 2UL);
    etl::dyn_matrix<Z> b(etl::parallel_threshold, 2UL);
    etl::dyn_matrix<Z> c(etl::parallel_threshold, 2UL);

    a = etl::uniform_generator(1000.0, 5000.0);
    b = etl::uniform_generator(1000.0, 5000.0);
    c = 1200.0;

    c /= a + b;

    for (std::size_t i = 0; i < c.size(); ++i) {
        REQUIRE(c[i] == Approx(Z(1200.0 / (a[i] + b[i]))));
    }
}

TEMPLATE_TEST_CASE_2("big/sum/div", "[big][add]", Z, double, float) {
    etl::dyn_matrix<Z> a(etl::sum_parallel_threshold, 2UL);
    etl::dyn_matrix<Z> b(etl::sum_parallel_threshold, 2UL);

    a = 1.0;
    b = 2.5;

    REQUIRE(etl::sum(a) == 1.0 * etl::sum_parallel_threshold * 2.0);
    REQUIRE(etl::sum(b) == 2.5 * etl::sum_parallel_threshold * 2.0);
}