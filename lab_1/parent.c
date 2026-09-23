#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>

int str_len(const char *str){
    int len = 0;
    while (str[len] != '\0'){
        len ++;
    }
    return len;
}

int main(){
    char filename[256];
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

    int pipe_fds[2];

    if (pipe(pipe_fds) == -1){
        const char *msg = "Не удалось создать pipe\n";
        write(2, msg, str_len(msg));
        return 1;
    }

    pid_t pid = fork();

    if (pid == -1){
        const char *msg = "Ошибка fork\n";
        write(2, msg, str_len(msg));
        return 1;
    }

    else if(pid == 0){
        int file_fd = open(filename, O_RDONLY);
        char *child_argv[] = {"./child", NULL};
        char *child_envp[] = {NULL};
        if (file_fd == -1){
            const char *msg = "Не удалось открыть файл\n";
            write(2, msg, str_len(msg));
            return 1;
        }

        dup2(file_fd, 0);
        close(file_fd);

        close(pipe_fds[0]);

        dup2(pipe_fds[1], 1);
        close(pipe_fds[1]);

        execve("./child", child_argv, child_envp);
        
        const char *msg = "Не удалось запустить ./child\n";
        write(2, msg, str_len(msg));
        return 1;
    }
    
    else{
        close(pipe_fds[1]);

        char buff[128];
        ssize_t n;

        const char *msg = "\nРезультаты из дочернего процесса\n";
        write(1, msg, str_len(msg));

        while ((n = read(pipe_fds[0], buff, sizeof(buff))) > 0){
            write(1, buff, n);
        }

        close(pipe_fds[0]);

        int status;
        waitpid(pid, &status, 0);
    }

    return 0;
}
