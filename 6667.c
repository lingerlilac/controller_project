#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <linux/types.h>
#include <mysql/mysql.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "thread1.h"
#include "hash_lin.h"
#include "newstructures.h"

typedef _Bool bool;

#define SEND_SIZE 1024
#define POOL_SIZE 10240000
#define BUFFER_STORE 10000
int port = 6667;
/**
 * fp is the file pointer to write information in.
 */
FILE *fp = NULL;
/**
 * This buffer pool is used to store data from recvfrom function.
 */
char buffer_pool[POOL_SIZE];
/**
 * tail is the tail point of buffer_pool, i.e., where the new data wroten.
 * head is the head of buffer_pool, where next data to read.
 */
int tail = 0;
int head = 0;
int count1 = 0;
int count2 = 0;
int packet_count = 0;
int packet_count1 = 0;

/**
 * Structures and variables for APIs.
 */

struct data_beacon_processed *beacon_buffer[BUFFER_STORE];
struct data_iw_processed *iw_buffer[BUFFER_STORE];
// struct data_queue_processed     *queue_buffer[BUFFER_STORE];
struct data_winsize_processed *winsize_buffer[BUFFER_STORE];
struct data_dropped_processed *dropped_buffer[BUFFER_STORE];
struct data_survey_processed *survey_buffer[BUFFER_STORE];
int bbstart = 0, bbend = 0, bbamount = 0, bbstart1 = 0, bbamount1 = 0;
int ibstart = 0, ibend = 0, ibamount = 0, ibstart1 = 0, ibamount1 = 0;
int dbstart = 0, dbend = 0, dbamount = 0, dbstart1 = 0, dbamount1 = 0;
int wbstart = 0, wbend = 0, wbamount = 0, wbstart1 = 0, wbamount1 = 0;
int sbstart = 0, sbend = 0, sbamount = 0;

/**
 * for debugging.
 */
struct timeval start, end, start1, end1;
suseconds_t msec_1 = 0, msec_2 = 0;
time_t sec1 = 0, sec2 = 0;

void *receive (void *arg);
void process (char *str, int length);
void decode_winsize (char *buf);
void decode_drops (char *buf);
void decode_iw (char *buf);
void decode_queue (char *buf);
void decode_beacon (char *buf);
void decode_survey (char *buf);
void print_mac (char *addr);
void print_string (char *str, int length);
void f_print_string (char *str, int length);
int strcmp_linger (char *s, char *t);
void *g_mes_tn_process (void *arg);
void f_write_string (char *str, int length);

void destroy (void);
void start_program (void);
void destroy_everything (void);
void start_threads (void);
void test_print (void);

/**
 * APIs
 */
int get_aps (struct aplist *ap);
void *get_neighbour_py (char *ap, struct aplist_py *neibours);
void *get_neighbour (char *ap, struct aplist *neibours);
void *get_clients (char *ap, struct aplist *clients);
void *get_clients_py (char *ap, struct aplist_py *clients);
void *get_flows (char *ap, struct flowlist *flowlists);
void *get_flow_drops (char *ap, struct flow_drop_ap *flow_drops);
bool get_wireless_info (char *ap, struct wireless_information_ap *clients);

pthread_mutex_t mutex_pool;
pthread_mutex_t mutex_data;

/**
 * [receive description]
 * @author linger 2018-04-13
 * @param  arg [*arg is used for pthread create, there is no meaning for it.]
 * @return     [void]
 */
void *
receive (void *arg)
{
	int sin_len;
	char message[SEND_SIZE];
	int sock;
	struct sockaddr_in sin;

	bzero (&sin, sizeof (sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl (INADDR_ANY);
	sin.sin_port = htons (port);
	sin_len = sizeof (sin);

	sock = socket (AF_INET, SOCK_DGRAM, 0);
	bind (sock, (struct sockaddr *) &sin, sizeof (sin));
	while (1)
	{
		while (1)
		{
			int len = 0;
			memset (message, 0, SEND_SIZE);
			len =
			    recvfrom (sock, message, SEND_SIZE, 0, (struct sockaddr *) &sin,
			              &sin_len);
			if (len <= 0)
			{
				printf ("receive error\n");
				continue;
			}
			// printf("length is %d\n", len);
			// f_print_string(message, 1024);
			// process(message, len);
			packet_count += 1;
			pthread_mutex_lock (&mutex_pool);
			memcpy (buffer_pool + tail, message, SEND_SIZE);
			tail = tail + SEND_SIZE;
			tail = tail % POOL_SIZE;
			pthread_mutex_unlock (&mutex_pool);
		}
		// printf("xxxs\n");
		usleep (1);
	}

	close (sock);
	// return (EXIT_SUCCESS);
}

/**
 * g_mes_tn_process - Get string from buffer_pool and then pass it to
 * process function to process.
 */
void *
g_mes_tn_process (void *arg)
{
	char buffer[SEND_SIZE];
	int ulen = 0;

	while (1)
	{
		ulen = (tail + POOL_SIZE - head) % POOL_SIZE;
		while (ulen > 1)
		{
			memset (buffer, 0, SEND_SIZE);
			pthread_mutex_lock (&mutex_pool);
			memcpy (buffer, buffer_pool + head, SEND_SIZE);
			head = (head + SEND_SIZE) % POOL_SIZE;
			pthread_mutex_unlock (&mutex_pool);
			ulen = (tail + POOL_SIZE - head) % POOL_SIZE;
			// f_print_string(buffer, 1024);
			process (buffer, 1024);
			packet_count1 += 1;
			if((packet_count1 % 1000) == 0)
				printf("packet received is %d\n", packet_count1);
			// gettimeofday(&start, NULL);

			// gettimeofday(&end, NULL);
			// msec_1 = end.tv_usec - start.tv_usec;
			// sec1 - end.tv_sec - start.tv_sec;
			// gettimeofday(&start1, NULL);
			// printf("xxxxya\n");
			// f_write_string(buffer, 1024);

			// gettimeofday(&end1, NULL);
			// sec2 = end1.tv_sec - start1.tv_sec;
			// msec_2 = end1.tv_usec - start1.tv_usec;
			// printf("time of fprintf is %u %u, fwrite %u %u\n", sec1, msec_1, sec2, msec_2);
			// process(buffer);
			// printf("hhhh\n");
		}
		usleep (1);
	}
}

/**
 * a switch function to process different messages. The receive function receive 1024 bits message and
 * send to process function, this function first scan the message type by memcpy a u16 value, based on
 * this value, the message types can be derived, then corresponding function are called to process this
 * message.
 * @author linger 2018-04-13
 * @param  str [description]
 */
void
process (char *buffer, int length)
{
	__u16 value;
	enum data_category value_enum;
	int offset = 0;

	// struct timeval start, end, start1, end1;
	// suseconds_t msec_1 = 0, msec_2 = 0;
	// time_t sec1 = 0, sec2 = 0;

	while (offset < length)
	{
		int out = 0;
		memcpy (&value, buffer + offset, sizeof (__u16));
		offset = offset + 2;
		reversebytes_uint16t (&value);
		// if((value == 5) || (value == 8))
		// printf("value is %u %02x\n", value, value);
		value_enum = (enum data_category) value;
		switch (value_enum)
		{
		case WINSIZE:
			decode_winsize (buffer + offset);
			offset += sizeof (struct data_winsize) + 6;
			break;
		case DROPS:
			decode_drops (buffer + offset);
			offset += sizeof (struct dp_packet) + 6;
			break;
		case IW:
			decode_iw (buffer + offset);
			offset += sizeof (struct data_iw) + 6;
			break;
		case QUEUE:
			decode_queue (buffer + offset);
			offset += sizeof (struct data_queue) + 6;
			break;
		case BEACON:
			decode_beacon (buffer + offset);
			offset += sizeof (struct data_beacon) + 6;
			break;
		case SURVEY:
			decode_survey (buffer + offset);
			offset += sizeof (struct data_survey) + 6;
			break;
		default:
			out = 1;
			break;
		}
		if (out == 1)
			break;
	}
}

/**
 * [decode_winsize description]
 * After the packet informations are decoded from buffer, A TCP state-machine are mantained by a hash
 * structure, a tcp flow with no updates of two minutes are deleted.
 * @author linger 2018-04-13
 * @param  buf [buffer that cantains winsize data]
 */
void
decode_winsize (char *buf)
{
	struct data_winsize rdata;
	// char fordebuging[200];

	int length = sizeof (struct data_winsize);
	int res, kind, wscale, cal_windowsize;
	__u64 time;
	__be64 timex = 0;

	__u32 wanip;
	// char insert_data[600];
	char mac[6];
	// int condition = 0;

	char mac_addr[18], mac_addr_origin[6], eth_src[18], eth_dst[18];
	unsigned char *ptr_uc = NULL;
	char ip_src[20];
	char ip_dst[20];

	char *str = NULL;


	gettimeofday (&start, NULL);

	str =
	    malloc (18);


	memset (ip_src, 0, 20);
	memset (ip_dst, 0, 20);
	memset (mac, 0, 6);
	memset (&rdata, 0, length);
	memcpy (mac, buf, 6);
	memcpy (&rdata, buf + 6, length);

	// condition =(int)mac[0] + (int)mac[1] + (int)mac[2] + (int)mac[3] + (int)mac[4] + (int)mac[5];
	// if(condition == 0)
	// {
	//      printf("condition is zero\n");
	//      print_mac(mac);
	// }

	// ptr_uc = (unsigned char *)malloc(sizeof(unsigned char));

	reversebytes_uint32t (&rdata.datalength);
	reversebytes_uint32t (&rdata.ip_src);
	reversebytes_uint32t (&rdata.ip_dst);
	reversebytes_uint16t (&rdata.sourceaddr);
	reversebytes_uint16t (&rdata.destination);
	reversebytes_uint32t (&rdata.sequence);
	reversebytes_uint32t (&rdata.ack_sequence);
	reversebytes_uint16t (&rdata.flags);
	reversebytes_uint16t (&rdata.windowsize);
	reversebytes_uint32t (&rdata.sec);
	reversebytes_uint32t (&rdata.usec);
	timex = rdata.sec;
	timex = timex * 1000000;
	timex += rdata.usec;

	memset (mac_addr, 0, sizeof (mac_addr));
	memset (mac_addr_origin, 0, 6);
	mac_tranADDR_toString_r (mac, mac_addr, 18);
	memcpy (mac_addr_origin, mac, 6);

	mac_tranADDR_toString_r (rdata.eth_src, eth_src, 18);
	mac_tranADDR_toString_r (rdata.eth_dst, eth_dst, 18);
	ptr_uc = (unsigned char *) &rdata.ip_src;
	sprintf (ip_src, "%u.%u.%u.%u", ptr_uc[3], ptr_uc[2], ptr_uc[1], ptr_uc[0]);
	ptr_uc = (unsigned char *) &rdata.ip_dst;
	sprintf (ip_dst, "%u.%u.%u.%u", ptr_uc[3], ptr_uc[2], ptr_uc[1], ptr_uc[0]);
	kind = (int) (rdata.wscale[0]);
	length = (int) rdata.wscale[1];
	wscale = (int) rdata.wscale[2];
	rdata.flags = rdata.flags & 0x0017;
	cal_windowsize = rdata.windowsize;


	if (rdata.flags == 2 || rdata.flags == 18)
	{
		memcpy(str, mac_addr_origin, 6);
		memcpy(str + 6, &rdata.ip_src, 4);
		memcpy(str + 10, &rdata.ip_dst, 4);
		memcpy(str + 14, &rdata.sourceaddr, 2);
		memcpy(str + 16, &rdata.destination, 2);
		time = getcurrenttime ();
		hash_table_insert (str, wscale, time);
	}
	else if (rdata.flags == 17 || rdata.flags & 0x0004 == 1)
	{
		memcpy(str, mac_addr_origin, 6);
		memcpy(str + 6, &rdata.ip_src, 4);
		memcpy(str + 10, &rdata.ip_dst, 4);
		memcpy(str + 14, &rdata.sourceaddr, 2);
		memcpy(str + 16, &rdata.destination, 2);
		if (hash_table_lookup (str) != NULL)
		{
			hash_table_remove (str);
		}
		memcpy(str, mac_addr_origin, 6);
		memcpy(str + 6, &rdata.ip_dst, 4);
		memcpy(str + 10, &rdata.ip_src, 4);
		memcpy(str + 14, &rdata.destination, 2);
		memcpy(str + 16, &rdata.sourceaddr, 2);
		if (hash_table_lookup (str) != NULL)
		{
			hash_table_remove (str);
		}
	}

	else if (rdata.flags == 16)
	{
		time = getcurrenttime ();
		memcpy(str, mac_addr_origin, 6);
		memcpy(str + 6, &rdata.ip_dst, 4);
		memcpy(str + 10, &rdata.ip_src, 4);
		memcpy(str + 14, &rdata.destination, 2);
		memcpy(str + 16, &rdata.sourceaddr, 2);
		if (hash_table_lookup (str) != NULL)
		{
			cal_windowsize =
			    rdata.windowsize << hash_table_lookup (str)->nValue;
			hash_table_lookup (str)->time = time;
		}
		else
		{
			time = getcurrenttime ();
			hash_table_insert (str, wscale, time);
		}
	}
	fprintf (fp,
	         "%d, %lld, %.18s, %.18s, %.18s, %s, %s, %u, %u, %u, %u, %u, %u, %llu, %u, %u, %u, %u, %u\n",
	         WINSIZE, timex, mac_addr, eth_src, eth_dst,
	         ip_src, ip_dst, rdata.sourceaddr, rdata.destination,
	         rdata.sequence, rdata.ack_sequence, rdata.windowsize,
	         cal_windowsize, rdata.datalength, rdata.flags, kind,
	         length, wscale);
	// printf("%s\n", fordebuging);

	free (str);
	str = NULL;

	/**
	 * Follows are for APIs.
	 */
	pthread_mutex_lock(&mutex_data);
	struct data_winsize_processed *tmp;
	tmp = winsize_buffer[wbend];

	tmp->ip_src = rdata.ip_src;
	tmp->ip_dst = rdata.ip_dst;
	tmp->sourceaddr = rdata.sourceaddr;
	tmp->destination = rdata.destination;
	tmp->sequence = rdata.sequence;
	tmp->ack_sequence = rdata.ack_sequence;
	tmp->flags = rdata.flags;
	tmp->windowsize = rdata.windowsize;
	tmp->sec = rdata.sec;
	tmp->usec = rdata.usec;
	tmp->length = length;
	tmp->kind = kind;

	strcpy (tmp->mac_addr, mac_addr);
	strcpy (tmp->eth_src, eth_src);
	strcpy (tmp->eth_dst, eth_dst);

	tmp->kind = kind;
	tmp->length = length;

	wbamount = wbamount + 1;
	wbamount1 = wbamount1 + 1;
	wbend = wbend + 1;
	wbend = wbend % BUFFER_STORE;

	if (wbamount > BUFFER_STORE)
	{
		wbstart = wbend + 1;
		wbstart = wbstart % BUFFER_STORE;
		wbamount = BUFFER_STORE;
	}
	if (wbamount1 > BUFFER_STORE)
	{
		wbstart1 = wbend + 1;
		wbstart1 = wbstart1 % BUFFER_STORE;
		wbamount1 = BUFFER_STORE;
	}
	pthread_mutex_unlock(&mutex_data);
	// gettimeofday(&end, NULL);
	// msec_1 = end.tv_usec - start.tv_usec;
	// sec1 - end.tv_sec - start.tv_sec;
	// printf("Time %u:%u\n", sec1, msec_1);
	// exit(-1);
}

/**
 * [decode_drops description]
 * Processing the dropped packets informations, also informatins based on tcp flow are recorded for
 * further computing, a typical application is "get_flow_drops".
 * @author linger 2018-04-13
 * @param  buf [description]
 */
void
decode_drops (char *buf)
{

	struct dp_packet rdata;
	// struct dp_packet rd;
	int offset = 0;
	int length = sizeof (struct dp_packet);

	unsigned char *ptr_uc = NULL;
	char ip_src[20];
	char ip_dst[20];
	char srcip[20];
	char mac_addr[18];
	char mac[6];
	__be64 timex = 0;

	memset (ip_src, 0, 20);
	memset (ip_dst, 0, 20);
	memset (srcip, 0, 20);
	memset (mac, 0, 6);
	memset (mac_addr, 0, 18);
	memset (&rdata, 0, length);
	// memset(&rd, 0, sizeof(struct dp_packet));
	memcpy (mac, buf, 6);
	memcpy (&rdata, buf + 6, length);

	// ptr_uc = malloc(sizeof(__be32));
	reversebytes_uint32t (&rdata.sec);
	reversebytes_uint32t (&rdata.usec);
	reversebytes_uint32t (&rdata.in_time);
	reversebytes_uint32t (&rdata.sequence);
	reversebytes_uint32t (&rdata.ack_sequence);
	reversebytes_uint32t (&rdata.ip_src);
	reversebytes_uint32t (&rdata.ip_dst);
	reversebytes_uint16t (&rdata.port_src);
	reversebytes_uint16t (&rdata.port_dst);
	reversebytes_uint16t (&rdata.dpl);
	reversebytes_uint32t (&rdata.drop_count);

	timex = rdata.sec;
	timex = timex * 1000000;
	timex += rdata.usec;
	// dpnew_to_dp(&rdata, &rd);
	mac_tranADDR_toString_r (mac, mac_addr, 18);
	ptr_uc = (unsigned char *) &rdata.ip_src;
	sprintf (ip_src, "%u.%u.%u.%u", ptr_uc[3], ptr_uc[2], ptr_uc[1], ptr_uc[0]);
	memcpy (srcip, ip_src, 20);
	// ptr_uc = NULL;
	ptr_uc = (unsigned char *) &rdata.ip_dst;
	sprintf (ip_src, "%u.%u.%u.%u", ptr_uc[3], ptr_uc[2], ptr_uc[1], ptr_uc[0]);
	fprintf (fp, "%d, %lld, %.18s, %s, %s, %u, %u, %u, %u, %u, %u, %u\n", DROPS, timex, mac_addr,
	         srcip, ip_src, rdata.port_src, rdata.port_dst, rdata.sequence,
	         rdata.ack_sequence, rdata.in_time, rdata.dpl, rdata.drop_count);

	pthread_mutex_lock(&mutex_data);
	struct data_dropped_processed *tmp;
	tmp = dropped_buffer[dbend];
	strcpy (tmp->mac_addr, mac_addr);
	tmp->ip_src = rdata.ip_src;
	tmp->ip_dst = rdata.ip_dst;
	tmp->port_src = rdata.port_src;
	tmp->port_dst = rdata.port_dst;
	tmp->sequence = rdata.sequence;
	tmp->ack_sequence = rdata.ack_sequence;
	tmp->drop_count = rdata.drop_count;
	tmp->dpl = rdata.dpl;
	tmp->in_time = rdata.in_time;

	dbamount = dbamount + 1;
	dbamount1 = dbamount1 + 1;
	dbend = dbend + 1;
	dbend = dbend % BUFFER_STORE;
	if (dbamount > BUFFER_STORE)
	{
		dbstart = dbend + 1;
		dbstart = dbstart % BUFFER_STORE;
		dbamount = BUFFER_STORE;
	}
	if (dbamount1 > BUFFER_STORE)
	{
		dbstart1 = dbend + 1;
		dbstart1 = dbstart1 % BUFFER_STORE;
		dbamount1 = BUFFER_STORE;
	}
	pthread_mutex_unlock(&mutex_data);
}

/**
 * [decode_iw description]
 * Decoding wireless information of data links from buffer. Also, a list of received links informations
 * are stored. Based on this list, the information of links can be derived.
 * @author linger 2018-04-13
 * @param  buf [description]
 */
void
decode_iw (char *buf)
{
	struct data_iw rdata;

	int length = sizeof (struct data_iw);
	__u32 expected_throughput_tmp;
	float expected_throughput;
	char station[18];
	char mac_addr[18];
	char mac[6];
	__be64 timex = 0;
	// struct data_iw old;

	memset (mac, 0, 6);
	// memset(&old, 0, sizeof(struct data_iw));
	memset (&rdata, 0, length);
	memcpy (mac, buf, 6);
	memcpy (&rdata, buf + 6, length);
	memset (station, 0, 18);
	memset (mac_addr, 0, 18);

	reversebytes_uint32t (&rdata.sec);
	reversebytes_uint32t (&rdata.usec);
	reversebytes_uint16t (&rdata.device);
	reversebytes_uint32t (&rdata.inactive_time);
	reversebytes_uint32t (&rdata.rx_bytes);
	reversebytes_uint32t (&rdata.rx_packets);
	reversebytes_uint32t (&rdata.tx_bytes);
	reversebytes_uint32t (&rdata.tx_packets);
	reversebytes_uint32t (&rdata.tx_retries);
	reversebytes_uint32t (&rdata.tx_failed);
	reversebytes_uint32t (&rdata.signal);
	reversebytes_uint32t (&rdata.signal_avg);
	reversebytes_uint32t (&rdata.expected_throughput);
	timex = rdata.sec;
	timex = timex * 1000000;
	timex += rdata.usec;
	expected_throughput_tmp = rdata.expected_throughput;
	expected_throughput = (float) expected_throughput_tmp / 1000.0;
	mac_tranADDR_toString_r (rdata.station, station, 18);

	mac_tranADDR_toString_r (mac, mac_addr, 18);
	fprintf (fp,
	         "%d, %lld, %.18s, %.18s, %u, %u, %u, %u, %u, %u, %u, %u, %d, %d, %f\n",
	         IW, timex, mac_addr, station, rdata.device, rdata.inactive_time,
	         rdata.rx_bytes, rdata.rx_packets, rdata.tx_bytes, rdata.tx_packets,
	         rdata.tx_retries, rdata.tx_failed, rdata.signal, rdata.signal_avg,
	         expected_throughput);
	// printf("IW: %.18s, %u, %u, %u, %u, %u, %u, %u, %u, %d, %d, %f\n",station, rdata.device, rdata.inactive_time, rdata.rx_bytes, rdata.rx_packets, rdata.tx_bytes, rdata.tx_packets, rdata.tx_retries, rdata.tx_failed, rdata.signal, rdata.signal_avg, expected_throughput);

	/**
	 * Followings are for APIs.
	 * ibamount for gettimex, _clients	 */
	pthread_mutex_lock(&mutex_data);
	struct data_iw_processed *tmp;
	tmp = iw_buffer[ibend];
	strcpy (tmp->mac_addr, mac_addr);
	strcpy (tmp->station, station);
	tmp->device = rdata.device;
	tmp->inactive_time = rdata.inactive_time;
	tmp->rx_bytes = rdata.rx_bytes;
	tmp->rx_packets = rdata.rx_packets;
	tmp->tx_bytes = rdata.tx_bytes;
	tmp->tx_packets = rdata.tx_packets;
	tmp->tx_retries = rdata.tx_retries;
	tmp->tx_failed = rdata.tx_failed;
	tmp->signal = rdata.signal;
	tmp->signal_avg = rdata.signal_avg;
	tmp->expected_throughput = expected_throughput;

	// tmp->noise = rdata.noise;
	// tmp->active_time = rdata.active_time;
	// tmp->connected_time = rdata.connected_time;
	// tmp->busy_time = rdata.busy_time;
	// tmp->receive_time = rdata.receive_time;
	// tmp->transmit_time = rdata.transmit_time;
	ibamount = ibamount + 1;
	ibamount1 = ibamount1 + 1;
	ibend = ibend + 1;
	ibend = ibend % BUFFER_STORE;
	if (ibamount > BUFFER_STORE)
	{
		ibstart = ibend + 1;
		ibstart = ibstart % BUFFER_STORE;
		ibamount = BUFFER_STORE;
	}
	if (ibamount1 > BUFFER_STORE)
	{
		ibstart1 = ibend + 1;
		ibstart1 = ibstart1 % BUFFER_STORE;
		ibamount1 = BUFFER_STORE;
	}
	pthread_mutex_unlock(&mutex_data);
}

/**
 * [decode_queue description]
 * Decoding queue informations.
 * @author linger 2018-04-13
 * @param  buf [description]
 */
void
decode_queue (char *buf)
{

	// int queue_id;

	struct data_queue rdata;
	int length = sizeof (struct data_queue);
	char mac_addr[18];
	char mac[6];
	__be64 timex = 0;

	memset (&rdata, 0, length);
	memset (mac, 0, 6);
	memset (mac_addr, 0, 18);
	memcpy (mac, buf, 6);
	memcpy (&rdata, buf + 6, length);

	reversebytes_uint32t (&rdata.sec);
	reversebytes_uint32t (&rdata.usec);
	reversebytes_uint64t (&rdata.bytes);
	reversebytes_uint32t (&rdata.queue_id);
	reversebytes_uint32t (&rdata.packets);
	reversebytes_uint32t (&rdata.qlen);
	reversebytes_uint32t (&rdata.backlog);
	reversebytes_uint32t (&rdata.drops);
	reversebytes_uint32t (&rdata.requeues);
	reversebytes_uint32t (&rdata.overlimits);
	timex = rdata.sec;
	timex = timex * 1000000;
	timex += rdata.usec;
	mac_tranADDR_toString_r (mac, mac_addr, 18);
	fprintf (fp, "%d, %lld, %.18s, %u, %llu, %u, %u, %u, %u, %u, %u\n", QUEUE, timex,
	         mac_addr, rdata.queue_id, rdata.bytes, rdata.packets, rdata.qlen,
	         rdata.backlog, rdata.drops, rdata.requeues, rdata.overlimits);
	// printf("Queue: %u, %llu, %u, %u, %u, %u, %u, %u\n", rdata.queue_id, rdata.bytes, rdata.packets, rdata.qlen, rdata.backlog, rdata.drops, rdata.requeues, rdata.overlimits);
}

/**
 * [decode_beacon description]
 * Based on beacon information, the informaiton of neighbours can be derived: how many neighbours, their interferences,
 * the frequency usage informations and so on.
 * @author linger 2018-04-13
 * @param  buf [description]
 */
void
decode_beacon (char *buf)
{
	int signal;
	char channel_type[] = "802.11a";
	struct data_beacon beacon;
	int length = sizeof (struct data_beacon);
	char bssid[18];
	char mac_addr[18];
	char mac[6];
	__be64 timex = 0;

	memset (&beacon, 0, length);
	memset (bssid, 0, 18);
	memset (mac_addr, 0, 18);
	memset (mac, 0, 6);
	memcpy (mac, buf, 6);
	memcpy (&beacon, buf + 6, length);

	// beacon_new_2_old(&old, &beacon);

	// reversebytes_uint16t(&beacon.signal);
	// reversebytes_uint16t(&beacon.data_rate);
	reversebytes_uint32t (&beacon.sec);
	reversebytes_uint32t (&beacon.usec);
	reversebytes_uint16t (&beacon.freq);
	mac_tranADDR_toString_r (beacon.bssid, bssid, 18);
	mac_tranADDR_toString_r (mac, mac_addr, 18);
	timex = beacon.sec;
	timex = timex * 1000000;
	timex += beacon.usec;
	fprintf (fp, "%d, %lld, %.18s, %llu, %u, %u, %d, %.18s\n", BEACON, timex, mac_addr,
	         beacon.data_rate, beacon.freq, beacon.signal, bssid);
	/**
	 * The following code copy processed beacon information to a buffer, APIs can use these informations
	 * to drive their intresting values.
	 * beacon_buffer is the buffer;
	 * bbamount is different from bbamount: bbamount for get_aps API;
	 * bbamount1 is for get_neibours series.
	 */
	pthread_mutex_lock(&mutex_data);
	struct data_beacon_processed *tmp;

	tmp = beacon_buffer[bbend];
	strcpy (tmp->mac_addr, mac_addr);
	strcpy (tmp->bssid, bssid);

	tmp->data_rate = beacon.data_rate;
	tmp->signal = beacon.signal;
	tmp->timein = beacon.timein;

	bbamount = bbamount + 1;
	bbamount1 = bbamount1 + 1;
	bbend = bbend + 1;
	bbend = bbend % BUFFER_STORE;

	if (bbamount > BUFFER_STORE)
	{
		bbstart = bbend + 1;
		bbstart = bbstart % BUFFER_STORE;
		bbamount = BUFFER_STORE;
	}

	if (bbamount1 > BUFFER_STORE)
	{
		bbstart1 = bbend + 1;
		bbstart1 = bbstart1 % BUFFER_STORE;
		bbamount1 = BUFFER_STORE;
	}
	pthread_mutex_unlock(&mutex_data);

}

/**
 * [decode_survey description]
 * Processing wireless information of ap, also, the information of received aps are stored in a list, based on this list,
 * wireless information of ap can be derived.
 * @author linger 2018-04-13
 * @param  buf [description]
 */
void
decode_survey (char *buf)
{
	struct data_survey rdata;
	int length = sizeof (struct data_survey);
	char mac[6];
	char mac_addr[18];
	__be64 timex = 0;

	memset (mac, 0, 6);
	memset (mac_addr, 0, 18);
	memcpy (mac, buf, 6);
	memset (&rdata, 0, length);
	memcpy (&rdata, buf + 6, length);

	reversebytes_uint32t (&rdata.sec);
	reversebytes_uint32t (&rdata.usec);
	reversebytes_uint64t (&rdata.time);
	reversebytes_uint64t (&rdata.time_busy);
	reversebytes_uint64t (&rdata.time_ext_busy);
	reversebytes_uint64t (&rdata.time_rx);
	reversebytes_uint64t (&rdata.time_tx);
	reversebytes_uint64t (&rdata.time_scan);
	reversebytes_uint32t (&rdata.filled);
	reversebytes_uint16t (&rdata.center_freq);
	mac_tranADDR_toString_r (mac, mac_addr, 18);
	timex = rdata.sec;
	timex = timex * 1000000;
	timex += rdata.usec;
	fprintf (fp, "%d, %lld, %.18s, %llu, %llu, %llu, %llu, %llu, %llu, %u, %d\n",
	         SURVEY, timex, mac_addr, rdata.time, rdata.time_busy, rdata.time_ext_busy,
	         rdata.time_rx, rdata.time_tx, rdata.time_scan, rdata.center_freq,
	         rdata.noise);
	pthread_mutex_lock(&mutex_data);
	struct data_survey_processed *tmp;

	tmp = survey_buffer[sbend];
	strcpy (tmp->mac_addr, mac_addr);
	tmp->time = rdata.time;
	tmp->time_busy = rdata.time_busy;
	tmp->time_ext_busy = rdata.time_ext_busy;
	tmp->time_rx = rdata.time_rx;
	tmp->time_tx = rdata.time_tx;
	tmp->time_scan = rdata.time_scan;
	tmp->noise = rdata.noise;
	tmp->center_freq = rdata.center_freq;


	sbamount = sbamount + 1;
	sbend = sbend + 1;
	sbend = sbend % BUFFER_STORE;
	if (sbamount > BUFFER_STORE)
	{
		sbstart = sbend + 1;
		sbstart = sbstart % BUFFER_STORE;
		sbamount = BUFFER_STORE;
	}
	pthread_mutex_unlock(&mutex_data);
}

void
start_threads (void)
{
	pthread_t tid1, tid2, tid3;
	int err = 0, i = 0;

    char buf[128];
    char *ptr = NULL;
    char appendix[] = "_other.txt";
    FILE *pp;

    if ( (pp = popen("date", "r")) == NULL )
    {
        printf("popen() error!/n");
        exit(1);
    }

    while (fgets(buf, sizeof buf, pp))
    {
        // printf("%s", buf);
    }
    ptr = buf;
    while(*ptr != '\0')
    {
        // printf("%s ", ptr);
        if(*ptr == ' ')
            *ptr = '_';
        ptr += 1;
    }
    ptr -= 1;
    if((ptr + strlen(appendix)) < (buf + 128))
    {
        memcpy(ptr, appendix, strlen(appendix));
    }
    ptr = ptr + strlen(appendix);
    *ptr = '\0';
    // strcat(buf, "txt");
    printf("%s\n", buf);
    pclose(pp);
	// int amount = 0;

	// struct aplist mac_test;
	// memset(&mac_test, 0, sizeof(struct aplist));

	// amount = get_aps (&mac_test);
	// printf ("amount is %d\n", amount);

	pthread_mutex_init (&mutex_pool, NULL);
	pthread_mutex_init (&mutex_data, NULL);
	if (fp = fopen (buf, "wb"))
		puts ("file open successfully");
	else
		puts ("file opened failed");
	hash_table_init ();
	memset (buffer_pool, 0, POOL_SIZE);

	for (i = 0; i < BUFFER_STORE; i++)
	{
		iw_buffer[i] =
		    (struct data_iw_processed *)
		    malloc (sizeof (struct data_iw_processed));
		beacon_buffer[i] =
		    (struct data_beacon_processed *)
		    malloc (sizeof (struct data_beacon_processed));
		// queue_buffer[i]     = (struct data_queue_processed *)   malloc(sizeof(struct data_queue_processed));
		dropped_buffer[i] =
		    (struct data_dropped_processed *)
		    malloc (sizeof (struct data_dropped_processed));
		winsize_buffer[i] =
		    (struct data_winsize_processed *)
		    malloc (sizeof (struct data_winsize_processed));
		survey_buffer[i] =
		    (struct data_survey_processed *)
		    malloc (sizeof (struct data_survey_processed));
	}

	err = pthread_create (&tid1, NULL, receive, NULL);
	if (err != 0)
	{
		printf ("receive creation failed \n");
		exit (1);
	}

	err = pthread_create (&tid2, NULL, g_mes_tn_process, NULL);
	if (err != 0)
	{
		printf ("g_mes_tn_process creation failed \n");
		exit (1);
	}
	else
	{
		printf ("23\n");
	}

}


void
destroy_everything (void)
{
	int i = 0;
	pthread_mutex_destroy (&mutex_pool);
	pthread_mutex_destroy (&mutex_data);
	pthread_exit (0);
	fclose (fp);
	hash_table_release ();

	for (i = 0; i < BUFFER_STORE; i++)
	{
		free (iw_buffer[i]);
		iw_buffer[i] = NULL;

		free (beacon_buffer[i]);
		beacon_buffer[i] = NULL;

		// free(queue_buffer[i]);
		// queue_buffer[i]     = NULL;

		free (dropped_buffer[i]);
		dropped_buffer[i] = NULL;

		free (winsize_buffer[i]);
		winsize_buffer[i] = NULL;

		free (survey_buffer[i]);
		survey_buffer[i] = NULL;
	}
}

void
start_program (void)
{
	// printf("here111\n");
	start_threads ();
	// return 0;
}

void
destroy (void)
{
	destroy_everything ();
}

int
main ()
{
	/* code */
	start_program ();
	destroy ();
	return 0;
}

// int main()
// {
//      pthread_t tid1, tid2, tid3;
//      int err = 0, i = 0;
//     pthread_mutex_init(&mutex_pool, NULL);
//     if(fp=fopen("debug.txt","wb"))
//         puts("file open successfully");
//     else
//         puts("file opened failed");
//     hash_table_init();
//      memset(buffer_pool, 0, POOL_SIZE);

//     for (i = 0; i < BUFFER_STORE; i++)
//     {
//         iw_buffer[i]        = (struct data_iw_processed *)      malloc(sizeof(struct data_iw_processed));
//         beacon_buffer[i]    = (struct data_beacon_processed *)  malloc(sizeof(struct data_beacon_processed));
//         // queue_buffer[i]     = (struct data_queue_processed *)   malloc(sizeof(struct data_queue_processed));
//         dropped_buffer[i]   = (struct data_dropped_processed *) malloc(sizeof(struct data_dropped_processed));
//         winsize_buffer[i]   = (struct data_winsize_processed *) malloc(sizeof(struct data_winsize_processed));
//         survey_buffer[i]   = (struct data_survey_processed *) malloc(sizeof(struct data_survey_processed));
//     }

//     err = pthread_create(&tid1, NULL, receive, NULL);
//     if(err != 0)
//     {
//         printf("receive creation failed \n");
//         exit(1);
//     }

//   err = pthread_create(&tid2, NULL, g_mes_tn_process, NULL);
//   if(err != 0)
//   {
//     printf("g_mes_tn_process creation failed \n");
//     exit(1);
//   }
//   else
//   {
//     printf("23\n");
//   }
//   // err = pthread_create(&tid3, NULL, receive6683, NULL);
//   // if(err != 0)
//   // {
//   //   printf("receive6683%d creation failed \n", i);
//   //   exit(1);
//   // }
//     // fclose(fp);
//     pthread_mutex_destroy(&mutex_pool);
//     pthread_exit(0);
//     fclose(fp);
//     hash_table_release();

//     for (i= 0; i< BUFFER_STORE; i++)
//     {
//         free(iw_buffer[i]);
//         iw_buffer[i]        = NULL;

//         free(beacon_buffer[i]);
//         beacon_buffer[i]    = NULL;

//         // free(queue_buffer[i]);
//         // queue_buffer[i]     = NULL;

//         free(dropped_buffer[i]);
//         dropped_buffer[i]   = NULL;

//         free(winsize_buffer[i]);
//         winsize_buffer[i]   = NULL;

//         free(survey_buffer[i]);
//         survey_buffer[i] = NULL;
//     }
// }

void
print_mac (char *addr)
{
	int len = 18;
	char str[18];
	mac_tranADDR_toString_r (addr, str, 18);
	printf ("%s\n", str);
}

void
print_string (char *str, int length)
{
	int i = 0;
	for (i = 0; i < length; i++)
	{
		printf ("%2x", str[i]);
	}
	printf ("\n");
}

void
f_print_string (char *str, int length)
{
	int i = 0, k = 0;
	char buf[100];
	memset (buf, 0, 100);
	i = 0;
	count1 += 1;
	fprintf (fp, "=======================================================\n");
	if ((count1 % 1000) == 0)
	{
		printf ("fprintf is %d\n", count1);
	}
	while (i < length)
	{
		// printf("%02x ", (unsigned char)*(str + i));
		snprintf (buf + k, 10, "%02x ", (unsigned char) * (str + i));
		i = i + 1;
		k = k + 3;
		if ((i % 16) == 0)
		{
			// i = i + 1;
			// snprintf(buf + i * 2, 10, "\n");
			fprintf (fp, "%s\n", buf);
			// printf("111 %s\n", buf);
			memset (buf, 0, 100);
			k = 0;
		}
	}

	// fprintf("%s\n", buf);

}

void
f_write_string (char *str, int length)
{
	// int i = 0, k = 0;
	// char buf[100];
	// memset(buf, 0, 100);
	// i = 0;
	// fprintf(fp, "=======================================================\n");

	// while(i < length)
	// {
	//     // printf("%02x ", (unsigned char)*(str + i));
	//     snprintf(buf + k, 10, "%02x ", (unsigned char)*(str + i));
	//     i = i + 1;
	//     k = k + 3;
	//     if((i %16) == 0)
	//     {
	//         // i = i + 1;
	//         // snprintf(buf + i * 2, 10, "\n");
	//         fwrite(str, length, 1, fp);
	//         // printf("111 %s\n", buf);
	//         memset(buf, 0, 100);
	//         k = 0;
	//     }
	// }
	count2 += 1;
	// if((count2 % 1000) == 0)
	// {
	//     // printf("fwrite is %d\n", count2);
	//     if(138000 == count2)
	//     {
	//         fclose(fp);
	//         exit(-1);
	//     }
	// }
	fwrite (str, sizeof (char), length, fp);
}

int
strcmp_linger (char *s, char *t)
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
	// printf("%d\n", condition);
	return (condition == 6) ? 0 : 1;
}

/**
 * The follows are the APIs.
 */

int
get_aps (struct aplist *ap)
{

	int i = 0, length = 0, j = 0, mac_list_last = 0;
	int amount = 0;
	struct aplist *neibours;

	if (!ap)
		return 0;
	pthread_mutex_lock(&mutex_data);
	while (bbamount > 0)
	{
		char tmp[18];
		memcpy(tmp, beacon_buffer[bbstart]->mac_addr, 18);
		bbstart += 1;
		bbamount -= 1;
		for (i = 0; i < mac_list_last; i++)
		{
			if (strcmp(tmp, ap->aplists[i]) == 0)
				continue;
		}
		memcpy(ap->aplists[mac_list_last], tmp, 18);
		mac_list_last += 1;
	}
	pthread_mutex_unlock(&mutex_data);
	return mac_list_last;
}

void *
get_neighbour_py (char *ap, struct aplist_py *neibours)
{
	int begin = 0;
	int end = 0, found = 0;
	int i = 0, length = 0, j = 0, mac_list_last = 0;
	char mac_list[20][18];
	// printf("%s ap\n", ap);
	// printf("wocao %d\n", neibours->length);

	struct data_beacon_processed *beacon_tmp[BUFFER_STORE];
	for (i = 0; i < BUFFER_STORE; i++)
		beacon_tmp[i] =
		    (struct data_beacon_processed *)
		    malloc (sizeof (struct data_beacon_processed));
	i = 0;
	begin = bbstart1;
	end = bbend;
	for (i = 0; i < BUFFER_STORE; i++)
	{
		memcpy (beacon_tmp[i], beacon_buffer[i],
		        sizeof (struct data_beacon_processed));
	}


	if (end >= begin)
		length = end - begin;
	else
		length = end + BUFFER_STORE - begin;
	j = begin;
	// printf("length is %d bbamount1 %d bbstart1 %d bbend %d\n", length, bbamount1, bbstart1, bbend);
	// for(i = 0; i < length; i++)
	// {
	//   printf("%s strlen %d\n", beacon_tmp[j]->mac_addr, strlen(beacon_tmp[j]->mac_addr));
	//   j = j + 1;
	//   j = j % BUFFER_STORE;
	//   bbstart1 = bbstart1 + 1;
	//   bbstart1 = bbstart1 % BUFFER_STORE;
	//   bbamount1 = bbamount1 - 1;
	//   if(bbamount1 < 0)
	//   {
	//     bbamount1 = 0;
	//   }
	// }

	for (i = 0; i < length; i++)
	{
		bbstart1 = bbstart1 + 1;
		bbstart1 = bbstart1 % BUFFER_STORE;
		bbamount1 = bbamount1 - 1;
		if (bbamount1 < 0)
		{
			bbamount1 = 0;
		}
		// printf("%s ap %s\n", beacon_tmp[j]->mac_addr, ap);
		found = 0;
		if (strcmp (beacon_tmp[j]->mac_addr, ap) == 0)
		{
			if (mac_list_last > 0)
			{
				int m = 0;
				for (m = 0; m < mac_list_last; m++)
				{
					if (!strcmp (mac_list[m], beacon_tmp[j]->bssid))
					{
						found = 1;
						break;
					}
				}
			}
			if (found == 1)
			{
				j = j + 1;
				j = j % BUFFER_STORE;
				continue;
			}
			// printf("hereeee\n");
			strcpy (mac_list[mac_list_last], beacon_tmp[j]->bssid);
			mac_list_last = mac_list_last + 1;
			j = j + 1;
			j = j % BUFFER_STORE;
		}
	}
	// printf("mac_list_last is %d\n", mac_list_last);
	if (mac_list_last > 0)
	{
		// printf("here1111111111\n");
		neibours->length = mac_list_last;
		// printf("here1111111111\n");
		// memcpy(neibours->aplists, mac_list, sizeof(mac_list));
		for (i = 0; i < mac_list_last; i++)
			strcpy (neibours->aplists[i], mac_list[i]);
		// printf("here2222222222\n");
		// printf("%s neibours in c\n", neibours->aplists[0]);
		i = 0;
		for (i = 0; i < BUFFER_STORE; i++)
		{
			free (beacon_tmp[i]);
		}
		// return neibours;
	}
	else
	{
		i = 0;
		for (i = 0; i < BUFFER_STORE; i++)
		{
			free (beacon_tmp[i]);
		}
		// return NULL;
	}
}

void *
get_neighbour (char *ap, struct aplist *neibours)
{
	int begin = 0;
	int end = 0, found = 0;
	int i = 0, length = 0, j = 0, mac_list_last = 0;
	char mac_list[20][18];
	// printf("%s ap\n", ap);
	// printf("wocao %d\n", neibours->length);

	struct data_beacon_processed *beacon_tmp[BUFFER_STORE];
	for (i = 0; i < BUFFER_STORE; i++)
		beacon_tmp[i] =
		    (struct data_beacon_processed *)
		    malloc (sizeof (struct data_beacon_processed));
	i = 0;
	begin = bbstart1;
	end = bbend;
	for (i = 0; i < BUFFER_STORE; i++)
	{
		memcpy (beacon_tmp[i], beacon_buffer[i],
		        sizeof (struct data_beacon_processed));
	}


	if (end >= begin)
		length = end - begin;
	else
		length = end + BUFFER_STORE - begin;
	j = begin;
	// // printf("length is %d bbamount1 %d bbstart1 %d bbend %d\n", length, bbamount1, bbstart1, bbend);
	// // for(i = 0; i < length; i++)
	// // {
	// //   printf("%s strlen %d\n", beacon_tmp[j]->mac_addr, strlen(beacon_tmp[j]->mac_addr));
	// //   j = j + 1;
	// //   j = j % BUFFER_STORE;
	// //   bbstart1 = bbstart1 + 1;
	// //   bbstart1 = bbstart1 % BUFFER_STORE;
	// //   bbamount1 = bbamount1 - 1;
	// //   if(bbamount1 < 0)
	// //   {
	// //     bbamount1 = 0;
	// //   }
	// // }

	for (i = 0; i < length; i++)
	{
		bbstart1 = bbstart1 + 1;
		bbstart1 = bbstart1 % BUFFER_STORE;
		bbamount1 = bbamount1 - 1;
		if (bbamount1 < 0)
		{
			bbamount1 = 0;
		}
		// printf("%s\n", beacon_tmp[j]->mac_addr);
		found = 0;
		if (strcmp (beacon_tmp[j]->mac_addr, ap) == 0)
		{
			if (mac_list_last > 0)
			{
				int m = 0;
				for (m = 0; m < mac_list_last; m++)
				{
					if (!strcmp (mac_list[m], beacon_tmp[j]->bssid))
					{
						found = 1;
						break;
					}
				}
			}
			if (found == 1)
			{
				j = j + 1;
				j = j % BUFFER_STORE;
				continue;
			}
			// printf("hereeee\n");
			strcpy (mac_list[mac_list_last], beacon_tmp[j]->bssid);
			mac_list_last = mac_list_last + 1;
			j = j + 1;
			j = j % BUFFER_STORE;
		}
	}
	if (mac_list_last > 0)
	{
		// printf("here1111111111\n");
		neibours->length = mac_list_last;
		memcpy (neibours->aplists, mac_list, sizeof (mac_list));
		i = 0;
		for (i = 0; i < BUFFER_STORE; i++)
		{
			free (beacon_tmp[i]);
		}
		// return neibours;
	}
	else
	{
		i = 0;
		for (i = 0; i < BUFFER_STORE; i++)
		{
			free (beacon_tmp[i]);
		}
		// return NULL;
	}
	neibours->length = 10;
	// printf("%s is the mac the length is %d\n", neibours->aplists[neibours->length - 1], neibours->length);
	// printf("fuck one\n");
}

void *
get_clients (char *ap, struct aplist *clients)
{
	int begin = 0, end = 0, found = 0;
	int i = 0, j = 0, length = 0, mac_list_last = 0;
	struct data_iw_processed *iw_tmp[BUFFER_STORE];
	char mac_list[20][18];
	// aplist *clients;
	for (i = 0; i < BUFFER_STORE; i++)
	{
		iw_tmp[i] =
		    (struct data_iw_processed *)
		    malloc (sizeof (struct data_iw_processed));
	}
	i = 0;
	for (i = 0; i < BUFFER_STORE; i++)
	{
		memcpy (iw_tmp[i], iw_buffer[i], sizeof (struct data_iw_processed));
	}

	begin = ibstart;
	end = ibend;
	if (end >= begin)
	{
		length = end - begin;
	}
	else
	{
		length = end + BUFFER_STORE - begin;
	}
	j = begin;
	// printf("length is %d ibamount %d ibstart %d ibend %d\n", length, ibamount, ibstart, ibend);
	for (i = 0; i < length; i++)
	{
		// printf("%s\n", iw_tmp[j]->station);
		ibstart = ibstart + 1;
		ibstart = ibstart % BUFFER_STORE;
		ibamount = ibamount - 1;
		if (ibamount < 0)
		{
			ibamount = 0;
		}
		found = 0;

		if (strcmp (iw_tmp[j]->mac_addr, ap) == 0)
		{
			if (mac_list_last > 0)
			{
				int m = 0;
				for (m = 0; m < mac_list_last; m++)
				{
					if (!strcmp (mac_list[m], iw_tmp[j]->station))
					{
						found = 1;
						break;
					}
				}
			}
			if (found == 1)
			{
				j = j + 1;
				j = j % BUFFER_STORE;
				continue;
			}
			// printf("hereeee\n");
			strcpy (mac_list[mac_list_last], iw_tmp[j]->station);
			mac_list_last = mac_list_last + 1;
			j = j + 1;
			j = j % BUFFER_STORE;
		}
	}
	// clients = (aplist *)malloc(sizeof(struct aplist));

	clients->length = mac_list_last;
	// printf("amount of clients is %d\n", clients->length);
	if (mac_list_last > 0)
		memcpy (clients->aplists, mac_list, sizeof(mac_list));
	for (i = 0; i < BUFFER_STORE; i++)
	{
		free (iw_tmp[i]);
	}
	// free(mac_list);
	// retrun clients;
}

void *
get_clients_py (char *ap, struct aplist_py *clients)
{
	int begin = 0, end = 0, found = 0;
	int i = 0, j = 0, length = 0, mac_list_last = 0;
	struct data_iw_processed *iw_tmp[BUFFER_STORE];
	char mac_list[20][18];
	// aplist *clients;
	for (i = 0; i < BUFFER_STORE; i++)
	{
		iw_tmp[i] =
		    (struct data_iw_processed *)
		    malloc (sizeof (struct data_iw_processed));
	}
	i = 0;
	for (i = 0; i < BUFFER_STORE; i++)
	{
		memcpy (iw_tmp[i], iw_buffer[i], sizeof (struct data_iw_processed));
	}

	begin = ibstart;
	end = ibend;
	if (end >= begin)
	{
		length = end - begin;
	}
	else
	{
		length = end + BUFFER_STORE - begin;
	}
	j = begin;
	// printf("length is %d ibamount %d ibstart %d ibend %d\n", length, ibamount, ibstart, ibend);
	for (i = 0; i < length; i++)
	{
		// printf("%s\n", iw_tmp[j]->station);
		ibstart = ibstart + 1;
		ibstart = ibstart % BUFFER_STORE;
		ibamount = ibamount - 1;
		if (ibamount < 0)
		{
			ibamount = 0;
		}
		found = 0;

		if (strcmp (iw_tmp[j]->mac_addr, ap) == 0)
		{
			if (mac_list_last > 0)
			{
				int m = 0;
				for (m = 0; m < mac_list_last; m++)
				{
					if (!strcmp (mac_list[m], iw_tmp[j]->station))
					{
						found = 1;
						break;
					}
				}
			}
			if (found == 1)
			{
				j = j + 1;
				j = j % BUFFER_STORE;
				continue;
			}
			// printf("hereeee\n");
			strcpy (mac_list[mac_list_last], iw_tmp[j]->station);
			mac_list_last = mac_list_last + 1;
			j = j + 1;
			j = j % BUFFER_STORE;
		}
	}
	// clients = (aplist *)malloc(sizeof(struct aplist));

	clients->length = mac_list_last;
	// if(mac_list_last > 0)
	// memcpy(clients->aplists, mac_list, sizeof(mac_list));
	// printf("amount of clients is %d\n", clients->length);
	for (i = 0; i < mac_list_last; i++)
	{
		strcpy (clients->aplists[i], mac_list[i]);
	}
	for (i = 0; i < BUFFER_STORE; i++)
	{
		free (iw_tmp[i]);
	}
	// free(mac_list);
	// retrun clients;
}

void *
get_flows (char *ap, struct flowlist *flowlists)
{
	int begin = 0, end = 0, found = 0;
	int i = 0, j = 0, length = 0, mac_list_last = 0;
	struct data_winsize_processed winsize_tmp[BUFFER_STORE];
	struct flow flows[40];
	// for(i = 0; i < BUFFER_STORE; i++)
	// {
	//   winsize_tmp[i] = (struct data_winsize_processed *)malloc(sizeof(struct data_winsize_processed));
	// }
	// printf("passed length is %d\n", flowlists->length);
	for (i = 0; i < BUFFER_STORE; i++)
	{
		memcpy (&winsize_tmp[i], winsize_buffer[i],
		        sizeof (struct data_winsize_processed));
	}
	i = 0;
	begin = wbstart;
	end = wbend;
	if (end >= begin)
	{
		length = end - begin;
	}
	else
	{
		length = end + BUFFER_STORE - begin;
	}
	j = begin;

	for (i = 0; i < length; i++)
	{
		int m = 0;
		wbstart = wbstart + 1;
		wbstart = wbstart % BUFFER_STORE;
		// printf("herab\n");
		wbamount = wbamount - 1;
		if (wbamount < 0)
		{
			wbamount = 0;
		}
		found = 0;
		// printf("%d\n", winsize_tmp[j]->ip_src);
		for (m = 0; m < mac_list_last; m++)
		{
			if ((winsize_tmp[j].ip_src == flows[m].ip_src)
			        && (winsize_tmp[j].ip_dst == flows[m].ip_dst)
			        && (winsize_tmp[j].sourceaddr == flows[m].port_src)
			        && (winsize_tmp[j].destination == flows[m].port_dst))
			{
				found = 1;
				break;
			}
		}
		if (found == 1)
		{
			j = j + 1;
			j = j % BUFFER_STORE;
			continue;
		}
		flows[mac_list_last].ip_src = winsize_tmp[j].ip_src;
		flows[mac_list_last].ip_dst = winsize_tmp[j].ip_dst;
		flows[mac_list_last].port_src = winsize_tmp[j].sourceaddr;
		flows[mac_list_last].port_dst = winsize_tmp[j].destination;
		mac_list_last = mac_list_last + 1;
		j = j + 1;
		j = j % BUFFER_STORE;
	}
	// printf("wbamount %d wbstart %d wbend %d\n", wbamount, wbstart, wbend);
	flowlists->length = mac_list_last;
	for (i = 0; i < mac_list_last; i++)
	{
		memcpy (&flowlists->flows[i], &flows[i], sizeof (struct flow));
	}
	// printf("h1 amount of flows is %d\n", mac_list_last);
	// i = 0;
	// for(i = 0; i < BUFFER_STORE; i++)
	// {
	//   if(winsize_tmp[i])
	//     free(winsize_tmp[i]);
	// }
	// printf("h4\n");
}

void *
get_flow_drops (char *ap, struct flow_drop_ap *flow_drops)
{
	int begin = 0, total_drops = 0;
	int end = 0;
	int i = 0, length = 0, j = 0, mac_list_last = 0;
	struct flow_and_dropped mac_list[50];


	struct data_dropped_processed *dropped_tmp[BUFFER_STORE];
	for (i = 0; i < BUFFER_STORE; i++)
		dropped_tmp[i] =
		    (struct data_dropped_processed *)
		    malloc (sizeof (struct data_dropped_processed));
	for (i = 0; i < BUFFER_STORE; i++)
	{
		memcpy (dropped_tmp[i], dropped_buffer[i],
		        sizeof (struct data_dropped_processed));
	}
	i = 0;
	begin = dbstart;
	end = dbend;

	if (end >= begin)
		length = end - begin;
	else
		length = end + BUFFER_STORE - begin;
	j = begin;
	for (i = 0; i < length; i++)
	{
		dbstart = dbstart + 1;
		dbstart = dbstart % BUFFER_STORE;
		dbamount = dbamount - 1;
		if (dbamount < 0)
		{
			dbamount = 0;
		}
		if (strcmp (dropped_buffer[j]->mac_addr, ap) == 0)
		{
			int k = 0;
			for (k = 0; k < mac_list_last; k++)
			{
				bool condition = 0;
				condition =
				    (dropped_tmp[j]->ip_src == mac_list[k].dataflow.ip_src)
				    && (dropped_tmp[j]->ip_dst == mac_list[k].dataflow.ip_dst);
				condition = condition
				            &&
				            ((dropped_tmp[j]->port_src == mac_list[k].dataflow.port_src)
				             && (dropped_tmp[j]->port_dst ==
				                 mac_list[k].dataflow.port_dst));
				if (condition)
				{
					mac_list[k].drops = mac_list[k].drops + 1;
					total_drops = total_drops + 1;
				}
			}
			j = j + 1;
			j = j % BUFFER_STORE;
		}
		else
		{
			j = j + 1;
			j = j % BUFFER_STORE;
			continue;
		}
	}
	flow_drops->drops_total = total_drops;
	flow_drops->length = mac_list_last;
	for (i = 0; i < mac_list_last; i++)
	{
		memcpy (&flow_drops->flowinfo[i], &mac_list[i],
		        sizeof (struct flow_and_dropped));
	}
	for (i = 0; i < BUFFER_STORE; i++)
	{
		free (dropped_tmp[i]);
	}
}

bool
get_wireless_info (char *ap, struct wireless_information_ap *clients)
{
	int begin = 0, end = 0;
	bool found = 0;
	int i = 0, j = 0, length = 0, mac_list_last = 0;
	struct data_iw_processed *iw_tmp[BUFFER_STORE];
	// aplist *clients;
	// printf("received busy_time is %d ap is %s\n", clients->busy_time, ap);
	for (i = 0; i < BUFFER_STORE; i++)
	{
		iw_tmp[i] =
		    (struct data_iw_processed *)
		    malloc (sizeof (struct data_iw_processed));
	}
	i = 0;
	for (i = 0; i < BUFFER_STORE; i++)
	{
		memcpy (iw_tmp[i], survey_buffer[i], sizeof (struct data_iw_processed));
	}

	begin = sbstart;
	end = sbend;
	if (end >= begin)
	{
		length = end - begin;
	}
	else
	{
		length = end + BUFFER_STORE - begin;
	}
	j = begin;
	for (i = 0; i < length; i++)
	{
		// printf("%llu\n", iw_tmp[j]->active_time);
		sbstart = sbstart + 1;
		sbstart = sbstart % BUFFER_STORE;
		sbamount = sbamount - 1;
		if (sbamount < 0)
		{
			sbamount = 0;
		}
		if (strcmp (iw_tmp[j]->mac_addr, ap) == 0)
		{
			// printf("ap matched\n");
			clients->noise = survey_buffer[j]->noise;
			clients->active_time = survey_buffer[j]->time;
			clients->busy_time = survey_buffer[j]->time_busy;
			clients->transmit_time = survey_buffer[j]->time_tx;
			clients->receive_time = survey_buffer[j]->time_rx;
			found = 1;
			break;
		}
		j = j + 1;
		j = j % BUFFER_STORE;
	}
	// clients = (aplist *)malloc(sizeof(struct aplist));

	for (i = 0; i < BUFFER_STORE; i++)
	{
		free (iw_tmp[i]);
	}
	return found;
}

void
test_print (void)
{
	printf ("xxxxxxok\n");
}
