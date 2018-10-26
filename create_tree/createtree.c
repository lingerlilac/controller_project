#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <linux/netlink.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include<arpa/inet.h>

#define NETLINK_USER 22
#define USER_MSG    (NETLINK_USER + 1)
#define MSG_LEN 150
#define ITEM_AMOUNT 100
#define MAX_PLOAD 150
#define FLOWS_KEEP 10
#define PORT_LINGER 12346

#define NETLINK_USER 22
#define USER_MSG    (NETLINK_USER + 1)
#define MSG_LEN 150
#define ITEM_AMOUNT 100
#define MAX_PLOAD 150
#define FLOWS_KEEP 10
#define PORT_LINGER 12346

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

struct flow_info
{
  __be16 port;
  __u32 snd_cwnd;
  __u32 srtt_us;
  __u32 mdev_us;
  __u32 timein;
  __u32 mss_linger;
};

struct tcp_flow
{
  __be32 ip_src;
  __be32 ip_dst;
  __be16 sourceaddr;
  __be16 destination;
  __u16 winsize;
  __u32 sec;
  __u32 usec;
  __u64 packets;
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
//  float drop_count;
//  float time_busy;
//  float time_ext_busy;
//  float time_rx;
//  float time_tx;
//  float time_scan;
//  float noise;
//  float packets;
//  float bytes;
//  float qlen;
//  float backlog;
//  float drops;
//  float requeues;
//  float overlimits;
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
/**
 * Used to get the max rate link, calculate link status
 */
struct iw_record
{
  char station[6];
  __u32 sec;
  __u32 usec;
  __u64 bytes;
  __u32 rates;
  __u16 computed;
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
void mac_tranADDR_toString_r(unsigned char* addr, char* str, size_t size);
void reversebytes_uint32t(__u32 *value);
void reversebytes_uint64t(__u64 *value);
void set_cwnd(struct tcp_flow* tflow, char *ret, int size, int window);
char* get_openflow_input(void);
int main_linger(void);
void assign_string(struct item_value* d, char *s);
void destroy_everything(void);
int init_linger_m(char * inputtxt);
int init_linger(char * inputtxt, char * item);
void free_rawdata(struct rawdata * head);
void print_rawdata(struct rawdata* raw);
struct treenode * create_tree(struct rawdata *ptr);
void print_tree(struct treenode *raw);
void destroy_tree(struct treenode *T);
struct rawdata * find_lchild(struct rawdata *ptr, int father);
struct rawdata * find_rchild(struct rawdata *ptr, int father);
int TreeDepth(struct treenode *T);
struct treenode* bst_search(struct treenode* node, struct item_value value[], int length);
char ** get_parameters(struct rawdata *ptr);
void destroy_parameters(char *ptr[80]);
int create_items(struct item_value items_list[], char *buffer);
void destroy_item_list(struct item_value items_list[]);
static void get_systime_linger(__be32 *t, __be32 *v);
int ppp(int port_linger, char *ip_linger);
int check_cwnd(int sock, char *ip_src, char *ip_dst, __be16 src_port, __be16 dst_port);
void reversebytes_uint16t(__u16 *value);
// void handle_msg(const struct ofpbuf *msg);

struct treenode *tnode = NULL;
struct item_value items_list[ITEM_AMOUNT];
char **parameters;
struct rawdata *node = NULL;
int skfd = -1;
int sock = -1;
/**
 * 0010 0100 -> 0100 0010
 * @param value 0010 0100
 */
void mac_tranADDR_toString_r(unsigned char* addr, char* str, size_t size)
{
  if (addr == NULL || str == NULL || size < 18)
    exit(1);

  snprintf(str, size, "%02x:%02x:%02x:%02x:%02x:%02x",
           addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
  str[17] = '\0';
}
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
void set_cwnd(struct tcp_flow* tflow, char *ret, int size, int window)
{
  // int i = 0;
  char ip_src[20];
  char ip_dst[20];
  char buff[200];
  // int cwnd = 0;
  unsigned char *ptr_uc;
  ptr_uc = (unsigned char *) & (tflow->ip_src);
  sprintf(ip_src, "%u.%u.%u.%u", ptr_uc[0], ptr_uc[1], ptr_uc[2], ptr_uc[3]);
  ptr_uc = (unsigned char *) & (tflow->ip_dst);
  sprintf(ip_dst, "%u.%u.%u.%u", ptr_uc[0], ptr_uc[1], ptr_uc[2], ptr_uc[3]);
  reversebytes_uint16t(&(tflow->winsize));
  reversebytes_uint16t(&(tflow->sourceaddr));
  reversebytes_uint16t(&(tflow->destination));

  memset(buff, 0, 200);
  sprintf(buff, "ovs-ofctl add-flow br0 -O openflow13 tcp,nw_dst=%s,nw_dst=%s,tcp_src=%d,tcp_dst=%d,actions=mod_tp_dst:%d",
          ip_src, ip_dst, tflow->sourceaddr, tflow->destination, window);
  strncpy(ret, buff, size);
}

char* get_openflow_input(void)
{
  char *str = NULL;
  char inputtxt[] = "0,0,requeues_3,5158.975,0|0,1,busy_time_1,808.831,2|1,2,leaf,0,2|1,3,allpackets_1,150.5,2|3,4,leaf,0,2|3,5,requeues_1,678.932,2|5,6,drops_1,3211.45,2|6,7,leaf,0,2|6,8,leaf,0,2|5,9,leaf,0,2|0,10,busy_time_1,808.542,0|10,11,retrans_4,14.5,0|11,12,allpackets_3,182.5,0|12,13,retrans_5,14.5,0|13,14,retrans_3,0.5,0|14,15,receive_time_1,122.551,0|15,16,leaf,0,0|15,17,leaf,0,1|14,18,transmit_time_4,32.177,0|18,19,leaf,0,0|18,20,leaf,0,0|13,21,leaf,0,0|12,22,allpackets_1,202.5,0|22,23,rbytes_3,257018.672,0|23,24,leaf,0,0|23,25,allpackets_1,170.5,0|25,26,leaf,0,0|25,27,leaf,0,0|22,28,leaf,0,0|11,29,leaf,0,0|10,30,leaf,0,2";
  int len = 0;
  len = strlen(inputtxt) + 1;

  str = (char *)malloc(sizeof(char) * len);
  memset(str, 0, len);
  strcpy(str, inputtxt);

  return str;
}

int main_linger(void)
{
  char *data = "flows";
  char *data1 = "states";
  char *data2 = "maxras";
  char *data3 = "flooss";
  struct sockaddr_nl  local, dest_addr;

  struct nlmsghdr *nlh = NULL;
  struct _my_msg info;
  int ret;
  // struct tcp_flow control;
  struct apstates apst;
  char ret_none[10];
  struct iw_record iw_rec;
  struct flow_info flowin;
  char mac_addr[18];

  struct treenode* search_results = NULL;
  if (skfd < 0)
  {
    skfd = socket(AF_NETLINK, SOCK_RAW, USER_MSG);
    if (skfd == -1)
    {
      printf("create socket error...%s\n", strerror(errno));
      destroy_everything();
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
      destroy_everything();
      return -1;
    }
  }
  if (skfd < 0)
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
  ret = recvfrom(skfd, &info, sizeof(struct _my_msg), 0, (struct sockaddr *)&dest_addr, (socklen_t*)sizeof(dest_addr));
  memset(ret_none, 0, 10);
  memcpy(ret_none, info.data, 8);

  if (!strcmp(ret_none, "nonedata"))
  {
    printf("3331111111111111111111111111111111%s\n", ret_none);
  }
  else
  {
    float times = 0.000001;
    struct item_value fstates[14];
    int i = 0;
    // char len[14][20] = {"drop_count", "time_busy", "time_ext_busy", "time_rx", "time_tx", "time_scan", "noise", "packets", "bytes", "qlen", "backlog", "drops", "requeues", "overlimits"};
    // memset(&fstates, 0, sizeof(struct apstates_float));
    memset(&apst, 0, sizeof(struct apstates));
    memcpy(&apst, info.data, sizeof(struct apstates));

    // reversebytes_uint32t(&(apst.sec));
    // reversebytes_uint32t(&(apst.usec));
    // reversebytes_uint64t(&(apst.time_busy));
    // reversebytes_uint64t(&(apst.time_ext_busy));
    // printf("W %u %u %u %llu %llu %llu %d\n", apst.sec, apst.usec, apst.drop_count, apst.time, apst.time_busy, apst.time_ext_busy, sizeof(struct apstates));
    fstates[drop_count].value = ((float)apst.drop_count) * times;
    assign_string(&fstates[drop_count], "drop_count");
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

    for (i = 0; i < 14; i++)
    {
      printf("%s: %f\t", fstates[i].name, fstates[i].value);
      if (!(i % 4))
        printf("\n");
    }
    printf("\n");

    search_results = bst_search(tnode, fstates, 14);
    if (search_results)
    {
      struct tcp_flow max_flow;

      memset(&max_flow, 0, sizeof(struct tcp_flow));
      printf("%s\n", data);
      memcpy(NLMSG_DATA(nlh), data, strlen(data));
      ret = sendto(skfd, nlh, nlh->nlmsg_len, 0, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_nl));

      if (!ret)
      {
        perror("sendto error1\n");
        close(skfd);
        exit(-1);
      }
      // memset(&info, 0, sizeof(info));
      // ret = recvfrom(skfd, &info, sizeof(struct _my_msg), 0, (struct sockaddr *)&dest_addr, (socklen_t*)sizeof(dest_addr));
      // memcpy(&control, info.data, sizeof(struct tcp_flow));

      // if (!ret)
      // {
      //  perror("recv form kernel error\n");
      //  close(skfd);
      //  exit(-1);
      // }

      for (i = 0; i < FLOWS_KEEP; i++)
      {

        struct tcp_flow tmp;
        // float weight = 0;
        // char *ret1 = NULL;

        memset(&tmp, 0, sizeof(struct tcp_flow));

        ret = recvfrom(skfd, &info, sizeof(struct _my_msg), 0, (struct sockaddr *)&dest_addr, (socklen_t*)sizeof(dest_addr));

        memcpy(&tmp, info.data, sizeof(struct tcp_flow));

        // ret1 = malloc(sizeof(char) * 200);
        // set_cwnd(search_results->class, &tmp, ret1, 200);

        // free(ret1);
        // ret1 = NULL;
        if (tmp.packets > max_flow.packets)
          max_flow = tmp;
      }
      if (max_flow.sec != 0)
      {
        char *ret1 = NULL;

        char ip_src[20];
        char ip_dst[20];


        int port = max_flow.sourceaddr;
        int window = 0;
        unsigned char *ptr_uc = NULL;

        memset(ip_src, 0, 20);
        memset(ip_dst, 0, 20);

        ptr_uc = (unsigned char *) &max_flow.ip_src;
        sprintf (ip_src, "%u.%u.%u.%u", ptr_uc[3], ptr_uc[2], ptr_uc[1], ptr_uc[0]);
        ptr_uc = (unsigned char *) &max_flow.ip_dst;
        sprintf (ip_dst, "%u.%u.%u.%u", ptr_uc[3], ptr_uc[2], ptr_uc[1], ptr_uc[0]);
        if (sock < 0)
          sock = ppp(port, ip_src);
        if (sock > 0)
        {
          window = check_cwnd(sock, ip_src, ip_dst, max_flow.sourceaddr, max_flow.destination);
          ret1 = malloc(sizeof(char) * 200);
          set_cwnd(&max_flow, ret1, 200, window);
          printf("ssss %s %d %d\n", ret1, max_flow.sourceaddr, max_flow.destination);
          free(ret1);
          ret1 = NULL;
        }
        // else
      }
      memcpy(NLMSG_DATA(nlh), data2, strlen(data2));
      ret = sendto(skfd, nlh, nlh->nlmsg_len, 0, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_nl));
      if (!ret)
      {
        perror("sendto error1\n");
        close(skfd);
        exit(-1);
      }
      memset(&info, 0, sizeof(info));
      ret = recvfrom(skfd, &info, sizeof(struct _my_msg), 0, (struct sockaddr *)&dest_addr, (socklen_t*)sizeof(dest_addr));
      memset(&iw_rec, 0, sizeof(struct iw_record));
      memcpy(&iw_rec, info.data, sizeof(struct iw_record));
      mac_tranADDR_toString_r((unsigned char*)iw_rec.station, mac_addr, 18);


      memcpy(NLMSG_DATA(nlh), data3, strlen(data3));
      ret = sendto(skfd, nlh, nlh->nlmsg_len, 0, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_nl));
      if (!ret)
      {
        perror("sendto error1\n");
        close(skfd);
        exit(-1);
      }

      memset(&info, 0, sizeof(info));
      ret = recvfrom(skfd, &info, sizeof(struct _my_msg), 0, (struct sockaddr *)&dest_addr, (socklen_t*)sizeof(dest_addr));
      for (i = 0; i < 5; i++)
      {
        memset(&flowin, 0, sizeof(struct flow_info));
        memcpy(&flowin, info.data, sizeof(struct flow_info));
        reversebytes_uint16t (&flowin.port);
        printf("flow info: %d %u %u %u %u %u\n", flowin.port, flowin.snd_cwnd, flowin.srtt_us, flowin.mdev_us, flowin.timein, flowin.mss_linger);
      }
    }
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
  else
    printf("not found\n");
  // get_flows();

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
}

/**
 * clear structures
 */
void destroy_everything(void)
{
  destroy_item_list(items_list);
  destroy_parameters(parameters);
  destroy_tree(tnode);
  free_rawdata(node);
  if (node)
    node = NULL;
  if (tnode)
    tnode = NULL;
  if (parameters)
    parameters = NULL;
  // if(items_list)
  //  items_list = NULL;
}
int init_linger_m(char * inputtxt)
{
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

  tnode = create_tree(node);
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
  char *str = NULL;

  const char * split = "|";
  char * p;
  struct rawdata *nodeptr = NULL;

  char *outer_ptr = NULL;
  char *inner_ptr = NULL;

  // struct item_value *value[80];

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
  tnode = create_tree(node);
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

struct treenode * create_tree(struct rawdata *ptr)
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
      newnode->lchild = create_tree(ptrtmp);
      ptrtmp = find_rchild(ptr->next, ptr->itself);
      newnode->rchild = create_tree(ptrtmp);
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
  return NULL;
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
  return NULL;
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
    if (found == 0)
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
    // char *str = NULL;
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

  for (j = 0; j < in; j++)
  {
    if (j % 2 ==  0)
    {

      items_list[index].name = (char *)malloc(sizeof(char) * strlen(p[j]));
      memset(items_list[index].name, 0, sizeof(char) * strlen(p[j]));
      strcpy(items_list[index].name, p[j]);
    }
    else
    {

      items_list[index].value = atof(p[j]);
      if (index >= ITEM_AMOUNT)
      {
        printf("outrage\n");
        exit(-1);
      }

      index = index + 1;
    }
  }
  return 0;
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



static void get_systime_linger(__be32 *t, __be32 *v)
{
  struct timeval tv;
  memset(&tv, 0, sizeof(struct timeval));
  gettimeofday(&tv, NULL);
  *t = tv.tv_sec;
  *v = tv.tv_usec;
}

int ppp(int port_linger, char *ip_linger)
{

  //创建一个用来通讯的socket
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0)
  {
    perror("socket");
    exit(-1);
  }

  //需要connect的是对端的地址，因此这里定义服务器端的地址结构体
  struct sockaddr_in server;
  struct sockaddr_in local;
  server.sin_family = AF_INET;
  server.sin_port = htons(port_linger);
  server.sin_addr.s_addr = inet_addr(ip_linger);
  socklen_t len = sizeof(struct sockaddr_in);

  local.sin_family = AF_INET;
  local.sin_port = htons(PORT_LINGER);
  local.sin_addr.s_addr = htonl(INADDR_ANY);

  socklen_t len1 = sizeof(local);
  if (bind(sock, (struct sockaddr*)&local , len1) < 0)
  {
    perror("bind");
    exit(2);
  }

  if (connect(sock, (struct sockaddr*)&server, len) < 0 )
  {
    perror("connect");
    exit(-1);
  }
  //连接成功进行收数据

  // check_cwnd(sock);
  // close(sock);

  return sock;
}

int check_cwnd(int sock, char *ip_src, char *ip_dst, __be16 src_port, __be16 dst_port)
{
  char buf[1024];
  __u32 sec0 = 0;
  __u32 usec0 = 0;
  __u32 sec1 = 0, usec1 = 0;
  __u32 duration = 0;
  int ret = 0;
  struct tcp_ss *tmp = NULL;
  // char ip_src[] = "192.168.11.108";
  // char ip_dst[] = "192.168.11.103";
  // __be16 src_port = 12300;
  // __be16 dst_port = 12301;
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
  _s = read(sock, buf, sizeof(buf) - 1);
  if (_s > 0)
  {
    struct cwnd_mss tmp;
    int leng_int = sizeof(struct cwnd_mss);
    memset(&tmp, 0, leng_int);
    memcpy(&tmp, buf, leng_int);
    printf("window is %d \t %d\n", tmp.cwnd, tmp.mss);
    ret = tmp.cwnd * tmp.mss;
  }
  get_systime_linger(&sec1, &usec1);
  duration = (sec1 - sec0) * 1000000 + (usec1 - usec0);
  printf("%u\n", duration);
  return ret;
}



int main()
{
  char *str = NULL;


  int i = 0;
  while (1)
  {
    str = get_openflow_input();
    init_linger_m(str);
    main_linger();
    i = i + 1;
    printf("%s %d\n", str, i);
    sleep(1);
  }
  if (str)
  {
    free(str);
    str = NULL;
  }
  close(sock);
  return 0;
}

