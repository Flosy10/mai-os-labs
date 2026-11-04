#include <unistd.h>
#include <math.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <semaphore.h>
#include "shared.h"
#include <stdbool.h>

int str_len(const char *str){
    int len = 0;
    while (str[len] != '\0'){
        len ++;
    }
    return len;
}

void pid_str(char buff[64], pid_t num) {
    int temp_num = (int) num;
    int len_num = 0;

    if (temp_num == 0){
        buff[0] = '0';
        buff[1] = '\0';
        return;
    }

    while (temp_num > 0) {
        len_num++;
        temp_num /= 10;
    }

    int i = len_num - 1;
    temp_num = (int) num;
    buff[i+1] = '\0';
    while (temp_num > 0) {
        buff[i--] = (temp_num % 10) + '0';
        temp_num /= 10;
    }

}

void make_shm_name(char *buffer, pid_t pid){
    char buff[64];
    const char *prefix = "/lab3_shm_";
    int len = str_len(prefix);
    int i = 0;

    pid_str(buff, pid);
    while (len != 0){
        buffer[i] = prefix[i];
        i++;
        len--;
    }
    len = str_len(buff);
    int buff_i = 0;
    while (len != 0){
        buffer[i] = buff[buff_i];
        i ++;
        buff_i ++;
        len --;
    }
    buffer[i] = '\0';
}

void make_empty_name(char *buffer, pid_t pid){
    char buff[64];
    const char *prefix = "/lab3_empty_";
    int len = str_len(prefix);
    int i = 0;

    pid_str(buff, pid);
    while (len != 0){
        buffer[i] = prefix[i];
        i++;
        len--;
    }
    len = str_len(buff);
    int buff_i = 0;
    while (len != 0){
        buffer[i] = buff[buff_i];
        i ++;
        buff_i ++;
        len --;
    }
    buffer[i] = '\0';
}

void make_full_name(char *buffer, pid_t pid){
    char buff[64];
    const char *prefix = "/lab3_full_";
    int len = str_len(prefix);
    int i = 0;

    pid_str(buff, pid);
    while (len != 0){
        buffer[i] = prefix[i];
        i++;
        len--;
    }
    len = str_len(buff);
    int buff_i = 0;
    while (len != 0){
        buffer[i] = buff[buff_i];
        i ++;
        buff_i ++;
        len --;
    }
    buffer[i] = '\0';
}

int float_str(char buff[64], float num) {
    int i = 0;

    if (num < 0) {
        buff[i++] = '-';
        num = fabs(num);
    }

    long long int res = (long long int)num;
    int int_frac = (int)((num - res) * 100.0f + 0.5f);

    if (int_frac >= 100) {
        res += 1;
        int_frac -= 100;
    }

    if (res == 0) {
        buff[i++] = '0';
    }
    else {
        long long int temp_res = res;
        int len_res = 0;
        while (temp_res > 0) {
            len_res++;
            temp_res /= 10;
        }

        int temp_i = i + len_res - 1;
        while (res > 0) {
            buff[temp_i--] = (res % 10) + '0';
            res /= 10;
        }
        i += len_res;
    }

    buff[i++] = '.';

    buff[i++] = (int_frac / 10) + '0';
    buff[i++] = (int_frac % 10) + '0';

    buff[i++] = '\n';
    buff[i] = '\0';

    return i;
}

int main(){
    char filename[256];
    char shm_name[64];
    char empty_name[64];
    char full_name[64];
    SharedData *data;
    sem_t *empty;
    sem_t *full;
    int result_code = 0;
    const char *msg = "Введите имя файла: ";
    write(1, msg, str_len(msg));
    ssize_t bytes_read = read(0, filename, sizeof(filename) - 1);

    if (bytes_read <= 0){
        return 1;
    }

    filename[bytes_read] = '\0';

    for (int i = 0; i < bytes_read; i ++){
        if (filename[i] == '\n'){
            filename[i] = '\0';
            break;
        }
    }

    make_shm_name(shm_name, getpid());
    int shm_fd = shm_open(shm_name, O_CREAT|O_EXCL|O_RDWR, 0600);
    if (shm_fd == -1){
        const char *msg = "Ошибка создания Shared Memory\n";
        write(2, msg, str_len(msg));
        return 1;
    }
    if (ftruncate(shm_fd, sizeof(SharedData)) == -1){
        const char *msg = "Ошибка работы ftruncate\n";
        write(2, msg, str_len(msg));
        close(shm_fd);
        shm_unlink(shm_name);
        return 1;
    }

    data = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    if (data == MAP_FAILED){
        const char *msg = "Ошибка в работе mmap\n";
        write(2, msg, str_len(msg));
        close(shm_fd);
        shm_unlink(shm_name);
        return 1;
    }

    close(shm_fd);

    make_empty_name(empty_name, getpid());
    make_full_name(full_name, getpid());

    empty = sem_open(empty_name, O_CREAT | O_EXCL, 0600, 1);
    if (empty == SEM_FAILED){
            const char *msg = "Ошибка в создании семафора empty\n";
            write(2, msg, str_len(msg));
            shm_unlink(shm_name);
            munmap(data, sizeof(SharedData));
            return 1;
        }

    full = sem_open(full_name, O_CREAT | O_EXCL, 0600, 0);

    if (full == SEM_FAILED){
        const char *msg = "Ошибка в создании семафора full\n";
        write(2, msg, str_len(msg));
        shm_unlink(shm_name);
        munmap(data, sizeof(SharedData));
        sem_close(empty);
        sem_unlink(empty_name);
        return 1;
    }

    int file_fd = open(filename, O_RDONLY);
    if (file_fd == -1){
        const char *msg = "Не удалось открыть файл\n";
        write(2, msg, str_len(msg));
        shm_unlink(shm_name);
        munmap(data, sizeof(SharedData));
        sem_unlink(empty_name);
        sem_close(empty);
        sem_unlink(full_name);
        sem_close(full);
        return 1;
    }

    pid_t pid = fork();

    if (pid == -1){
        const char *msg = "Ошибка fork\n";
        write(2, msg, str_len(msg));
        shm_unlink(shm_name);
        munmap(data, sizeof(SharedData));
        sem_unlink(empty_name);
        sem_close(empty);
        sem_unlink(full_name);
        sem_close(full);
        close(file_fd);
        return 1;
    }

    else if(pid == 0){
        char *child_argv[] = {"./child", shm_name, empty_name, full_name, NULL};
        char *child_envp[] = {NULL};

        if (dup2(file_fd, 0) == -1){
            const char *msg = "Ошибка dup2\n";
            write(2, msg, str_len(msg));
            _exit(1);
        }
        close(file_fd);

        execve("./child", child_argv, child_envp);
        
        const char *msg = "Не удалось запустить ./child\n";
        write(2, msg, str_len(msg));
        _exit(1);
    }
    
    else{
        close(file_fd);
        char buff[64];

        const char *msg = "\nРезультаты из дочернего процесса\n";
        write(1, msg, str_len(msg));

        while (true){
            sem_wait(full);
            if (data->status == 1){
                int len = float_str(buff, data->sum);
                write(1, buff, len);
                sem_post(empty);
            }
            else if(data->status == 2){
                sem_post(empty);
                break;
            }
            else{
                const char *msg = "Произошла ошибка\n";
                write(2, msg, str_len(msg));
                sem_post(empty);
                result_code = 1;
                break;
            }
        }

        int status;
        waitpid(pid, &status, 0);
        shm_unlink(shm_name);
        munmap(data, sizeof(SharedData));
        sem_unlink(empty_name);
        sem_close(empty);
        sem_unlink(full_name);
        sem_close(full);
    }

    return result_code;
}
