#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/types.h>
#include <sys/time.h>
#define _LINE_LENGTH 300

enum states
{
    SYN-SENT,ESTAB, LAST-ACK, CLOSE-WAIT
};

struct tcp_ss
{
    __u32 state;
    char ip_src[18];
    char ip_dst[18];
    __be16 src_port;
    __be16 dst_port;
    __u32 cwnd;
};
int main(void) 
{
    FILE *file;
    char line[_LINE_LENGTH];
    char *ret = NULL;
    struct timeval tv, tv1;
    int64_t sec = 0;
    int64_t usec = 0;
    file = popen("ss -4 -t -i", "r");
    if (NULL != file)
    {
        while (fgets(line, _LINE_LENGTH, file) != NULL)
        {
            printf("%s\n", line);
        }
    }
    else
    {
        return 1;
    }
    free(ret);
    ret = NULL; 
    pclose(file);
 return 0;
}

