/*
    Simple udp server
*/
#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>
#include <linux/types.h>
#define BUFLEN 1024  //Max length of buffer
#define PORT 12345   //The port on which to listen for incoming data
#define _LINE_LENGTH 300

struct tcp_ss
{
    __u32 state;
    char ip_src[18];
    char ip_dst[18];
    __be16 src_port;
    __be16 dst_port;
    __u32 cwnd;
};
struct cwnd_mss
{
	int cwnd;
	int mss;
};
int strlinger ( char *buf, char *sub)
{
    if(!*sub)
        return -1;
    char *bp, *sp;
    int ind = 0;
    while (*buf)
    {
        bp = buf;
        sp = sub;
        do
        {
            if(!*sp)
                return ind;
        } while (*bp++ == *sp++);
        buf++;
        ind++;
    }
    return -1;
}

void die(char *s)
{
    perror(s);
    exit(1);
}
 
int main(void)
{
    struct sockaddr_in si_me, si_other;
     
    int s, i, slen = sizeof(si_other) , _s;
    char buf[BUFLEN];
     
    //create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
     
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
     
    //bind socket to port
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }
     
    //keep listening for data
    while(1)
    {
        // printf("Waiting for data...");
        fflush(stdout);
         
        //try to receive some data, this is a blocking call
        _s = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen);
        if(_s > 0)
        {
            struct tcp_ss *tmp;
            int len_ss = 0;
            FILE *file;
            char line[_LINE_LENGTH];
            char *ret = NULL;
            int ip_src_found = 0;
            int ip_dst_found = 0;

            len_ss = sizeof(struct tcp_ss) * sizeof(char);
            tmp = (struct tcp_ss *)malloc(len_ss);
            memset(tmp, 0, len_ss);
            memcpy(tmp, buf, len_ss);
            printf("%d %d %s %s\n", tmp->src_port, tmp->dst_port, tmp->ip_src, tmp->ip_dst);

            file = popen("ss -4 -t -i", "r");
            if (NULL != file)
            {
                int found_cwnd = 0;
                int cwnd = 0;
                int mss = 0;
                while (fgets(line, _LINE_LENGTH, file) != NULL)
                {
                    int index1 = 0;
                    int index2 = 0;
                    // printf("%s\n", line);
                    index1 = strlinger(line, tmp->ip_src);
                    index2 = strlinger(line, tmp->ip_dst);
                    if ((index1 > 0) && (index2 > 0))
                    {
                        char *ret = NULL;
                        int index3 = 0;
                        int index4 = 0;
                        char * str = NULL;
                        int length_tmp = 0;
                        int port_src_found = 0;
                        int port_dst_found = 0;
                        
                        // printf("%s\n", line);
                        index3 = strlinger(line + index1, ":");
                        index3 = index3 + 1 + index1;
                        index4 = strlinger(line + index3, " ") + index3;
                        length_tmp = index4 - index3;
                        length_tmp = length_tmp * sizeof(char);
                        if (length_tmp > 0)
                        {
                            str = (char *)malloc(length_tmp);
                            memcpy(str, line + index3, length_tmp);
                            port_src_found = atoi(str);
                            free(str);
                            str = NULL;
                        }
                        else
                            continue;
                        // printf("%d %d\n", port_src_found, tmp->ip_src);
                        if(port_src_found != tmp->src_port)
                            continue;
                        index3 = strlinger(line + index4, ":") + index4;
                        index3 += 1;
                        index4 = strlinger(line + index3, " ") + index3;
                        length_tmp = index4 - index3;
                        length_tmp = length_tmp * sizeof(char);
                        // printf("%d %d %d\n", index3, index4, length_tmp);
                        if (length_tmp > 0)
                        {
                            str = (char *)malloc(length_tmp);
                            memcpy(str, line + index3, length_tmp);
                            // printf("111 %s\n", str);
                            if (strcmp("http", str) == 0)
                                port_dst_found = 80;
                            else if(strcmp("https", str) == 0)
                                port_dst_found = 8089;
                            else
                                port_dst_found = atoi(str);
                            free(str);
                            str = NULL;
                        }
                        else
                            continue;
                        // printf("%d\n", port_dst_found);
                        if(port_dst_found != tmp->dst_port)
                            continue;
                        ret = fgets(line, _LINE_LENGTH, file);
                        // printf("%s\n", line);
                        if(ret == NULL)
                            break;
                        else
                        {
                            index3 = strlinger(line, "cwnd");
                            index3 += 5;
                            index4 = strlinger(line + index3, " ") + index3;
                            length_tmp = index4 - index3;
                            length_tmp = length_tmp * sizeof(char);
                            // printf("%d %d %d\n", index3, index4, length_tmp);
                            if (length_tmp > 0)
                            {
                                str = (char *)malloc(length_tmp);
                                memcpy(str, line + index3, length_tmp);
                                cwnd = atoi(str);
                                free(str);
                                str = NULL;
                                found_cwnd = 1;
                            }
                            else
                                break;
                            index3 = strlinger(line, "mss");
                            index3 += 4;
                            index4 = strlinger(line + index3, " ") + index3;
                            length_tmp = index4 - index3;
                            length_tmp = length_tmp * sizeof(char);
                            // printf("%d %d %d\n", index3, index4, length_tmp);
                            if (length_tmp > 0)
                            {
                                str = (char *)malloc(length_tmp);
                                memcpy(str, line + index3, length_tmp);
                                mss = atoi(str);
                                free(str);
                                str = NULL;
                            }
                            else
                                break;
                        }
                    }
                    else
                    {
                        continue;
                    }

                }
                if(found_cwnd == 1)
                {
                    char buf[1024];
                    struct cwnd_mss tmp;
                    tmp.cwnd = cwnd;
                    tmp.mss = mss;
                    int leng_int = sizeof(struct cwnd_mss);
                    memcpy(buf, &tmp, leng_int);
                    sendto(s, buf, leng_int, 0, (struct sockaddr*) &si_other, slen);
                }
                else
                {
                    char buf[1024];
                    struct cwnd_mss tmp;
                    tmp.cwnd = -1;
                    tmp.mss = -1;
                    int leng_int = sizeof(struct cwnd_mss);
                    memcpy(buf, &tmp, leng_int);
                    sendto(s, buf, leng_int, 0, (struct sockaddr*) &si_other, slen);                         
                }
            }
            else
                continue;
            free(ret);
            ret = NULL; 
            pclose(file);

            free(tmp);
            tmp = NULL;
        }
        else
        {
            sleep(1);
            continue;
        }
         
    }
 
    close(s);
    return 0;
}