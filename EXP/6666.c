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
// #include <time.h>
#include <sys/time.h>
#include "thread1.h"
#include "hash_lin.h"
#include "newstructures.h"

typedef _Bool bool;

#define SEND_SIZE 1024
#define POOL_SIZE 10240000
// #define BUFFER_STORE 10000
#define SEQLIST 10000
#define THUNDRED 100000
/**
 * fp is the file pointer to write information in.
 */
FILE *fp = NULL;
FILE *fp_winsize = NULL;
FILE *fretrans = NULL;
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
// int packet_count = 0;
// int packet_count1 = 0;

/**
 * used to compute retrans, record seq of each dataflow
 */
struct winsize_record flow_seq_list[SEQLIST];
struct data_queue_list *queue_list_3 = NULL, *queue_list_last_3 = NULL;

struct data_queue_list *queue_list_f = NULL, *queue_list_last_f = NULL;
struct data_queue_computed queue_global_f;
struct data_survey_list *survey_list = NULL, *survey_list_last = NULL;
struct data_survey_computed survey_global;
/**
 * used to record current stations.
 */
struct station_list *station_lists = NULL, *station_l_last = NULL;
__u32 drop_count = 0;
struct drop_time last_100_ms_drop;
/**
 * Structures and variables for APIs.
 */
/**
 * record #stations
 */
struct station_list *neibour_lists = NULL, *neibour_l_last = NULL;
__u32 neibour_count = 0;
struct time_structure retrans_times[THUNDRED];
int retrans_head = 0;
int retrans_tail = 0;
struct time_structure winsize_times[THUNDRED];
int winsize_head = 0;
int winsize_tail = 0;
// struct data_beacon_processed *beacon_buffer[BUFFER_STORE];
// struct data_iw_processed *iw_buffer[BUFFER_STORE];
// // struct data_queue_processed     *queue_buffer[BUFFER_STORE];
// struct data_winsize_processed *winsize_buffer[BUFFER_STORE];
// struct data_dropped_processed *dropped_buffer[BUFFER_STORE];
// struct data_survey_processed *survey_buffer[BUFFER_STORE];
// int bbstart = 0, bbend = 0, bbamount = 0, bbstart1 = 0, bbamount1 = 0;
// int ibstart = 0, ibend = 0, ibamount = 0, ibstart1 = 0, ibamount1 = 0;
// int dbstart = 0, dbend = 0, dbamount = 0, dbstart1 = 0, dbamount1 = 0;
// int wbstart = 0, wbend = 0, wbamount = 0, wbstart1 = 0, wbamount1 = 0;
// int sbstart = 0, sbend = 0, sbamount = 0;

/**
 * for debugging.
 */
// struct timeval start, end, start1, end1;
// suseconds_t msec_1 = 0, msec_2 = 0;
// time_t sec1 = 0, sec2 = 0;

void *receive (void *arg);
void process (char *str, int length);
void decode_winsize (char *buf);
void decode_drops (char *buf);
void decode_iw (char *buf);
void decode_queue (char *buf);
void decode_beacon (char *buf);
void decode_survey (char *buf);
void print_mac (unsigned char *addr);
void print_string (char *str, int length);
void f_print_string (char *str, int length);
int strcmp_linger (unsigned char *s, unsigned char *t);
void *g_mes_tn_process (void *arg);
void f_write_string (char *str, int length);

void destroy (void);
void start_program (void);
void destroy_everything (void);
void start_threads (void);
void test_print (void);
void insert_flow_info(struct data_winsize *ptr);
void clear_flow_info(struct data_winsize *ptr);
void ack_flow_info(struct data_winsize *ptr);
void *format_data(void *arg);
/**
 * APIs
 */
// int get_aps (struct aplist *ap);
// void get_neighbour_py (char *ap, struct aplist_py *neibours);
// void get_neighbour (char *ap, struct aplist *neibours);
// void get_clients (char *ap, struct aplist *clients);
// void get_clients_py (char *ap, struct aplist_py *clients);
// void get_flows (char *ap, struct flowlist *flowlists);
// void get_flow_drops (char *ap, struct flow_drop_ap *flow_drops);
// bool get_wireless_info (char *ap, struct wireless_information_ap *clients);

pthread_mutex_t mutex_pool;
// pthread_mutex_t mutex_data;
pthread_mutex_t mutex_rdata;

/**
 * [receive description]
 * @author linger 2018-04-13
 * @param  arg [*arg is used for pthread create, there is no meaning for it.]
 * @return     [void]
 */
void *
receive (void *arg)
{
	int sin_len, sin_len_other;
	int port_winsize = 6666;
	int port_other = 6667;
	int sock, sock_other;
	struct sockaddr_in sin_winsize, sin_other;

	bzero (&sin_winsize, sizeof (sin_winsize));
	sin_winsize.sin_family = AF_INET;
	sin_winsize.sin_addr.s_addr = htonl (INADDR_ANY);
	sin_winsize.sin_port = htons (port_winsize);
	sin_len = sizeof (sin_winsize);

	bzero (&sin_other, sizeof (sin_other));
	sin_other.sin_family = AF_INET;
	sin_other.sin_addr.s_addr = htonl (INADDR_ANY);
	sin_other.sin_port = htons (port_other);
	sin_len_other = sizeof (sin_other);

	sock_other = socket (AF_INET, SOCK_DGRAM, 0);
	bind (sock_other, (struct sockaddr *) &sin_other, sizeof (sin_other));

	sock = socket (AF_INET, SOCK_DGRAM, 0);
	bind (sock, (struct sockaddr *) &sin_winsize, sizeof (sin_winsize));
	while (1)
	{
		char message[SEND_SIZE];
		char message_other[SEND_SIZE];
		while (1)
		{
			int len = 0;
			memset (message, 0, SEND_SIZE);
			len =
			    recvfrom (sock, message, SEND_SIZE, 0, (struct sockaddr *) &sin_winsize,
			              (socklen_t*)&sin_len);
			if (len <= 0)
			{
				printf ("receive error\n");
				continue;
			}
			len =
			    recvfrom (sock_other, message_other, SEND_SIZE, 0, (struct sockaddr *) &sin_other,
			              (socklen_t*)&sin_len_other);
			if (len <= 0)
			{
				printf ("receive error\n");
				continue;
			}

			pthread_mutex_lock (&mutex_pool);
			memcpy (buffer_pool + tail, message, SEND_SIZE);
			tail = tail + SEND_SIZE;
			tail = tail % POOL_SIZE;
			memcpy (buffer_pool + tail, message_other, SEND_SIZE);
			tail = tail + SEND_SIZE;
			tail = tail % POOL_SIZE;
			pthread_mutex_unlock (&mutex_pool);
		}
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
		// if((packet_count % 1000) == 0)
		// 	printf("packet received is %d %d\n",packet_count, packet_count1);
		switch (value_enum)
		{
		case WINSIZE:
			// packet_count += 1;
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
			// packet_count1 += 1;
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
	int kind, wscale, cal_windowsize;
	// __u64 time;
	__be64 timex = 0;

	// char insert_data[600];
	unsigned char mac[6];
	// int condition = 0;

	char mac_addr[18];
	unsigned char mac_addr_origin[6];
	unsigned char *ptr_uc = NULL;
	char ip_src[20];
	char ip_dst[20];

	char *str = NULL;


	// gettimeofday (&start, NULL);

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


	ptr_uc = (unsigned char *) &rdata.ip_src;
	sprintf (ip_src, "%u.%u.%u.%u", ptr_uc[3], ptr_uc[2], ptr_uc[1], ptr_uc[0]);
	ptr_uc = (unsigned char *) &rdata.ip_dst;
	sprintf (ip_dst, "%u.%u.%u.%u", ptr_uc[3], ptr_uc[2], ptr_uc[1], ptr_uc[0]);
	kind = (int) (rdata.wscale[0]);
	length = (int) rdata.wscale[1];
	wscale = (int) rdata.wscale[2];
	rdata.flags = rdata.flags & 0x0017;
	cal_windowsize = rdata.windowsize;
	pthread_mutex_lock(&mutex_rdata);
	winsize_times[winsize_head].sec = rdata.sec;
	winsize_times[winsize_head].usec = rdata.usec;
	winsize_head += 1;
	winsize_head = winsize_head % THUNDRED;
	pthread_mutex_unlock(&mutex_rdata);

	if (rdata.flags == 2 || rdata.flags == 18)
	{
		insert_flow_info(&rdata);
	}
	else if (rdata.flags == 17 || (rdata.flags & 0x0004) == 1)
	{
		clear_flow_info(&rdata);
	}
	else if (rdata.flags == 16)
	{
		ack_flow_info(&rdata);
	}

	// if (rdata.flags == 2 || rdata.flags == 18)
	// {
	// 	memcpy(str, mac_addr_origin, 6);
	// 	memcpy(str + 6, &rdata.ip_src, 4);
	// 	memcpy(str + 10, &rdata.ip_dst, 4);
	// 	memcpy(str + 14, &rdata.sourceaddr, 2);
	// 	memcpy(str + 16, &rdata.destination, 2);
	// 	time = getcurrenttime ();
	// 	hash_table_insert (str, wscale, time);
	// }
	// else if (rdata.flags == 17 || (rdata.flags & 0x0004) == 1)
	// {
	// 	memcpy(str, mac_addr_origin, 6);
	// 	memcpy(str + 6, &rdata.ip_src, 4);
	// 	memcpy(str + 10, &rdata.ip_dst, 4);
	// 	memcpy(str + 14, &rdata.sourceaddr, 2);
	// 	memcpy(str + 16, &rdata.destination, 2);
	// 	if (hash_table_lookup (str) != NULL)
	// 	{
	// 		hash_table_remove (str);
	// 	}
	// 	memcpy(str, mac_addr_origin, 6);
	// 	memcpy(str + 6, &rdata.ip_dst, 4);
	// 	memcpy(str + 10, &rdata.ip_src, 4);
	// 	memcpy(str + 14, &rdata.destination, 2);
	// 	memcpy(str + 16, &rdata.sourceaddr, 2);
	// 	if (hash_table_lookup (str) != NULL)
	// 	{
	// 		hash_table_remove (str);
	// 	}
	// }

	// else if (rdata.flags == 16)
	// {
	// 	time = getcurrenttime ();
	// 	memcpy(str, mac_addr_origin, 6);
	// 	memcpy(str + 6, &rdata.ip_dst, 4);
	// 	memcpy(str + 10, &rdata.ip_src, 4);
	// 	memcpy(str + 14, &rdata.destination, 2);
	// 	memcpy(str + 16, &rdata.sourceaddr, 2);
	// 	if (hash_table_lookup (str) != NULL)
	// 	{
	// 		cal_windowsize =
	// 		    rdata.windowsize << hash_table_lookup (str)->nValue;
	// 		hash_table_lookup (str)->time = time;
	// 	}
	// 	else
	// 	{
	// 		time = getcurrenttime ();
	// 		hash_table_insert (str, wscale, time);
	// 	}
	// }
	fprintf (fp_winsize,
	         "%d, %lld, %.18s, %.18s, %.18s, %u, %u, %u, %u, %u, %u, %d, %u, %u, %d, %u\n",
	         WINSIZE, timex, mac_addr,
	         ip_src, ip_dst, rdata.sourceaddr, rdata.destination,
	         rdata.sequence, rdata.ack_sequence, rdata.windowsize,
	         cal_windowsize, rdata.datalength, rdata.flags, kind,
	         length, wscale);

	free (str);
	str = NULL;

	/**
	 * Follows are for APIs.
	 */
	// pthread_mutex_lock(&mutex_data);
	// struct data_winsize_processed *tmp;
	// tmp = winsize_buffer[wbend];

	// tmp->ip_src = rdata.ip_src;
	// tmp->ip_dst = rdata.ip_dst;
	// tmp->sourceaddr = rdata.sourceaddr;
	// tmp->destination = rdata.destination;
	// tmp->sequence = rdata.sequence;
	// tmp->ack_sequence = rdata.ack_sequence;
	// tmp->flags = rdata.flags;
	// tmp->windowsize = rdata.windowsize;
	// tmp->sec = rdata.sec;
	// tmp->usec = rdata.usec;
	// tmp->length = length;
	// tmp->kind = kind;

	// strcpy (tmp->mac_addr, mac_addr);

	// tmp->kind = kind;
	// tmp->length = length;

	// wbamount = wbamount + 1;
	// wbamount1 = wbamount1 + 1;
	// wbend = wbend + 1;
	// wbend = wbend % BUFFER_STORE;

	// if (wbamount > BUFFER_STORE)
	// {
	// 	wbstart = wbend + 1;
	// 	wbstart = wbstart % BUFFER_STORE;
	// 	wbamount = BUFFER_STORE;
	// }
	// if (wbamount1 > BUFFER_STORE)
	// {
	// 	wbstart1 = wbend + 1;
	// 	wbstart1 = wbstart1 % BUFFER_STORE;
	// 	wbamount1 = BUFFER_STORE;
	// }
	// pthread_mutex_unlock(&mutex_data);
	// gettimeofday(&end, NULL);
	// msec_1 = end.tv_usec - start.tv_usec;
	// sec1 - end.tv_sec - start.tv_sec;
	// printf("Time %u:%u\n", sec1, msec_1);
	// exit(-1);
}

void insert_flow_info(struct data_winsize *ptr)
{
	int i = 0;
	int found = 0;
	struct winsize_record *tmp = NULL;
	for (i = 0; i < SEQLIST; i++)
	{
		tmp = flow_seq_list + i;
		if (tmp->sourceaddr == 0)
		{
			found = 1;
			break;
		}
	}
	if (found == 0) // full
	{
		__u32 sec_max = flow_seq_list[0].sec;
		struct winsize_record *tmp_r = NULL;
		int found_tmp = 0;
		for (i = 1; i < SEQLIST; i++)
		{
			tmp = flow_seq_list + i;
			if (tmp->sec < sec_max)
			{
				sec_max = tmp->sec;
				tmp_r = tmp;
				found_tmp = 1;
			}
		}
		if (found_tmp == 1)
			tmp = tmp_r;
		else
			tmp = flow_seq_list;
		tmp->ip_src = ptr->ip_src;
		tmp->ip_dst = ptr->ip_dst;
		tmp->sourceaddr = ptr->sourceaddr;
		tmp->destination = ptr->destination;
		tmp->seq_list[0].seq = ptr->sequence;
		tmp->seq_list[0].sec = ptr->sec;
		tmp->seq_list[0].usec = ptr->usec;
		tmp->sec = ptr->sec;
		// printf("hhh\n");
	}
	else
	{
		tmp->ip_src = ptr->ip_src;
		tmp->ip_dst = ptr->ip_dst;
		tmp->sourceaddr = ptr->sourceaddr;
		tmp->destination = ptr->destination;
		tmp->seq_list[0].seq = ptr->sequence;
		tmp->seq_list[0].sec = ptr->sec;
		tmp->seq_list[0].usec = ptr->usec;
		tmp->sec = ptr->sec;
		// tmp->usec = ptr->usec;
		// printf("%d %d\n", ptr->sourceaddr,ptr->destination);
	}
}
// struct winsize_record
// {
//   __be32 ip_src;
//   __be32 ip_dst;
//   __be16 sourceaddr;
//   __be16 destination;
//   struct seq_info seq_list[100];
//   __be32 ack_seq;
//   __be32 sec;
//   // __be32 usec;
// };
void clear_flow_info(struct data_winsize *ptr)
{
	int i = 0;
	struct winsize_record *tmp = NULL;
	for (i = 0; i < SEQLIST; i++)
	{
		int condition = 0;
		tmp = flow_seq_list + i;
		condition = (tmp->ip_src == ptr->ip_src) + (tmp->ip_dst == ptr->ip_dst) +
		            (tmp->sourceaddr == ptr->sourceaddr) + (tmp->destination == ptr->destination);
		if (condition == 4)
		{
			memset(tmp, 0, sizeof(struct winsize_record));
			break;
		}
	}
}
void ack_flow_info(struct data_winsize *ptr)
{
	int i = 0;
	// int insert = 0;

	struct winsize_record *tmp = NULL;
	for (i = 0; i < SEQLIST; i++)
	{
		int condition1 = 0, condition2 = 0;
		int condition = 0;
		tmp = flow_seq_list + i;
		if (ptr->datalength != 0)
		{
			condition = (tmp->ip_src == ptr->ip_src) + (tmp->ip_dst == ptr->ip_dst) +
			            (tmp->sourceaddr == ptr->sourceaddr) + (tmp->destination == ptr->destination);
			if (condition == 4)
			{
				int j = 0;
				struct seq_info *insert_place = NULL;
				struct seq_info *tmps = NULL;
				for (j = 0; j < SEQLIST; j++) //found, insert, find retrans.
				{
					tmps = tmp->seq_list + j;
					// printf("%d %d\n", j, tmps->seq);
					if (tmps->seq == ptr->sequence) // found retrans, update sequence.
					{
						// __be64 time_tmp = 0;
						// time_tmp = (__be64)ptr->sec * (__be64)1000000;
						// time_tmp += (__be64)ptr->usec;
						// fprintf(fretrans, "%d, %llu\n", 9, time_tmp);
						pthread_mutex_lock(&mutex_rdata);
						retrans_times[retrans_head].sec = ptr->sec;
						retrans_times[retrans_head].usec = ptr->usec;
						retrans_head += 1;
						retrans_head = retrans_head % THUNDRED;
						pthread_mutex_unlock(&mutex_rdata);
						tmps->sec = ptr->sec;
						tmps->usec = ptr->usec;
						// insert = 1;
						// printf("1111\n");
						break;
					}
					else if (tmps->seq == -1) //find inset place.
					{
						if (insert_place == NULL)
							insert_place = tmps;
						// printf("2222\n");
					}
					else if (tmps->seq == 0) //no retrans, to the end.
					{
						if (insert_place == NULL)
							insert_place = tmps;
						// printf("3333\n");
						break;
					}

				}
				if (insert_place	!= NULL) // no retrans, should insert here.
				{
					insert_place->seq = ptr->sequence;
					insert_place->sec = ptr->sec;
					insert_place->usec = ptr->usec;
					// insert = 1;
				}
				condition1 = 1;
				if (condition2 == 1)
					break;
			}
		}
		else
		{
			condition1 = 1;
		}
		condition = (tmp->ip_src == ptr->ip_dst) + (tmp->ip_dst == ptr->ip_src) + //clear seq < r.ack
		            (tmp->sourceaddr == ptr->destination) + (tmp->destination == ptr->sourceaddr);
		if (condition == 4)
		{
			int j = 0;
			int seq = ptr->ack_sequence;
			struct seq_info *tmps = NULL;
			tmp->ack_seq = seq;
			for (j = 0; j < SEQLIST; j++)
			{
				tmps = tmp->seq_list + j;
				if (tmps->seq <= seq)
				{
					tmps->seq = -1;
					// tmps->sec = 0;
					// tmps->usec = 0;
				}
			}
			condition2 = 1;
			if ((condition1 == 1) && (condition2 == 1))
				break;
		}
	}
	// if(insert == 0)
	// {
	// 	printf("insert error1\n");
	// }
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

	int length = sizeof (struct dp_packet);

	unsigned char *ptr_uc = NULL;
	char ip_src[20];
	char ip_dst[20];
	char srcip[20];
	char mac_addr[18];
	unsigned char mac[6];
	__be64 timex = 0;
	__be64 timetmp = 0;

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
	timetmp = last_100_ms_drop.sec;
	timetmp = timetmp * 1000000;
	timetmp += last_100_ms_drop.usec;
	if ((timex - timetmp) >= 900000)
	{
		drop_count = rdata.drop_count - last_100_ms_drop.count;
		last_100_ms_drop.sec = rdata.sec;
		last_100_ms_drop.usec = rdata.usec;
		last_100_ms_drop.count = rdata.drop_count;
	}
	// pthread_mutex_lock(&mutex_data);
	// struct data_dropped_processed *tmp;
	// tmp = dropped_buffer[dbend];
	// strcpy (tmp->mac_addr, mac_addr);
	// tmp->ip_src = rdata.ip_src;
	// tmp->ip_dst = rdata.ip_dst;
	// tmp->port_src = rdata.port_src;
	// tmp->port_dst = rdata.port_dst;
	// tmp->sequence = rdata.sequence;
	// tmp->ack_sequence = rdata.ack_sequence;
	// tmp->drop_count = rdata.drop_count;
	// tmp->dpl = rdata.dpl;
	// tmp->in_time = rdata.in_time;

	// dbamount = dbamount + 1;
	// dbamount1 = dbamount1 + 1;
	// dbend = dbend + 1;
	// dbend = dbend % BUFFER_STORE;
	// if (dbamount > BUFFER_STORE)
	// {
	// 	dbstart = dbend + 1;
	// 	dbstart = dbstart % BUFFER_STORE;
	// 	dbamount = BUFFER_STORE;
	// }
	// if (dbamount1 > BUFFER_STORE)
	// {
	// 	dbstart1 = dbend + 1;
	// 	dbstart1 = dbstart1 % BUFFER_STORE;
	// 	dbamount1 = BUFFER_STORE;
	// }
	// pthread_mutex_unlock(&mutex_data);
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
	unsigned char mac[6];
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
	reversebytes_sint32t (&rdata.signal);
	reversebytes_sint32t (&rdata.signal_avg);
	reversebytes_uint32t (&rdata.expected_throughput);
	timex = rdata.sec;
	timex = timex * 1000000;
	timex += rdata.usec;
	expected_throughput_tmp = rdata.expected_throughput;
	expected_throughput = (float) expected_throughput_tmp / 1000.0;
	mac_tranADDR_toString_r ((unsigned char *)rdata.station, station, 18);

	mac_tranADDR_toString_r (mac, mac_addr, 18);
	fprintf (fp,
	         "%d, %lld, %.18s, %.18s, %u, %u, %u, %u, %u, %u, %u, %u, %d, %d, %f\n",
	         IW, timex, mac_addr, station, rdata.device, rdata.inactive_time,
	         rdata.rx_bytes, rdata.rx_packets, rdata.tx_bytes, rdata.tx_packets,
	         rdata.tx_retries, rdata.tx_failed, rdata.signal, rdata.signal_avg,
	         expected_throughput);
	if (station_lists == NULL)
	{
		struct station_list *tlist = NULL;
		int len = sizeof(struct station_list);
		tlist = (struct station_list *)malloc(len);
		memset(tlist, 0, len);
		memcpy(tlist->station, rdata.station, 6);
		tlist->sec = rdata.sec;
		tlist->next = NULL;
		tlist->count = 1;
		station_lists = tlist;
		station_l_last = station_lists;
	}
	else
	{
		struct station_list *tlist = NULL, *last = NULL;
		int found = 0;
		tlist = station_lists;
		last = NULL;

		while(tlist) // remove the old ones.
		{
			if((rdata.sec - tlist->sec) > 10) //out of time item, delete.
			{
				struct station_list *ptr = NULL;
				if(tlist == station_lists) // head
				{
					if(station_lists == station_l_last) //only one, replace it.
					{
						free(station_lists);
						station_l_last = NULL;
						station_lists = NULL;
						break; //done
					}
					else
					{
						ptr = tlist;
						tlist = tlist->next;
						station_lists = tlist;
						free(ptr);
						ptr = NULL;
						continue; //alread tlist = tlist->next.
					}
				}
				else if(tlist == station_l_last) // tail
				{
					struct station_list *tmp1 = NULL, *pre = NULL;
					tmp1 = station_lists;
					while(tmp1 != station_l_last)
					{
						pre = tmp1;
						tmp1 = tmp1->next;
					}
					pre->next = NULL;
					free(station_l_last);
					station_l_last = pre;
					break; //done
				}
				else // middle
				{
					ptr = tlist;
					tlist = tlist->next;
					last->next = tlist;
					free(ptr);
					ptr = NULL;
				}
			}
			last = tlist;
			tlist = tlist->next;
		}
		tlist = station_lists;
		while(tlist) // insert.
		{
			if (strcmp_linger((unsigned char *)tlist->station, (unsigned char *)rdata.station) == 0)
			{
				found = 1;
				tlist->sec = rdata.sec;
				break;
			}			
			tlist = tlist->next;
		}
		if (found == 0)
		{
			struct station_list *new = NULL;
			int len = sizeof(struct station_list);
			new = (struct station_list *)malloc(len);
			memset(new, 0, len);
			memcpy(new->station, rdata.station, 6);
			new->sec = rdata.sec;
			new->next = NULL;
			if(station_lists != NULL)
			{
				station_l_last->next = new;
				station_l_last = new;
				station_lists->count += 1;
			}
			else
			{
				station_lists = new;
				station_lists->count = 1;
				station_l_last = station_lists;
			}
		}	
	}
	// printf("IW: %.18s, %u, %u, %u, %u, %u, %u, %u, %u, %d, %d, %f\n",station, rdata.device, rdata.inactive_time, rdata.rx_bytes, rdata.rx_packets, rdata.tx_bytes, rdata.tx_packets, rdata.tx_retries, rdata.tx_failed, rdata.signal, rdata.signal_avg, expected_throughput);

	/**
	 * Followings are for APIs.
	 * ibamount for gettimex, _clients	 */
	// pthread_mutex_lock(&mutex_data);
	// struct data_iw_processed *tmp;
	// tmp = iw_buffer[ibend];
	// strcpy (tmp->mac_addr, mac_addr);
	// strcpy (tmp->station, station);
	// tmp->device = rdata.device;
	// tmp->inactive_time = rdata.inactive_time;
	// tmp->rx_bytes = rdata.rx_bytes;
	// tmp->rx_packets = rdata.rx_packets;
	// tmp->tx_bytes = rdata.tx_bytes;
	// tmp->tx_packets = rdata.tx_packets;
	// tmp->tx_retries = rdata.tx_retries;
	// tmp->tx_failed = rdata.tx_failed;
	// tmp->signal = rdata.signal;
	// tmp->signal_avg = rdata.signal_avg;
	// tmp->expected_throughput = expected_throughput;

	// // tmp->noise = rdata.noise;
	// // tmp->active_time = rdata.active_time;
	// // tmp->connected_time = rdata.connected_time;
	// // tmp->busy_time = rdata.busy_time;
	// // tmp->receive_time = rdata.receive_time;
	// // tmp->transmit_time = rdata.transmit_time;
	// ibamount = ibamount + 1;
	// ibamount1 = ibamount1 + 1;
	// ibend = ibend + 1;
	// ibend = ibend % BUFFER_STORE;
	// if (ibamount > BUFFER_STORE)
	// {
	// 	ibstart = ibend + 1;
	// 	ibstart = ibstart % BUFFER_STORE;
	// 	ibamount = BUFFER_STORE;
	// }
	// if (ibamount1 > BUFFER_STORE)
	// {
	// 	ibstart1 = ibend + 1;
	// 	ibstart1 = ibstart1 % BUFFER_STORE;
	// 	ibamount1 = BUFFER_STORE;
	// }
	// pthread_mutex_unlock(&mutex_data);
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
	unsigned char mac[6];
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

	if (rdata.queue_id == 3)
	{
		struct data_queue_list *new = NULL;
		int len = 0;
		len = sizeof(struct data_queue_list);
		new = (struct data_queue_list *)malloc(len);
		memset(new, 0, len);
		new->queue = rdata;
		new->next = NULL;
		pthread_mutex_lock(&mutex_rdata);
		if (queue_list_3 == NULL)
		{
			queue_list_3 = new;
			queue_list_last_3 = queue_list_3;
		}
		else
		{
			queue_list_last_3->next = new;
			queue_list_last_3 = new;
		}
		pthread_mutex_unlock(&mutex_rdata);
	}
	else if(rdata.queue_id == (__u32)4294967295)
	{
		struct data_queue_list *new = NULL;
		int len = 0;
		len = sizeof(struct data_queue_list);
		new = (struct data_queue_list *)malloc(len);
		memset(new, 0, len);
		new->queue = rdata;
		new->next = NULL;
		pthread_mutex_lock(&mutex_rdata);
		if (queue_list_f == NULL)
		{
			queue_list_f = new;
			queue_list_last_f = queue_list_f;
		}
		else
		{
			queue_list_last_f->next = new;
			queue_list_last_f = new;
		}
		pthread_mutex_unlock(&mutex_rdata);
	}

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
	// int signal;
	// char channel_type[] = "802.11a";
	struct data_beacon beacon;
	int length = sizeof (struct data_beacon);
	char bssid[18];
	char mac_addr[18];
	unsigned char mac[6];
	__be64 timex = 0;
	int signal1 = 0;

	memset (&beacon, 0, length);
	memset (bssid, 0, 18);
	memset (mac_addr, 0, 18);
	memset (mac, 0, 6);
	memcpy (mac, buf, 6);
	memcpy (&beacon, buf + 6, length);

	// beacon_new_2_old(&old, &beacon);

	// reversebytes_uint16t(&beacon.signal);
	reversebytes_uint16t(&beacon.data_rate);
	reversebytes_uint32t (&beacon.sec);
	reversebytes_uint32t (&beacon.usec);
	reversebytes_uint16t (&beacon.freq);
	mac_tranADDR_toString_r ((unsigned char *)beacon.bssid, bssid, 18);
	mac_tranADDR_toString_r (mac, mac_addr, 18);
	timex = beacon.sec;
	timex = timex * 1000000;
	timex += beacon.usec;
	signal1 = (int)beacon.signal;
	fprintf (fp, "%d, %lld, %.18s, %u, %u, %u, %.18s\n", BEACON, timex, mac_addr,
	         beacon.data_rate, beacon.freq, signal1, (char *)bssid);
	pthread_mutex_lock(&mutex_rdata);
	if (neibour_lists == NULL)
	{
		struct station_list *tlist = NULL;
		int len = sizeof(struct station_list);
		tlist = (struct station_list *)malloc(len);
		memset(tlist, 0, len);
		memcpy(tlist->station, mac, 6);
		tlist->sec = beacon.sec;
		tlist->next = NULL;
		tlist->count = 1;
		neibour_lists = tlist;
		neibour_l_last = neibour_lists;
	}
	else
	{
		struct station_list *tlist = NULL, *last = NULL;
		int found = 0;
		tlist = neibour_lists;
		last = NULL;
		while(tlist) // remove the old ones.
		{
			if((beacon.sec - tlist->sec) > 10) //out of time item, delete.
			{
				struct station_list *ptr = NULL;
				if(tlist == neibour_lists) // head
				{
					if(neibour_lists == neibour_l_last) //only one, replace it.
					{
						free(neibour_lists);
						neibour_l_last = NULL;
						neibour_lists = NULL;
						break; //done
					}
					else
					{
						ptr = tlist;
						tlist = tlist->next;
						neibour_lists = tlist;
						free(ptr);
						ptr = NULL;
						continue; //alread tlist = tlist->next.
					}
				}
				else if(tlist == neibour_l_last) // tail
				{
					struct station_list *tmp1 = NULL, *pre;
					tmp1 = neibour_lists;
					while(tmp1 != neibour_l_last)
					{
						pre = tmp1;
						tmp1 = tmp1->next;
					}
					pre->next = NULL;
					free(neibour_l_last);
					neibour_l_last = pre;
					break; //done
				}
				else // middle
				{
					ptr = tlist;
					tlist = tlist->next;
					last->next = tlist;
					free(ptr);
					ptr = NULL;
				}
			}
			last = tlist;
			tlist = tlist->next;
		}
		tlist = neibour_lists;
		while(tlist) // insert.
		{
			if (strcmp_linger((unsigned char *)tlist->station, (unsigned char *)beacon.bssid) == 0)
			{
				found = 1;
				tlist->sec = beacon.sec;
				break;
			}			
			tlist = tlist->next;
		}
		if (found == 0)
		{
			struct station_list *new = NULL;
			int len = sizeof(struct station_list);
			new = (struct station_list *)malloc(len);
			memset(new, 0, len);
			memcpy(new->station, beacon.bssid, 6);
			new->sec = beacon.sec;
			new->next = NULL;
			if(neibour_lists != NULL)
			{
				neibour_l_last->next = new;
				neibour_l_last = new;
				neibour_lists->count += 1;
			}
			else
			{
				neibour_lists = new;
				neibour_lists->count = 1;
				neibour_l_last = neibour_lists;
			}
		}	
	}
	pthread_mutex_unlock(&mutex_rdata);
	/**
	 * The following code copy processed beacon information to a buffer, APIs can use these informations
	 * to drive their intresting values.
	 * beacon_buffer is the buffer;
	 * bbamount is different from bbamount: bbamount for get_aps API;
	 * bbamount1 is for get_neibours series.
	 */
	// pthread_mutex_lock(&mutex_data);
	// struct data_beacon_processed *tmp;

	// tmp = beacon_buffer[bbend];
	// strcpy (tmp->mac_addr, mac_addr);
	// strcpy (tmp->bssid, bssid);

	// tmp->data_rate = beacon.data_rate;
	// tmp->signal = beacon.signal;
	// tmp->timein = beacon.timein;

	// bbamount = bbamount + 1;
	// bbamount1 = bbamount1 + 1;
	// bbend = bbend + 1;
	// bbend = bbend % BUFFER_STORE;

	// if (bbamount > BUFFER_STORE)
	// {
	// 	bbstart = bbend + 1;
	// 	bbstart = bbstart % BUFFER_STORE;
	// 	bbamount = BUFFER_STORE;
	// }

	// if (bbamount1 > BUFFER_STORE)
	// {
	// 	bbstart1 = bbend + 1;
	// 	bbstart1 = bbstart1 % BUFFER_STORE;
	// 	bbamount1 = BUFFER_STORE;
	// }
	// pthread_mutex_unlock(&mutex_data);

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
	unsigned char mac[6];
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

	struct data_survey_list *new = NULL;
	int len = 0;
	len = sizeof(struct data_survey_list);
	new = (struct data_survey_list *)malloc(len);
	memset(new, 0, len);
	new->survey = rdata;
	new->next = NULL;
	pthread_mutex_lock(&mutex_rdata);
	if (survey_list == NULL)
	{
		survey_list = new;
		survey_list_last = survey_list;
	}
	else
	{
		survey_list_last->next = new;
		survey_list_last = new;
	}
	pthread_mutex_unlock(&mutex_rdata);


	// pthread_mutex_lock(&mutex_data);
	// struct data_survey_processed *tmp;

	// tmp = survey_buffer[sbend];
	// strcpy (tmp->mac_addr, mac_addr);
	// tmp->time = rdata.time;
	// tmp->time_busy = rdata.time_busy;
	// tmp->time_ext_busy = rdata.time_ext_busy;
	// tmp->time_rx = rdata.time_rx;
	// tmp->time_tx = rdata.time_tx;
	// tmp->time_scan = rdata.time_scan;
	// tmp->noise = rdata.noise;
	// tmp->center_freq = rdata.center_freq;


	// sbamount = sbamount + 1;
	// sbend = sbend + 1;
	// sbend = sbend % BUFFER_STORE;
	// if (sbamount > BUFFER_STORE)
	// {
	// 	sbstart = sbend + 1;
	// 	sbstart = sbstart % BUFFER_STORE;
	// 	sbamount = BUFFER_STORE;
	// }
	// pthread_mutex_unlock(&mutex_data);
}

void *format_data(void *arg)
{
	struct data_queue_computed *queue_p_buffer_3 = NULL, *queue_p_last_3 = NULL;
	struct data_queue_computed *queue_p_buffer_f = NULL, *queue_p_last_f = NULL;
	struct data_survey_computed *survey_p_buffer = NULL, *survey_p_last = NULL;
	while (1)
	{
		struct time_structure time_tp;
		int d1 = 0, d2 = 0;
		float duration = 0.0;
		int drop = 0;
		struct data_queue *new = NULL, *last = NULL;
		struct data_survey *new_survey = NULL, *s_last = NULL;
		struct data_queue_computed *newitem_queue_3 = NULL;
		struct data_queue_computed*newitem_queue_f = NULL;
		struct data_survey_computed *newitem_survey = NULL;
		// __u64 time_last = 0, time_current = 0;
		int amount = 0;

		// time_last = getcurrenttime();
		memset(&time_tp, 0, sizeof(struct time_structure));
		pthread_mutex_lock(&mutex_rdata);
		new = &(queue_list_3->queue);
		last = &(queue_list_last_3->queue);


		if ((last != NULL) && (new != NULL))
		{
			d1 = (int)(last->sec) - (int)(new->sec);
			d2 = (int)(last->usec) - (int)(new->usec);
			duration = (float)(d1 * 1000 + d2 / 1000);
		}
		if (duration >= 900) //statisfy compute condition, compute
		{
			struct data_queue_list *ptr = NULL;
			int len = sizeof(struct data_queue_computed);
			newitem_queue_3 = (struct data_queue_computed *)malloc(len);
			memset(newitem_queue_3, 0, len);
			newitem_queue_3->packets = (float)(last->packets - new->packets) / duration;
			newitem_queue_3->bytes = (float)(last->bytes - new->bytes) / duration;
			newitem_queue_3->qlen = (float)last->qlen;
			newitem_queue_3->backlog = (float)last->backlog;
			newitem_queue_3->requeues = (float)(last->requeues - new->requeues) / duration;
			newitem_queue_3->overlimits = (float)(last->overlimits - new->overlimits) / duration;
			newitem_queue_3->sec = last->sec;
			newitem_queue_3->usec = last->usec;
			ptr = queue_list_3;
			queue_list_3 = queue_list_3->next;
			free(ptr);
			ptr = NULL;
		}
		pthread_mutex_unlock(&mutex_rdata);
		if (newitem_queue_3 != NULL)
		{
			// printf("1111\n");
			if (queue_p_buffer_3 == NULL) //null, create
			{
				queue_p_buffer_3 = newitem_queue_3;
				queue_p_last_3 = newitem_queue_3;
			}
			else
			{
				int condition = 0;
				if (newitem_queue_3->sec > queue_p_last_3->sec)
					condition = 1;
				else if ((newitem_queue_3->sec == queue_p_last_3->sec) && (newitem_queue_3->usec >= queue_p_last_3->usec))
					condition = 1;
				if (condition) // insert to tail.
				{
					queue_p_last_3->next = newitem_queue_3;
					queue_p_last_3 = newitem_queue_3;
				}
				else // out or order, find the right position to insert.
				{
					struct data_queue_computed *ptr = NULL, *ptrpre = NULL;
					ptr = queue_p_buffer_3;
					while (ptr) //if found, ptr is not null
					{
						if (ptr->sec > newitem_queue_3->sec)
						{
							break;
						}
						ptrpre = ptr;
						ptr = ptr->next;
					}
					if (ptrpre) //have searched, insert
					{
						ptrpre->next = newitem_queue_3;
						newitem_queue_3->next = ptr;
					}
				}

			}
		}

		pthread_mutex_lock(&mutex_rdata);
		new = &(queue_list_f->queue);
		last = &(queue_list_last_f->queue);

		if ((last != NULL) && (new != NULL))
		{
			d1 = (int)(last->sec) - (int)(new->sec);
			d2 = (int)(last->usec) - (int)(new->usec);
			duration = (float)(d1 * 1000 + d2 / 1000);
		}
		duration = (float)(d1 * 1000 + d2 / 1000);
		if (duration >= 900) //statisfy compute condition, compute
		{
			struct data_queue_list *ptr = NULL;
			int len = sizeof(struct data_queue_computed);
			newitem_queue_f = (struct data_queue_computed *)malloc(len);
			memset(newitem_queue_f, 0, len);
			newitem_queue_f->packets = (float)(last->packets - new->packets) / duration;
			newitem_queue_f->bytes = (float)(last->bytes - new->bytes) / duration;
			newitem_queue_f->qlen = (float)last->qlen;
			newitem_queue_f->backlog = (float)last->backlog;
			newitem_queue_f->requeues = (float)(last->requeues - new->requeues) / duration;
			newitem_queue_f->overlimits = (float)(last->overlimits - new->overlimits) / duration;
			newitem_queue_f->sec = last->sec;
			newitem_queue_f->usec = last->usec;
			ptr = queue_list_f;
			queue_list_f = queue_list_f->next;
			free(ptr);
			ptr = NULL;
		}
		pthread_mutex_unlock(&mutex_rdata);
		if (newitem_queue_f != NULL)
		{
			if (queue_p_buffer_f == NULL) //null, create
			{
				queue_p_buffer_f = newitem_queue_f;
				queue_p_last_f = newitem_queue_f;
			}
			else
			{
				int condition = 0;
				if (newitem_queue_f->sec > queue_p_last_f->sec)
					condition = 1;
				else if ((newitem_queue_f->sec == queue_p_last_f->sec) && (newitem_queue_f->usec >= queue_p_last_f->usec))
					condition = 1;
				if (condition) // insert to tail.
				{
					queue_p_last_f->next = newitem_queue_f;
					queue_p_last_f = newitem_queue_f;
				}
				else // out or order, find the right position to insert.
				{
					struct data_queue_computed *ptr = NULL, *ptrpre = NULL;
					ptr = queue_p_buffer_f;
					while (ptr) //if found, ptr is not null
					{
						if (ptr->sec > newitem_queue_f->sec)
						{
							break;
						}
						ptrpre = ptr;
						ptr = ptr->next;
					}
					if (ptrpre) //have searched, insert
					{
						ptrpre->next = newitem_queue_f;
						newitem_queue_f->next = ptr;
					}
				}

			}

		}
		pthread_mutex_lock(&mutex_rdata);
		new_survey = &(survey_list->survey);
		s_last = &(survey_list_last->survey);
		if ((s_last != NULL) && (new_survey != NULL))
		{
			duration = (float)(s_last->time - new_survey->time);
		}

		if (duration >= 900) //statisfy compute condition, compute
		{
			struct data_survey_list *ptr = NULL;
			int len = sizeof(struct data_survey_computed);
			newitem_survey = (struct data_survey_computed *)malloc(len);
			memset(newitem_survey, 0, len);

			newitem_survey->time_busy = (float)(s_last->time_busy - new_survey->time_busy) / duration;
			newitem_survey->time_ext_busy = (float)(s_last->time_ext_busy - new_survey->time_ext_busy) / duration;
			newitem_survey->noise = s_last->noise;
			newitem_survey->time_rx = (float)(s_last->time_rx - new_survey->time_rx) / duration;
			newitem_survey->time_tx = (float)(s_last->time_tx - new_survey->time_tx) / duration;
			newitem_survey->time_scan = (float)(s_last->time_scan - new_survey->time_scan) / duration;
			newitem_survey->sec = s_last->sec;
			newitem_survey->usec = s_last->usec;
			ptr = survey_list;
			survey_list = survey_list->next;
			free(ptr);
			ptr = NULL;
		}
		pthread_mutex_unlock(&mutex_rdata);
		if (newitem_survey != NULL)
		{
			if (survey_p_buffer == NULL) //null, create
			{
				survey_p_buffer = newitem_survey;
				survey_p_last = newitem_survey;
			}
			else
			{
				int condition = 0;
				if (newitem_survey->sec > survey_p_last->sec)
					condition = 1;
				else if ((newitem_survey->sec == survey_p_last->sec) && (newitem_survey->usec >= survey_p_last->usec))
					condition = 1;
				if (condition) // insert to tail.
				{
					survey_p_last->next = newitem_survey;
					survey_p_last = newitem_survey;
				}
				else // out or order, find the right position to insert.
				{
					struct data_survey_computed *ptr = NULL, *ptrpre = NULL;
					ptr = survey_p_buffer;
					while (ptr) //if found, ptr is not null
					{
						if (ptr->sec > newitem_survey->sec)
						{
							break;
						}
						ptrpre = ptr;
						ptr = ptr->next;
					}
					if (ptrpre) //have searched, insert
					{
						ptrpre->next = newitem_survey;
						newitem_survey->next = ptr;
					}
				}

			}
		}
		if((survey_p_last == NULL) || (queue_p_last_3 == NULL))
			continue;
		// time_current = getcurrenttime();
		// printf("duration is %llu %u %u\n", time_current - time_last, retrans_head, retrans_tail);
		// time_last = time_current;
		pthread_mutex_lock(&mutex_rdata);
		if (retrans_head > retrans_tail) // read and update tail.
		{
			time_tp = retrans_times[retrans_tail];
			if((time_tp.sec < queue_p_last_3->sec) && (time_tp.sec < survey_p_last->sec))
			{
				retrans_tail += 1;
				retrans_tail = retrans_tail % THUNDRED;
				drop = 1;
			}
			else
			{
				time_tp.sec = 0;
				time_tp.usec = 0;
			}
		}
		else
		{
			time_tp.sec = 0;
			time_tp.usec = 0;
		}
		pthread_mutex_unlock(&mutex_rdata);

		if (time_tp.sec == 0)
		{
			
			pthread_mutex_lock(&mutex_rdata);
			if(winsize_head > winsize_tail)
				amount = winsize_head - winsize_tail;
			else
				amount = winsize_head > winsize_tail + THUNDRED;
			pthread_mutex_unlock(&mutex_rdata);
			while(amount > 0)
			{
				pthread_mutex_lock(&mutex_rdata);
				time_tp = winsize_times[winsize_tail];
				if((time_tp.sec < queue_p_last_3->sec) && (time_tp.sec < survey_p_last->sec))
				{
					winsize_tail += 1;
					winsize_tail = winsize_tail % THUNDRED;
					amount -= 1;
				}
				else
				{	
					pthread_mutex_unlock(&mutex_rdata);
					break;				
				}			
				pthread_mutex_unlock(&mutex_rdata);
				if (time_tp.sec) //compute statistic when time_tp
				{
					struct data_queue_computed *q_tmp_3 = NULL,  *q_wait_to_del = NULL;
					struct data_queue_computed *q_tmp_f = NULL;
					struct data_survey_computed *s_tmp = NULL, *s_wait_to_del = NULL;
					struct data_queue_computed q_keep_3;
					struct data_queue_computed q_keep_f;
					struct data_survey_computed s_keep;

					// memset(&q_keep_3, 0, sizeof(struct data_queue_computed));
					// memset(&q_keep_f, 0, sizeof(struct data_queue_computed));
					if (queue_p_buffer_3 == NULL)
						continue;
					if (survey_p_buffer == NULL)
						continue;
					if (queue_p_buffer_f == NULL)
						continue;
					q_tmp_3 = queue_p_buffer_3;
					q_tmp_f = queue_p_buffer_f;
					s_tmp = survey_p_buffer;

					while (q_tmp_3) // search time, above,
					{
						if ((time_tp.sec < q_tmp_3->sec) || ((time_tp.sec == q_tmp_3->sec) && (time_tp.usec <= q_tmp_3->usec))) // find state after it.
						{
							q_keep_3 = *q_tmp_3;
							break;
						}
						else if ((time_tp.sec - q_tmp_3->sec) > 10) // delte old data.
						{
							q_wait_to_del = q_tmp_3;
						}
						q_tmp_3 = q_tmp_3->next;
					}
					q_tmp_3 = queue_p_buffer_3;
					if (q_wait_to_del != NULL) // del the old items.
					{
						struct data_queue_computed *tmp = NULL;
						while (q_tmp_3 != q_wait_to_del)
						{
							tmp = q_tmp_3;
							q_tmp_3 = q_tmp_3->next;
							free(tmp);
							tmp = NULL;
						}
						queue_p_buffer_3 = q_wait_to_del;
					}
					q_wait_to_del = NULL;
					while (q_tmp_f) // search time, above,
					{
						if ((time_tp.sec < q_tmp_f->sec) || ((time_tp.sec == q_tmp_f->sec) && (time_tp.usec <= q_tmp_f->usec)))
						{
							q_keep_f = *q_tmp_f;
							break;
						}
						else if ((time_tp.sec - q_tmp_f->sec) > 10) // delte old data.
						{
							q_wait_to_del = q_tmp_f;
						}
						q_tmp_f = q_tmp_f->next;
					}
					q_tmp_f = queue_p_buffer_f;
					if (q_wait_to_del != NULL) // delete the old items.
					{
						struct data_queue_computed *tmp = NULL;
						while (q_tmp_f != q_wait_to_del)
						{
							tmp = q_tmp_f;
							q_tmp_f = q_tmp_f->next;
							free(tmp);
							tmp = NULL;
						}
						queue_p_buffer_f = q_wait_to_del;
					}
					while (s_tmp)
					{
						if ((time_tp.sec < s_tmp->sec) || ((time_tp.sec == s_tmp->sec) && (time_tp.usec <= s_tmp->usec)))
						{
							s_keep = *s_tmp;
							break;
						}
						else if ((time_tp.sec - s_tmp->sec) > 10)
						{
							s_wait_to_del = s_tmp;
						}
						s_tmp = s_tmp->next;
					}
					if (s_wait_to_del != NULL)
					{
						struct data_survey_computed *tmp = NULL;
						s_tmp = survey_p_buffer;
						while (s_tmp != s_wait_to_del)
						{
							tmp = s_tmp;
							s_tmp = s_tmp->next;
							// printf("1 %02x\n", tmp);
							free(tmp);
							// printf("2\n");
							tmp = NULL;
						}
						survey_p_buffer = s_wait_to_del;
					}
					// printf("abc %u %u\n", s_keep.sec, q_keep_3.sec);
					if ((s_keep.sec != 0) && (q_keep_3.sec != 0))
					{
						// fprintf(fretrans, "%d %u, %.4f, %.4f, %.4f, %.4f, %.4f, %d, %f, %.4f, %.4f, %f, %f, %.4f, %.4f, %.4f\n",
						//         1, drop_count, survey_global.time_busy, survey_global.time_ext_busy,
						//         survey_global.time_rx, survey_global.time_tx, survey_global.time_scan,
						//         2412, (float)survey_global.noise, queue_global_3.bytes,
						//         queue_global_3.packets, queue_global_3.qlen, queue_global_3.backlog,
						//         queue_global_3.drops, queue_global_3.requeues, queue_global_3.overlimits);
						int neibours = 0;
						int stations = 0;
						int duration = 0;
						struct time_structure tmp, tmp1;
						if(s_keep.sec > q_keep_3.sec)
						{
							tmp.sec = q_keep_3.sec;
							tmp.usec = q_keep_3.usec;
						}
						else if (s_keep.sec == q_keep_3.sec)
						{
							if(s_keep.usec > q_keep_3.usec)
							{
								tmp.sec = q_keep_3.sec;
								tmp.usec = q_keep_3.usec;
							}
							else
							{
								tmp.sec = s_keep.sec;
								tmp.usec = s_keep.usec;
							}
						}
						else
						{
							tmp.sec = s_keep.sec;
							tmp.usec = s_keep.usec;
						}
						pthread_mutex_lock(&mutex_rdata);
						tmp1 = winsize_times[winsize_tail];
						duration = ((int)tmp1.sec - (int)tmp.sec) + ((int)tmp1.usec - (int)tmp.usec) * 1000000;
						while((duration < 0) && amount > 0)
						{
							winsize_tail += 1;
							winsize_tail = winsize_tail % THUNDRED;
							if(winsize_head > winsize_tail)
								amount = winsize_head - winsize_tail;
							else
								amount = winsize_head > winsize_tail + THUNDRED;
							tmp1 = winsize_times[winsize_tail];
							duration = ((int)tmp1.sec - (int)tmp.sec) + ((int)tmp1.usec - (int)tmp.usec) * 1000000;
						}
						pthread_mutex_unlock(&mutex_rdata);
						if(neibour_lists)
							neibours = neibour_lists->count;
						if(station_lists)
							stations = station_lists->count;
						fprintf(fretrans, "%d, %u, %u, %u, %.4f, %.4f, %.4f, %.4f, %.4f, %d, %f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f\n",
						        drop, drop_count, neibours, stations, s_keep.time_busy, s_keep.time_ext_busy,
						        s_keep.time_rx, s_keep.time_tx, s_keep.time_scan,
						        2412, (float)s_keep.noise, q_keep_3.bytes,
						        q_keep_3.packets, q_keep_3.qlen, q_keep_3.backlog,
						        q_keep_3.drops, q_keep_3.requeues, q_keep_3.overlimits,
						        q_keep_f.bytes, q_keep_f.packets, q_keep_f.qlen, q_keep_f.backlog,
						        q_keep_f.drops, q_keep_f.requeues, q_keep_f.overlimits);
					}
				}
			}
			
		}
		if (time_tp.sec) //compute statistic when time_tp
		{
			struct data_queue_computed *q_tmp_3 = NULL,  *q_wait_to_del = NULL;
			struct data_queue_computed *q_tmp_f = NULL;
			struct data_survey_computed *s_tmp = NULL, *s_wait_to_del = NULL;
			struct data_queue_computed q_keep_3;
			struct data_queue_computed q_keep_f;
			struct data_survey_computed s_keep;

			// memset(&q_keep_3, 0, sizeof(struct data_queue_computed));
			// memset(&q_keep_f, 0, sizeof(struct data_queue_computed));
			if (queue_p_buffer_3 == NULL)
				continue;
			if (survey_p_buffer == NULL)
				continue;
			if (queue_p_buffer_f == NULL)
				continue;
			q_tmp_3 = queue_p_buffer_3;
			q_tmp_f = queue_p_buffer_f;
			s_tmp = survey_p_buffer;

			while (q_tmp_3) // search time, above,
			{
				if ((time_tp.sec < q_tmp_3->sec) || ((time_tp.sec == q_tmp_3->sec) && (time_tp.usec <= q_tmp_3->usec))) // find state after it.
				{
					q_keep_3 = *q_tmp_3;
					break;
				}
				else if ((time_tp.sec - q_tmp_3->sec) > 10) // delte old data.
				{
					q_wait_to_del = q_tmp_3;
				}
				q_tmp_3 = q_tmp_3->next;
			}
			q_tmp_3 = queue_p_buffer_3;
			if (q_wait_to_del != NULL) // del the old items.
			{
				struct data_queue_computed *tmp = NULL;
				while (q_tmp_3 != q_wait_to_del)
				{
					tmp = q_tmp_3;
					q_tmp_3 = q_tmp_3->next;
					free(tmp);
					tmp = NULL;
				}
				queue_p_buffer_3 = q_wait_to_del;
			}
			q_wait_to_del = NULL;
			while (q_tmp_f) // search time, above,
			{
				if ((time_tp.sec < q_tmp_f->sec) || ((time_tp.sec == q_tmp_f->sec) && (time_tp.usec <= q_tmp_f->usec)))
				{
					q_keep_f = *q_tmp_f;
					break;
				}
				else if ((time_tp.sec - q_tmp_f->sec) > 10) // delte old data.
				{
					q_wait_to_del = q_tmp_f;
				}
				q_tmp_f = q_tmp_f->next;
			}
			q_tmp_f = queue_p_buffer_f;
			if (q_wait_to_del != NULL) // delete the old items.
			{
				struct data_queue_computed *tmp = NULL;
				while (q_tmp_f != q_wait_to_del)
				{
					tmp = q_tmp_f;
					q_tmp_f = q_tmp_f->next;
					free(tmp);
					tmp = NULL;
				}
				queue_p_buffer_f = q_wait_to_del;
			}
			while (s_tmp)
			{
				if ((time_tp.sec < s_tmp->sec) || ((time_tp.sec == s_tmp->sec) && (time_tp.usec <= s_tmp->usec)))
				{
					s_keep = *s_tmp;
					break;
				}
				else if ((time_tp.sec - s_tmp->sec) > 10)
				{
					s_wait_to_del = s_tmp;
				}
				s_tmp = s_tmp->next;
			}
			if (s_wait_to_del != NULL)
			{
				struct data_survey_computed *tmp = NULL;
				s_tmp = survey_p_buffer;
				while (s_tmp != s_wait_to_del)
				{
					tmp = s_tmp;
					s_tmp = s_tmp->next;
					// printf("1 %02x\n", tmp);
					free(tmp);
					// printf("2\n");
					tmp = NULL;
				}
				survey_p_buffer = s_wait_to_del;
			}
			// printf("abc %u %u\n", s_keep.sec, q_keep_3.sec);
			if ((s_keep.sec != 0) && (q_keep_3.sec != 0))
			{
				// fprintf(fretrans, "%d %u, %.4f, %.4f, %.4f, %.4f, %.4f, %d, %f, %.4f, %.4f, %f, %f, %.4f, %.4f, %.4f\n",
				//         1, drop_count, survey_global.time_busy, survey_global.time_ext_busy,
				//         survey_global.time_rx, survey_global.time_tx, survey_global.time_scan,
				//         2412, (float)survey_global.noise, queue_global_3.bytes,
				//         queue_global_3.packets, queue_global_3.qlen, queue_global_3.backlog,
				//         queue_global_3.drops, queue_global_3.requeues, queue_global_3.overlimits);
				int neibours = 0;
				int stations = 0;
				if(neibour_lists)
					neibours = neibour_lists->count;
				if(station_lists)
					stations = station_lists->count;
				fprintf(fretrans, "%d, %u, %u, %u, %.4f, %.4f, %.4f, %.4f, %.4f, %d, %f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f, %.4f\n",
				        drop, drop_count, neibours, stations, s_keep.time_busy, s_keep.time_ext_busy,
				        s_keep.time_rx, s_keep.time_tx, s_keep.time_scan,
				        2412, (float)s_keep.noise, q_keep_3.bytes,
				        q_keep_3.packets, q_keep_3.qlen, q_keep_3.backlog,
				        q_keep_3.drops, q_keep_3.requeues, q_keep_3.overlimits,
				        q_keep_f.bytes, q_keep_f.packets, q_keep_f.qlen, q_keep_f.backlog,
				        q_keep_f.drops, q_keep_f.requeues, q_keep_f.overlimits);
			}
		}
		// time_current = getcurrenttime();
		// printf("duration1 is %llu %u\n", time_current - time_last, amount);
		// time_last = time_current;
		usleep(10000);
		// sleep(1);
	}
}
void
start_threads (void)
{
	pthread_t tid1, tid2, tid3;
	int err = 0;

	char buf[128];
	char *ptr = NULL, *ptr1 = NULL;
	char appendix[] 	= "_winsize.txt";
	char appendix1[] 	= "_other.txt";
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
	while (*ptr != '\0')
	{
		// printf("%s ", ptr);
		if ((*ptr == ' ') || (*ptr == ':'))
			*ptr = '_';
		ptr += 1;
	}
	ptr -= 1;
	if ((ptr + strlen(appendix)) < (buf + 128))
	{
		ptr1 = ptr;
		memcpy(ptr, appendix, strlen(appendix));
	}
	ptr = ptr + strlen(appendix);
	*ptr = '\0';
	fp_winsize = fopen (buf, "wb");
	if (fp_winsize != NULL)
		puts ("winsize open successfully");
	else
	{
		puts ("winsize opened failed");
		exit(-1);
	}
	if ((ptr1 + strlen(appendix)) < (buf + 128))
	{
		memcpy(ptr1, appendix1, strlen(appendix1));
	}
	ptr1 = ptr1 + strlen(appendix1);
	*ptr1 = '\0';
	fp = fopen (buf, "wb");
	if (fp != NULL)
		puts ("other open successfully");
	else
	{
		puts ("other opened failed");
		exit(-1);
	}
	fretrans = fopen("retrans.txt", "ab");
	if (fretrans == NULL)
	{
		printf("retrans file openfailed\n");
		exit(-1);
	}
	// strcat(buf, "txt");
	printf("%s\n", buf);
	pclose(pp);
	// int amount = 0;

	// struct aplist mac_test;
	// memset(&mac_test, 0, sizeof(struct aplist));

	// amount = get_aps (&mac_test);
	// printf ("amount is %d\n", amount);

	pthread_mutex_init (&mutex_pool, NULL);
	// pthread_mutex_init (&mutex_data, NULL);
	pthread_mutex_init (&mutex_rdata, NULL);

	hash_table_init ();
	memset (buffer_pool, 0, POOL_SIZE);

	// for (i = 0; i < BUFFER_STORE; i++)
	// {
	// 	iw_buffer[i] =
	// 	    (struct data_iw_processed *)
	// 	    malloc (sizeof (struct data_iw_processed));
	// 	beacon_buffer[i] =
	// 	    (struct data_beacon_processed *)
	// 	    malloc (sizeof (struct data_beacon_processed));
	// 	// queue_buffer[i]     = (struct data_queue_processed *)   malloc(sizeof(struct data_queue_processed));
	// 	dropped_buffer[i] =
	// 	    (struct data_dropped_processed *)
	// 	    malloc (sizeof (struct data_dropped_processed));
	// 	winsize_buffer[i] =
	// 	    (struct data_winsize_processed *)
	// 	    malloc (sizeof (struct data_winsize_processed));
	// 	survey_buffer[i] =
	// 	    (struct data_survey_processed *)
	// 	    malloc (sizeof (struct data_survey_processed));
	// }

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
	// else
	// {
	// 	printf ("23\n");
	// }
	err = pthread_create (&tid3, NULL, format_data, NULL);
	if (err != 0)
	{
		printf ("format_data creation failed \n");
		exit (1);
	}
	// else
	// {
	// 	printf ("format_data\n");
	// }
}

void
destroy_everything (void)
{
	// int i = 0;
	pthread_mutex_destroy (&mutex_pool);
	pthread_mutex_destroy (&mutex_rdata);
	// pthread_mutex_destroy (&mutex_data);
	// printf("Fuck you\n");
	pthread_exit (0);
	fclose (fp);
	fclose (fp_winsize);
	fclose(fretrans);
	hash_table_release ();

	// for (i = 0; i < BUFFER_STORE; i++)
	// {
	// 	free (iw_buffer[i]);
	// 	iw_buffer[i] = NULL;

	// 	free (beacon_buffer[i]);
	// 	beacon_buffer[i] = NULL;

	// 	// free(queue_buffer[i]);
	// 	// queue_buffer[i]     = NULL;

	// 	free (dropped_buffer[i]);
	// 	dropped_buffer[i] = NULL;

	// 	free (winsize_buffer[i]);
	// 	winsize_buffer[i] = NULL;

	// 	free (survey_buffer[i]);
	// 	survey_buffer[i] = NULL;
	// }
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
	printf("destroy_everything\n");
	destroy_everything ();
}

int
main ()
{
	/* code */
	start_program ();
	//
	while (1)
	{
		sleep(1);
	}
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
print_mac (unsigned char *addr)
{
	// int len = 18;
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
strcmp_linger (unsigned char *s, unsigned char *t)
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

// int
// get_aps (struct aplist *ap)
// {

// 	int i = 0, mac_list_last = 0;
// 	// int amount = 0;
// 	// struct aplist *neibours;

// 	if (!ap)
// 		return 0;
// 	pthread_mutex_lock(&mutex_data);
// 	while (bbamount > 0)
// 	{
// 		char tmp[18];
// 		memcpy(tmp, beacon_buffer[bbstart]->mac_addr, 18);
// 		bbstart += 1;
// 		bbamount -= 1;
// 		for (i = 0; i < mac_list_last; i++)
// 		{
// 			if (strcmp(tmp, ap->aplists[i]) == 0)
// 				continue;
// 		}
// 		memcpy(ap->aplists[mac_list_last], tmp, 18);
// 		mac_list_last += 1;
// 	}
// 	pthread_mutex_unlock(&mutex_data);
// 	return mac_list_last;
// }

// void
// get_neighbour_py (char *ap, struct aplist_py *neibours)
// {
// 	int begin = 0;
// 	int end = 0, found = 0;
// 	int i = 0, length = 0, j = 0, mac_list_last = 0;
// 	char mac_list[20][18];
// 	// printf("%s ap\n", ap);
// 	// printf("wocao %d\n", neibours->length);

// 	struct data_beacon_processed *beacon_tmp[BUFFER_STORE];
// 	for (i = 0; i < BUFFER_STORE; i++)
// 		beacon_tmp[i] =
// 		    (struct data_beacon_processed *)
// 		    malloc (sizeof (struct data_beacon_processed));
// 	i = 0;
// 	begin = bbstart1;
// 	end = bbend;
// 	for (i = 0; i < BUFFER_STORE; i++)
// 	{
// 		memcpy (beacon_tmp[i], beacon_buffer[i],
// 		        sizeof (struct data_beacon_processed));
// 	}


// 	if (end >= begin)
// 		length = end - begin;
// 	else
// 		length = end + BUFFER_STORE - begin;
// 	j = begin;
// 	// printf("length is %d bbamount1 %d bbstart1 %d bbend %d\n", length, bbamount1, bbstart1, bbend);
// 	// for(i = 0; i < length; i++)
// 	// {
// 	//   printf("%s strlen %d\n", beacon_tmp[j]->mac_addr, strlen(beacon_tmp[j]->mac_addr));
// 	//   j = j + 1;
// 	//   j = j % BUFFER_STORE;
// 	//   bbstart1 = bbstart1 + 1;
// 	//   bbstart1 = bbstart1 % BUFFER_STORE;
// 	//   bbamount1 = bbamount1 - 1;
// 	//   if(bbamount1 < 0)
// 	//   {
// 	//     bbamount1 = 0;
// 	//   }
// 	// }

// 	for (i = 0; i < length; i++)
// 	{
// 		bbstart1 = bbstart1 + 1;
// 		bbstart1 = bbstart1 % BUFFER_STORE;
// 		bbamount1 = bbamount1 - 1;
// 		if (bbamount1 < 0)
// 		{
// 			bbamount1 = 0;
// 		}
// 		// printf("%s ap %s\n", beacon_tmp[j]->mac_addr, ap);
// 		found = 0;
// 		if (strcmp (beacon_tmp[j]->mac_addr, ap) == 0)
// 		{
// 			if (mac_list_last > 0)
// 			{
// 				int m = 0;
// 				for (m = 0; m < mac_list_last; m++)
// 				{
// 					if (!strcmp (mac_list[m], beacon_tmp[j]->bssid))
// 					{
// 						found = 1;
// 						break;
// 					}
// 				}
// 			}
// 			if (found == 1)
// 			{
// 				j = j + 1;
// 				j = j % BUFFER_STORE;
// 				continue;
// 			}
// 			// printf("hereeee\n");
// 			strcpy (mac_list[mac_list_last], beacon_tmp[j]->bssid);
// 			mac_list_last = mac_list_last + 1;
// 			j = j + 1;
// 			j = j % BUFFER_STORE;
// 		}
// 	}
// 	// printf("mac_list_last is %d\n", mac_list_last);
// 	if (mac_list_last > 0)
// 	{
// 		// printf("here1111111111\n");
// 		neibours->length = mac_list_last;
// 		// printf("here1111111111\n");
// 		// memcpy(neibours->aplists, mac_list, sizeof(mac_list));
// 		for (i = 0; i < mac_list_last; i++)
// 			strcpy (neibours->aplists[i], mac_list[i]);
// 		// printf("here2222222222\n");
// 		// printf("%s neibours in c\n", neibours->aplists[0]);
// 		i = 0;
// 		for (i = 0; i < BUFFER_STORE; i++)
// 		{
// 			free (beacon_tmp[i]);
// 		}
// 		// return neibours;
// 	}
// 	else
// 	{
// 		i = 0;
// 		for (i = 0; i < BUFFER_STORE; i++)
// 		{
// 			free (beacon_tmp[i]);
// 		}
// 		// return NULL;
// 	}

// }

// void
// get_neighbour (char *ap, struct aplist *neibours)
// {
// 	int begin = 0;
// 	int end = 0, found = 0;
// 	int i = 0, length = 0, j = 0, mac_list_last = 0;
// 	char mac_list[20][18];
// 	// printf("%s ap\n", ap);
// 	// printf("wocao %d\n", neibours->length);

// 	struct data_beacon_processed *beacon_tmp[BUFFER_STORE];
// 	for (i = 0; i < BUFFER_STORE; i++)
// 		beacon_tmp[i] =
// 		    (struct data_beacon_processed *)
// 		    malloc (sizeof (struct data_beacon_processed));
// 	i = 0;
// 	begin = bbstart1;
// 	end = bbend;
// 	for (i = 0; i < BUFFER_STORE; i++)
// 	{
// 		memcpy (beacon_tmp[i], beacon_buffer[i],
// 		        sizeof (struct data_beacon_processed));
// 	}


// 	if (end >= begin)
// 		length = end - begin;
// 	else
// 		length = end + BUFFER_STORE - begin;
// 	j = begin;
// 	// // printf("length is %d bbamount1 %d bbstart1 %d bbend %d\n", length, bbamount1, bbstart1, bbend);
// 	// // for(i = 0; i < length; i++)
// 	// // {
// 	// //   printf("%s strlen %d\n", beacon_tmp[j]->mac_addr, strlen(beacon_tmp[j]->mac_addr));
// 	// //   j = j + 1;
// 	// //   j = j % BUFFER_STORE;
// 	// //   bbstart1 = bbstart1 + 1;
// 	// //   bbstart1 = bbstart1 % BUFFER_STORE;
// 	// //   bbamount1 = bbamount1 - 1;
// 	// //   if(bbamount1 < 0)
// 	// //   {
// 	// //     bbamount1 = 0;
// 	// //   }
// 	// // }

// 	for (i = 0; i < length; i++)
// 	{
// 		bbstart1 = bbstart1 + 1;
// 		bbstart1 = bbstart1 % BUFFER_STORE;
// 		bbamount1 = bbamount1 - 1;
// 		if (bbamount1 < 0)
// 		{
// 			bbamount1 = 0;
// 		}
// 		// printf("%s\n", beacon_tmp[j]->mac_addr);
// 		found = 0;
// 		if (strcmp (beacon_tmp[j]->mac_addr, ap) == 0)
// 		{
// 			if (mac_list_last > 0)
// 			{
// 				int m = 0;
// 				for (m = 0; m < mac_list_last; m++)
// 				{
// 					if (!strcmp (mac_list[m], beacon_tmp[j]->bssid))
// 					{
// 						found = 1;
// 						break;
// 					}
// 				}
// 			}
// 			if (found == 1)
// 			{
// 				j = j + 1;
// 				j = j % BUFFER_STORE;
// 				continue;
// 			}
// 			// printf("hereeee\n");
// 			strcpy (mac_list[mac_list_last], beacon_tmp[j]->bssid);
// 			mac_list_last = mac_list_last + 1;
// 			j = j + 1;
// 			j = j % BUFFER_STORE;
// 		}
// 	}
// 	if (mac_list_last > 0)
// 	{
// 		// printf("here1111111111\n");
// 		neibours->length = mac_list_last;
// 		memcpy (neibours->aplists, mac_list, sizeof (mac_list));
// 		i = 0;
// 		for (i = 0; i < BUFFER_STORE; i++)
// 		{
// 			free (beacon_tmp[i]);
// 		}
// 		// return neibours;
// 	}
// 	else
// 	{
// 		i = 0;
// 		for (i = 0; i < BUFFER_STORE; i++)
// 		{
// 			free (beacon_tmp[i]);
// 		}
// 		// return NULL;
// 	}
// 	neibours->length = 10;
// 	// printf("%s is the mac the length is %d\n", neibours->aplists[neibours->length - 1], neibours->length);
// 	// printf("fuck one\n");
// }

// void
// get_clients (char *ap, struct aplist *clients)
// {
// 	int begin = 0, end = 0, found = 0;
// 	int i = 0, j = 0, length = 0, mac_list_last = 0;
// 	struct data_iw_processed *iw_tmp[BUFFER_STORE];
// 	char mac_list[20][18];
// 	// aplist *clients;
// 	for (i = 0; i < BUFFER_STORE; i++)
// 	{
// 		iw_tmp[i] =
// 		    (struct data_iw_processed *)
// 		    malloc (sizeof (struct data_iw_processed));
// 	}
// 	i = 0;
// 	for (i = 0; i < BUFFER_STORE; i++)
// 	{
// 		memcpy (iw_tmp[i], iw_buffer[i], sizeof (struct data_iw_processed));
// 	}

// 	begin = ibstart;
// 	end = ibend;
// 	if (end >= begin)
// 	{
// 		length = end - begin;
// 	}
// 	else
// 	{
// 		length = end + BUFFER_STORE - begin;
// 	}
// 	j = begin;
// 	// printf("length is %d ibamount %d ibstart %d ibend %d\n", length, ibamount, ibstart, ibend);
// 	for (i = 0; i < length; i++)
// 	{
// 		// printf("%s\n", iw_tmp[j]->station);
// 		ibstart = ibstart + 1;
// 		ibstart = ibstart % BUFFER_STORE;
// 		ibamount = ibamount - 1;
// 		if (ibamount < 0)
// 		{
// 			ibamount = 0;
// 		}
// 		found = 0;

// 		if (strcmp (iw_tmp[j]->mac_addr, ap) == 0)
// 		{
// 			if (mac_list_last > 0)
// 			{
// 				int m = 0;
// 				for (m = 0; m < mac_list_last; m++)
// 				{
// 					if (!strcmp (mac_list[m], iw_tmp[j]->station))
// 					{
// 						found = 1;
// 						break;
// 					}
// 				}
// 			}
// 			if (found == 1)
// 			{
// 				j = j + 1;
// 				j = j % BUFFER_STORE;
// 				continue;
// 			}
// 			// printf("hereeee\n");
// 			strcpy (mac_list[mac_list_last], iw_tmp[j]->station);
// 			mac_list_last = mac_list_last + 1;
// 			j = j + 1;
// 			j = j % BUFFER_STORE;
// 		}
// 	}
// 	// clients = (aplist *)malloc(sizeof(struct aplist));

// 	clients->length = mac_list_last;
// 	// printf("amount of clients is %d\n", clients->length);
// 	if (mac_list_last > 0)
// 		memcpy (clients->aplists, mac_list, sizeof(mac_list));
// 	for (i = 0; i < BUFFER_STORE; i++)
// 	{
// 		free (iw_tmp[i]);
// 	}
// 	// free(mac_list);
// 	// retrun clients;
// 	i = 0;
// }

// void
// get_clients_py (char *ap, struct aplist_py *clients)
// {
// 	int begin = 0, end = 0, found = 0;
// 	int i = 0, j = 0, length = 0, mac_list_last = 0;
// 	struct data_iw_processed *iw_tmp[BUFFER_STORE];
// 	char mac_list[20][18];
// 	// aplist *clients;
// 	for (i = 0; i < BUFFER_STORE; i++)
// 	{
// 		iw_tmp[i] =
// 		    (struct data_iw_processed *)
// 		    malloc (sizeof (struct data_iw_processed));
// 	}
// 	i = 0;
// 	for (i = 0; i < BUFFER_STORE; i++)
// 	{
// 		memcpy (iw_tmp[i], iw_buffer[i], sizeof (struct data_iw_processed));
// 	}

// 	begin = ibstart;
// 	end = ibend;
// 	if (end >= begin)
// 	{
// 		length = end - begin;
// 	}
// 	else
// 	{
// 		length = end + BUFFER_STORE - begin;
// 	}
// 	j = begin;
// 	// printf("length is %d ibamount %d ibstart %d ibend %d\n", length, ibamount, ibstart, ibend);
// 	for (i = 0; i < length; i++)
// 	{
// 		// printf("%s\n", iw_tmp[j]->station);
// 		ibstart = ibstart + 1;
// 		ibstart = ibstart % BUFFER_STORE;
// 		ibamount = ibamount - 1;
// 		if (ibamount < 0)
// 		{
// 			ibamount = 0;
// 		}
// 		found = 0;

// 		if (strcmp (iw_tmp[j]->mac_addr, ap) == 0)
// 		{
// 			if (mac_list_last > 0)
// 			{
// 				int m = 0;
// 				for (m = 0; m < mac_list_last; m++)
// 				{
// 					if (!strcmp (mac_list[m], iw_tmp[j]->station))
// 					{
// 						found = 1;
// 						break;
// 					}
// 				}
// 			}
// 			if (found == 1)
// 			{
// 				j = j + 1;
// 				j = j % BUFFER_STORE;
// 				continue;
// 			}
// 			// printf("hereeee\n");
// 			strcpy (mac_list[mac_list_last], iw_tmp[j]->station);
// 			mac_list_last = mac_list_last + 1;
// 			j = j + 1;
// 			j = j % BUFFER_STORE;
// 		}
// 	}
// 	// clients = (aplist *)malloc(sizeof(struct aplist));

// 	clients->length = mac_list_last;
// 	// if(mac_list_last > 0)
// 	// memcpy(clients->aplists, mac_list, sizeof(mac_list));
// 	// printf("amount of clients is %d\n", clients->length);
// 	for (i = 0; i < mac_list_last; i++)
// 	{
// 		strcpy (clients->aplists[i], mac_list[i]);
// 	}
// 	for (i = 0; i < BUFFER_STORE; i++)
// 	{
// 		free (iw_tmp[i]);
// 	}
// 	// free(mac_list);
// 	// retrun clients;
// 	i = 0;
// }

// void
// get_flows (char *ap, struct flowlist *flowlists)
// {
// 	int begin = 0, end = 0, found = 0;
// 	int i = 0, j = 0, length = 0, mac_list_last = 0;
// 	struct data_winsize_processed winsize_tmp[BUFFER_STORE];
// 	struct flow flows[40];
// 	// for(i = 0; i < BUFFER_STORE; i++)
// 	// {
// 	//   winsize_tmp[i] = (struct data_winsize_processed *)malloc(sizeof(struct data_winsize_processed));
// 	// }
// 	// printf("passed length is %d\n", flowlists->length);
// 	for (i = 0; i < BUFFER_STORE; i++)
// 	{
// 		memcpy (&winsize_tmp[i], winsize_buffer[i],
// 		        sizeof (struct data_winsize_processed));
// 	}
// 	i = 0;
// 	begin = wbstart;
// 	end = wbend;
// 	if (end >= begin)
// 	{
// 		length = end - begin;
// 	}
// 	else
// 	{
// 		length = end + BUFFER_STORE - begin;
// 	}
// 	j = begin;

// 	for (i = 0; i < length; i++)
// 	{
// 		int m = 0;
// 		wbstart = wbstart + 1;
// 		wbstart = wbstart % BUFFER_STORE;
// 		// printf("herab\n");
// 		wbamount = wbamount - 1;
// 		if (wbamount < 0)
// 		{
// 			wbamount = 0;
// 		}
// 		found = 0;
// 		// printf("%d\n", winsize_tmp[j]->ip_src);
// 		for (m = 0; m < mac_list_last; m++)
// 		{
// 			if ((winsize_tmp[j].ip_src == flows[m].ip_src)
// 			        && (winsize_tmp[j].ip_dst == flows[m].ip_dst)
// 			        && (winsize_tmp[j].sourceaddr == flows[m].port_src)
// 			        && (winsize_tmp[j].destination == flows[m].port_dst))
// 			{
// 				found = 1;
// 				break;
// 			}
// 		}
// 		if (found == 1)
// 		{
// 			j = j + 1;
// 			j = j % BUFFER_STORE;
// 			continue;
// 		}
// 		flows[mac_list_last].ip_src = winsize_tmp[j].ip_src;
// 		flows[mac_list_last].ip_dst = winsize_tmp[j].ip_dst;
// 		flows[mac_list_last].port_src = winsize_tmp[j].sourceaddr;
// 		flows[mac_list_last].port_dst = winsize_tmp[j].destination;
// 		mac_list_last = mac_list_last + 1;
// 		j = j + 1;
// 		j = j % BUFFER_STORE;
// 	}
// 	// printf("wbamount %d wbstart %d wbend %d\n", wbamount, wbstart, wbend);
// 	flowlists->length = mac_list_last;
// 	for (i = 0; i < mac_list_last; i++)
// 	{
// 		memcpy (&flowlists->flows[i], &flows[i], sizeof (struct flow));
// 	}
// 	// printf("h1 amount of flows is %d\n", mac_list_last);
// 	// i = 0;
// 	// for(i = 0; i < BUFFER_STORE; i++)
// 	// {
// 	//   if(winsize_tmp[i])
// 	//     free(winsize_tmp[i]);
// 	// }
// 	// printf("h4\n");
// 	i = 0;
// }

// void get_flow_drops (char *ap, struct flow_drop_ap *flow_drops)
// {
// 	int begin = 0, total_drops = 0;
// 	int end = 0;
// 	int i = 0, length = 0, j = 0, mac_list_last = 0;
// 	struct flow_and_dropped mac_list[50];


// 	struct data_dropped_processed *dropped_tmp[BUFFER_STORE];
// 	for (i = 0; i < BUFFER_STORE; i++)
// 		dropped_tmp[i] =
// 		    (struct data_dropped_processed *)
// 		    malloc (sizeof (struct data_dropped_processed));
// 	for (i = 0; i < BUFFER_STORE; i++)
// 	{
// 		memcpy (dropped_tmp[i], dropped_buffer[i],
// 		        sizeof (struct data_dropped_processed));
// 	}
// 	i = 0;
// 	begin = dbstart;
// 	end = dbend;

// 	if (end >= begin)
// 		length = end - begin;
// 	else
// 		length = end + BUFFER_STORE - begin;
// 	j = begin;
// 	for (i = 0; i < length; i++)
// 	{
// 		dbstart = dbstart + 1;
// 		dbstart = dbstart % BUFFER_STORE;
// 		dbamount = dbamount - 1;
// 		if (dbamount < 0)
// 		{
// 			dbamount = 0;
// 		}
// 		if (strcmp (dropped_buffer[j]->mac_addr, ap) == 0)
// 		{
// 			int k = 0;
// 			for (k = 0; k < mac_list_last; k++)
// 			{
// 				bool condition = 0;
// 				condition =
// 				    (dropped_tmp[j]->ip_src == mac_list[k].dataflow.ip_src)
// 				    && (dropped_tmp[j]->ip_dst == mac_list[k].dataflow.ip_dst);
// 				condition = condition
// 				            &&
// 				            ((dropped_tmp[j]->port_src == mac_list[k].dataflow.port_src)
// 				             && (dropped_tmp[j]->port_dst ==
// 				                 mac_list[k].dataflow.port_dst));
// 				if (condition)
// 				{
// 					mac_list[k].drops = mac_list[k].drops + 1;
// 					total_drops = total_drops + 1;
// 				}
// 			}
// 			j = j + 1;
// 			j = j % BUFFER_STORE;
// 		}
// 		else
// 		{
// 			j = j + 1;
// 			j = j % BUFFER_STORE;
// 			continue;
// 		}
// 	}
// 	flow_drops->drops_total = total_drops;
// 	flow_drops->length = mac_list_last;
// 	for (i = 0; i < mac_list_last; i++)
// 	{
// 		memcpy (&flow_drops->flowinfo[i], &mac_list[i],
// 		        sizeof (struct flow_and_dropped));
// 	}
// 	for (i = 0; i < BUFFER_STORE; i++)
// 	{
// 		free (dropped_tmp[i]);
// 	}
// 	i = 0;
// }

// bool
// get_wireless_info (char *ap, struct wireless_information_ap *clients)
// {
// 	int begin = 0, end = 0;
// 	bool found = 0;
// 	int i = 0, j = 0, length = 0;
// 	struct data_iw_processed *iw_tmp[BUFFER_STORE];
// 	// aplist *clients;
// 	// printf("received busy_time is %d ap is %s\n", clients->busy_time, ap);
// 	for (i = 0; i < BUFFER_STORE; i++)
// 	{
// 		iw_tmp[i] =
// 		    (struct data_iw_processed *)
// 		    malloc (sizeof (struct data_iw_processed));
// 	}
// 	i = 0;
// 	for (i = 0; i < BUFFER_STORE; i++)
// 	{
// 		memcpy (iw_tmp[i], survey_buffer[i], sizeof (struct data_iw_processed));
// 	}

// 	begin = sbstart;
// 	end = sbend;
// 	if (end >= begin)
// 	{
// 		length = end - begin;
// 	}
// 	else
// 	{
// 		length = end + BUFFER_STORE - begin;
// 	}
// 	j = begin;
// 	for (i = 0; i < length; i++)
// 	{
// 		// printf("%llu\n", iw_tmp[j]->active_time);
// 		sbstart = sbstart + 1;
// 		sbstart = sbstart % BUFFER_STORE;
// 		sbamount = sbamount - 1;
// 		if (sbamount < 0)
// 		{
// 			sbamount = 0;
// 		}
// 		if (strcmp (iw_tmp[j]->mac_addr, ap) == 0)
// 		{
// 			// printf("ap matched\n");
// 			clients->noise = survey_buffer[j]->noise;
// 			clients->active_time = survey_buffer[j]->time;
// 			clients->busy_time = survey_buffer[j]->time_busy;
// 			clients->transmit_time = survey_buffer[j]->time_tx;
// 			clients->receive_time = survey_buffer[j]->time_rx;
// 			found = 1;
// 			break;
// 		}
// 		j = j + 1;
// 		j = j % BUFFER_STORE;
// 	}
// 	// clients = (aplist *)malloc(sizeof(struct aplist));

// 	for (i = 0; i < BUFFER_STORE; i++)
// 	{
// 		free (iw_tmp[i]);
// 	}
// 	return found;
// }

