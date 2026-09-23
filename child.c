#include <math.h>
#include <unistd.h>

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

int float_str(char buff[64], float num) {
    int i = 0;

    if (num < 0) {
        buff[i++] = '-';
        num = fabs(num);
    }

    int res = (int)num;
    int int_frac = (int)((num - res) * 100.0f + 0.5f);

    if (int_frac >= 100) {
        res += 1;
        int_frac -= 100;
    }

    if (res == 0) {
        buff[i++] = '0';
    }
    else {
        int temp_res = res;
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
    char num_buff[64], ch;
    int num_len = 0, has_num = 0;
    float line_sum = 0.0f;
    while (read(0, &ch, 1) > 0){
        if (ch >= '0' && ch <= '9' || ch == '.' || (ch == '-' && num_len == 0)){
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
                    char out_buff[64];
                    int out_len = float_str(out_buff, line_sum);
                    write(1, out_buff, out_len);
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
        char out_buff[64];
        int out_len = float_str(out_buff, line_sum);
        write(1, out_buff, out_len);
    }

    return 0;
}
