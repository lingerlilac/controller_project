#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <linux/netlink.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

#define NETLINK_USER 22
#define USER_MSG    (NETLINK_USER + 1)
#define MSG_LEN 150
#define ITEM_AMOUNT 100
#define MAX_PLOAD 150


struct tcp_flow
{
	/**
	 * used to label a tcp flow.
	 */
	__be32 ip_src;
	__be32 ip_dst;
	__be16 sourceaddr;
	__be16 destination;
	__u16 winsize;
	__u32 sec;
	__u32 usec;
};

struct apstates
{
    __u32 sec;
    __u32 usec;
    __u32 drop_count;
    __u64 time;
    __u64 time_busy;
    __u64 time_ext_busy;
    __u64 time_rx;
    __u64 time_tx;
    __u64 time_scan;
    __s8 noise;
    __u8 stations;
    __u32 packets;
    __u64 bytes;
    __u32 qlen;
    __u32 backlog;
    __u32 drops;
    __u32 requeues;
    __u32 overlimits;
    __u16 pad;   
};

// /**
//  * apstates_float is used to represent the real value of
//  * ap states. Beacause float is not supported in kernel.
//  */
// struct apstates_float
// {
// 	float drop_count;
// 	float time_busy;
// 	float time_ext_busy;
// 	float time_rx;
// 	float time_tx;
// 	float time_scan;
// 	float noise;
// 	float packets;
// 	float bytes;
// 	float qlen;
// 	float backlog;
// 	float drops;
// 	float requeues;
// 	float overlimits;
// };
enum sta_l
{
	drop_count, time_busy, time_ext_busy, time_rx, time_tx, time_scan, noise, packets, bytes, qlen, backlog, drops, requeues, overlimits
};
struct _my_msg
{
	/**
	 * used to transmit nl message.
	 */
	struct nlmsghdr hdr;
	int8_t  data[MSG_LEN];
};

struct rawdata
{
	int father;
	int itself;
	char *name;
	float value;
	int class;
	struct rawdata * next;
};

struct treenode
{
	/**
	 * used to represent a tree node of the decision tree.
	 */
	int itself;
	char *name;
	float value;
	int class;
	struct treenode * lchild;
	struct treenode * rchild;
	struct treenode * parent;
};
struct item_value
{
	/**
	 * used to represent this: noise: -91; q_len:18;...
	 */
	char *name;
	float value;
};

typedef enum windowsize
{
	/**
	 * used to set cwnd.
	 */
	keep,
	half,
	quater,
	one
} windsz;
void free_rawdata(struct rawdata *);
void print_rawdata(struct rawdata*);
struct treenode * create_tree(struct rawdata *, struct treenode *);
void destroy_tree(struct treenode *);
void print_tree(struct treenode *);
struct rawdata * find_rchild(struct rawdata *, int );
struct rawdata * find_lchild(struct rawdata *, int );
char ** get_parameters(struct rawdata *);
int TreeDepth(struct treenode *);
void destroy_parameters(char **);
struct treenode* bst_search(struct treenode*, struct item_value value[], int length);
int create_items(struct item_value items_list[], char *buffer);
void destroy_item_list(struct item_value items_list[]);
int init_linger(char *, char *);
int init_linger_m(char *);
void destroy_everything();

void reversebytes_uint32t(__u32 *value);
void reversebytes_uint16t(__u16 *value);
void reversebytes_uint64t(__u64 *value);
void set_cwnd(int cwnd, struct tcp_flow* tflow, char *ret, int size);
void assign_string(struct item_value* d, char *s);
int main_linger(void);

struct treenode *tnode = NULL;
struct item_value items_list[ITEM_AMOUNT];
char **parameters;
struct rawdata *node = NULL;
int skfd = -1;

/**
 * 0010 0100 -> 0100 0010
 * @param value 0010 0100
 */
void reversebytes_uint32t(__u32 *value)
{
	*value = (*value & 0x000000FFU) << 24 | (*value & 0x0000FF00U) << 8 |
	         (*value & 0x00FF0000U) >> 8 | (*value & 0xFF000000U) >> 24;
}
void reversebytes_uint16t(__u16 *value)
{
	*value = (*value & 0x00FF) << 8 | (*value & 0xFF00) >> 8;
}
void reversebytes_uint64t(__u64 *value)
{
	__u64 low = (*value & 0x00000000FFFFFFFF);
	__u64 high = (*value & 0xFFFFFFFF00000000) >> 32;
	__u32 low_32 = (__u32) low;
	__u32 high_32 = (__u32) high;
	reversebytes_uint32t(&low_32);
	reversebytes_uint32t(&high_32);
	low = ((__u64) low_32) << 32;
	high = ((__u64) high_32);
	*value = low | high;
}
/**
 * [set_cwnd get a flow structure tflow, and return ofctl command.]
 * @param  cwnd  cwnd value to set
 * @param  tflow flow structure
 */
void set_cwnd(int class1, struct tcp_flow* tflow, char *ret, int size)
{
	int i = 0;
	char ip_src[20];
	char ip_dst[20];
	char buff[200];
	int cwnd = 0;
	unsigned char *ptr_uc;
	ptr_uc = (unsigned char *) & (tflow->ip_src);
	sprintf(ip_src, "%u.%u.%u.%u", ptr_uc[0], ptr_uc[1], ptr_uc[2], ptr_uc[3]);
	ptr_uc = (unsigned char *) & (tflow->ip_dst);
	sprintf(ip_dst, "%u.%u.%u.%u", ptr_uc[0], ptr_uc[1], ptr_uc[2], ptr_uc[3]);
	reversebytes_uint16t(&(tflow->winsize));
	reversebytes_uint16t(&(tflow->sourceaddr));
	reversebytes_uint16t(&(tflow->destination));

	switch (class1)
	{
	case 1:
		cwnd = tflow->winsize;
		break;
	case 2:
		cwnd = tflow->winsize * 3 / 4;
		break;
	case 3:
		cwnd = tflow->winsize / 2;
		break;
	case 4:
		cwnd = 1;
	}
	// printf("%d\n", cwnd);
	// printf("ip_src %s ip_dst %s cwnd %d\n", ip_src, ip_dst, tflow->winsize);


	memset(buff, 0, 200);
	sprintf(buff, "ovs-ofctl add-flow br0 -O openflow13 tcp,nw_dst=%s,nw_dst=%s,tcp_src=%d,tcp_dst=%d,actions=mod_tp_dst:%d",
	        ip_src, ip_dst, tflow->sourceaddr, tflow->destination, cwnd);
	// printf("%s\n", buff);
	strncpy(ret, buff, size);
}

char* get_openflow_input(void)
{
	char *str = NULL;
	char inputtxt[] = "0,0,requeues_3,5158.975,0|0,1,busy_time_1,808.831,2|1,2,leaf,0,2|1,3,allpackets_1,150.5,2|3,4,leaf,0,2|3,5,requeues_1,678.932,2|5,6,drops_1,3211.45,2|6,7,leaf,0,2|6,8,leaf,0,2|5,9,leaf,0,2|0,10,busy_time_1,808.542,0|10,11,retrans_4,14.5,0|11,12,allpackets_3,182.5,0|12,13,retrans_5,14.5,0|13,14,retrans_3,0.5,0|14,15,receive_time_1,122.551,0|15,16,leaf,0,0|15,17,leaf,0,1|14,18,transmit_time_4,32.177,0|18,19,leaf,0,0|18,20,leaf,0,0|13,21,leaf,0,0|12,22,allpackets_1,202.5,0|22,23,rbytes_3,257018.672,0|23,24,leaf,0,0|23,25,allpackets_1,170.5,0|25,26,leaf,0,0|25,27,leaf,0,0|22,28,leaf,0,0|11,29,leaf,0,0|10,30,leaf,0,2";
	int len = 0;
	len = strlen(inputtxt) + 1;
	printf("len is %d\n", len);
	str = (char *)malloc(sizeof(char) * len);
	memset(str, 0, len);
	// memcpy(str, inputtxt, len);
	strcpy(str, inputtxt);
	printf("xxx\n");
	// return inputtxt;
	return str;
}

int main_linger()
{
	char *data = "flows";
	char *data1 = "states";
	struct sockaddr_nl  local, dest_addr;

	struct nlmsghdr *nlh = NULL;
	struct _my_msg info;
	int ret;
	struct tcp_flow control;
	struct apstates apst;
	char ret_none[10];
	int i;

	// char item[] = "requeues_3,5000;busy_time_1,809;allpackets_1,160;requeues_1,670;drops_1,3220;retrans_4,0;allpackets_3,0;retrans_5,0;retrans_3,0;receive_time_1,0;transmit_time_4,0;rbytes_3,0";
	struct treenode* search_results = NULL;

	// init_linger(inputtxt, item);
	
	if(skfd < 0)
	{
		skfd = socket(AF_NETLINK, SOCK_RAW, USER_MSG);
		if (skfd == -1)
		{
			printf("create socket error...%s\n", strerror(errno));
			return -1;
		}

		memset(&local, 0, sizeof(local));
		local.nl_family = AF_NETLINK;
		local.nl_pid = 50;
		local.nl_groups = 0;
		if (bind(skfd, (struct sockaddr *)&local, sizeof(local)) != 0)
		{
			printf("bind() error\n");
			close(skfd);
			return -1;
		}
	}
	if(skfd < 0)
		return -1;
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0; // to kernel
	dest_addr.nl_groups = 0;

	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PLOAD));
	memset(nlh, 0, sizeof(struct nlmsghdr));
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PLOAD);
	nlh->nlmsg_flags = 0;
	nlh->nlmsg_type = 0;
	nlh->nlmsg_seq = 0;
	nlh->nlmsg_pid = local.nl_pid; //self port



	memcpy(NLMSG_DATA(nlh), data1, strlen(data1));
	ret = sendto(skfd, nlh, nlh->nlmsg_len, 0, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_nl));

	if (!ret)
	{
		perror("sendto error1\n");
		close(skfd);
		exit(-1);
	}
	memset(&info, 0, sizeof(info));
	ret = recvfrom(skfd, &info, sizeof(struct _my_msg), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
	memset(ret_none, 0, 10);
	memcpy(ret_none, info.data, 8);
	if (!strcmp(ret_none, "nonedata"))
	{
		printf("333%s\n", ret_none);
	}
	else
	{
		float times = 0.000001;
		struct item_value fstates[14];
		int i = 0;
		char len[14][20] = {"drop_count", "time_busy", "time_ext_busy", "time_rx", "time_tx", "time_scan", "noise", "packets", "bytes", "qlen", "backlog", "drops", "requeues", "overlimits"};
		// memset(&fstates, 0, sizeof(struct apstates_float));
		memset(&apst, 0, sizeof(struct apstates));
		memcpy(&apst, info.data, sizeof(struct apstates));

		// reversebytes_uint32t(&(apst.sec));
		// reversebytes_uint32t(&(apst.usec));
		// reversebytes_uint64t(&(apst.time_busy));
		// reversebytes_uint64t(&(apst.time_ext_busy));
		printf("%02x %02x\n", info.data, apst.sec);
		printf("W %u %u %u %llu %llu %llu %d\n", apst.sec, apst.usec, apst.drop_count, apst.time, apst.time_busy, apst.time_ext_busy, sizeof(struct apstates));
		// printf("%d\n", drop_count);
		fstates[drop_count].value = ((float)apst.drop_count) * times;
		assign_string(&fstates[drop_count], "drop_count");
		// printf("%02x %f\n", &(fstates[drop_count]), fstates[drop_count].value);
		fstates[time_busy].value = ((float)apst.time_busy) * times;
		assign_string(&fstates[time_busy] , "time_busy");
		fstates[time_ext_busy].value = ((float)apst.time_ext_busy) * times;
		assign_string(&fstates[time_ext_busy] , "time_ext_busy");
		fstates[time_rx].value = ((float)apst.time_rx) * times;
		assign_string(&fstates[time_rx] , "time_rx");
		fstates[time_tx].value = ((float)apst.time_tx) * times;
		assign_string(&fstates[time_tx], "time_tx");
		fstates[time_scan].value = ((float)apst.time_scan) * times;
		assign_string(&fstates[time_scan] , "time_scan");
		fstates[noise].value = (float)apst.noise;
		assign_string(&fstates[noise] , "noise");
		fstates[packets].value = ((float)apst.packets) * times;
		assign_string(&fstates[packets] , "packets");
		fstates[bytes].value = ((float)apst.bytes * times);
		assign_string(&fstates[bytes] , "bytes");
		fstates[qlen].value = ((float)apst.qlen);
		assign_string(&fstates[qlen] , "qlen");
		fstates[backlog].value = ((float)apst.backlog);
		assign_string(&fstates[backlog] , "backlog");
		fstates[drops].value = ((float)apst.drops);
		assign_string(&fstates[drops] , "drops");
		fstates[requeues].value = ((float)requeues * times);
		assign_string(&fstates[requeues] , "requeues");
		fstates[overlimits].value = ((float)overlimits * times);
		assign_string(&fstates[overlimits] , "overlimits");
		// printf("def %d\n", overlimits);
		for (i = 0; i < 14; i++)
		{
			printf("%s: %f\t", fstates[i].name, fstates[i].value);
			if(!(i % 4))
				printf("\n");
		}
		printf("\n");
		search_results = bst_search(tnode, fstates, 14);
		if (search_results)
		{
			printf("%s\n", data);
			memcpy(NLMSG_DATA(nlh), data, strlen(data));
			ret = sendto(skfd, nlh, nlh->nlmsg_len, 0, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_nl));

			if (!ret)
			{
				perror("sendto error1\n");
				close(skfd);
				exit(-1);
			}
			printf("wait kernel msg!\n");
			memset(&info, 0, sizeof(info));
			ret = recvfrom(skfd, &info, sizeof(struct _my_msg), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
			memcpy(&control, info.data, sizeof(struct tcp_flow));

			if (!ret)
			{
				perror("recv form kernel error\n");
				close(skfd);
				exit(-1);
			}

			for (i = 0; i < control.ip_src; i++)
			{
				char *ret1 = NULL;
				struct tcp_flow tmp;
				float weight = 0;

				memset(&tmp, 0, sizeof(struct tcp_flow));

				ret = recvfrom(skfd, &info, sizeof(struct _my_msg), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
				// printf("xxxx\n");
				memcpy(&tmp, info.data, sizeof(struct tcp_flow));
				// printf("winsize is : %u %d  %u %u %d\n", tmp.ip_src, tmp.winsize, tmp.sourceaddr, tmp.destination, ret);
				ret1 = malloc(sizeof(char) * 200);
				set_cwnd(search_results->class, &tmp, ret1, 200);
				printf("ret is %s\n", ret1);
			}
		}
		// printf("ad\n");
		else
			printf("not found111\n");
		destroy_item_list(fstates);
	}

	if (!ret)
	{
		perror("recv form kernel error\n");
		close(skfd);
		exit(-1);
	}

	// search_results = bst_search(tnode, items_list, ITEM_AMOUNT);
	if (search_results)
	{
		printf("found\n");
	}
	// printf("ad\n");
	else
		printf("not found\n");
	// get_flows();

	// printf("msg receive from kernel:%s\n", info.data);
	// close(skfd);

	free((void *)nlh);
	destroy_everything();
	return 0;

}

void assign_string(struct item_value* d, char *s)
{
	int len = strlen(s);
	d->name = (char *)malloc(sizeof(char) * len);
	memset(d->name, 0, len);
	memcpy(d->name, s, len);
	// strcpy(d, s);
	// printf("%s %d %02x\n", d->name, len, &d);
}

/**
 * clear structures
 */
void destroy_everything()
{
	destroy_item_list(items_list);
	destroy_parameters(parameters);
	destroy_tree(tnode);
	free_rawdata(node);
	if(node)
		node = NULL;
	if(tnode)
		tnode = NULL;
	if(parameters)
		parameters = NULL;
	// if(items_list)
	// 	items_list = NULL;
}
int init_linger_m(char * inputtxt)
{
	// char inputtxt[] = "0,0,requeues_3,4583.405,0|0,1,allpackets_1,145.5,2|1,2,receive_time_5,406.741,2|2,3,active_time_4,925.101,2|3,4,leaf,0,2|3,5,drops_4,23788.1,2|5,6,drops_3,109624.0,2|6,7,receive_time_5,148.055,2|7,8,leaf,0,0|7,9,busy_time_4,750.129,2|9,10,leaf,0,0|9,11,leaf,0,2|6,12,leaf,0,0|5,13,leaf,0,2|2,14,leaf,0,0|1,15,packets_2,0.58,2|15,16,leaf,0,1|15,17,requeues_1,669.086,2|17,18,drops_1,7715.755,2|18,19,active_time_4,931.829,2|19,20,requeues_3,1174.415,2|20,21,backlogs_2,4843.15,2|21,22,allpackets_4,11.5,2|22,23,leaf,0,2|22,24,leaf,0,2|21,25,leaf,0,2|20,26,allpackets_1,319.0,2|26,27,leaf,0,2|26,28,leaf,0,2|19,29,noise_1,-89.5,2|29,30,retrans_3,50.5,2|30,31,leaf,0,2|30,32,allpackets_5,147.0,2|32,33,leaf,0,2|32,34,leaf,0,2|29,35,leaf,0,2|18,36,leaf,0,2|17,37,requeues_3,4473.6,2|37,38,active_time_5,922.379,2|38,39,leaf,0,2|38,40,leaf,0,2|37,41,leaf,0,2|0,42,busy_time_1,811.901,0|42,43,retrans_4,16.5,0|43,44,transmit_time_4,62.532,0|44,45,allpackets_1,168.5,0|45,46,busy_time_2,708.081,0|46,47,retrans_5,15.5,0|47,48,rbytes_2,165222.906,0|48,49,allpackets_4,216.0,0|49,50,busy_time_1,399.099,0|50,51,leaf,0,0|50,52,retrans_3,0.5,0|52,53,drops_1,13282.95,0|53,54,rbytes_4,200832.266,0|54,55,requeues_1,1269.48,0|55,56,leaf,0,0|55,57,rbytes_5,113537.703,1|57,58,transmit_time_1,53.347,0|58,59,leaf,0,1|58,60,leaf,0,0|57,61,leaf,0,1|54,62,leaf,0,0|53,63,leaf,0,0|52,64,packets_3,76.374,0|64,65,leaf,0,0|64,66,allpackets_5,154.5,0|66,67,retrans_3,9.5,0|67,68,packets_4,150.492,0|68,69,leaf,0,0|68,70,leaf,0,0|67,71,leaf,0,0|66,72,leaf,0,1|49,73,leaf,0,0|48,74,leaf,0,0|47,75,leaf,0,0|46,76,drops_5,95043.797,0|76,77,leaf,0,0|76,78,requeues_1,1332.805,0|78,79,transmit_time_1,31.125,0|79,80,leaf,0,0|79,81,allpackets_1,139.5,0|81,82,active_time_5,921.647,0|82,83,leaf,0,0|82,84,active_time_4,934.706,0|84,85,leaf,0,1|84,86,requeues_4,1519.65,0|86,87,leaf,0,0|86,88,leaf,0,0|81,89,allpackets_2,52.5,0|89,90,leaf,0,0|89,91,packets_2,102.88,0|91,92,packets_3,74.465,0|92,93,leaf,0,0|92,94,busy_time_1,796.27,0|94,95,busy_time_2,803.941,0|95,96,active_time_4,936.588,0|96,97,leaf,0,0|96,98,rbytes_1,181646.922,0|98,99,leaf,0,0|98,100,leaf,0,0|95,101,leaf,0,0|94,102,leaf,0,1|91,103,leaf,0,0|78,104,leaf,0,0|45,105,receive_time_5,269.211,0|105,106,retrans_5,15.5,0|106,107,rbytes_4,126370.078,0|107,108,leaf,0,1|107,109,drops_4,25934.4,0|109,110,leaf,0,0|109,111,packets_1,136.702,0|111,112,allpackets_3,155.5,0|112,113,active_time_3,933.032,0|113,114,leaf,0,1|113,115,leaf,0,0|112,116,leaf,0,0|111,117,rbytes_4,150856.359,0|117,118,leaf,0,0|117,119,allpackets_2,22.5,0|119,120,allpackets_2,15.5,1|120,121,leaf,0,0|120,122,leaf,0,1|119,123,allpackets_3,59.5,0|123,124,leaf,0,0|123,125,allpackets_4,214.5,0|125,126,transmit_time_3,23.069,0|126,127,leaf,0,0|126,128,active_time_4,936.779,1|128,129,leaf,0,0|128,130,transmit_time_4,29.219,1|130,131,allpackets_3,74.5,1|131,132,leaf,0,1|131,133,leaf,0,1|130,134,transmit_time_5,57.634,1|134,135,leaf,0,0|134,136,leaf,0,1|125,137,leaf,0,0|106,138,leaf,0,1|105,139,packets_1,144.73,0|139,140,allpackets_1,204.5,0|140,141,retrans_2,16.5,0|141,142,transmit_time_3,203.117,0|142,143,backlogs_3,4739.775,0|143,144,retrans_3,80.5,0|144,145,packets_3,326.613,0|145,146,packets_4,202.219,0|146,147,leaf,0,0|146,148,leaf,0,0|145,149,leaf,0,0|144,150,leaf,0,0|143,151,leaf,0,0|142,152,leaf,0,1|141,153,leaf,0,2|140,154,leaf,0,1|139,155,rbytes_3,519237.875,0|155,156,allpackets_1,191.5,0|156,157,requeues_1,1294.575,0|157,158,active_time_4,931.829,0|158,159,leaf,0,0|158,160,requeues_4,1631.26,0|160,161,receive_time_4,147.458,0|161,162,leaf,0,0|161,163,leaf,0,0|160,164,leaf,0,0|157,165,leaf,0,0|156,166,busy_time_3,801.711,0|166,167,packets_4,158.564,0|167,168,active_time_4,934.376,0|168,169,leaf,0,0|168,170,leaf,0,0|167,171,leaf,0,0|166,172,rbytes_4,177486.375,0|172,173,leaf,0,0|172,174,leaf,0,0|155,175,leaf,0,2|44,176,receive_time_1,110.609,0|176,177,leaf,0,0|176,178,busy_time_3,796.891,1|178,179,leaf,0,1|178,180,requeues_4,1546.595,0|180,181,leaf,0,0|180,182,leaf,0,1|43,183,allpackets_1,194.5,0|183,184,allpackets_5,283.5,0|184,185,packets_4,78.635,0|185,186,leaf,0,2|185,187,requeues_3,5247.93,1|187,188,leaf,0,1|187,189,active_time_4,937.092,0|189,190,leaf,0,0|189,191,leaf,0,1|184,192,leaf,0,0|183,193,leaf,0,2|42,194,requeues_3,5170.32,2|194,195,leaf,0,2|194,196,allpackets_5,122.5,2|196,197,leaf,0,2|196,198,receive_time_1,193.969,0|198,199,drops_5,106547.0,0|199,200,leaf,0,1|199,201,leaf,0,0|198,202,leaf,0,2";
	char *str = NULL;

	const char * split = "|";
	char * p;
	struct rawdata *nodeptr = NULL;

	char *outer_ptr = NULL;
	char *inner_ptr = NULL;

	int i = 0;
	str = inputtxt;
	p = strtok_r(str, split, &outer_ptr);
	printf("%s\n", inputtxt);
	while (p != NULL)
	{
		char * tmp = NULL;
		char * q;
		struct rawdata *newnode = NULL;
		int k = 0;
		const char *split1 = ",";
		// printf("%s\n", p);
		tmp = p;
		q = strtok_r(tmp, split1, &inner_ptr);
		k = 0;
		newnode = (struct rawdata *)malloc(sizeof(struct rawdata));
		memset(newnode, 0, sizeof(struct rawdata));
		while (q != NULL)
		{
			if (k == 0)
				newnode->father = atoi(q);
			if (k == 1)
			{
				newnode->itself = atoi(q);
			}
			if (k == 2)
			{
				int len = strlen(q);
				newnode->name = (char *)malloc(sizeof(char) * len);
				memset(newnode->name, 0, sizeof(char) * len);
				strcpy(newnode->name , q);
			}
			if (k == 3)
				newnode->value = atof(q);
			if (k == 4)
				newnode->class = atoi(q);
			k = k + 1;
			q = strtok_r(NULL, split1, &inner_ptr);
		}
		if (node == NULL)
		{
			node = newnode;
			node->next = NULL;
			nodeptr = node;
		}
		else
		{
			nodeptr->next = newnode;
			nodeptr = newnode;
			nodeptr->next = NULL;
		}
		p = strtok_r(NULL, split, &outer_ptr);
	}
	print_rawdata(node);
	printf("xxxxxxxxxx\n");
	tnode = create_tree(node, NULL);
	print_tree(tnode);
	printf("the highth of tree is %d\n", TreeDepth(tnode));
	parameters = get_parameters(node);
	while (parameters[i])
	{
		printf("%d: %s\t", i, parameters[i]);
		i = i + 1;
	}

	return 0;
}
int init_linger(char * inputtxt, char * item)
{
	// char inputtxt[] = "0,0,requeues_3,4583.405,0|0,1,allpackets_1,145.5,2|1,2,receive_time_5,406.741,2|2,3,active_time_4,925.101,2|3,4,leaf,0,2|3,5,drops_4,23788.1,2|5,6,drops_3,109624.0,2|6,7,receive_time_5,148.055,2|7,8,leaf,0,0|7,9,busy_time_4,750.129,2|9,10,leaf,0,0|9,11,leaf,0,2|6,12,leaf,0,0|5,13,leaf,0,2|2,14,leaf,0,0|1,15,packets_2,0.58,2|15,16,leaf,0,1|15,17,requeues_1,669.086,2|17,18,drops_1,7715.755,2|18,19,active_time_4,931.829,2|19,20,requeues_3,1174.415,2|20,21,backlogs_2,4843.15,2|21,22,allpackets_4,11.5,2|22,23,leaf,0,2|22,24,leaf,0,2|21,25,leaf,0,2|20,26,allpackets_1,319.0,2|26,27,leaf,0,2|26,28,leaf,0,2|19,29,noise_1,-89.5,2|29,30,retrans_3,50.5,2|30,31,leaf,0,2|30,32,allpackets_5,147.0,2|32,33,leaf,0,2|32,34,leaf,0,2|29,35,leaf,0,2|18,36,leaf,0,2|17,37,requeues_3,4473.6,2|37,38,active_time_5,922.379,2|38,39,leaf,0,2|38,40,leaf,0,2|37,41,leaf,0,2|0,42,busy_time_1,811.901,0|42,43,retrans_4,16.5,0|43,44,transmit_time_4,62.532,0|44,45,allpackets_1,168.5,0|45,46,busy_time_2,708.081,0|46,47,retrans_5,15.5,0|47,48,rbytes_2,165222.906,0|48,49,allpackets_4,216.0,0|49,50,busy_time_1,399.099,0|50,51,leaf,0,0|50,52,retrans_3,0.5,0|52,53,drops_1,13282.95,0|53,54,rbytes_4,200832.266,0|54,55,requeues_1,1269.48,0|55,56,leaf,0,0|55,57,rbytes_5,113537.703,1|57,58,transmit_time_1,53.347,0|58,59,leaf,0,1|58,60,leaf,0,0|57,61,leaf,0,1|54,62,leaf,0,0|53,63,leaf,0,0|52,64,packets_3,76.374,0|64,65,leaf,0,0|64,66,allpackets_5,154.5,0|66,67,retrans_3,9.5,0|67,68,packets_4,150.492,0|68,69,leaf,0,0|68,70,leaf,0,0|67,71,leaf,0,0|66,72,leaf,0,1|49,73,leaf,0,0|48,74,leaf,0,0|47,75,leaf,0,0|46,76,drops_5,95043.797,0|76,77,leaf,0,0|76,78,requeues_1,1332.805,0|78,79,transmit_time_1,31.125,0|79,80,leaf,0,0|79,81,allpackets_1,139.5,0|81,82,active_time_5,921.647,0|82,83,leaf,0,0|82,84,active_time_4,934.706,0|84,85,leaf,0,1|84,86,requeues_4,1519.65,0|86,87,leaf,0,0|86,88,leaf,0,0|81,89,allpackets_2,52.5,0|89,90,leaf,0,0|89,91,packets_2,102.88,0|91,92,packets_3,74.465,0|92,93,leaf,0,0|92,94,busy_time_1,796.27,0|94,95,busy_time_2,803.941,0|95,96,active_time_4,936.588,0|96,97,leaf,0,0|96,98,rbytes_1,181646.922,0|98,99,leaf,0,0|98,100,leaf,0,0|95,101,leaf,0,0|94,102,leaf,0,1|91,103,leaf,0,0|78,104,leaf,0,0|45,105,receive_time_5,269.211,0|105,106,retrans_5,15.5,0|106,107,rbytes_4,126370.078,0|107,108,leaf,0,1|107,109,drops_4,25934.4,0|109,110,leaf,0,0|109,111,packets_1,136.702,0|111,112,allpackets_3,155.5,0|112,113,active_time_3,933.032,0|113,114,leaf,0,1|113,115,leaf,0,0|112,116,leaf,0,0|111,117,rbytes_4,150856.359,0|117,118,leaf,0,0|117,119,allpackets_2,22.5,0|119,120,allpackets_2,15.5,1|120,121,leaf,0,0|120,122,leaf,0,1|119,123,allpackets_3,59.5,0|123,124,leaf,0,0|123,125,allpackets_4,214.5,0|125,126,transmit_time_3,23.069,0|126,127,leaf,0,0|126,128,active_time_4,936.779,1|128,129,leaf,0,0|128,130,transmit_time_4,29.219,1|130,131,allpackets_3,74.5,1|131,132,leaf,0,1|131,133,leaf,0,1|130,134,transmit_time_5,57.634,1|134,135,leaf,0,0|134,136,leaf,0,1|125,137,leaf,0,0|106,138,leaf,0,1|105,139,packets_1,144.73,0|139,140,allpackets_1,204.5,0|140,141,retrans_2,16.5,0|141,142,transmit_time_3,203.117,0|142,143,backlogs_3,4739.775,0|143,144,retrans_3,80.5,0|144,145,packets_3,326.613,0|145,146,packets_4,202.219,0|146,147,leaf,0,0|146,148,leaf,0,0|145,149,leaf,0,0|144,150,leaf,0,0|143,151,leaf,0,0|142,152,leaf,0,1|141,153,leaf,0,2|140,154,leaf,0,1|139,155,rbytes_3,519237.875,0|155,156,allpackets_1,191.5,0|156,157,requeues_1,1294.575,0|157,158,active_time_4,931.829,0|158,159,leaf,0,0|158,160,requeues_4,1631.26,0|160,161,receive_time_4,147.458,0|161,162,leaf,0,0|161,163,leaf,0,0|160,164,leaf,0,0|157,165,leaf,0,0|156,166,busy_time_3,801.711,0|166,167,packets_4,158.564,0|167,168,active_time_4,934.376,0|168,169,leaf,0,0|168,170,leaf,0,0|167,171,leaf,0,0|166,172,rbytes_4,177486.375,0|172,173,leaf,0,0|172,174,leaf,0,0|155,175,leaf,0,2|44,176,receive_time_1,110.609,0|176,177,leaf,0,0|176,178,busy_time_3,796.891,1|178,179,leaf,0,1|178,180,requeues_4,1546.595,0|180,181,leaf,0,0|180,182,leaf,0,1|43,183,allpackets_1,194.5,0|183,184,allpackets_5,283.5,0|184,185,packets_4,78.635,0|185,186,leaf,0,2|185,187,requeues_3,5247.93,1|187,188,leaf,0,1|187,189,active_time_4,937.092,0|189,190,leaf,0,0|189,191,leaf,0,1|184,192,leaf,0,0|183,193,leaf,0,2|42,194,requeues_3,5170.32,2|194,195,leaf,0,2|194,196,allpackets_5,122.5,2|196,197,leaf,0,2|196,198,receive_time_1,193.969,0|198,199,drops_5,106547.0,0|199,200,leaf,0,1|199,201,leaf,0,0|198,202,leaf,0,2";
	char *str = NULL;

	const char * split = "|";
	char * p;
	struct rawdata *nodeptr = NULL;

	char *outer_ptr = NULL;
	char *inner_ptr = NULL;

	struct item_value *value[80];

	int i = 0;
	str = inputtxt;
	p = strtok_r(str, split, &outer_ptr);
	while (p != NULL)
	{
		char * tmp = NULL;
		char * q;
		struct rawdata *newnode = NULL;
		int k = 0;
		const char *split1 = ",";
		// printf("%s\n", p);
		tmp = p;
		q = strtok_r(tmp, split1, &inner_ptr);
		k = 0;
		newnode = (struct rawdata *)malloc(sizeof(struct rawdata));
		memset(newnode, 0, sizeof(struct rawdata));
		while (q != NULL)
		{
			if (k == 0)
				newnode->father = atoi(q);
			if (k == 1)
			{
				newnode->itself = atoi(q);
			}
			if (k == 2)
			{
				int len = strlen(q);
				newnode->name = (char *)malloc(sizeof(char) * len);
				memset(newnode->name, 0, sizeof(char) * len);
				strcpy(newnode->name , q);
			}
			if (k == 3)
				newnode->value = atof(q);
			if (k == 4)
				newnode->class = atoi(q);
			k = k + 1;
			q = strtok_r(NULL, split1, &inner_ptr);
		}
		if (node == NULL)
		{
			node = newnode;
			node->next = NULL;
			nodeptr = node;
		}
		else
		{
			nodeptr->next = newnode;
			nodeptr = newnode;
			nodeptr->next = NULL;
		}
		p = strtok_r(NULL, split, &outer_ptr);
	}
	tnode = create_tree(node, NULL);
	print_tree(tnode);
	printf("the highth of tree is %d\n", TreeDepth(tnode));
	parameters = get_parameters(node);
	while (parameters[i])
	{
		printf("%d: %s\t", i, parameters[i]);
		i = i + 1;
	}
	for (i = 0; i < ITEM_AMOUNT; i++)
	{
		// memset(items_list[i], 0, sizeof(struct item_value));
		items_list[i].name = NULL;
		items_list[i].value = -1000000.0;
	}
	create_items(items_list, item);
	for (i = 0; i < ITEM_AMOUNT; i++)
	{
		if (items_list[i].name == NULL)
			break;
		printf("name %s value %f\n", items_list[i].name, items_list[i].value);
		// i = i + 1;
	}

	return 0;
}

void free_rawdata(struct rawdata * head)
{
	struct rawdata * before = NULL;
	while (head)
	{
		before = head;
		head = head->next;
		free(before->name);
		before->name = NULL;
		free(before);
		before = NULL;
	}
}

void print_rawdata(struct rawdata* raw)
{
	while (raw != NULL)
	{
		printf("%d\t%d\t%s\t%f\t%d\n", raw->father, raw->itself, raw->name, raw->value, raw->class);
		raw = raw->next;
	}
}

struct treenode * create_tree(struct rawdata *ptr, struct treenode * parent)
{
	struct treenode * newnode = NULL;
	if (ptr != NULL)
	{
		newnode = (struct treenode *)malloc(sizeof(struct treenode));
		memset(newnode, 0, sizeof(struct treenode));
		newnode->name = (char *)malloc(sizeof(char) * strlen(ptr->name));
		newnode->itself = ptr->itself;
		strcpy(newnode->name, ptr->name);
		newnode->value = ptr->value;
		newnode->class = ptr->class;
		newnode->lchild = NULL;
		newnode->rchild = NULL;
		newnode->parent = NULL;
		if (strcmp(ptr->name, "leaf") != 0)
		{
			struct rawdata *ptrtmp = NULL;
			ptrtmp = find_lchild(ptr->next, ptr->itself);
			newnode->lchild = create_tree(ptrtmp, newnode);
			ptrtmp = find_rchild(ptr->next, ptr->itself);
			newnode->rchild = create_tree(ptrtmp, newnode);
		}
	}
	return newnode;
}

void print_tree(struct treenode *raw)
{
	if (raw)
	{
		printf("%d\t%s\t%f\t%d\n", raw->itself, raw->name, raw->value, raw->class);
		print_tree(raw->lchild);
		print_tree(raw->rchild);
	}
}
void destroy_tree(struct treenode *T)
{
	if (T)
	{
		destroy_tree(T->lchild);
		destroy_tree(T->rchild);
		free(T->name);
		T->name = NULL;
		free(T);
		T = NULL;
	}
}
struct rawdata * find_lchild(struct rawdata *ptr, int father)
{
	while (ptr)
	{
		if (ptr->father == father)
		{
			return ptr;
		}
		ptr = ptr->next;
	}
}
struct rawdata * find_rchild(struct rawdata *ptr, int father)
{
	int rchild = 0;
	while (ptr)
	{
		if (ptr->father == father)
		{
			if (rchild == 1)
				return ptr;
			rchild = rchild + 1;
		}
		ptr = ptr->next;
	}
}

int TreeDepth(struct treenode *T)
{
	int rightdep = 0;
	int leftdep = 0;

	if (T == NULL)
		return -1;

	if (T->lchild != NULL)
		leftdep = TreeDepth(T->lchild);
	else
		leftdep = -1;

	if (T->rchild != NULL)
		rightdep = TreeDepth(T->rchild);
	else
		rightdep = -1;

	return (rightdep > leftdep) ? rightdep + 1 : leftdep + 1;
}

struct treenode* bst_search(struct treenode* node, struct item_value value[], int length)
{
	int found = 0;
	struct treenode* node_pre = NULL;
	node_pre = node;
	while (node && (strcmp(node->name, "leaf") != 0))
	{
		int i = 0;
		found = 0;
		for (i = 0; i < length; i++)
		{
			if (strcmp(node->name, value[i].name) == 0)
			{
				found = 1;
				break;
			}
			if (value[i].name == NULL)
				break;
		}
		if (found = 0)
			return NULL;
		if (value[i].value <= node->value)
		{
			node_pre = node;
			node = node->lchild;
		}
		else
		{
			node_pre = node;
			node = node->rchild;
		}
	}
	if (found)
	{
		printf("1 name %s value %f\n", node_pre->name, node_pre->value);
		return node;
	}
	else
	{
		printf("2 name %s value %f\n", node_pre->name, node_pre->value);
		return node_pre;
	}
}

char ** get_parameters(struct rawdata *ptr)
{
	char **tmp;
	int i = 0, found = 0, last = 0;
	tmp = (char **)malloc(80 * sizeof(char *));
	for (i = 0; i < 80; i++)
	{
		tmp[i] = NULL;
	}
	while (ptr)
	{
		char *str = NULL;
		if (strcmp(ptr->name, "leaf") == 0)
		{
			ptr = ptr->next;
			continue;
		}
		i = 0;
		found = 0;
		while (i < last)
		{
			if (strcmp(tmp[i], ptr->name) == 0)
			{
				found = 1;
				break;
			}
			i = i + 1;
		}
		if (found == 1)
		{
			ptr = ptr->next;
			continue;
		}
		else
		{
			tmp[last] = (char *)malloc(sizeof(char) * strlen(ptr->name));
			memset(tmp[last], 0, sizeof(char) * strlen(ptr->name));
			strcpy(tmp[last], ptr->name);
			last = last + 1;
		}

		ptr = ptr->next;
	}
	return tmp;
}
void destroy_parameters(char *ptr[80])
{
	int i = 0;
	for (i = 0; i < 80; i++)
	{
		free(ptr[i]);
	}
	free(ptr);
}

int create_items(struct item_value items_list[], char *buffer)
{
	int j, in = 0;
	// char buffer[] = "requeues_3,5000;busy_time_1,809;allpackets_1,160;requeues_1,670;drops_1,3220;retrans_4,0;allpackets_3,0;retrans_5,1;retrans_3,0;receive_time_1,0;transmit_time_4,0;rbytes_3,0";
	char *p[40];
	int index = 0;
	char *buf = buffer;
	char *outer_ptr = NULL;
	char *inner_ptr = NULL;
	while ((p[in] = strtok_r(buf, ";", &outer_ptr)) != NULL)
	{
		buf = p[in];
		while ((p[in] = strtok_r(buf, ",", &inner_ptr)) != NULL)
		{
			in++;
			buf = NULL;
		}
		buf = NULL;
	}
	// printf("Here we have %d strings\n", in);
	for (j = 0; j < in; j++)
	{
		if (j % 2 ==  0)
		{
			// printf("name: %s\n", p[j]);
			items_list[index].name = (char *)malloc(sizeof(char) * strlen(p[j]));
			memset(items_list[index].name, 0, sizeof(char) * strlen(p[j]));
			strcpy(items_list[index].name, p[j]);
		}
		else
		{
			// printf("value: %s %f\n", p[j], atof(p[j]));
			items_list[index].value = atof(p[j]);
			if (index >= ITEM_AMOUNT)
			{
				printf("outrage\n");
				exit(-1);
			}

			index = index + 1;
		}
	}
}

void destroy_item_list(struct item_value items_list[])
{
	int i = 0;
	while (items_list[i].name)
	{
		free(items_list[i].name);
		items_list[i].name = NULL;
	}
}

int main()
{
	char *str = NULL;
	
	
	int i = 0;
	while(1)
	{
		str = get_openflow_input();
		init_linger_m(str);
		main_linger();
		i = i + 1;
		printf("%s %d\n", str, i);

		// break;
		sleep(1);
	}
	if(str)
	{
		free(str);
		str = NULL;
	}
	return 0;
}