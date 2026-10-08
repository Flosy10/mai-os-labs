#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

double** create_matrix(int n) {
    double **matrix = (double **)malloc(n * sizeof(double *));
    for (int i = 0; i < n; i++) {
        matrix[i] = (double *)malloc(n * sizeof(double));
    }
    return matrix;
}

void clear(double **matrix, int n) {
    for (int i = 0; i < n; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

double** get_minor(double **matrix, double **minor, int row, int col, int n) {
    int i2 = 0;
    for (int i1 = 0; i1 < n; i1++) {
        if (i1 == row){
            continue;
        }
        int j2 = 0;
        for (int j1 = 0; j1 < n; j1++) {
            if (j1 == col){
                continue;
            }
            minor[i2][j2] = matrix[i1][j1];
            j2++;
        }
        i2++;
    }
    return minor;
}

double determinant(double **matrix, int n) {
    if (n == 1){
        return matrix[0][0];
    }
    if (n == 2){
        return matrix[0][0] * matrix[1][1] - (matrix[0][1] * matrix[1][0]);
    }

    double det = 0.0;
    double **minor = create_matrix(n - 1);
    int i = 0;
    int sign;

    for (int j = 0; j < n; j++) {
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

void print_str(char *str) {
    int len = 0;
    while (str[len] != '\0'){
        len++;
    }
    write(1, str, len);
}

void print_double(double x) {
    char buffer[64];
    int len_res = 0;
    int i = 0;
    if (x < 0.0) buffer[i++] = '-';
    x = fabs(x);
    long long res = (long long)x;
    int frac = (int)((x - res) * 100.0f + 0.5f);
    if (frac >= 100){
        res += 1;
        frac -= 100;
    }
    int len_frac = 2;
    
    long long temp_res = res;
    if (temp_res == 0){
        len_res = 1;
    }
    else {
        while (temp_res > 0){
            temp_res /= 10;
            len_res++;
        }
    }
    while (len_res != 0) {
        long long digit = res / (long long)pow(10, len_res - 1);
        buffer[i++] = (char)(digit) + '0';
        res -= digit * (long long)pow(10, len_res - 1);
        len_res--;
    }
    buffer[i++] = '.';
    while (len_frac != 0) {
        int digit = frac / (int)pow(10, len_frac - 1);
        buffer[i++] = (char)(digit) + '0';
        frac -= digit * (int)pow(10, len_frac - 1);
        len_frac--;
    }
    buffer[i] = '\0';
    print_str(buffer);
}

int str_to_int(char *str) {
    int res = 0;
    int i = 0;
    while (str[i] != '\0') {
        res = res * 10 + (str[i] - '0');
        i++;
    }
    return res;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_str("Использование: ./a.out <размер_матрицы>\n");
        return 0;
    }

    int n = str_to_int(argv[1]);
    double **matrix = create_matrix(n);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            matrix[i][j] = (rand() % 10) + 1; 
        }
    }

    struct timespec start, end;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    double res = determinant(matrix, n);
    clock_gettime(CLOCK_MONOTONIC, &end);

    double diff = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_nsec - start.tv_nsec) / 1000000.0;

    print_str("Последовательный определитель равен: ");
    print_double(res);
    print_str(",время работы программы: ");
    print_double(diff);
    print_str(" мс\n");

    clear(matrix, n);
    return 0;
}
