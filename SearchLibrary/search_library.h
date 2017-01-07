//
//  SearchLibrary.h
//  SearchLibrary
//
//  Created by huangjiaxiong on 2016/11/9.
//  Copyright © 2016年 huangjiaxiong. All rights reserved.
//

#ifndef SearchLibrary_h
#define SearchLibrary_h

#include "util.h"
#include "uthash.h"
#include "utlist.h"
#include "utarray.h"
#include "utstring.h"

#define N_GRAM 1

/* 倒排列表（以文档编号和位置信息为元素的链表结构）*/
typedef struct postingsList {
    int documentId;             /* 文档编号 */
    UT_array *positions;         /* 位置信息的数组 */
    int positionsCount;         /* 位置信息的条数 */
    struct postingsList *next; /* 指向下一个倒排列表的指针 */
} postingsList;

/* 倒排索引（以词元编号为键，以倒排列表为值的关联数组） */
typedef struct {
    int token_id;
    char *token;                 /* 词元（Token）*/
    postingsList *postingsList; /* 指向包含该词元的倒排列表的指针 */
    int docsCount;               /* 出现过该词元的文档数 */
    int positionsCount;          /* 该词元在所有文档中的出现次数之和 */
    UT_hash_handle hh;            /* 用于将该结构体转化为哈希表 */
} invertedIndexHash, invertedIndexValue;

/* 应用程序的全局配置 */
typedef struct searchLibEnv {
    int tokenLen;                  /* 词元的长度。N-gram中N的取值 */
    invertedIndexHash *iiBuffer; /* 用于更新倒排索引的缓冲区（Buffer） */
    int iiBufferCount;            /* 用于更新倒排索引的缓冲区中的文档数 */
    int iiBufferUpdateThreshold; /* 缓冲区中文档数的阈值 */
    int indexedCount;              /* 建立了索引的文档数 */
    
} searchLibEnv;


void slSearchInit(searchLibEnv *env);

void slAddDocument(searchLibEnv *env, const int documentId, const char *text);

void slSearch(searchLibEnv *env, const char *query);

#endif /* SearchLibrary_h */
