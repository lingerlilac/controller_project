/*
    Simple udp server
*/
#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>
#include<linux/types.h>
#include<errno.h>
#include <unistd.h>
 
#define BUFLEN_LOCAL 512  //Max length of buffer
#define PORT_LOCAL 7777   //The port on which to listen for incoming data
#define BUFLEN 512
void die(char *s)
{
    perror(s);
    exit(1);
}
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

struct cwnd_mss run_linger(struct tcp_ss *str)
{
    struct sockaddr_in si_other;
    int s, slen=sizeof(si_other);
    char buf[BUFLEN];
    char message[BUFLEN];
 
    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
   
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(8888);
    printf("ip_src is %s\n", str->ip_src);
    if (inet_aton(str->ip_src , &si_other.sin_addr) == 0) 
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
 

        
    int length = sizeof(struct cwnd_mss);
    struct cwnd_mss ret;

    memcpy(message, str, sizeof(struct tcp_ss));
    //send the message
    if (sendto(s, message, sizeof(struct tcp_ss) , 0 , (struct sockaddr *) &si_other, slen)==-1)
    {
        die("sendto()");
    }
     
    //receive a reply and print it
    //clear the buffer by filling null, it might have previously received data
    memset(buf,'\0', BUFLEN);
    //try to receive some data, this is a blocking call
    if (recvfrom(s, buf, length, 0, (struct sockaddr *) &si_other, (socklen_t*)&slen) == -1)
    {
       printf("%d\n", (int)errno);
    }

    memset(&ret, 0, length);
    memcpy(&ret, buf, length);
    printf("%d %d\n", ret.cwnd, ret.mss);
    
    close(s);
    return ret;
}


int main(void)
{
    struct sockaddr_in si_me, si_other;
     
    int s, slen = sizeof(si_other) , recv_len;
    char buf[BUFLEN];
     
    //create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
     
    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
     
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT_LOCAL);
    si_me.sin_addr.s_addr = inet_addr("127.0.0.1");
     
    //bind socket to port
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }
     
    //keep listening for data
    while(1)
    {
        struct cwnd_mss window_mss;
        struct tcp_ss tcp_info;
        int length_ss = sizeof(struct tcp_ss);
        char command[200];
        int window = 0;
        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, (socklen_t*)&slen)) == -1)
        {
            usleep(5000);
            continue;
        }
        memcpy(&tcp_info, buf, length_ss);
        window_mss = run_linger(&tcp_info);
        // printf("%d %d\n", window_mss.cwnd, window_mss.mss);
        window = window_mss.cwnd * window_mss.mss;
        memset(command, 0, 200);
        sprintf(command, "ovs-ofctl add-flow br0 -O openflow13 tcp,nw_src=%s,nw_dst=%s,tcp_src=%d,tcp_dst=%d,actions=mod_tp_dst:%d",
          tcp_info.ip_src, tcp_info.ip_dst, tcp_info.src_port, tcp_info.dst_port, window);
        printf("%s\n", command);
    }
 
    close(s);
    return 0;
}