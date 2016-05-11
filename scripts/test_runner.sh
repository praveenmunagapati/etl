#!/bin/bash
set -e

# Disable default options
export ETL_NO_DEFAULT=true
unset ETL_DEFAULTS
unset ETL_MKL
unset ETL_BLAS
unset ETL_CUBLAS
unset ETL_CUFFT

# Use gcc
export CXX=$ETL_GPP
export LD=$ETL_GPP

echo "Test 1. GCC (debug default)"

export ETL_DEFAULTS="-DCPP_UTILS_ASSERT_EXCEPTION"

make clean
make $ETL_THREADS debug/bin/etl_test
./debug/bin/etl_test
gcovr -x -b -r . --object-directory=debug/test > coverage_1.xml

echo "Test 2. GCC (debug vectorize avx)"

export ETL_DEFAULTS="-DETL_VECTORIZE_FULL -mavx"

make clean
make $ETL_THREADS debug/bin/etl_test
./debug/bin/etl_test
gcovr -x -b -r . --object-directory=debug/test > coverage_2.xml

echo "Test 3. GCC (debug vectorize sse)"

export ETL_DEFAULTS="-DETL_VECTORIZE_FULL -msse3 -msse4"

make clean
make $ETL_THREADS debug/bin/etl_test
./debug/bin/etl_test
gcovr -x -b -r . --object-directory=debug/test > coverage_3.xml

echo "Test 4. GCC (debug mkl)"

unset ETL_DEFAULTS
export ETL_MKL=true

make clean
make $ETL_THREADS debug/bin/etl_test
./debug/bin/etl_test
gcovr -x -b -r . --object-directory=debug/test > coverage_4.xml

echo "Test 5. GCC (debug parallel)"

unset ETL_MKL
export ETL_DEFAULTS="-DETL_PARALLEL"

make clean
make $ETL_THREADS debug/bin/etl_test
./debug/bin/etl_test
gcovr -x -b -r . --object-directory=debug/test > coverage_5.xml

echo "Test 6. GCC (debug vectorize sse avx parallel)"

unset ETL_MKL
export ETL_DEFAULTS="-DETL_PARALLEL -DETL_VECTORIZE_FULL -msse3 -msse4 -mavx"

make clean
make $ETL_THREADS debug/bin/etl_test
./debug/bin/etl_test
gcovr -x -b -r . --object-directory=debug/test > coverage_6.xml

if [ "$ETL_NO_GPU" == "" ]
then
    echo "Test 7. GCC (debug cublas cufft)"

    unset ETL_DEFAULTS
    unset ETL_MKL
    export ETL_CUBLAS=true
    export ETL_CUFFT=true

    make clean
    make $ETL_THREADS debug/bin/etl_test
    ./debug/bin/etl_test
    gcovr -x -b -r . --object-directory=debug/test > coverage_7.xml

    echo "Merge the coverage reports"

    merge-xml-coverage.py -o coverage_report.xml coverage_1.xml coverage_2.xml coverage_3.xml coverage_4.xml coverage_5.xml coverage_6.xml coverage_7.xml
else
    echo "Merge the coverage reports"

    merge-xml-coverage.py -o coverage_report.xml coverage_1.xml coverage_2.xml coverage_3.xml coverage_4.xml coverage_5.xml coverage_6.xml coverage_7.xml
fi
