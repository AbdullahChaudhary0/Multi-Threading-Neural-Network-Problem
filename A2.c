#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <pthread.h>

void swap(int* x, int* y) {
    int temp = *x;
    *x = *y;
    *y = temp;
}

// Function to generate permutations in lexicographic order
void generatePermutations(int* array, int start, int end, int** permMatrix, int* permIndex) {
    if (start == end) {
        for (int i = 0; i <= end; i++) {
            permMatrix[*permIndex][i] = array[i];
        }
        (*permIndex)++;
    } else {
        for (int i = start; i <= end; i++) {
            // Swap and generate permutations
            swap(&array[start], &array[i]);
            generatePermutations(array, start + 1, end, permMatrix, permIndex);
            swap(&array[start], &array[i]); // backtrack
        }
    }
}

// Function to calculate dot product
int dotProduct(int* array1, int* array2, int size) {
    int result = 0;
    for (int i = 0; i < size; i++) {
        result += array1[i] * array2[i];
    }
    return result;
}

// Function to adjust weights using the logarithmic rule
void adjustWeights(int* weights, int size) {
    for (int i = 0; i < size / 2; i++) {
        int j = size - 1 - i;
        int difference = abs(weights[i] - weights[j]);
        if (difference > 0) {
            weights[i] -= log(difference + 1); // +1 to avoid log(0)
        }
        // Prevent weights from becoming negative or zero
        if (weights[i] < 1) {
            weights[i] = 1;
        }
    }
}

// Function to print the permutation matrix
void printPermutationMatrix(int** permMatrix, int rows, int cols) {
    printf("Permutation Matrix:\n");
    for (int i = 0; i < rows; i++) {
        printf("[ ");
        for (int j = 0; j < cols; j++) {
            printf("%d ", permMatrix[i][j]);
        }
        printf("]\n");
    }
}

// Function to swap two rows in the permutation matrix
void swapRows(int** permMatrix, int rows, int cols) {
    if (rows < 2) {
        printf("Not enough rows to swap.\n");
        return;
    }
    int lastRow = rows - 1;
    int secondLastRow = rows - 2;
    
    for (int i = 0; i < cols; i++) {
        swap(&permMatrix[lastRow][i], &permMatrix[secondLastRow][i]);
    }
}

// Thread arguments structure
typedef struct {
    int start;
    int end;
    int* weights;
    int* intermediate;
    int* output;
    int factorial;
} ThreadArgs;

// Function for computing output in a thread
void* computeOutput(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    int start = args->start;
    int end = args->end;
    int* weights = args->weights;
    int* intermediate = args->intermediate;
    int* output = args->output;
    int factorial = args->factorial;

    for (int i = start; i < end; i++) {
        output[i] = 0;
        for (int j = 0; j < factorial; j++) {
            output[i] += weights[j] * intermediate[i];
        }
    }
    return NULL;
}

int main() {
    // Open the file
    FILE *file = fopen("Input.txt", "r");
    if (file == NULL) {
        printf("Error opening file.\n");
        return 1;
    }

    int iterations;
    fscanf(file, "%d", &iterations);

    int *array = NULL;
    int size = 0;
    int value;

    while (fscanf(file, "%d", &value) != EOF) {
        array = (int *)realloc(array, (size + 1) * sizeof(int));
        array[size++] = value;
    }

    fclose(file);

    // Declarations and allocations outside the loop
    int factorial = 1;
    int* weights = NULL;
    int** permMatrix = NULL;
    int* intermediate = NULL;
    int* output = NULL;

    // Calculate factorial
    for (int i = 1; i <= size; i++) {
        factorial *= i;
    }

    // Allocate memory
    permMatrix = (int**)malloc(factorial * sizeof(int*));
    for (int i = 0; i < factorial; i++) {
        permMatrix[i] = (int*)malloc(size * sizeof(int));
    }

    weights = (int*)malloc(factorial * sizeof(int));
    intermediate = (int*)malloc(size * sizeof(int));
    output = (int*)malloc(size * sizeof(int));

    // Initial array transformation
    for (int i = 0; i < size; i++) {
        intermediate[i] = array[i] - array[size - 1 - i];
    }

    printf("Intermediate array:\n[");
    for (int i = 0; i < size; i++) {
        printf("%d ", intermediate[i]);
    }
    printf("]\n");

    // Start time measurement
    clock_t start_time = clock();

    for (int iter = 0; iter < iterations; iter++) {
        int permIndex = 0;
        generatePermutations(array, 0, size - 1, permMatrix, &permIndex);

        // Swap the last and second last rows
        swapRows(permMatrix, factorial, size);

        // Print the permutation matrix
        printPermutationMatrix(permMatrix, factorial, size);

        for (int i = 0; i < factorial; i++) {
            weights[i] = dotProduct(permMatrix[i], array, size);
        }

        printf("Iteration %d\n", iter + 1);
        printf("Initial Input:\n[");
        for (int i = 0; i < size; i++) {
            printf("%d ", array[i]);
        }
        printf("]\n");

        if (iter != 0)
            adjustWeights(weights, factorial);

        printf("Weights:\n[");
        for (int i = 0; i < factorial; i++) {
            printf("%d ", weights[i]);
        }
        printf("]\n");

        // Multi-threaded output computation
        int numThreads = 4; // Adjust based on your system
        pthread_t threads[numThreads];
        ThreadArgs args[numThreads];
        int chunkSize = (size + numThreads - 1) / numThreads; // Ensure proper chunk division

        for (int i = 0; i < numThreads; i++) {
            args[i].start = i * chunkSize;
            args[i].end = (i == numThreads - 1) ? size : (i + 1) * chunkSize;
            args[i].weights = weights;
            args[i].intermediate = intermediate;
            args[i].output = output;
            args[i].factorial = factorial;
            pthread_create(&threads[i], NULL, computeOutput, &args[i]);
        }

        for (int i = 0; i < numThreads; i++) {
            pthread_join(threads[i], NULL);
        }

        printf("Output After Iteration %d:\n[", iter + 1);
        for (int i = 0; i < size; i++) {
            printf("%d ", output[i]);
        }
        printf("]\n");

        // Adjust weights using the logarithmic rule
        adjustWeights(weights, factorial);

        // Update the array to the new output for the next iteration
        for (int i = 0; i < size; i++) {
            array[i] = output[i];
        }

        printf("\n");
    }

    // End time measurement
    clock_t end_time = clock();
    double time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

    // Final output after all iterations
    printf("Final Output after %d iterations:\n[", iterations);
    for (int i = 0; i < size; i++) {
        printf("%d ", array[i]);
    }
    printf("]\n");

    // Print the time taken for iterations
    printf("Time taken for the iterations: %.4f seconds\n", time_taken);

    // Free allocated memory
    for (int i = 0; i < factorial; i++) {
        free(permMatrix[i]);
    }
    free(permMatrix);
    free(weights);
    free(intermediate);
    free(output);
    free(array);

    return 0;
}

