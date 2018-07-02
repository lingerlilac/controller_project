
#include <linux/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "stateMachine.h"

struct tcphdr {
    __be16  source;
    __be16  dest;
    __be32  seq;
    __be32  ack_seq;
#if defined(__LITTLE_ENDIAN_BITFIELD)
    __u16   res1:4,
        doff:4,
        fin:1,
        syn:1,
        rst:1,
        psh:1,
        ack:1,
        urg:1,
        ece:1,
        cwr:1;
#elif defined(__BIG_ENDIAN_BITFIELD)
    __u16   doff:4,
        res1:4,
        cwr:1,
        ece:1,
        urg:1,
        ack:1,
        psh:1,
        rst:1,
        syn:1,
        fin:1;
#else
#error  "Adjust your <asm/byteorder.h> defines"
#endif  
    __be16  window;
    __sum16 check;
    __be16  urg_ptr;
};

/*
maintaining a tcp information list, updating this list dynamiclly. What should be added to 
the item? For example:
1. Windowsize, calculated windowsize.
2. Last congestion time, used to calculate cwnd;
3. Current cwnd, current cwnd should be drived from initial cwnd, but how to get initial cwnd?
initial cwnd is 1 based on classical protocol?
4. How to find all the congestion event?
 */

/**
 * found_congestion aims to find every congestion event, congestion event is used to calculated
 * new congestion window, so, it is important to find every congestion event.
 * @return 1 if found, 0 otherwise;
 * ?To find congestion event based on triple acks? of retransmissions?
 */

enum eventType {
  congestion,
  over_threshold,
  zero_window,
};


static struct state slow_start, congestion_process, congeston_undo, common_state, congestion_avoid;

static struct state slow_start = {
   .parentState = NULL,
   /* The entry state is defined in order to demontrate that the 'reset'
    * transtition, going to this group state, will be 'redirected' to the
    * 'idle' state (the transition could of course go directly to the 'idle'
    * state): */
   .entryState = &congestion_avoid,
   .transitions = (struct transition[]){
      { syn, (void *)(intptr_t)'!', &compareKeyboardChar,
         &printReset, &idleState, },
      { Event_keyboard, NULL, NULL, &printUnrecognisedChar, &idleState, },
   },
   .numTransitions = 2,
   .data = "group",
   .entryAction = &printEnterMsg,
   .exitAction = &printExitMsg,
};

static struct state congestion_avoid = {
   .parentState = slow_start,
   /* The entry state is defined in order to demontrate that the 'reset'
    * transtition, going to this group state, will be 'redirected' to the
    * 'idle' state (the transition could of course go directly to the 'idle'
    * state): */
   .entryState = &congestion_process,
   .transitions = (struct transition[]){
      { Event_keyboard, (void *)(intptr_t)'!', &compareKeyboardChar,
         &printReset, &idleState, },
      { Event_keyboard, NULL, NULL, &printUnrecognisedChar, &idleState, },
   },
   .numTransitions = 2,
   .data = "group",
   .entryAction = &printEnterMsg,
   .exitAction = &printExitMsg,
};

static struct state congestion_process = {
   .parentState = congestion_process,
   /* The entry state is defined in order to demontrate that the 'reset'
    * transtition, going to this group state, will be 'redirected' to the
    * 'idle' state (the transition could of course go directly to the 'idle'
    * state): */
   .entryState = &congestion_undo,
   .transitions = (struct transition[]){
      { Event_keyboard, (void *)(intptr_t)'!', &compareKeyboardChar,
         &printReset, &idleState, },
      { Event_keyboard, NULL, NULL, &printUnrecognisedChar, &idleState, },
   },
   .numTransitions = 2,
   .data = "group",
   .entryAction = &printEnterMsg,
   .exitAction = &printExitMsg,
};

static struct state congeston_undo = {
   .parentState = congestion_process,
   /* The entry state is defined in order to demontrate that the 'reset
       * transtition, going to this group state, will be 'redirected' to the
    * 'idle' state (the transition could of course go directly to the 'idle'
    * state): */
   .entryState = &congestion_avoid,
   .transitions = (struct transition[]){
      { Event_keyboard, (void *)(intptr_t)'!', &compareKeyboardChar,
         &printReset, &idleState, },
      { Event_keyboard, NULL, NULL, &printUnrecognisedChar, &idleState, },
   },
   .numTransitions = 2,
   .data = "group",
   .entryAction = &printEnterMsg,
   .exitAction = &printExitMsg,
};


/**
 * used to specified a data flow.
 */
struct flow_l
{
    int sequence;
    int ack_sequ;
    int src_port;
    int dst_port;
    struct flow_l *next;
};

/**
 * found_congestion aims to find every congestion event for data flow f/
 * @param  f data flow f
 * @return   return 1 if congestion happens for f, 0 otherwise.
 */
bool found_congestion(struct flow_l *f)
{

    return 1;
}

/**
 * get_initial_cwnd aims to get initial cwnd for data flow, maybe this value is 1;
 * @param  f specified data flow
 * @return   inital congestion window.
 */
int get_initial_cwnd(struct flow_l *f)
{
    int cwnd = 0;

    return cwnd;
}

/**
 * get_last_congestion_time aims to get the time when last congestion happened, 
 * last congestion times for flows should stored in a structures.
 * @param  f specified flow
 * @return   duration between now and last congestion time.
 */
int get_last_congestion_time(struct flow_l *f)
{
    int duration = 0;

    return duration;
}

int process(skb)
{
    struct tcphdr *tcp = tcp_hdr(skb);
    
    int type = 0;
    type = (int)tcp.syn;

    switch(type)
    {
        case 2:
            process_common_syn(tcp);
            break;
        case 4:
            process_rst(tcp);
        case 16:
            process_common_ack(tcp);
            break;

        case 17:

            break;

        case 18:
            process_ack_syn(tcp);
            break;

        default:

    }
}

void process_common_ack(struct tcphdr *tcp)
{

}

int process_syn(struct tcphdr *tcp)
{
    int cwnd = 0;
    cwnd = get_initial_cwnd_linger(tcp);
}

void process_rst()
{

}

void process_fin()
{

}

