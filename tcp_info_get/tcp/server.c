#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/types.h>
#include <string.h>

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

int startup(int _port,const char* _ip)
{
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock < 0)
	{
		perror("socket");
		exit(1);
	}

	int opt=1;
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons( _port);
	local.sin_addr.s_addr = inet_addr(_ip);
	socklen_t len = sizeof(local);

	if(bind(sock,(struct sockaddr*)&local , len) < 0)
	{
		perror("bind");
		exit(2);
	}

	if(listen(sock, 5) < 0)	//允许连接的最大数量为5
	{
		perror("listen");
		exit(3);
	}

	return sock;
}

int main(int argc,const char* argv[])
{
	if(argc != 3)
	{
		printf("Usage:%s [loacl_ip] [loacl_port]\n",argv[0]);
		return 1;
	}

	int listen_sock = startup(atoi(argv[2]),argv[1]);//初始化

	//用来接收客户端的socket地址结构体
	struct sockaddr_in remote;
	socklen_t len = sizeof(struct sockaddr_in);

	while(1)
	{
		int sock = accept(listen_sock, (struct sockaddr*)&remote, &len);
		if(sock < 0)
		{
			perror("accept");
			continue;
		}

		//每次建立一个连接后fork出一个子进程进行收发数据
		pid_t id = fork();
		if(id > 0)
		{//father
			close(sock);
		//	while(waitpid(-1, NULL, WNOHANG) > 0);
		}
		else if(id == 0)
		{//child
			//printf("get a client, ip:%s, port:%d\n",inet_ntoa(remote.sin_addr),ntohs(remote.sin_port));
			if(fork() > 0)
				exit(0);
			close(listen_sock);
			char buf[1024];
			while(1)
			{
				ssize_t _s = read(sock, buf, sizeof(buf)-1);
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
				        	write(sock, buf, leng_int);
				        }
				        else
				        {
				        	char buf[1024];
				        	struct cwnd_mss tmp;
				        	tmp.cwnd = -1;
				        	tmp.mss = -1;
				        	int leng_int = sizeof(struct cwnd_mss);
				        	memcpy(buf, &tmp, leng_int);
				        	write(sock, buf, leng_int);				        	
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
					printf("client is quit!\n");
					break;
				}
			}
		}
		else
		{
			perror("fork");
			return 2;
		}
	}
	return 0;
}