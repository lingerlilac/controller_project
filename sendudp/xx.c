#include <linux/init.h>
#include <linux/module.h>
#include <linux/ieee80211.h>
#include <linux/nl80211.h>
#include <linux/rtnetlink.h>
#include <linux/slab.h>
#include <net/net_namespace.h>
#include <linux/rcupdate.h>
#include <linux/if_ether.h>
#include <net/cfg80211.h>
#include <linux/time.h>
#include <linux/timer.h>
#include <net/sock.h>

MODULE_LICENSE("Dual BSD/GPL");

#define BUFF_SIZE 1000 //buffer size in openvswitch/datapath/flow.c
#define SEND_SIZE 1024 //send buffer size
#define B_S 	1024 /*public buffer size*/


int control_beacon = 0; // used to control the send rate of buffer informations
static 	struct timer_list tm; // used to periodly send informations
struct 	timeval oldtv; // used to get current time
char 	public_buffer[B_S]; // public buffer to write data and then send them when buffer size reach 1024
int 	write_begin = 0; // point that data can be wrote in in public buffer
/*
* The folling __u16 items are enum datatype, however, enum are length 4 bits, so, we use
* __u16 2-bytes data type instead to work with __u8 mac[6]. 2 + 6 = 8, 4 + 6 -> 16 is realy waste.
*/
__u16 WINSIZE = 1;
__u16 DROPS = 2;
__u16 IW = 3;
__u16 QUEUE = 4;
__u16 BEACON = 5;
__u16 SURVEY = 6;
__u16 KEEP_IW = 7;
__u16 KEEP_BEACON = 8;

/*
* dp_packet is a structure used to record data packet that dropped in mac80211 or
* queue algorithms, it has an extra struct dp_packet *next member than ,
* this is because a chain should be created to record dropped packets.
*/
struct dp_packet
{
	__u32 ip_src;
	__u32 ip_dst;
	__be32 sec;
	__be32 usec;
	__u16 port_src;
	__u16 port_dst;
	__u32 sequence;
	__u32 ack_sequence;
	__u32 drop_count;
	__u16 dpl;
	__u16 pad;
	__u32 in_time;
	struct dp_packet *next;
	__u32 pad1;
};


/*
* data_iw is used to record iw information in status.c of mac80211.
*  is its transmission version, there is a transformation
* between this two data structrues.
*/
struct data_iw
{
	char station[6];
	__u16 device;
	__be32 sec;
	__be32 usec;
	__u32 inactive_time;
	__u32 rx_bytes;
	__u32 rx_packets;
	__u32 tx_bytes;
	__u32 tx_packets;
	__u32 tx_retries;
	__u32 tx_failed;
	__s32 signal;
	__s32 signal_avg;
	__u32 expected_throughput;
	struct data_iw *next;
	__u32 pad;
};


/*
* data_survey is used to record iw dev wlan0 survey infromation.
* it is a ap information, so there is no *next;
*/
struct data_survey
{
	__u64 time;
	__be32 sec;
	__be32 usec;
	__u64 time_busy;
	__u64 time_ext_busy;
	__u64 time_rx;
	__u64 time_tx;
	__u64 time_scan;
	__u32 filled;
	__s8 noise;
	__u8 pad;
	__u16 center_freq;
};

/*
* data_beacon is used to record beacon information from scan.c of mac80211
* it has *next mumber to record neibour APs informations
* is transmission version of beacon informations.
*/
struct data_beacon
{
	__be32 sec;
	__be32 usec;
	__u16 data_rate;
	__u16 freq;
	__u32 timein;
	__s8 signal;
	__u8 pad;
	__u8 bssid[6];
	struct data_beacon *next;
	__u32 pad1;
};


/* data_queue is used to record queue information from sch_fq_codel.
* no *next mumber
*/
struct data_queue
{
	__u32 queue_id;
	__u32 packets;
	__u64 bytes;
	__be32 sec;
	__be32 usec;
	__u32 qlen;
	__u32 backlog;
	__u32 drops;
	__u32 requeues;
	__u32 overlimits;
	__u32 pad;
};

/*
* data_winsize is used to receive packet information from global buffer ubuf
*/
struct data_winsize {
	__be32 ip_src;
	__be32 ip_dst;
	__be16 sourceaddr;
	__be16 destination;
	__be32 sequence;
	__be32 ack_sequence;
	__be16 flags;
	__be16 windowsize;
	__be32 sec;
	__be32 usec;
	__be32 datalength;
	char wscale[3];
	// char eth_src[6];
	// char eth_dst[6];
	char pad[1];
};

/*
 * The forllowing _t data structures are transmission versions of different data
 * types. category is the data category, mac is the AP generates them.
 */
struct winsize_t
{
	__u16 category;
	char mac[6];
	struct 	data_winsize winsize;
};
struct dp_t
{
	__u16 category;
	char mac[6];
	struct 	dp_packet drops;
};
struct di_t
{
	__u16 category;
	char mac[6];
	struct 	data_iw iw;
};
struct dq_t
{
	__u16 category;
	char mac[6];
	struct 	data_queue queue;
};
struct db_t
{
	__u16 category;
	char mac[6];
	struct 	data_beacon beacon;
};
struct ds_t
{
	__u16 category;
	char mac[6];
	struct 	data_survey survey;
};

/*
* keep_ statuctures are use to decrease information redundant
*/
struct keep_iw
{
	__u16 category;
	char station[6];
	char mac[6];
	__u16 pad;
};

struct keep_beacon
{
	__u16 category;
	char bssid[6];
	char mac[6];
	__u16 pad;
};


/*
* iwlist_last is a iw data chain that stores data of last transmission.
* when new iw informations generated, they are stored and waiting for
* transmission, check and transmit different data items. Generating new
* iwlist_last after each transmission process.
* queue_last works differently from iwlist_last and beacon_last. This is
* because only a single queue data structure can store last transmitted
* queue information, while a chain should be created to store iw or
* beacon informations.
*/
struct 	data_iw 		*iwlist_last = NULL;
struct 	data_queue 		*queue_last  = NULL;
struct 	data_beacon 	*beacon_last = NULL;
struct socket *sock_global;
char mac_global[6];
long last_sec = 0;
long last_usec = 0;

long last_sec_iw = 0;
long last_usec_iw = 0;

long last_sec_beacon = 0;
long last_usec_beacon = 0;

long last_sec_queue = 0;
long last_usec_queue = 0;

long last_sec_survey = 0;
long last_usec_survey = 0;

long last_sec_drop_mac = 0;
long last_usec_drop_mac = 0;

long last_sec_drop_queue = 0;
long last_usec_drop_queue = 0;
/*
* The following functions are global functions from status.c, scan.c in mac80211,
* codel.h and sch_fq_codel.c.
*/
extern struct 	data_iw *ieee80211_dump_station_linger(struct data_survey *); //
extern struct 	dp_packet *dpt_mac_linger(void); //
extern void 	destroy_iwlist(struct data_iw *iwlist);
// extern struct 	data_queue *dump_class_stats_linger(void); //
extern struct 	dp_packet *dpt_queue_linger(void); //
extern struct 	data_beacon *get_beacon_global_linger(void); //
extern void 	destroy_after_transmitted_codel(void);
extern void 	destroy_after_transmitted_mac80211(void);
extern struct data_queue queue_global[6];
struct data_queue queue_previous[6];

void write_2_public_buffer(void *data, int length);
void send_the_buffer(void);
void get_information(void);
void destroy_data(void);
void compare_and_iwlist(struct data_iw *ptr, struct data_survey *sur);
void del_this_node_iw(struct data_iw *ptr_pre, struct data_iw *ptr);
void del_this_node_beacon(struct data_beacon *ptr_pre, struct data_beacon *ptr);
void callback(void);
void destroy_list_beacon(struct data_beacon *ptr);
void destroy_list_iw(struct data_iw *ptr);
void copy_last_iw(struct data_iw *curr);
void copy_last_beacon(struct data_beacon *curr);
void compare_and_queue(void);
void compare_and_beacon(struct data_beacon *ptr);
struct socket * get_sock(void);
void print_iwlist(struct data_iw *ptr, char *str);
void print_mac(char *addr);
void mac_tranADDR_toString_r(unsigned char* addr, char* str, size_t size);
int strcmp_linger(char *s, char *t);
/**
 * [mac_tranADDR_toString_r u8 address[6] to "01:2b:4c:5d:6a:7e"]
 * @linger
 * @DateTime  2018-05-06T01:03:55-0500
 * @param     addr                     [description]
 * @param     str                      [description]
 * @param     size                     [description]
 */
void mac_tranADDR_toString_r(unsigned char* addr, char* str, size_t size)
{
	if (addr == NULL || str == NULL || size < 18)
		// exit(1);
		printk(KERN_DEBUG "fff\n");

	snprintf(str, size, "%02x:%02x:%02x:%02x:%02x:%02x",
	         addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
	str[17] = '\0';
}

/**
 * [print_mac print mac address, used when debugging]
 * @AuthorHTL
 * @DateTime  2018-05-06T01:41:54-0500
 * @param     addr                     __u8 addr[6]
 */
void print_mac(char *addr)
{
	char str[18];
	mac_tranADDR_toString_r(addr, str, 18);
	printk(KERN_DEBUG "%s\n", str);
}


/**
 * write data to public buffer. First, check if the buffer will be
 * full when data is added, if full, tansmit and write, write then
 * transmit otherwise.
 * @AuthorHTL
 * @DateTime  2018-05-06T01:47:55-0500
 * @param     data                     data to be transmitted
 * @param     length                   length of data
 */
void write_2_public_buffer(void *data, int length)
{
	if ((write_begin + length) <= B_S)
	{
		memcpy(public_buffer + write_begin, data, length);
		write_begin = write_begin + length;
	}
	else
	{
		send_the_buffer();
		memcpy(public_buffer + write_begin, data, length);
		write_begin = write_begin + length;
	}
}

/**
 * [send_the_buffer description]
 * @Linger
 * @Principle: This part is from web
 * @DateTime   2018-05-06T01:54:56-0500
 */
void send_the_buffer(void)
{
	struct kvec iov;
	struct msghdr msg = {.msg_flags = MSG_DONTWAIT | MSG_NOSIGNAL};
	int len = 0;

	char buffer[SEND_SIZE];

	memset(&buffer, 0, SEND_SIZE);
	memcpy(buffer, public_buffer, write_begin);

	iov.iov_base = (void *)buffer;
	iov.iov_len = write_begin;
	len = kernel_sendmsg(sock_global, &msg, &iov, 1, write_begin);
	if (len != write_begin)
	{
		if (len == -ECONNREFUSED)
		{
			printk(KERN_ALERT "Receive Port Unreachable packet!\n");
		}
	}

	memset(public_buffer, 0, B_S);
	write_begin = 0;

}

/**
 * [Used to create a global socket to transmit data]
 * @Linger
 * @DateTime 2018-05-06T01:58:30-0500
 * @return                            socket created
 */
struct socket * get_sock(void)
{
	struct socket *sock;
	struct sockaddr_in s_addr;
	__s16 dstport = 6667;
	__u32 dstip = 0xc0a80b64;
	int ret = 0;

	memset(&s_addr, 0, sizeof(s_addr));
	s_addr.sin_family = AF_INET;
	s_addr.sin_port = cpu_to_be16(dstport);

	s_addr.sin_addr.s_addr = cpu_to_be32(dstip); /*server ip is 192.168.209.134*/
	sock = (struct socket *)kmalloc(sizeof(struct socket), GFP_KERNEL);

	/*create a socket*/
	// ret=sock_create_kern(&init_net, AF_INET, SOCK_STREAM,0,&sock);
	ret = sock_create_kern(&init_net, AF_INET, SOCK_DGRAM, IPPROTO_UDP, &sock);

	if (ret < 0) {
		printk("client:socket create error!\n");
		return NULL;
	}
	printk("client: socket create ok!\n");

	/*connect server*/
	ret = sock->ops->connect(sock, (struct sockaddr *)&s_addr, sizeof(s_addr), 0);
	if (ret != 0) {
		printk("client:connect error!\n");
		return NULL;
	}
	printk("client:connect ok!\n");
	return sock;
}

/**
 * callback function scheduled every 1ms, it calls packet_t()
 * check if there is data to transmitted by checking uhead and
 * utail, if there is get corresponding data and transmitted.
 * @Linger
 * @DateTime 2018-05-06T01:59:46-0500
 */
void callback(void)
{
	struct timeval tv;

	do_gettimeofday(&tv);
	get_information();
	oldtv = tv;
	tm.expires = jiffies + 1;
	add_timer(&tm);        //重新开始计时
}

/**
 * Delete iw list node when it is out of date. Check if ptr_pre is NULL,
 * delete ptr if is, otherwise connected ptr's pre-node and node after it.
 * @Linger
 * @DateTime 2018-05-06T02:02:06-0500
 * @param    ptr_pre                  node before ptr
 * @param    ptr                      ptr
 */
void del_this_node_iw(struct data_iw *ptr_pre, struct data_iw *ptr)
{
	struct data_iw *tmp = NULL;
	if (ptr_pre == NULL)
	{
		tmp = ptr->next;
		ptr = ptr->next;
		kfree(tmp);
		tmp = NULL;
	}
	else
	{
		tmp = ptr->next;
		ptr_pre->next = tmp;
		tmp = NULL;
		kfree(ptr);
		ptr = NULL;
	}
}

/**
 * Delete beacon node when it is out of date.
 * @Linger
 * @DateTime 2018-05-06T02:10:50-0500
 * @param    ptr_pre                  node before it
 * @param    ptr                      node to be delete.
 */
void del_this_node_beacon(struct data_beacon *ptr_pre, struct data_beacon *ptr)
{
	struct data_beacon *tmp = NULL;
	if (ptr_pre == NULL)
	{
		tmp = ptr->next;
		ptr = ptr->next;
		kfree(tmp);
		tmp = NULL;
	}
	else
	{
		tmp = ptr->next;
		ptr_pre->next = tmp;
		tmp = NULL;
		kfree(ptr);
		ptr = NULL;
	}
}

/**
 * Delete the whole iw data list
 * @Linger
 * @DateTime 2018-05-06T02:11:34-0500
 * @param    ptr                      Head address of iw list.
 */
void destroy_list_iw(struct data_iw *ptr)
{
	struct data_iw *tmp = NULL;
	tmp = ptr;
	while (tmp)
	{
		void *pre = NULL;
		pre = tmp;
		tmp = tmp->next;
		kfree(pre);
		pre = NULL;
	}
}

/**
 * Destroy the whole beacon list.
 * @Linger
 * @DateTime 2018-05-06T02:18:49-0500
 * @param    ptr                      head address of beacon list.
 */
void destroy_list_beacon(struct data_beacon *ptr)
{
	struct data_beacon *tmp = NULL;
	tmp = ptr;
	while (tmp)
	{
		void *pre = NULL;
		pre = tmp;
		tmp = tmp->next;
		kfree(pre);
		pre = NULL;
	}
}

/**
 * Copy the whole iwlist got from status.c to local global iwlist.
 * @Linger
 * @DateTime 2018-05-06T02:21:15-0500
 * @param    curr                     iwlist head address of status.c
 */
void copy_last_iw(struct data_iw *curr)
{
	struct data_iw *keep = NULL;
	struct data_iw *ptr = NULL, *last_keep = NULL;
	ptr = curr;
	while (ptr)
	{
		if (!keep)
		{
			keep = (struct data_iw*)kmalloc(sizeof(struct data_iw), GFP_KERNEL);
			memset(keep, 0, sizeof(struct data_iw));

			memcpy(keep->station, ptr->station, 6);
			keep->device 				= ptr->device;
			keep->inactive_time 		= ptr->inactive_time;
			keep->rx_bytes 				= ptr->rx_bytes;
			keep->rx_packets 			= ptr->rx_packets;
			keep->tx_bytes 				= ptr->tx_bytes;
			keep->tx_packets 			= ptr->tx_packets;
			keep->tx_retries 			= ptr->tx_retries;
			keep->tx_failed 			= ptr->tx_failed;
			keep->signal 				= ptr->signal;
			keep->signal_avg 			= ptr->signal_avg;
			keep->expected_throughput 	= ptr->expected_throughput;
			keep->next = NULL;
			last_keep = keep;
		}
		else
		{
			struct data_iw *new = NULL;
			new = (struct data_iw*)kmalloc(sizeof(struct data_iw), GFP_KERNEL);
			memset(new, 0, sizeof(struct data_iw));

			memcpy(new->station, ptr->station, 6);
			new->device 				= ptr->device;
			new->inactive_time 			= ptr->inactive_time;
			new->rx_bytes 				= ptr->rx_bytes;
			new->rx_packets 			= ptr->rx_packets;
			new->tx_bytes 				= ptr->tx_bytes;
			new->tx_packets 			= ptr->tx_packets;
			new->tx_retries 			= ptr->tx_retries;
			new->tx_failed 				= ptr->tx_failed;
			new->signal 				= ptr->signal;
			new->signal_avg 			= ptr->signal_avg;
			new->expected_throughput 	= ptr->expected_throughput;
			new->next = NULL;
			last_keep->next = new;
			last_keep = new;
		}
		ptr = ptr->next;
	}
	iwlist_last = keep;
}

/**
 * Copy the whole beaconlist from scan.c to local beacon list.
 * @Linger
 * @DateTime 2018-05-06T02:23:11-0500
 * @param    curr                     beaconlist head address.
 */
void copy_last_beacon(struct data_beacon *curr)
{
	struct data_beacon *keep = NULL;
	struct data_beacon *ptr = NULL, *last_keep = NULL;
	ptr = curr;
	while (ptr)
	{
		if (!keep)
		{
			keep = (struct data_beacon*)kmalloc(sizeof(struct data_beacon), GFP_KERNEL);
			memset(keep, 0, sizeof(struct data_beacon));

			keep->data_rate = ptr->data_rate;
			keep->freq 		= ptr->freq;
			keep->signal 	= ptr->signal;
			memcpy(keep->bssid, ptr->bssid, 6);

			keep->next 		= NULL;
			last_keep 		= keep;
		}
		else
		{
			struct data_beacon *new = NULL;
			new = (struct data_beacon*)kmalloc(sizeof(struct data_beacon), GFP_KERNEL);
			memset(new, 0, sizeof(struct data_beacon));

			new->data_rate 	= ptr->data_rate;
			new->freq 		= ptr->freq;
			new->signal 	= ptr->signal;
			memcpy(new->bssid, ptr->bssid, 6);

			new->next 		= NULL;
			last_keep->next = new;
			last_keep 		= new;
		}
		ptr = ptr->next;
	}
	beacon_last = keep;
}

/**
 * Check whether items in iwlist from status.c are transmitted in last round,
 * if transmitted, transmitted keep_iw, transmit new iw data item otherwise.
 * Condition is used to check whether this item is transmitted. tmp_t % 10 is used
 * for this senario: information mainted in server side missed, and it can no longer
 * decode the iw informaiton, so, we should update iw information periodly to avoid
 * this senarios.
 * Also, mac address are patched to data structure to indicate its origin AP.
 * @Linger
 * @DateTime 2018-05-06T02:23:48-0500
 * @param    ptr                      iwlist head address from status.c
 * @param    sur                      address of survey information got from status.c
 */

void compare_and_iwlist(struct data_iw *ptr, struct data_survey *sur)
{
	struct data_iw 	*tmp = NULL;
	struct ds_t 	s_t;
	memset(&s_t, 0, sizeof(struct ds_t));
	tmp = ptr;
	/**
	 * processing survey information.
	 */
	s_t.category 	= 	SURVEY;
	s_t.survey 		=  	*sur;
	memcpy(&(s_t.mac), mac_global, 6);
	if(s_t.survey.sec > last_sec_survey)
	{
		write_2_public_buffer(&s_t, sizeof(struct ds_t));
		last_sec_survey = s_t.survey.sec;
		last_usec_survey = s_t.survey.usec;
	}
	else if ((s_t.survey.sec == last_sec_survey) && (s_t.survey.usec > last_usec_survey))
	{
		write_2_public_buffer(&s_t, sizeof(struct ds_t));
		last_usec_survey = s_t.survey.usec;
	}

	while (tmp)
	{
		struct di_t t;
		memset(&t, 0, sizeof(struct di_t));
		t.category = IW;
		memcpy(&(t.mac), mac_global, 6);
		t.iw = *tmp;
		t.iw.next = NULL;
		if(t.iw.sec > last_sec_iw)
		{
			write_2_public_buffer(&t, sizeof(struct di_t));
			last_sec_iw = t.iw.sec;
			last_usec_iw = t.iw.usec;
		}
		else if ((t.iw.sec == last_sec_iw) && (t.iw.usec > last_usec_iw))
		{
			write_2_public_buffer(&t, sizeof(struct di_t));
			last_usec_iw = t.iw.usec;
		}
		
		tmp = tmp->next;
	}
}
/**
 * Check whether to transmit current queue information got.
 * @Linger
 * @DateTime 2018-05-06T02:32:44-0500
 * @param    ptr                      Address of queue got from sch_fq_codel.c
 */
// void compare_and_queue(struct data_queue *ptr)
// {
// 	// int condition = 0;
// 	// condition = (queue_last->queue_id 	== ptr->queue_id) 	+
// 	//             (queue_last->qlen 		== ptr->qlen) 		+
// 	//             (queue_last->backlog 	== ptr->backlog) 	+
// 	//             (queue_last->drops 		== ptr->drops) 		+
// 	//             (queue_last->requeues 	== ptr->requeues) 	+
// 	//             (queue_last->overlimits == ptr->overlimits);
// 	// condition = 5;
// 	// if (condition != 6)
// 	// {
// 	// 	struct dq_t t;
// 	// 	memset(&t, 0, sizeof(struct dq_t));
// 	// 	t.category = QUEUE;
// 	// 	t.queue.queue_id 	= ptr->queue_id;
// 	// 	t.queue.bytes 		= ptr->bytes;
// 	// 	t.queue.packets 	= ptr->packets;
// 	// 	t.queue.qlen 		= ptr->qlen;
// 	// 	t.queue.backlog 	= ptr->backlog;
// 	// 	t.queue.drops 		= ptr->drops;
// 	// 	t.queue.requeues 	= ptr->requeues;
// 	// 	t.queue.overlimits 	= ptr->overlimits;
// 	// 	t.queue.sec 		= ptr->sec;
// 	// 	t.queue.usec		= ptr->usec;
// 	// 	memcpy(&(t.mac), mac_global, 6);
// 	// 	if(t.queue.sec > last_sec_queue)
// 	// 	{
// 	// 		write_2_public_buffer(&t, sizeof(struct dq_t));
// 	// 		last_sec_queue = t.queue.sec;
// 	// 		last_usec_queue = t.queue.usec;
// 	// 	}
// 	// 	else if ((t.queue.sec == last_sec_queue) && (t.queue.usec > last_usec_queue))
// 	// 	{
// 	// 		write_2_public_buffer(&t, sizeof(struct dq_t));
// 	// 		last_usec_queue = t.queue.usec;
// 	// 	}
		
// 	// 	*queue_last = *ptr;
// 	// }
// 	if(ptr->bytes != 0)
// 	{
// 		struct dq_t t;
// 		printk(KERN_DEBUG "sendudp %llu\n", ptr->bytes);
// 		memset(&t, 0, sizeof(struct dq_t));
// 		t.category = QUEUE;
// 		t.queue.queue_id 	= ptr->queue_id;
// 		t.queue.bytes 		= ptr->bytes;
// 		t.queue.packets 	= ptr->packets;
// 		t.queue.qlen 		= ptr->qlen;
// 		t.queue.backlog 	= ptr->backlog;
// 		t.queue.drops 		= ptr->drops;
// 		t.queue.requeues 	= ptr->requeues;
// 		t.queue.overlimits 	= ptr->overlimits;
// 		t.queue.sec 		= ptr->sec;
// 		t.queue.usec		= ptr->usec;
// 		memcpy(&(t.mac), mac_global, 6);
// 		write_2_public_buffer(&t, sizeof(struct dq_t));	
// 		*queue_last = *ptr;		
// 	}
// }

void compare_and_queue(void)
{
	int i = 0;
	struct data_queue *tmp = NULL, *pre = NULL;
	for(i = 0; i < 6; i++)
	{
		struct dq_t t;
		tmp = queue_global + i;
		pre = queue_previous + i;

		if(tmp->bytes == pre->bytes)
			continue;
		
		// printk(KERN_DEBUG "%u %llu %u %llu %u\n", tmp->queue_id, tmp->bytes, tmp->qlen, tmp->backlog,tmp->drops);
		memset(&t, 0, sizeof(struct dq_t));
		memcpy(&(t.mac), mac_global, 6);
		t.category = QUEUE;
		t.queue.queue_id 	= tmp->queue_id;
		t.queue.bytes 		= tmp->bytes;
		t.queue.packets 	= tmp->packets;
		t.queue.qlen 		= tmp->qlen;
		t.queue.backlog 	= tmp->backlog;
		t.queue.drops 		= tmp->drops;
		t.queue.requeues 	= tmp->requeues;
		t.queue.overlimits 	= tmp->overlimits;
		t.queue.sec 		= tmp->sec;
		t.queue.usec		= tmp->usec;
		*pre = *tmp;
		write_2_public_buffer(&t, sizeof(struct dq_t));
	}
}
// struct data_queue
// {
// 	__u32 queue_id;
// 	__u32 packets;
// 	__u64 bytes;
// 	__be32 sec;
// 	__be32 usec;
// 	__u32 qlen;
// 	__u32 backlog;
// 	__u32 drops;
// 	__u32 requeues;
// 	__u32 overlimits;
// 	__u32 pad;
// };
/**
 * Simillar as compare_and_iwlist
 * @Linger
 * @DateTime 2018-05-06T02:34:26-0500
 * @param    ptr                      [description]
 */
void compare_and_beacon(struct data_beacon *ptr)
{
	struct data_beacon *tmp = NULL;
	tmp = ptr;
	while (tmp)
	{
		struct db_t t;
		memset(&t, 0, sizeof(struct db_t));
		t.category = BEACON;
		memcpy(&(t.mac), mac_global, 6);
		t.beacon = *tmp;
		t.beacon.next = NULL;
		if(t.beacon.sec > last_sec_beacon)
		{
			write_2_public_buffer(&t, sizeof(struct db_t));
			last_sec_beacon = t.beacon.sec;
			last_usec_beacon = t.beacon.usec;
		}
		else if((t.beacon.sec == last_sec_beacon) && (t.beacon.usec > last_usec_beacon))
		{
			write_2_public_buffer(&t, sizeof(struct db_t));
			last_usec_beacon = t.beacon.usec;
		}
		tmp = tmp->next;
	}
	destroy_list_beacon(beacon_last);
	copy_last_beacon(ptr);
}

/**
 * Get network information from drivers.
 * @Linger
 * @DateTime 2018-05-06T02:35:33-0500
 */
void get_information(void)
{
	struct 	data_iw 	*iwlist = NULL;
	struct 	dp_packet 	*dp_mac = NULL;
	struct 	dp_packet 	*dp_que = NULL;
	// struct 	data_queue 	*queue 	= NULL;
	struct 	data_beacon *beacon = NULL;
	struct data_survey survey_linger;

	memset(queue_last, 0, sizeof(struct data_queue));

	control_beacon += 1;
	control_beacon = control_beacon % 10000000;
	// if ((control_beacon % 3) == 0)
	// {
	memset(&survey_linger, 0, sizeof(struct data_survey));
	iwlist 		= ieee80211_dump_station_linger(&survey_linger);
	// }
	if ((control_beacon % 10) == 0)
		beacon 		= get_beacon_global_linger();
	// queue 		= dump_class_stats_linger();
	dp_que 		= dpt_queue_linger();
	dp_mac 		= dpt_mac_linger();

	if (iwlist)
	{
		compare_and_iwlist(iwlist, &survey_linger);
		destroy_iwlist(iwlist);
	}

	if (dp_mac)
	{
		struct dp_packet *ptr = NULL;
		ptr = dp_mac;
		while (ptr)
		{
			struct dp_t dp_m_w;
			memset(&dp_m_w, 0, sizeof(struct dp_t));
			dp_m_w.drops = *ptr;
			dp_m_w.drops.next = NULL;
			dp_m_w.category = DROPS;
			memcpy(&(dp_m_w.mac), mac_global, 6);
			if(dp_m_w.drops.sec > last_sec_drop_mac)
			{
				write_2_public_buffer(&dp_m_w, sizeof(struct dp_t));
				last_sec_drop_mac = dp_m_w.drops.sec;
				last_usec_drop_mac = dp_m_w.drops.usec;
			}
			else if ((dp_m_w.drops.sec == last_sec_drop_mac) && (dp_m_w.drops.usec > last_usec_drop_mac))
			{
				write_2_public_buffer(&dp_m_w, sizeof(struct dp_t));
				last_usec_drop_mac = dp_m_w.drops.usec;
			}
			
			ptr = ptr->next;
		}
		destroy_after_transmitted_mac80211();
		// printk(KERN_DEBUG "destroy_after_transmitted_mac80211 in sendudp\n");
	}

	if (beacon)
	{
		compare_and_beacon(beacon);
	}

	// if (queue)
	// {
		// printk(KERN_DEBUG "before %llu\n", queue->bytes);
		compare_and_queue();
	// }

	if (dp_que)
	{
		struct dp_packet *ptr = NULL;
		ptr = dp_que;
		while (ptr)
		{
			struct dp_t dp_q_w;
			memset(&dp_q_w, 0, sizeof(struct dp_t));
			dp_q_w.drops = *ptr;
			dp_q_w.drops.next = NULL;
			dp_q_w.category = DROPS;
			memcpy(&(dp_q_w.mac), mac_global, 6);
			if(dp_q_w.drops.sec > last_sec_drop_queue)
			{
				write_2_public_buffer(&dp_q_w, sizeof(struct dp_t));
				last_sec_drop_queue = dp_q_w.drops.sec;
				last_usec_drop_queue = dp_q_w.drops.usec;
			}
			else if ((dp_q_w.drops.sec == last_sec_drop_queue) && (dp_q_w.drops.usec > last_usec_drop_queue))
			{
				write_2_public_buffer(&dp_q_w, sizeof(struct dp_t));
				last_usec_drop_queue = dp_q_w.drops.usec;
			}
			
			ptr = ptr->next;
		}
		destroy_after_transmitted_codel();
		// printk(KERN_DEBUG "destroy_after_transmitted_codel in sendudp\n");
	}
}

/**
 * Delete global variables and data sturctures when module exit.
 * @Linger
 * @DateTime 2018-05-06T02:39:12-0500
 */
void destroy_data(void)
{
	struct data_iw *pre_iw = NULL;
	struct data_iw *ptr_iw = NULL;
	struct data_queue *ptr_que = NULL;
	struct data_beacon *pre_beacon = NULL;
	struct data_beacon *ptr_beacon = NULL;

	ptr_iw = iwlist_last;
	while (ptr_iw)
	{
		pre_iw = ptr_iw;
		ptr_iw = ptr_iw->next;
		kfree(pre_iw);
		pre_iw = NULL;
	}

	ptr_que = queue_last;
	if (ptr_que)
	{
		kfree(ptr_que);
		ptr_que = NULL;
	}
	ptr_beacon = beacon_last;
	while (ptr_beacon)
	{
		pre_beacon = ptr_beacon;
		ptr_beacon = ptr_beacon->next;
		kfree(pre_beacon);
		pre_beacon = NULL;
	}
}


/**
 * Print iwlist members, used when debugging.
 * @Linger
 * @DateTime 2018-05-06T02:43:44-0500
 * @param    t                        iwlist address
 * @param    str                      extra informations.
 */
void print_iwlist(struct data_iw *t, char *str)
{
	struct data_iw *ptr = NULL;
	ptr = t;
	while (ptr)
	{
		printk(KERN_DEBUG "%s\t%u\n", str, ptr->rx_packets);
		ptr = ptr->next;
	}
}

static int hello_init(void)
{
	struct net_device *dev;
	char *ifname = "br0";
	queue_last 		= (struct data_queue *)kmalloc(sizeof(struct data_queue), GFP_KERNEL);

	printk(KERN_DEBUG "Hello, kernel %d %d %d %d", sizeof(struct data_iw), sizeof(struct data_iw *), sizeof(struct dp_packet), sizeof(struct data_beacon));
	sock_global = get_sock();

	dev = __dev_get_by_name(sock_net(sock_global->sk), ifname);
	if (dev)
	{
		memset(mac_global, 0, 6);
		memcpy(mac_global, dev->dev_addr, 6);
	}

	write_begin = 0;
	memset(&public_buffer, 0, B_S);

	init_timer(&tm);    //初始化内核定时器

	do_gettimeofday(&oldtv);        //获取当前时间
	tm.function = (void *)&callback;           //指定定时时间到后的回调函数
	// tm.data    = (unsigned long)"hello world";        //回调函数的参数
	tm.expires = jiffies + 1;       //定时时间
	add_timer(&tm);

	return 0;
}

static void hello_exit(void)
{
	if (sock_global)
	{
		sock_release(sock_global);
		sock_global = NULL;
	}
	del_timer(&tm);
	destroy_data();
	printk(KERN_ALERT "Goodbye,Cruel world\n");
}

int strcmp_linger(char *s, char *t)
{
	int i = 0;
	int condition = 0;
	for (i = 0; i < 6; i++)
	{
		int tmp = 1000;
		tmp = (int) * (s + i) - (int) * (t + i);
		if (tmp == 0)
			condition += 1;
	}
	return (condition == 6) ? 0 : 1;
}

module_init(hello_init);
module_exit(hello_exit);