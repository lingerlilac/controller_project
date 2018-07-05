/* 
 * Author: puresky 
 * Date: 2011/01/08 
 * Purpose: a simple implementation of HashTable in C 
 */  
  
#include <stdio.h>  
#include <stdlib.h>  
#include <string.h>  
#include <time.h>

  
/*=================hash table start=========================================*/  
  
#define HASH_TABLE_MAX_SIZE 10000  
typedef struct HashNode_Struct HashNode;  
int count_count = 0;
struct HashNode_Struct  
{  
    char* sKey;
    __u64 time;  
    int cwnd;
    HashNode* pNext;  
};

HashNode* hashTable[HASH_TABLE_MAX_SIZE]; //hash table data strcutrue  
int hash_table_size;  //the number of key-value pairs in the hash table!  

int strcmp_lin(const char *s, const char *t)
{
    int i = 0;
    int condition = 0;
    for(i = 0; i < 6; i++)
    {
        int tmp = 1000;
        tmp = (int)*(s + i) - (int)*(t + i);
        if(tmp == 0)
            condition += 1;
    }
    return (condition == 6)? 0:1;
}

//initialize hash table  
void hash_table_init()  
{  
    hash_table_size = 0;  
    memset(hashTable, 0, sizeof(HashNode*) * HASH_TABLE_MAX_SIZE);  
}  
  
  
//string hash function  
unsigned int hash_table_hash_str(const char* skey)  
{  
    const signed char *p = (const signed char*)skey;  
    unsigned int h = *p;  
    if(h)  
    {  
        for(p += 1; *p != '\0'; ++p)  
            h = (h << 5) - h + *p;  
    }  
    return h;  
}  
  
//insert key-value into hash table  
void hash_table_insert(const char* skey, int nvalue, __u64 time)  
{  
    int found_skey = 0;
    if(hash_table_size >= HASH_TABLE_MAX_SIZE)  
    {  
        printf("out of hash table memory!\n");  
        return;  
    }  
    unsigned int pos = hash_table_hash_str(skey) % HASH_TABLE_MAX_SIZE;  
    
    HashNode* pHead =  hashTable[pos]; 
    found_skey = 0; 
    while(pHead)  
    {  
        if(strcmp_lin(pHead->sKey, skey) == 0)  
        {  
            // printf("%s already exists!\n", skey); 
            found_skey = 1;
            pHead->cwnd = nvalue;
            pHead->time = time;
            break; 
            // hash_table_size --;  
        }  
        pHead = pHead->pNext;  
    }  
    if (found_skey == 0)
    {
        HashNode* pNewNode = (HashNode*)malloc(sizeof(HashNode));  
        memset(pNewNode, 0, sizeof(HashNode));  
        pNewNode->sKey = (char*)malloc(sizeof(char) * (strlen(skey) + 1));  
        strcpy(pNewNode->sKey, skey);
        pNewNode->time = time;  
        pNewNode->cwnd = nvalue;  
      
        pNewNode->pNext = hashTable[pos];  
        hashTable[pos] = pNewNode;  
      
      
        hash_table_size++;  
    }
    // printf("hash_table_size is %d \n", hash_table_size);
}  
//remove key-value frome the hash table  
void hash_table_remove(const char* skey)  
{  
    unsigned int pos = hash_table_hash_str(skey) % HASH_TABLE_MAX_SIZE;
    // printf("remove remove remove\n");
    if(hashTable[pos])  
    {  
        HashNode* pHead = hashTable[pos];  
        HashNode* pLast = NULL;  
        HashNode* pRemove = NULL; 
        while(pHead)  
        {  
            if(strcmp_lin(skey, pHead->sKey) == 0)  
            {  
                pRemove = pHead;  
                break;  
            }  
            pLast = pHead;  
            pHead = pHead->pNext;  
        }  
        if(pRemove)  
        {  
            if(pLast)  
                pLast->pNext = pRemove->pNext;  
            else
            {
                if (pRemove->pNext)
                    hashTable[pos] = pRemove->pNext;
                else
                    hashTable[pos] = NULL;
            }    
  
            free(pRemove->sKey);  
            free(pRemove); 
            hash_table_size = hash_table_size - 1;
            // printf("hash_table_size afterremoveis %d \n", hash_table_size);
            // printf("move successfully\n"); 
        }  
    }  
}  
  
//lookup a key in the hash table  
HashNode* hash_table_lookup(const char* skey)  
{  
    unsigned int pos = hash_table_hash_str(skey) % HASH_TABLE_MAX_SIZE;  
    if(hashTable[pos])  
    {  
        HashNode* pHead = hashTable[pos];  
        while(pHead)  
        {  
            if(strcmp_lin(skey, pHead->sKey) == 0)  
                return pHead;  
            pHead = pHead->pNext;  
        }  
    }  
    return NULL;  
}  
  
//print the content in the hash table  
void hash_table_print()  
{  
    count_count = 0;
    printf("===========content of hash table=================\n");  
    int i;  
    for(i = 0; i < HASH_TABLE_MAX_SIZE; ++i)  
        if(hashTable[i])  
        {  
            HashNode* pHead = hashTable[i];  
            printf("%d=>", i);  
            while(pHead)  
            {  
                count_count = count_count + 1;
                printf("%s:%d  size is %d", pHead->sKey, pHead->cwnd, count_count);  
                pHead = pHead->pNext;  
            }  
            printf("\n");  
        }  
    if (count_count != hash_table_size)
    {
        printf("fuck happens\n");
        exit(0);
    }
}


//free the memory of the hash table  
void hash_table_release()  
{  
    int i;  
    for(i = 0; i < HASH_TABLE_MAX_SIZE; ++i)  
    {  
        if(hashTable[i])  
        {  
            HashNode* pHead = hashTable[i];  
            while(pHead)  
            {  
                HashNode* pTemp = pHead;  
                pHead = pHead->pNext;  
                if(pTemp)  
                {  
                    free(pTemp->sKey);  
                    free(pTemp);  
                }  
  
            }  
        }  
    }  
}  