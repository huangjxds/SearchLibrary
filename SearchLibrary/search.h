//
//  search.h
//  SearchLibrary
//
//  Created by huangjiaxiong on 2016/11/27.
//  Copyright © 2016年 huangjiaxiong. All rights reserved.
//

#ifndef search_h
#define search_h

#include <stdio.h>
#include "search_library.h"

typedef invertedIndexHash queryTokenHash;
typedef invertedIndexValue queryTokenValue;
typedef postingsList tokenPositionsList;

typedef struct {
    tokenPositionsList *documents; /* 文档编号的序列 */
     tokenPositionsList *current;   /* 当前的文档编号 */
} docSearchCursor;

typedef struct {
    const UT_array *positions; /* 位置信息 */
    int base;                  /* 词元在查询中的位置 */
    int *current;              /* 当前的位置信息 */
} phraseSearchCursor;

typedef struct {
    int documentId;           /* 检索出的文档编号 */
    double score;              /* 检索得分 */
    UT_hash_handle hh;         /* 用于将该结构体转化为哈希表 */
} searchResults;

void search(searchLibEnv *env, const char *query);

#endif /* search_h */
