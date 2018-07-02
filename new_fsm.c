#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash_lin.h"

void process(int flags,
             __be32 ip_src,
             __be32 ip_dst,
             __be16 sourceaddr,
             __be16 destination)
{
    if (flags == 2 || flags == 18)
    {
        memcpy(str, mac_addr_origin, 6);
        memcpy(str + 6, &ip_src, 4);
        memcpy(str + 10, &ip_dst, 4);
        memcpy(str + 14, &sourceaddr, 2);
        memcpy(str + 16, &destination, 2);
        time = getcurrenttime ();
        hash_table_insert (str, wscale, time);
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
        time = getcurrenttime ();
        memcpy(str, mac_addr_origin, 6);
        memcpy(str + 6, &ip_dst, 4);
        memcpy(str + 10, &ip_src, 4);
        memcpy(str + 14, &destination, 2);
        memcpy(str + 16, &sourceaddr, 2);
        if (hash_table_lookup (str) != NULL)
        {
            cal_windowsize =
                windowsize << hash_table_lookup (str)->nValue;
            hash_table_lookup (str)->time = time;
        }
        else
        {
            time = getcurrenttime ();
            hash_table_insert (str, wscale, time);
        }
    }
}