#include <cstdio>
#include <cassert>
#include <cmath>
#include <vector>
#include "mkl.h"
#include "mkl_spblas.h"
#include <chrono>  // for high_resolution_clock
#include <iostream>

int main() {
    std::cout << mkl_get_max_threads() << std::endl;
    std::vector<MKL_INT> indptr;
    std::vector<MKL_INT> indices;

    long int tmp_lint;
    int tmp_int;
    float tmp_float;
    std::cout << "MAK_INT_MAX=" << (~((MKL_UINT) 0) >> 1) << std::endl;

    FILE* findptr = fopen("../sparse_youtube_3.netsmf_indptr.txt", "r");
    while (fscanf(findptr, "%ld", &tmp_lint) != EOF) {
        indptr.push_back(tmp_lint);
    }
    fclose(findptr);
    printf("%lu\n", indptr.size());

    MKL_INT N = indptr.size() - 1;
    std::cout << indptr[N] << std::endl;
    std::cout << N << std::endl;
    MKL_INT d = 256;
    float* x = new float[N * d];
    float* y = new float[N * d];
    std::cout << "create dummy x done" << std::endl;

    FILE* findices = fopen("../sparse_youtube_3.netsmf_indices.txt", "r");
    while (fscanf(findices, "%d", &tmp_int) != EOF) {
        indices.push_back(tmp_int);
    }
    fclose(findices);
    printf("%lu\n", indices.size());

    std::vector<float> value;
    FILE* fvalue = fopen("../sparse_youtube_3.netsmf_value.txt", "r");
    while (fscanf(fvalue, "%f", &tmp_float) != EOF) {
        value.push_back(tmp_float);
    }
    fclose(findices);
    // std::vector<float> value(indices.size(), 1.0);
    printf("%lu\n", value.size());
    // struct matrix_descr descrA;
    // sparse_status_t status;
    sparse_matrix_t csrA;
    MKL_INT status;
    std::cout << "calling mkl_sparse_s_create_csr" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    status = mkl_sparse_s_create_csr(&csrA,
                        SPARSE_INDEX_BASE_ZERO,
                        N, N,
                        indptr.data(),
                        indptr.data() + 1,
                        indices.data(),
                        value.data());
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = finish - start;
    std::cout << "Elapsed time: " << elapsed.count() << " s, status=" << status << std::endl;
    std::cout << "create sparse csr matrix done, status=" << status << std::endl;

    mkl_sparse_optimize ( csrA );

    // Create matrix descriptor
    struct matrix_descr descrA;
    descrA.type = SPARSE_MATRIX_TYPE_GENERAL;
    start = std::chrono::high_resolution_clock::now();
    std::cout << "going to d_mm" << std::endl;
    status = mkl_sparse_s_mm(SPARSE_OPERATION_NON_TRANSPOSE,
                        1.,
                        csrA,
                        descrA,
                        SPARSE_LAYOUT_COLUMN_MAJOR,
                        x,
                        d,   // number of right hand sides
                        N,      // ldx
                        0.,
                        y,
                        N);
    // status = mkl_sparse_d_mv( SPARSE_OPERATION_NON_TRANSPOSE, 1.0, csrA, descrA, x, 0.0, y);
    finish = std::chrono::high_resolution_clock::now();
    elapsed = finish - start;
    std::cout << "Elapsed time: " << elapsed.count() << " s, status=" << status << std::endl;
    // https://software.intel.com/en-us/articles/implementation-of-classic-gram-schmidt-in-a-reservoir-simulator
    return 0;
}
