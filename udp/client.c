/*
    Simple udp client
*/
#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //exit(0);
#include<arpa/inet.h>
#include<sys/socket.h>
#include<linux/types.h>
#include <fcntl.h>
#include <errno.h>
#define SERVER "192.168.11.101"
#define BUFLEN 512  //Max length of buffer
#define PORT 12345   //The port on which to send data
 

 
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
void die(char *s)
{
    perror(s);
    exit(1);
}

void run_linger(struct tcp_ss *str)
{
    struct sockaddr_in si_other;
    int s, i, slen=sizeof(si_other);
    char buf[BUFLEN];
    char message[BUFLEN];
 
    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
    struct timeval timeout;
    timeout.tv_sec = 3;
    timeout.tv_usec = 10000;
    if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1) {
      printf("setsockopt failed:");
    }
    
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
     
    if (inet_aton(SERVER , &si_other.sin_addr) == 0) 
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
    if (recvfrom(s, buf, length, 0, (struct sockaddr *) &si_other, &slen) == -1)
    {
       printf("%d\n", (int)errno);
    }

    memset(&ret, 0, length);
    memcpy(&ret, buf, length);
    printf("%d %d\n", ret.cwnd, ret.mss);
    sleep(1);

 
    close(s);
}
int main(void)
{
    struct tcp_ss str;
    strcpy(str.ip_dst, "192.168.11.107");
    strcpy(str.ip_src, "192.168.11.101");

    str.dst_port = 42388;
    str.src_port = 12307;
    while(1)
    {
        run_linger(&str);
    }
    return 0;
}