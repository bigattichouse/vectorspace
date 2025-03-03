#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "vs_core.h"
#include "vs_dimensionindex.h"

// Utility to generate random float value between 0 and 1
float random_float() {
    return (float)rand() / (float)RAND_MAX;
}

// Utility to generate a unique t_uuid for testing
t_uuid generate_test_uuid(int index) {
    t_uuid id = newid();
    // Modify a few bytes to make it more predictable for testing
    unsigned char *bytes = (unsigned char *)&id;
    bytes[0] = (index & 0xFF);
    bytes[1] = ((index >> 8) & 0xFF);
    return id;
}

// Basic functionality test
void test_basic_vector_operations() {
    printf("Testing basic vector operations...\n");
    
    // Create a new vector
    t_uuid vid = newid();
    vector *v = vs_createvector(vid, 10);
    
    // Set some values
    t_uuid dim1 = generate_test_uuid(1);
    t_uuid dim2 = generate_test_uuid(2);
    t_uuid dim3 = generate_test_uuid(3);
    
    vs_setvalue(v, dim1, 1.5);
    vs_setvalue(v, dim2, 2.5);
    vs_setvalue(v, dim3, 3.5);
    
    // Test finding dimensions
    long idx1 = vs_finddimension(v, dim1);
    long idx2 = vs_finddimension(v, dim2);
    long idx3 = vs_finddimension(v, dim3);
    
    printf("Dimension 1 index: %ld\n", idx1);
    printf("Dimension 2 index: %ld\n", idx2);
    printf("Dimension 3 index: %ld\n", idx3);
    
    // Test getting values
    dimension *d1 = vs_getvalue(v, dim1);
    dimension *d2 = vs_getvalue(v, dim2);
    dimension *d3 = vs_getvalue(v, dim3);
    
    printf("Dimension 1 value: %f\n", d1 ? d1->value : -1.0);
    printf("Dimension 2 value: %f\n", d2 ? d2->value : -1.0);
    printf("Dimension 3 value: %f\n", d3 ? d3->value : -1.0);
    
    // Test magnitude calculation
    float mag = vs_magnitude(v);
    printf("Vector magnitude: %f\n", mag);
    
    // Create a second vector for comparison operations
    t_uuid vid2 = newid();
    vector *v2 = vs_createvector(vid2, 10);
    
    vs_setvalue(v2, dim1, 1.0);
    vs_setvalue(v2, dim2, 2.0);
    vs_setvalue(v2, dim3, 3.0);
    
    // Test vector operations
    float dot = vs_dotproduct(v, v2);
    float cos = vs_cosine(v, v2);
    
    printf("Dot product: %f\n", dot);
    printf("Cosine similarity: %f\n", cos);
    
    // Test vector merging
    vs_mergevectors(v, v2);
    
    // Check the result
    d1 = vs_getvalue(v, dim1);
    d2 = vs_getvalue(v, dim2);
    d3 = vs_getvalue(v, dim3);
    
    printf("After merge, dimension 1 value: %f\n", d1 ? d1->value : -1.0);
    printf("After merge, dimension 2 value: %f\n", d2 ? d2->value : -1.0);
    printf("After merge, dimension 3 value: %f\n", d3 ? d3->value : -1.0);
    
    // Clean up
    vs_destroyvector(&v);
    vs_destroyvector(&v2);
    
    printf("Basic vector operations test complete.\n\n");
}

// Test the index functionality
void test_dimension_index() {
    printf("Testing dimension index functionality...\n");
    
    // Create a vector with an index
    t_uuid vid = newid();
    vector *v = vs_createvector(vid, 100);
    
    // Create and populate dimension index
    __vdim_createvectorindex(v);
    
    // Add dimensions
    const int num_dims = 100;
    t_uuid dims[num_dims];
    for (int i = 0; i < num_dims; i++) {
        dims[i] = generate_test_uuid(i);
        vs_setvalue(v, dims[i], (float)i * 0.5);
    }
    
    // Measure lookup time with index
    clock_t start = clock();
    for (int i = 0; i < num_dims; i++) {
        long idx = vs_finddimension(v, dims[i]);
        if (idx == -1) {
            printf("Error: Failed to find dimension %d with index!\n", i);
        }
    }
    clock_t end = clock();
    double time_with_index = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    // Clear the index and measure without it
    vs_clearindex(v);
    
    start = clock();
    for (int i = 0; i < num_dims; i++) {
        long idx = vs_finddimension(v, dims[i]);
        if (idx == -1) {
            printf("Error: Failed to find dimension %d without index!\n", i);
        }
    }
    end = clock();
    double time_without_index = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Lookup time with index: %f seconds\n", time_with_index);
    printf("Lookup time without index: %f seconds\n", time_without_index);
    printf("Speedup factor: %f\n", time_without_index / time_with_index);
    
    // Clean up
    vs_destroyvector(&v);
    
    printf("Dimension index test complete.\n\n");
}

// Test vector clone functionality
void test_vector_clone() {
    printf("Testing vector clone functionality...\n");
    
    // Create a source vector
    t_uuid vid = newid();
    vector *src = vs_createvector(vid, 10);
    
    // Add some dimensions
    for (int i = 0; i < 10; i++) {
        t_uuid dimid = generate_test_uuid(i);
        vs_setvalue(src, dimid, (float)i);
    }
    
    // Create a destination vector
    t_uuid vid2 = newid();
    vector *dst = vs_createvector(vid2, 10);
    
    // Clone the source to destination
    vs_clone(src, dst);
    
    // Verify dimensions and values
    float src_mag = vs_magnitude(src);
    float dst_mag = vs_magnitude(dst);
    float cos = vs_cosine(src, dst);
    
    printf("Source vector magnitude: %f\n", src_mag);
    printf("Destination vector magnitude: %f\n", dst_mag);
    printf("Cosine similarity between source and clone: %f\n", cos);
    
    // Clean up
    vs_destroyvector(&src);
    vs_destroyvector(&dst);
    
    printf("Vector clone test complete.\n\n");
}

// Performance test with bulk operations
void test_bulk_operations(int vector_count, int dim_count) {
    printf("Testing bulk operations with %d vectors of %d dimensions each...\n", vector_count, dim_count);
    
    // Create dimension IDs
    t_uuid *dim_ids = malloc(dim_count * sizeof(t_uuid));
    for (int i = 0; i < dim_count; i++) {
        dim_ids[i] = generate_test_uuid(i);
    }
    
    // Create a template vector with all dimensions
    t_uuid template_id = newid();
    vector *template_vec = vs_createvector(template_id, dim_count);
    for (int i = 0; i < dim_count; i++) {
        vs_setvalue(template_vec, dim_ids[i], random_float());
    }
    
    // Create an array of vectors
    printf("Creating %d vectors...\n", vector_count);
    clock_t start = clock();
    
    vector **vectors = malloc(vector_count * sizeof(vector *));
    
    for (int i = 0; i < vector_count; i++) {
        t_uuid vid = newid();
        vectors[i] = vs_createvector(vid, dim_count);
        
        // Populate with some random dimensions from the template
        for (int j = 0; j < dim_count; j++) {
            if (rand() % 2) {  // 50% chance to include each dimension
                vs_setvalue(vectors[i], dim_ids[j], random_float());
            }
        }
        
        // Create index for each vector
        __vdim_createvectorindex(vectors[i]);
        
        if (i % (vector_count / 1000) == 0) {
            printf("Created %d vectors (%.1f%%)\n", i, 100.0 * i / vector_count);
        }
    }
    
    clock_t end = clock();
    double creation_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Vector creation time: %f seconds\n", creation_time);
    
    // Test cosine similarity between random pairs
    printf("Testing cosine similarity between random pairs...\n");
    start = clock();
    
    const int pair_tests = 10000;
    float total_similarity = 0.0;
    
    for (int i = 0; i < pair_tests; i++) {
        int idx1 = rand() % vector_count;
        int idx2 = rand() % vector_count;
        
        float cosine = vs_cosine(vectors[idx1], vectors[idx2]);
        total_similarity += cosine;
    }
    
    end = clock();
    double similarity_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Average similarity: %f\n", total_similarity / pair_tests);
    printf("Time for %d similarity calculations: %f seconds\n", pair_tests, similarity_time);
    
    // Test dimension lookup speed
    printf("Testing dimension lookup speed...\n");
    start = clock();
    
    const int lookup_tests = 10000;
    long lookup_success = 0;
    
    for (int i = 0; i < lookup_tests; i++) {
        int vec_idx = rand() % vector_count;
        int dim_idx = rand() % dim_count;
        
        long found_idx = vs_finddimension(vectors[vec_idx], dim_ids[dim_idx]);
        if (found_idx != -1) {
            lookup_success++;
        }
    }
    
    end = clock();
    double lookup_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Successful lookups: %ld out of %d\n", lookup_success, lookup_tests);
    printf("Time for %d dimension lookups: %f seconds\n", lookup_tests, lookup_time);
    
    // Clean up
    for (int i = 0; i < vector_count; i++) {
        vs_destroyvector(&vectors[i]);
        
        if (i % (vector_count / 10) == 0) {
            printf("Cleaned up %d vectors (%.1f%%)\n", i, 100.0 * i / vector_count);
        }
    }
    
    free(vectors);
    free(dim_ids);
    vs_destroyvector(&template_vec);
    
    printf("Bulk operations test complete.\n\n");
}

// Test multi-vector operations
void test_vector_operations() {
    printf("Testing vector operations...\n");
    
    // Create test vectors
    t_uuid vid1 = newid();
    t_uuid vid2 = newid();
    vector *v1 = vs_createvector(vid1, 10);
    vector *v2 = vs_createvector(vid2, 10);
    
    // Create dimension IDs
    t_uuid dim_ids[5];
    for (int i = 0; i < 5; i++) {
        dim_ids[i] = generate_test_uuid(i);
    }
    
    // Set values in v1
    vs_setvalue(v1, dim_ids[0], 1.0);
    vs_setvalue(v1, dim_ids[1], 2.0);
    vs_setvalue(v1, dim_ids[2], 3.0);
    
    // Set values in v2
    vs_setvalue(v2, dim_ids[0], 2.0);
    vs_setvalue(v2, dim_ids[2], 4.0);
    vs_setvalue(v2, dim_ids[3], 5.0);
    
    // Print original vectors
    printf("Vector 1 original:\n");
    vs_printvector(v1);
    printf("Vector 2 original:\n");
    vs_printvector(v2);
    
    // Test merge vectors
    printf("\nTesting merge vectors...\n");
    vector *v_merge = vs_createvector(newid(), 10);
    vs_clone(v1, v_merge);
    vs_mergevectors(v_merge, v2);
    printf("Merged result:\n");
    vs_printvector(v_merge);
    
    // Test sum vectors
    printf("\nTesting sum vectors...\n");
    vector *v_sum = vs_createvector(newid(), 10);
    vs_clone(v1, v_sum);
    vs_sumvectors(v_sum, v2);
    printf("Sum result:\n");
    vs_printvector(v_sum);
    
    // Test weighted operations
    printf("\nTesting weighted operations...\n");
    vector *v_weighted = vs_createvector(newid(), 10);
    vs_clone(v1, v_weighted);
    vs_sumvectorsweighted(v_weighted, v2, 0.5);
    printf("Weighted sum result (v1 + 0.5*v2):\n");
    vs_printvector(v_weighted);
    
    // Clean up
    vs_destroyvector(&v1);
    vs_destroyvector(&v2);
    vs_destroyvector(&v_merge);
    vs_destroyvector(&v_sum);
    vs_destroyvector(&v_weighted);
    
    printf("Vector operations test complete.\n\n");
}

// Function to run scaled-down tests for debugging
void run_mini_bulk_test() {
    printf("Running mini bulk test for debugging...\n");
    test_bulk_operations(1000, 100);  // 1000 vectors with 100 dimensions each
}

// Function to run the full-scale test
void run_full_bulk_test() {
    printf("Running full bulk test...\n");
    test_bulk_operations(50000, 1000);  // 1 million vectors with 1000 dimensions each
}

int main(int argc, char **argv) {
    // Seed the random number generator
    srand(time(NULL));
    
    printf("Vector System Test Script\n");
    printf("======================\n\n");
    
    // Parse command line arguments
    int run_full_test = 0;
    if (argc > 1) {
        if (strcmp(argv[1], "--full") == 0) {
            run_full_test = 1;
        }
    }
    
    // Run basic tests
    test_basic_vector_operations();
    test_dimension_index();
    test_vector_clone();
    test_vector_operations();
    
    // Run bulk tests
    if (run_full_test) {
        printf("Running full-scale bulk tests (this may take a while)...\n");
        run_full_bulk_test();
    } else {
        printf("Running mini bulk tests (use --full for full-scale tests)...\n");
        run_mini_bulk_test();
    }
    
    printf("All tests complete.\n");
    return 0;
}
