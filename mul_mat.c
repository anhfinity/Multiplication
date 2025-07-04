#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h> // nhớ cài extension C/C++ runner

#define BLOCK 512 // bội của 16 (16,32,48,64,96)
#define MAX_THREAD 4 // 4 core

typedef struct{
    int **a, **b, **marr;
    int C1, C2;
    int startRow, endRow;
} ThreadData;

int **create2DMat(int R, int C)
{
    int **arr = malloc(R * sizeof(int *));
    for (int i = 0; i < R; i++)
        arr[i] = calloc(C, sizeof(int));

    // ktra xem tạo được chưa
    if (arr == NULL)
    {
        printf("Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    return arr;
}
void saveMat(char *filename, int **arr, int R, int C)
{
    FILE *fp;
    fp = fopen(filename, "w");
    fprintf(fp, "%d %d\n", R, C);
    for (int i = 0; i < R; i++)
    {
        for (int j = 0; j < C; j++)
            fprintf(fp, "%d ", arr[i][j]);
        fprintf(fp, "\n");
    }
    fclose(fp);
}
void setMat(char *filename, int R, int C)
{
    int **arr = create2DMat(R, C);

    for (int i = 0; i < R; i++)
        for (int j = 0; j < C; j++)
            arr[i][j] = rand() % 30 + 1;

    // Store matrix in file
    saveMat(filename, arr, R, C);

    for (int i = 0; i < R; i++)
        free(arr[i]);
    free(arr);
}
int **getMat(char *filename)
{
    FILE *fp;
    int R, C;

    fp = fopen(filename, "r");
    fscanf(fp, "%d %d", &R, &C);
    int **arr = create2DMat(R, C);

    for (int i = 0; i < R; i++)
    {
        for (int j = 0; j < C; j++)
            fscanf(fp, "%d", &arr[i][j]);
    }

    fclose(fp);
    return arr;
}
void disp(int **arr, int R, int C)
{
    printf("Matrix \n");
    for (int i = 0; i < R; i++)
    {
        for (int j = 0; j < C; j++)
            printf("%d ", arr[i][j]);
        printf("\n");
    }
}

void RC_mulMat(int **a, int **b, int R1, int C1, int C2) // PP1: row col
{
    int **marr = malloc(R1 * sizeof(int *));
    for (int i = 0; i < R1; i++)
        marr[i] = calloc(C2, sizeof(int));

    int val = 0;
    clock_t start, end;
    start = clock();
    for (int i = 0; i < R1; i++)
    {
        int *iRowA = a[i];
        int *iRowMarr = marr[i];
        for (int j = 0; j < C2; j++)
        {
            for (int k = 0; k < C1; k++)
            {
                // val += a[i][k] * b[k][j];
                val += iRowA[k] * b[k][j];
            }
            iRowMarr[j] = val;
            // marr[i][j] = val;
            val = 0;
        }
    }
    end = clock();
    printf("Total time pp1: %.3lf \n", ((double)(end - start)) / CLOCKS_PER_SEC);

    // disp(marr, R1, C2);
    char filename[] = "RC_matrix_m";
    saveMat(filename, marr, R1, C2);

    for (int i = 0; i < R1; i++)
        free(marr[i]);
    free(marr);
}

void MC_mulMat(int **a, int **b, int R1, int C1, int C2) // PP2: mat col
{
    int **marr = malloc(R1 * sizeof(int *));
    for (int i = 0; i < R1; i++)
        marr[i] = calloc(C2, sizeof(int));

    int val = 0;
    clock_t start, end;
    start = clock();
    for (int j = 0; j < C2; j++)
    {
        for (int i = 0; i < R1; i++)
        {
            for (int k = 0; k < C1; k++)
            {
                val += a[i][k]*b[k][j];
            }
            marr[i][j] = val;
            val = 0;
        }
    }
    end = clock();
    printf("Total time pp2: %.3lf \n", ((double)(end - start)) / CLOCKS_PER_SEC);

    // disp(marr, R1, C2);
    char filename[] = "MC_matrix_m";
    saveMat(filename, marr, R1, C2);

    for (int i = 0; i < R1; i++)
        free(marr[i]);
    free(marr);
}

void RM_mulMat(int **a, int **b, int R1, int C1, int C2) // PP3: row mat
{
    int **marr = malloc(R1 * sizeof(int *));
    for (int i = 0; i < R1; i++)
        marr[i] = calloc(C2, sizeof(int));

    clock_t start, end;
    start = clock();
    for (int i = 0; i < R1; i++) {
        for (int k = 0; k < C1; k++) {
            for (int j = 0; j < C2; j++) {
                marr[i][j] += a[i][k]*b[k][j];
            }
        }
    }
    end = clock();
    printf("Total time pp3: %.3lf \n", ((double)(end - start)) / CLOCKS_PER_SEC);

    // disp(marr, R1, C2);
    char filename[] = "RM_matrix_m";
    saveMat(filename, marr, R1, C2);

    for (int i = 0; i < R1; i++)
        free(marr[i]);
    free(marr);
}

void faster_RM_mulMat(int **a, int **b, int R1, int C1, int C2) // PP3: still row mat but faster
{
    int **marr = malloc(R1 * sizeof(int *));
    for (int i = 0; i < R1; i++)
        marr[i] = calloc(C2, sizeof(int));
    clock_t start, end;
    start = clock();
    for (int i = 0; i < R1; i++)
    {
        int *iRowA = a[i];   // lấy hàng thứ i của a
        int *iRowMarr = marr[i]; // hàng thứ i của marr
        for (int k = 0; k < C1; k++)
        {
            int *kRowB = b[k];  // hàng thứ i của b
            int ikA = iRowA[k]; // element a[i][k]
            for (int j = 0; j < C2; j++)
            {
                iRowMarr[j] += ikA * kRowB[j]; // marr[i][j] += a[i][k] * b[k][j]
            }
        }
    }
    end = clock();
    printf("Total time pp3_optim: %.3lf \n", ((double)(end - start)) / CLOCKS_PER_SEC);
    // disp(marr, R1, C2);
    char filename[] = "faster_RM_matrix_m";
    saveMat(filename, marr, R1, C2);
    for (int i = 0; i < R1; i++)
        free(marr[i]);
    free(marr);
}

void sumMul(int **a, int **b, int R1, int C1, int C2) // PP4: sum mul
{
    int **marr = malloc(R1 * sizeof(int *));
    for (int i = 0; i < R1; i++)
        marr[i] = calloc(C2, sizeof(int));

    clock_t start, end;
    start = clock();
    for (int k = 0; k < C1; k++) {
        int *kRowB = b[k];
        for (int i = 0; i < R1; i++) {
            int aik = a[i][k];
            int *iRowMarr = marr[i];
            for (int j = 0; j < C2; j++) {
                iRowMarr[j] += aik * kRowB[j];
            }
        }
    }
    end = clock();
    printf("Total time pp4: %.3lf \n", ((double)(end - start)) / CLOCKS_PER_SEC);
    // disp(marr, R1, C2);
    char filename[] = "sumMul_matrix_m";
    saveMat(filename, marr, R1, C2);
    for (int i = 0; i < R1; i++)
        free(marr[i]);
    free(marr);
}

void block_mulMat(int **a, int **b, int R1, int C1, int C2) // PP5: chia nhỏ ma trận
{
    int **marr = malloc(R1 * sizeof(int *));
    for (int i = 0; i < R1; i++)
        marr[i] = calloc(C2, sizeof(int));

    clock_t start, end;
    start = clock();

    // block + pp1
    // for (int ii = 0; ii < R1; ii += BLOCK) {
    //     for (int jj = 0; jj < C2; jj += BLOCK) {
    //         for (int kk = 0; kk < C1; kk += BLOCK) {
    //             for (int i = ii; i < ii + BLOCK && i < R1; i++) {
    //                 int *iRowA = a[i];
    //                 int *iRowMarr = marr[i];
    //                 for (int j = jj; j < jj + BLOCK && j < C2; j++) {
    //                     for (int k = kk; k < kk + BLOCK && k < C1; k++) {
    //                         iRowMarr[j] += iRowA[k] * b[k][j];
    //                     }
    //                 }
    //             }
    //         }
    //     }
    // }
    // end = clock();
    // printf("Total time pp5+pp1: %.3lf \n", ((double)(end - start)) / CLOCKS_PER_SEC);
    
    // block + pp2
    // int val = 0;
    // for (int ii = 0; ii < R1; ii += BLOCK) {
    //     for (int jj = 0; jj < C2; jj += BLOCK) {
    //         for (int kk = 0; kk < C1; kk += BLOCK) {
    //             for (int j = jj; j < jj + BLOCK && j < C2; j++) {
    //                 for (int i = ii; i < ii + BLOCK && i < R1; i++) {
    //                     for (int k = kk; k < kk + BLOCK && k < C1; k++) {
    //                         val += a[i][k]*b[k][j];
    //                     }
    //                     marr[i][j] = val;
    //                     val = 0;
    //                 }
    //             }
    //         }
    //     }
    // }
    // end = clock();
    // printf("Total time pp5+pp2: %.3lf \n", ((double)(end - start)) / CLOCKS_PER_SEC);

    // block + pp3
    // for (int ii = 0; ii < R1; ii += BLOCK) { // ii : ii + Block size
    //     for (int jj = 0; jj < C2; jj += BLOCK) { // jj : jj + Block size
    //         for (int kk = 0; kk < C1; kk += BLOCK) { // kk : kk + Block size
    //             for (int i = ii; i < ii + BLOCK && i < R1; i++) {
    //                 int *iRowA = a[i];
    //                 int *iRowMarr = marr[i];
    //                 for (int k = kk; k < kk + BLOCK && k < C1; k++) {
    //                     int aik = iRowA[k];
    //                     int *kRowB = b[k];
    //                     for (int j = jj; j < jj + BLOCK && j < C2; j++) {
    //                         iRowMarr[j] += aik*kRowB[j];
    //                     }
    //                 }
    //             }
    //         }
    //     }
    // }

    // block + pp4
    for (int ii = 0; ii < R1; ii += BLOCK) {
        for (int jj = 0; jj < C2; jj += BLOCK) {
            for (int kk = 0; kk < C1; kk += BLOCK) {
                for (int k = kk; k < kk + BLOCK && k < C1; k++) {
                    int *kRowB = b[k];
                    for (int i = ii; i < ii + BLOCK && i < R1; i++) {
                        int aik = a[i][k];
                        int *iRowMarr = marr[i];
                        for (int j = jj; j < jj + BLOCK && j < C2; j++) {
                            iRowMarr[j] += aik*kRowB[j];
                        }
                    }
                }
            }
        }
    }
    end = clock();
    printf("Total time pp5+pp4: %.3lf \n", ((double)(end - start)) / CLOCKS_PER_SEC);
    
    // disp(marr, R1, C2);
    char filename[] = "block_matrix_m";
    saveMat(filename, marr, R1, C2);
    for (int i = 0; i < R1; i++)
        free(marr[i]);
    free(marr);
}

void* mult_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int **a = data->a;
    int **b = data->b;
    int **marr = data->marr;
    int C1 = data->C1;
    int C2 = data->C2;

    // pp1
    // for (int i = data->startRow; i < data->endRow; i++) {
    //     for (int j = 0; j < C2; j++) {
    //         int sum = 0;
    //         for (int k = 0; k < C1; k++) {
    //             sum += a[i][k] * b[k][j];
    //         }
    //         marr[i][j] = sum;
    //     }
    // }

    // pp3
    for (int i = data->startRow; i < data->endRow; i++) {
        int *iRowA = a[i];   // lấy hàng thứ i của a
        int *iRowMarr = marr[i]; // hàng thứ i của marr
        for (int k = 0; k < C1; k++)
        {
            int *kRowB = b[k];  // hàng thứ i của b
            int ikA = iRowA[k]; // element a[i][k]
            for (int j = 0; j < C2; j++)
            {
                iRowMarr[j] += ikA * kRowB[j]; // marr[i][j] += a[i][k] * b[k][j]
            }
        }
    }
    pthread_exit(NULL);
}

void multiThread(int **a, int **b, int R1, int C1, int C2)
{
    int **marr = malloc(R1 * sizeof(int *));
    for (int i = 0; i < R1; i++)
        marr[i] = calloc(C2, sizeof(int));

    pthread_t* thread = (pthread_t*)malloc(MAX_THREAD*sizeof(pthread_t));

    ThreadData thread_data[MAX_THREAD];
    int rows_per_thread = R1 / MAX_THREAD; // chia hàng của ma trận a theo thread

    // tạo thread và chạy từng thread
    for (int i = 0; i < MAX_THREAD; i++) {
        thread_data[i].a = a;
        thread_data[i].b = b;
        thread_data[i].marr = marr;
        thread_data[i].C1 = C1;
        thread_data[i].C2 = C2;
        thread_data[i].startRow = i * rows_per_thread;
        thread_data[i].endRow = (i == MAX_THREAD - 1) ? R1 : (i + 1) * rows_per_thread;
        pthread_create(&thread[i], NULL, mult_thread, &thread_data[i]);
    }

    for (int i = 0; i < MAX_THREAD; i++) 
        pthread_join(thread[i], NULL);

    // disp(marr, R1, C2);
    char filename[] = "multiThread_matrix_m";
    saveMat(filename, marr, R1, C2);
    for (int i = 0; i < R1; i++)
        free(marr[i]);
    free(marr);

    free(thread);
}

int main()
{
    int R1, C1, R2, C2;
    printf("Enter size row/col max1: ");
    scanf("%d %d", &R1, &C1);
    printf("Enter size row/col max2: ");
    scanf("%d %d", &R2, &C2);

    char filename_a[] = "matrix_a";
    char filename_b[] = "matrix_b";

    // gen matA and matB
    setMat(filename_a, R1, C1);
    setMat(filename_b, R2, C2);

    // get mat from file
    int **a = getMat(filename_a);
    int **b = getMat(filename_b);
    // display mat
    // disp(a, R1, C1);
    // disp(b, R2, C2);

    if (R2 != C1)
        printf("Can't multiply these 2 matrix!\n");
    else
    {
        RC_mulMat(a, b, R1, C1, C2);
        MC_mulMat(a, b, R1, C1, C2);
        RM_mulMat(a, b, R1, C1, C2);
        faster_RM_mulMat(a, b, R1, C1, C2);
        sumMul(a, b, R1, C1, C2);
        block_mulMat(a, b, R1, C1, C2); // pp5 + pp4
        
        clock_t start, end;
        start = clock();
        multiThread(a, b, R1, C1, C2); // multithreading + pp3
        end = clock();
        printf("Total time pp multithreading+pp3: %.3lf \n", ((double)(end - start)) / CLOCKS_PER_SEC);
    }

    for (int i = 0; i < C1; i++)
        free(a[i]);
    free(a);
    for (int i = 0; i < C2; i++)
        free(b[i]);
    free(b);

    return 0;
}