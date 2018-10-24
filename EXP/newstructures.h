
enum data_category {WINSIZE = 1, DROPS, IW, QUEUE, BEACON, SURVEY, KEEP_IW, KEEP_BEACON};
enum found_results {FOUND_SAME = 1, FOUND_DIFF, NOT_FOUND};

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
  // struct dp_packet *next;
  __u32 pad2;
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
  __u32 pad2;
  // __u32 pad;
};


// struct data_iw_mac
// {
//   char station[6];
//   __u8 mac[6];
//   __u16 device;
//   __be32 sec;
//   __be32 usec;
//   __u32 inactive_time;
//   __u32 rx_bytes;
//   __u32 rx_packets;
//   __u32 tx_bytes;
//   __u32 tx_packets;
//   __u32 tx_retries;
//   __u32 tx_failed;
//   __s32 signal;
//   __s32 signal_avg;
//   __u32 expected_throughput;
//   struct data_iw_mac *next;
// };

// struct data_iw_new
// {
//     char station[6];
//     __u16 device;
//     __u32 inactive_time;
//     __u32 rx_bytes;
//     __u32 rx_packets;
//     __u32 tx_bytes;
//     __u32 tx_packets;
//     __u32 tx_retries;
//     __u32 tx_failed;
//     __s32 signal;
//     __s32 signal_avg;
//     __u32 expected_throughput;
// };

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

struct data_survey_list
{
  struct data_survey survey;
  struct data_survey_list *next;
};

struct data_survey_computed
{
  __u32 sec;
  __u32 usec;
  float time_busy;
  float time_ext_busy;
  float time_rx;
  float time_tx;
  float time_scan;
  __s8 noise;
  struct data_survey_computed *next;
};

struct time_structure
{
  __u32 sec;
  __u32 usec;
};
struct drop_time
{
  __u32 sec;
  __u32 usec;
  __u32 count;
};
struct data_survey_c
{
  __u8  mac[6];
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
  struct data_survey_c *next;
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
  __u32 pad2;
  // struct data_beacon *next;
  __u32 pad1;
};


struct data_beacon_mac
{
  __be32 sec;
  __be32 usec;
  __u16 data_rate;
  __u16 freq;
  __u32 timein;
  __s8 signal;
  __u8 pad;
  __u8 bssid[6];
  __u8 mac[6];
  __u16 pad1;
  struct data_beacon *next;
};

// struct data_beacon_new
// {
//     __u16 data_rate;
//     __u16 freq;
//     __s8 signal;
//     __u8 bssid[6];
//     __u16 pad;
// };
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

struct data_queue_computed
{
  float packets;
  float bytes;
  __u32 sec;
  __u32 usec;
  float qlen;
  float backlog;
  float drops;
  float requeues;
  float overlimits;
  struct data_queue_computed *next;
};
struct data_queue_list
{
  struct data_queue queue;
  struct data_queue_list *next;
};

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
struct seq_info
{
  __be32 seq;
  __u32 sec;
  __u32 usec;
};
struct winsize_record
{
  __be32 ip_src;
  __be32 ip_dst;
  __be16 sourceaddr;
  __be16 destination;
  struct seq_info seq_list[100];
  __be32 ack_seq;
  __be32 sec;
  // __be32 usec;
};

struct station_list 
{
  char station[6];
  __u32 sec;
  __u32 count;
  struct station_list *next;
};
struct winsize_t
{
  __u16 category;
  char mac[6];
  struct  data_winsize winsize;
};
struct dp_t
{
  __u16 category;
  char mac[6];
  struct  dp_packet drops;
};
struct di_t
{
  __u16 category;
  char mac[6];
  struct  data_iw iw;
};
struct dq_t
{
  __u16 category;
  char mac[6];
  struct  data_queue queue;
};
struct db_t
{
  __u16 category;
  char mac[6];
  struct  data_beacon beacon;
};
struct ds_t
{
  __u16 category;
  char mac[6];
  struct  data_survey survey;
};

// struct keep_iw
// {
//   __u16 category;
//   char station[6];
//   char mac[6];
//   __u16 pad;
// };

// struct keep_beacon
// {
//   __u16 category;
//   char bssid[6];
//   char mac[6];
//   __u16 pad;
// };

// struct aplist_py
// {
//   char **aplists;
//   int length;
// };

// struct aplist
// {
//   char aplists[20][6];
//   int length;
// };


// struct flow
// {
//   __u8 mac[6];
//   __be32 ip_src;
//   __be32 ip_dst;
//   __u16 port_src;
//   __u16 port_dst;
// };

// struct flow_and_dropped
// {
//   struct flow dataflow;
//   int drops;
// };

// struct flow_and_dropped_c
// {
//   struct flow dataflow;
//   int drops;
//   struct flow_and_dropped_c *next;
// };

// struct flow_drop_ap_pointer
// {
//   struct flow_and_dropped *flowinfo;
//   int drops_total;
//   int length;
// };


// struct flow_drop_ap_array
// {
//   struct flow_and_dropped flowinfo[50];
//   int drops_total;
//   int length;
// };

// struct wireless_information_ap
// {
//   __s32 noise;
//   __u32 connected_time;
//   __u64 active_time;
//   __u64 busy_time;
//   __u64 receive_time;
//   __u64 transmit_time;
// };

// struct flowlist
// {
//   struct flow flows[40];
//   int length;
// };

__u64 getcurrenttime(void)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (__u64)tv.tv_sec * 1000000 + (__u64)tv.tv_usec;
}

//big endian to small endian
void reversebytes_uint16t(__u16 *value)
{
  *value = (*value & 0x00FF) << 8 | (*value & 0xFF00) >> 8;
}
void reversebytes_uint32t(__u32 *value)
{
  *value = (*value & 0x000000FFU) << 24 | (*value & 0x0000FF00U) << 8 |
           (*value & 0x00FF0000U) >> 8 | (*value & 0xFF000000U) >> 24;
}

void reversebytes_sint32t(__s32 *value)
{
  *value = (*value & 0x000000FFU) << 24 | (*value & 0x0000FF00U) << 8 |
           (*value & 0x00FF0000U) >> 8 | (*value & 0xFF000000U) >> 24;
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

void inttou64(char tmp[], __u64 n)
{
  memcpy(tmp, &n, 8);
}

__u64 chartou64(char a[])
{
  __u64 n = 0;
  memcpy(&n, a, 8);
  return n;
}

unsigned long strtou32(char *str)
{
  unsigned long temp = 0;
  unsigned long fact = 1;
  unsigned char len = strlen(str);
  unsigned char i;
  for (i = len; i > 0; i--)
  {
    temp += ((str[i - 1] - 0x30) * fact);
    fact *= 10;
  }
  return temp;

}

void u16tostr(__u16 dat, char *str)
{
  char temp[20];
  unsigned char i = 0, j = 0;
  i = 0;
  while (dat)
  {
    temp[i] = dat % 10 + 0x30;
    i++;
    dat /= 10;
  }
  j = i;
  for (i = 0; i < j; i++)
  {
    str[i] = temp[j - i - 1];
  }
  if (!i)
  {
    str[i++] = '0';
  }
  str[i] = 0;
}
//used to tranform 04a151a3571a to 04:a1:51:a3:57:1a
void mac_tranADDR_toString_r(unsigned char* addr, char* str, size_t size)
{
  if (addr == NULL || str == NULL || size < 18)
    exit(1);

  snprintf(str, size, "%02x:%02x:%02x:%02x:%02x:%02x",
           addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
  str[17] = '\0';
}

struct data_winsize_processed {
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
  char mac_addr[18];
  // char eth_src[18];
  // char eth_dst[18];
  int kind;
  int length;
};


struct data_iw_processed
{
  char station[18];
  char mac_addr[18];
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
  __s32 noise;
  __u32 expected_throughput;
};
struct data_queue_processed {
  __u64 time;
  __u32 queue_id;
  __u64 bytes;
  __be32 sec;
  __be32 usec;
  __u32 packets;
  __u32 qlen;
  __u32 backlog;
  __u32 drops;
  __u32 requeues;
  char mac_addr[18];
};

struct data_beacon_processed
{
  char mac_addr[18];
  __be32 sec;
  __be32 usec;
  __u16 data_rate;
  __u16 freq;
  __u32 timein;
  __s8 signal;
  char bssid[18];
};

struct data_dropped_processed
{
  char mac_addr[18];
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
  __u32 in_time;
};

struct data_survey_processed
{
  char mac_addr[18];
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

struct aplist
{
  char aplists[20][18];
  int length;
};

struct aplist_py
{
  char **aplists;
  int length;
};

struct flow
{
  __be32 ip_src;
  __be32 ip_dst;
  __u16 port_src;
  __u16 port_dst;
};
struct flowlist
{
  struct flow flows[40];
  int length;
};

struct flow_and_dropped
{
  struct flow dataflow;
  int drops;
};

struct flow_drop_ap
{
  struct flow_and_dropped flowinfo[50];
  int drops_total;
  int length;
};

struct wireless_information_ap
{
  __s32 noise;
  __u32 connected_time;
  __u64 active_time;
  __u64 busy_time;
  __u64 receive_time;
  __u64 transmit_time;
};


