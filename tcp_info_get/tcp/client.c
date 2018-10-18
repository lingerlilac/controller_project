#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<stdlib.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<time.h>
#include <linux/types.h>
#include <string.h>
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
#define PORT_LINGER 12346

static void get_systime_linger(__be32 *t, __be32 *v)
{
	struct timeval tv;
	memset(&tv, 0, sizeof(struct timeval));
	gettimeofday(&tv);
	*t = tv.tv_sec;
	*v = tv.tv_usec;
}

int main(int argc,const char* argv[])
{
	if(argc != 3)
	{
		printf("Usage:%s [ip] [port]\n",argv[0]);
		return 0;
	}

	//创建一个用来通讯的socket
	int sock = socket(AF_INET,SOCK_STREAM, 0);
	if(sock < 0)
	{
		perror("socket");
		return 1;
	}

	//需要connect的是对端的地址，因此这里定义服务器端的地址结构体
	struct sockaddr_in server;
	struct sockaddr_in local;
	server.sin_family = AF_INET;
	server.sin_port = htons(atoi(argv[2]));
	server.sin_addr.s_addr = inet_addr(argv[1]);
	socklen_t len = sizeof(struct sockaddr_in);

	local.sin_family = AF_INET;
	local.sin_port = htons(PORT_LINGER);
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	// printf("dfadf\n");
	socklen_t len1 = sizeof(local);
	// printf("sdafads\n");
	if(bind(sock,(struct sockaddr*)&local , len1) < 0)
	{
		perror("bind");
		exit(2);
	}

	if(connect(sock, (struct sockaddr*)&server, len) < 0 )
	{
		perror("connect");
		return 2;
	}
	//连接成功进行收数据
	char buf[1024];
	while(1)
	{
		__u32 sec0 = 0;
		__u32 usec0 = 0;
		__u32 sec1 = 0, usec1 = 0;
		__u32 duration = 0;
		struct tcp_ss *tmp = NULL;
		char ip_src[] = "192.168.11.103";
		char ip_dst[] = "192.168.11.102";
		__be16 src_port = 12345;
		__be16 dst_port = 12346;
		int len_ss;
		ssize_t _s = 0;

		len_ss = sizeof(struct tcp_ss) * sizeof(char);
		tmp = (struct tcp_ss *)malloc(len_ss);
		memset(tmp, 0, len_ss);

		strcpy(tmp->ip_src, ip_src);
		strcpy(tmp->ip_dst, ip_dst);
		tmp->src_port = src_port;
		tmp->dst_port = dst_port;
		memcpy(buf, tmp, len_ss);
		write(sock, buf, len_ss);
		sleep(1);
		get_systime_linger(&sec0, &usec0);
		_s = read(sock, buf, sizeof(buf)-1);
		if(_s > 0)
		{
			struct cwnd_mss tmp;
			int leng_int = sizeof(struct cwnd_mss);
			memset(&tmp, 0, leng_int);
			memcpy(&tmp, buf, leng_int);
			printf("window is %d \t %d\n", tmp.cwnd, tmp.mss);
		}
		get_systime_linger(&sec1, &usec1);
		duration = (sec1 - sec0) * 1000000 + (usec1 - usec0);
		printf("%u\n", duration);
	}
	close(sock);

	return 0;
}