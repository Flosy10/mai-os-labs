#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "shared.h"

int str_len(const char *str){
    int len = 0;
    while (str[len] != '\0'){
        len ++;
    }
    return len;
}

float parse_float(const char* str){
    float res = 0.0;
    float frac = 1.0;
    int i = 0;
    int is_frac = 0;
    int sign = 1;
    if (str[0] == '-'){
        sign = -1;
        i ++;
    }
    for (; str[i] != '\0'; i ++){
        if (str[i] == '.'){
            is_frac = 1;
            continue;
        }
        if (str[i] >= '0' && str[i] <= '9'){
            if (!is_frac){
                res = res * 10.0 + (str[i] - '0');
                continue;
            }
            else {
                frac = frac / 10;
                res = res + (str[i] - '0') * frac;
            }
        }
    }
    return res * sign;
}

int main(int argc, char *argv[]){
    SharedData *data;
    sem_t *empty;
    sem_t *full;
    if (argc != 4){
        const char *msg = "Некорректное использование\n";
        write(2, msg, str_len(msg));
        return 1;
    }
    int shm_fd = shm_open(argv[1], O_RDWR, 0);
    if (shm_fd == -1){
        const char *msg = "Ошибка shm_open\n";
        write(2, msg, str_len(msg));
        return 1;
    }

    data = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (data == MAP_FAILED){
        const char *msg = "Ошибка mmap\n";
        write(2, msg, str_len(msg));
        close(shm_fd);
        return 1;
    }
    close(shm_fd);

    empty = sem_open(argv[2], 0);
    if (empty == SEM_FAILED){
        const char *msg = "Ошибка семафора empty\n";
        write(2, msg, str_len(msg));
        munmap(data, sizeof(SharedData));
        return 1;
    }

    full = sem_open(argv[3], 0);
    if (full == SEM_FAILED){
        const char *msg = "Ошибка семафора full\n";
        write(2, msg, str_len(msg));
        munmap(data, sizeof(SharedData));
        sem_close(empty);
        return 1;
    }

    char num_buff[64], ch;
    int num_len = 0, has_num = 0;
    float line_sum = 0.0f;
    while (read(0, &ch, 1) > 0){
        if ((ch >= '0' && ch <= '9') || ch == '.' || (ch == '-' && num_len == 0)){
            if (num_len < 63){
            num_buff[num_len ++] = ch;
            }
        }
        else{
            if (num_len > 0){
                num_buff[num_len] = '\0';
                line_sum += parse_float(num_buff);
                num_len = 0;
                has_num = 1;
            }
            if (ch == '\n'){
                if (has_num){
                    sem_wait(empty);
                    data->sum = line_sum;
                    data->status = 1;
                    sem_post(full);
                }
                line_sum = 0.0f;
                has_num = 0;
           }
        }
    }
    if (num_len > 0){
        num_buff[num_len] = '\0';
        line_sum += parse_float(num_buff);
        num_len = 0;
        has_num = 1;
    }
    if (has_num){
        sem_wait(empty);
        data->sum = line_sum;
        data->status = 1;
        sem_post(full);
    }

    sem_wait(empty);
    data->status = 2;
    sem_post(full);

    munmap(data, sizeof(SharedData));
    sem_close(full);
    sem_close(empty);
    return 0;
}
