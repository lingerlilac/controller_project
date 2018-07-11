#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/tcp.h>
#include "hash_lin.h"
/*
congestion, initial, over_threshold, undo, zero_window
 */

/**
 * process function process the packet. The process is based on flags in
 * tcp header: CWND is initialized when SYN (2 or 18). TCP flow finished when
 * FIN, delete it from hash table. When flags is 16, check if this ack is triple-acks
 * event, retransmission or common ack: triple-acks and retransmission events trigger
 * cubic algorithm to get new cwnd, common ack event increases cwnd based on cubic.
 * @param tcp [description]
 */
void process(struct tcphdr *tcp)
{
    if (flags == 2 || flags == 18)
    {
        memcpy(str, mac_addr_origin, 6);
        memcpy(str + 6, &ip_src, 4);
        memcpy(str + 10, &ip_dst, 4);
        memcpy(str + 14, &sourceaddr, 2);
        memcpy(str + 16, &destination, 2);
        time = getcurrenttime ();
        cwnd = get_inital_cwnd(tcp)
               hash_table_insert (str, cwnd, time);
    }
    else if (flags == 17 || flags & 0x0004 == 1)
    {
        memcpy(str, mac_addr_origin, 6);
        memcpy(str + 6, &ip_src, 4);
        memcpy(str + 10, &ip_dst, 4);
        memcpy(str + 14, &sourceaddr, 2);
        memcpy(str + 16, &destination, 2);
        if (hash_table_lookup (str) != NULL)
        {
            hash_table_remove (str);
        }
        memcpy(str, mac_addr_origin, 6);
        memcpy(str + 6, &ip_dst, 4);
        memcpy(str + 10, &ip_src, 4);
        memcpy(str + 14, &destination, 2);
        memcpy(str + 16, &sourceaddr, 2);
        if (hash_table_lookup (str) != NULL)
        {
            hash_table_remove (str);
        }
    }

    else if (flags == 16)
    {
        int cwnd = 0;

        cwnd = find_congestion(tcp);

        if (cwnd > 0)
        {
            time = getcurrenttime ();
            memcpy(str, mac_addr_origin, 6);
            memcpy(str + 6, &ip_dst, 4);
            memcpy(str + 10, &ip_src, 4);
            memcpy(str + 14, &destination, 2);
            memcpy(str + 16, &sourceaddr, 2);
            if (hash_table_lookup (str) != NULL)
            {
                hash_table_lookup (str)->cwnd = cwnd;
                hash_table_lookup (str)->time = time;
            }
            else
            {
                time = getcurrenttime ();
                hash_table_insert (str, cwnd, time);
            }
        }
        else
        {
            time = getcurrenttime ();
            memcpy(str, mac_addr_origin, 6);
            memcpy(str + 6, &ip_dst, 4);
            memcpy(str + 10, &ip_src, 4);
            memcpy(str + 14, &destination, 2);
            memcpy(str + 16, &sourceaddr, 2);
            cwnd = increase_cwnd_cubic(tcp);
            if (hash_table_lookup (str) != NULL)
            {
                hash_table_lookup (str)->time = time;
                hash_table_lookup(str)->cwnd = cwnd;
            }
            else
            {
                time = getcurrenttime ();
                hash_table_insert (str, cwnd, time);
            }

        }

    }
}

int find_congestion(struct tcphdr *tcp)
{
    int res = 0;
    res = check_triple_acks(tcp);
    if (res)
    {
        cwnd = new_cwnd(tcp);
        return cwnd;
    }
    res = check_retransmission(tcp);
    if (res)
    {
        if (last_update(tcp))
            return -1;
        else
        {
            cwnd = new_cwnd(tcp);
            return cwnd;
        }
    }
    return -1;

}

static __u64 getcurrenttime(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int new_cwnd(struct tcphdr *tcp)
{
    int new_cwnd = 0;

    return new_cwnd;
}

int last_update(struct tcphdr *tcp)
{
    int last_updated = -1;

    return last_updated;
}

/**
 * check_retransmission checks if the sequence has been received.
 * @param  tcp tcp header
 * @return     1 if this packet is a retransmission. -1 otherwise.
 */
int check_retransmission(struct tcphdr *tcp)
{
    int res = -1;

    return res;
}

/**
 * check_triple_acks check if triple acks event happens. This function
 * is duplicated with check_retransmission, however, it aims to find
 * congestion immediately. Because its function is duplicated with
 * check_retransmission, we use last_update to find the duplicated updates.
 * @param  tcp tcp header
 * @return     return 1 if triple-acks event happens, -1 otherwise.1
 */
int check_triple_acks(struct tcphdr *tcp)
{
    int res = -1;

    return res;
}


void process_packet(struct tcphdr *tcp)
{
    getcurrenttime();
}

static void get_systime_linger(__be32 *t, __be32 *v)
{
    struct timeval tv;
    memset(&tv, 0, sizeof(struct timeval));
    do_gettimeofday(&tv);
    *t = tv.tv_sec;
    *v = tv.tv_usec;
}


void process_linger(struct tcphdr *tcp)
{
    int length = 0;
    length = get_length(tcp);
    if(length > 0)
    {
        switch(length)
        {
            case 1:
                process_fin(tcp);
                break;
            case 2:
                process_syn(tcp);
                break;
            case 4:
                process_fin(tcp);
                break;
            case 16:
                process_common_ack(tcp);
                break;
            case 17:
                process_fin(tcp);
                break;
            case 18:
                process_syn_ack(tcp);
                break;
            case 20:
                process_fin(tcp);
                break;
            default:
                break; 
        }
    }
}

void process_fin(struct tcphdr *tcp)
{
    del_sequence_from_queue(tcp);
    update(cwnd, tcp, 0);
}

void process_syn(struct tcphdr *tcp)
{
    int cwnd = 0;
    put_sequence_into_queue(tcp->seq);
    cwnd = get_initial_cwnd_linger(tcp);
    update(cwnd, tcp, 1);
}

void process_common_ack(struct tcphdr *tcp)
{
    int sequence = tcp->seq;
    int congestion = 0;
    int cwnd = 0;
    congestion = get_congestion(tcp);
    if(condition)
    {
        cwnd = congestion_new_cwnd(tcp);
    }
    else
    {
        cwnd = common_new_cwnd(tcp);
    }
    update(cwnd, tcp, 1);
}

void process_syn_ack(struct tcphdr *tcp)
{
    int cwnd = 0;
    put_sequence_into_queue(tcp->seq);
    cwnd = get_initial_cwnd_linger(tcp);
    update(cwnd, tcp, 1);
}

int congestion_new_cwnd(struct tcphdr *tcp)
{
    int cwnd = 0;

    return cwnd;
}

int common_new_cwnd(struct tcphdr *tcp)
{
    int cwnd = 0;

    return cwnd;
}

void del_sequence_from_queue(struct tcphdr *tcp)
{

}

void update(int cwnd, struct tcphdr *tcp, int condition)
{

}

int get_initial_cwnd_linger(struct tcphdr *tcp)
{

}

void put_sequence_into_queue(struct tcphdr *tcp)
{

}

