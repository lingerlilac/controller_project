#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <linux/types.h>
#include <sys/time.h>
#define _LINE_LENGTH 300


struct flow_linger
{
    char ip_src[20];
    char ip_dst[20];
    int  port_src;
    int  port_dst;
    int winsize;
};
void generate_command(struct flow_linger *ptr, char *ret, int size)
{
    char buff[200];

    memset(buff, 0, 200);
    sprintf(buff, "ovs-ofctl add-flow br0 -O openflow13 tcp,nw_dst=%s,nw_dst=%s,tcp_src=%d,tcp_dst=%d,actions=set_rwnd:%d", 
        ptr->ip_src, ptr->ip_dst, ptr->port_src, ptr->port_dst, ptr->winsize);
    // printf("%s\n", buff);
    strncpy(ret, buff, size);
}

int main(void) 
{
    FILE *file;
    char line[_LINE_LENGTH];
    char *ret = NULL;
    struct timeval tv, tv1;
    int64_t sec = 0;
    int64_t usec = 0;
    file = popen("ls", "r");
    if (NULL != file)
    {
        while (fgets(line, _LINE_LENGTH, file) != NULL)
        {
            printf("line=%s\n", line);
        }
    }
    else
    {
        return 1;
    }
    struct flow_linger flow1;
    strcpy(flow1.ip_src, "192.168.1.120");
    strcpy(flow1.ip_dst, "202.118.228.111");
    flow1.port_src = 80;
    flow1.port_dst = 210;
    flow1.winsize = 100;
    ret = malloc(sizeof(char) * 200);
    generate_command(&flow1, ret, 200);
    printf("ret is %s\n", ret);

    gettimeofday(&tv, NULL);
    file = popen(ret, "r");
    // if (NULL != file)
    // {
        // while (fgets(line, _LINE_LENGTH, file) != NULL)
        // {
            // printf("line=%s\n", line);
        // }
    // }
    // else
    // {
    //     printf("sxdfsd\n");
    //     return 1;
    // }

    gettimeofday(&tv1, NULL);
    sec = tv1.tv_sec - tv.tv_sec;
    usec = tv1.tv_usec - tv.tv_usec;
    if(usec < 0)
    {
        usec = usec + 1000000;
    }
    printf("sec is %lld, usec is %lld\n", sec, usec);
    free(ret);
    ret = NULL; 
    pclose(file);
 return 0;
}

