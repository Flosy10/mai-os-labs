#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <math.h>
#ifndef NUM_THREADS
#define NUM_THREADS 1
#endif

typedef struct ThreadData{
    double **matrix;
    int row;
    int col;
    int n;
    int sign;
    double result;
} ThreadData;

double** create_matrix (int n){
    double **matrix = (double **)malloc(n * sizeof(double *));
    for (int i = 0; i < n; i ++){
        matrix[i] = (double *)malloc(n * sizeof(double));
    }
    return matrix;
}

void clear(double **matrix, int n){
    for (int i = 0; i < n; i ++){
        free(matrix[i]);
    }
    free(matrix);
}

double **get_minor(double **matrix, double **minor, int row, int col, int n){
    int i2 = 0;
    for (int i1 = 0; i1 < n; i1 ++){
        if (i1 == row){
            continue;
        }
        int j2 = 0;
        for (int j1 = 0; j1 < n; j1 ++){
            if (j1 == col){
                continue;
            }
            minor[i2][j2] = matrix[i1][j1];
            j2 ++; 
        }
        i2 ++;
    }

    return minor;
}

double determinant(double **matrix, int n){
    double det = 0.0;
    int sign;
    if (n == 1){
        return matrix[0][0];
    }

    else if (n == 2){
        return matrix[0][0] * matrix[1][1] - (matrix[0][1] * matrix[1][0]);
    }

    else{
        double **minor = create_matrix(n - 1);
        int i = 0;
        for (int j = 0; j < n; j ++){
            if ((i + j) % 2 == 0){
                sign = 1;
            }
            else{
                sign = -1;
            }
            det += sign * matrix[i][j] * determinant(get_minor(matrix, minor, i, j, n), n - 1);
        }
        clear(minor, n - 1);
        return det;
    }
}

void *parallel_det(void *arg){
    ThreadData *data = (ThreadData *)arg;

    double **minor = create_matrix(data->n - 1);
    
    if ((data->row + data->col) % 2 == 0){
        data->sign = 1;
    }
    else{
        data->sign = -1;
    }

    data->result = data->sign * data->matrix[data->row][data->col] * determinant(get_minor(data->matrix, minor, data->row, data->col, data->n), data->n - 1);
    clear(minor, data->n - 1);
    return NULL;
}

void print_str(char *str){
    int len = 0;
    while (str[len] != '\0'){
        len ++;
    }

    write(1, str, len);
}

void print_double(double x){
    char buffer[64];
    int len_res = 0;
    int i = 0;
    if (x < 0.0){
        buffer[i++] = '-';
    }
    x = fabs(x);
    long long res = (long long) x;
    int frac = (int)((x - res) * 100.0f + 0.5f);
    if (frac >= 100) {
        res += 1;
        frac -= 100;
    }
    int len_frac = 2;
    
    long long temp_res = res;
    if (temp_res == 0){
        len_res = 1;
    }
    else{
        while (temp_res > 0){
            temp_res /= 10;
            len_res ++;
        }
    }

    while (len_res != 0){
        long long digit = res / pow(10, len_res - 1);
        buffer[i++] = (char)(digit) + '0';
        res -= digit * pow(10, len_res - 1);
        len_res -= 1;
    }
    buffer[i++] = '.';
    while (len_frac != 0){
        int digit = frac / pow(10, len_frac - 1);
        buffer[i++] = (char)(digit) + '0';
        frac -= digit * pow(10, len_frac - 1);
        len_frac -= 1;
    }

    buffer[i] = '\0';

    print_str(buffer);
}

int str_to_int(char *str){
    int res = 0;
    int i = 0;
    while (str[i] != '\0'){
        res = res * 10 + str[i] - '0';
        i ++;
    }

    return res;
}

int main(int argc, char *argv[]){
    if (argc < 2){
        print_str("Использование ./a.out <размер_матрицы>\n");
        return 0;
    }
    pthread_t threads[NUM_THREADS];
    ThreadData args[NUM_THREADS];
    int active_threads = 0;
    double res = 0.0;
    int i = 0;
    int j = 0;
    int n, elem;

    n = str_to_int(argv[1]);
    double **matrix = create_matrix(n);
    for (int i = 0; i < n; i ++){
        for (int j = 0; j < n; j ++){
            matrix[i][j] = (rand() % 10) + 1;
        }
    }

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    while (j < n){
        if (active_threads == NUM_THREADS){
            for (int i = 0; i < NUM_THREADS; i ++){
                pthread_join(threads[i], NULL);
                res += args[i].result;
            }
            active_threads = 0;
        }
        args[active_threads].matrix = matrix;
        args[active_threads].row = i;
        args[active_threads].col = j;
        args[active_threads].n = n;
        pthread_create(&threads[active_threads], NULL, parallel_det, &args[active_threads]);
        active_threads ++;
        j ++;
    };

    if (active_threads > 0){
        for (int i = 0; i < active_threads; i ++){
            pthread_join(threads[i], NULL);
            res += args[i].result;
        }
        active_threads = 0;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    double diff = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1000000.0;
    print_str("Параллельный определитель равен: ");
    print_double(res);
    print_str(",время работы программы: ");
    print_double(diff);
    print_str(" мс\n");
}
