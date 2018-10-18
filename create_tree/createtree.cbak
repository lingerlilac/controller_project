#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ITEM_AMOUNT 100

struct flow_linger
{
    char ip_src[20];
    char ip_dst[20];
    int  port_src;
    int  port_dst;
    int winsize;
};
void generate_command(struct flow_linger *ptr, char *ret, int size)
{
    char buff[200];

    memset(buff, 0, 200);
    sprintf(buff, "ovs-ofctl add-flow br0 -O openflow13 tcp,nw_dst=%s,nw_dst=%s,tcp_src=%d,tcp_dst=%d,actions=set_rwnd:%d", 
        ptr->ip_src, ptr->ip_dst, ptr->port_src, ptr->port_dst, ptr->winsize);
    // printf("%s\n", buff);
    strncpy(ret, buff, size);
}

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
	char *name;
	float value;
};
struct tcp_flow
{
	char ipsrc[16];
	char ipdst[16];
	int srcport;
	int dstport;
	int winsize;
	struct tcp_flow* next;
};
typedef enum windowsize
{
	keep,
	half,
	quater,
	zero
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
struct treenode* bst_search(struct treenode*, struct item_value value[]);
int create_items(struct item_value items_list[], char *buffer);
void destroy_item_list(struct item_value items_list[]);
int init(char *, char *);
void destroy_everything();

struct treenode *tnode = NULL;
struct item_value items_list[ITEM_AMOUNT];
char **parameters;
struct rawdata *node = NULL;

int main()
{
	char inputtxt[] = "0,0,requeues_3,5158.975,0|0,1,busy_time_1,808.831,2|1,2,leaf,0,2|1,3,allpackets_1,150.5,2|3,4,leaf,0,2|3,5,requeues_1,678.932,2|5,6,drops_1,3211.45,2|6,7,leaf,0,2|6,8,leaf,0,2|5,9,leaf,0,2|0,10,busy_time_1,808.542,0|10,11,retrans_4,14.5,0|11,12,allpackets_3,182.5,0|12,13,retrans_5,14.5,0|13,14,retrans_3,0.5,0|14,15,receive_time_1,122.551,0|15,16,leaf,0,0|15,17,leaf,0,1|14,18,transmit_time_4,32.177,0|18,19,leaf,0,0|18,20,leaf,0,0|13,21,leaf,0,0|12,22,allpackets_1,202.5,0|22,23,rbytes_3,257018.672,0|23,24,leaf,0,0|23,25,allpackets_1,170.5,0|25,26,leaf,0,0|25,27,leaf,0,0|22,28,leaf,0,0|11,29,leaf,0,0|10,30,leaf,0,2";
 	char item[] = "requeues_3,5000;busy_time_1,809;allpackets_1,160;requeues_1,670;drops_1,3220;retrans_4,0;allpackets_3,0;retrans_5,0;retrans_3,0;receive_time_1,0;transmit_time_4,0;rbytes_3,0";
 	struct treenode* search_results = NULL;
 	windsz wsize;
 	char *ret = NULL;
 	char ipsrc[] = "192.168.1.120";
 	char ipdst[] = "202.118.228.111";
 	int port_src = 80;
 	int port_dst = 210;

 	init(inputtxt, item);

 	search_results = bst_search(tnode, items_list);
	if(search_results)
	{
	    struct flow_linger flow1;
	    strcpy(flow1.ip_src, ipsrc);
	    strcpy(flow1.ip_dst, ipdst);
	    flow1.port_src = port_src;
	    flow1.port_dst = port_dst;
	    flow1.winsize = 100;
		printf("%d class\n", search_results->class);
		wsize= (windsz)search_results->class;
	    ret = malloc(sizeof(char) * 200);
	    generate_command(&flow1, ret, 200);
	    printf("ret is %s\n", ret);
	    free(ret);
	    ret = NULL; 
	}
		// printf("ad\n");
	else
		printf("not found\n");
	destroy_everything();
}
void destroy_everything()
{
	destroy_item_list(items_list);
	destroy_parameters(parameters);
	destroy_tree(tnode);
	free_rawdata(node);	
}
int init(char * inputtxt, char * item)
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
	while(p!=NULL)
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
		while(q != NULL)
		{
			if(k == 0)
				newnode->father = atoi(q);
			if(k == 1)
			{
				newnode->itself = atoi(q);
			}
			if(k == 2)
			{
				int len = strlen(q);
				newnode->name = (char *)malloc(sizeof(char) * len);
				memset(newnode->name, 0, sizeof(char) * len);
				strcpy(newnode->name , q);
			}
			if(k == 3)
				newnode->value = atof(q);
			if(k == 4)
				newnode->class = atoi(q);
			k = k + 1;
			q = strtok_r(NULL, split1, &inner_ptr);
		}
		if(node == NULL)
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
		p = strtok_r(NULL,split, &outer_ptr);
	}
	tnode = create_tree(node, NULL);
	print_tree(tnode);
	printf("the highth of tree is %d\n", TreeDepth(tnode));
	parameters = get_parameters(node);
	while(parameters[i])
	{
		printf("%d: %s\t", i, parameters[i]);
		i = i + 1;
	}
	for(i = 0; i < ITEM_AMOUNT; i++)
	{
	// memset(items_list[i], 0, sizeof(struct item_value));
	items_list[i].name = NULL;
	items_list[i].value = -1000000.0;
	} 
	create_items(items_list, item);
	for(i = 0; i < ITEM_AMOUNT; i++)
	{
	if(items_list[i].name == NULL)
	  break;
	printf("name %s value %f\n", items_list[i].name, items_list[i].value);
	// i = i + 1;
	}

	return 0;
}

void free_rawdata(struct rawdata * head)
{
	struct rawdata * before = NULL;
	while(head)
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
	while(raw != NULL)
	{
		printf("%d\t%d\t%s\t%f\t%d\n", raw->father, raw->itself, raw->name, raw->value, raw->class);
		raw = raw->next;
	}
}

struct treenode * create_tree(struct rawdata *ptr, struct treenode * parent)
{
	struct treenode * newnode = NULL;
	if(ptr != NULL)
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
		if(strcmp(ptr->name, "leaf") != 0)
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
	if(raw)
	{
		printf("%d\t%s\t%f\t%d\n", raw->itself, raw->name, raw->value, raw->class);
		print_tree(raw->lchild);
		print_tree(raw->rchild);
	}
}
void destroy_tree(struct treenode *T)
{
	if(T)
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
	while(ptr)
	{
		if(ptr->father == father)
		{
			return ptr;
		}
		ptr = ptr->next;
	}
}
struct rawdata * find_rchild(struct rawdata *ptr, int father)
{
	int rchild = 0;
	while(ptr)
	{
		if(ptr->father == father)
		{
			if(rchild == 1)
				return ptr;
			rchild = rchild + 1;
		}
		ptr = ptr->next;
	}
}

int TreeDepth(struct treenode *T)  
{  
	int rightdep=0;  
	int leftdep=0;  

	if(T==NULL)  
	return -1;  

	if(T->lchild!=NULL)  
	leftdep=TreeDepth(T->lchild);  
	else  
	leftdep=-1;  

	if(T->rchild!=NULL)  
	rightdep=TreeDepth(T->rchild);  
	else  
	rightdep=-1;  

	return (rightdep>leftdep) ? rightdep+1 : leftdep+1;  
}

struct treenode* bst_search(struct treenode* node, struct item_value value[]) 
{
	int found = 0;
	struct treenode* node_pre = NULL;
	node_pre = node;
	while(node && (strcmp(node->name, "leaf") != 0))
	{
		int i = 0;
		found = 0;
		for(i=0; i < ITEM_AMOUNT; i++)
		{
			if(strcmp(node->name, value[i].name) == 0)
			{
				found = 1;
				break;
			}
			if(value[i].name == NULL)
				break;
		}
		if(found = 0)
			return NULL;
		if(value[i].value <= node->value)
		{
			node_pre =node;
			node = node->lchild;
		}
		else
		{
			node_pre =node;
			node = node->rchild;
		}
	}
	if(found)
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
	for(i = 0; i < 80; i++)
	{
		tmp[i] = NULL;
	}
	while(ptr)
	{
		char *str = NULL;
		if(strcmp(ptr->name, "leaf") == 0)
		{
			ptr = ptr->next;
			continue;
		}
		i = 0;
		found = 0;
		while(i < last)
		{
			if(strcmp(tmp[i], ptr->name) == 0)
			{
				found = 1;
				break;
			}
			i = i + 1;
		}
		if(found == 1)
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
	for(i = 0; i < 80; i++)
	{
		free(ptr[i]);
	}
	free(ptr);
}

int create_items(struct item_value items_list[], char *buffer)
{
  int j,in = 0;
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
    if(j % 2 ==  0)
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
      if(index >= ITEM_AMOUNT)
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
  while(items_list[i].name)
  {
    free(items_list[i].name);
    items_list[i].name = NULL;
  }
}
// windsz computed_window(int cls)
// {
// 	windsz ret;
// 	ret = (windsz)cls;
// 	return ret;
// }
int set_windsz(int cls)
{

}


struct tcp_flow * get_tcplist(void)
{
    struct tcp_flow * ret = NULL;
    
    return ret;

}