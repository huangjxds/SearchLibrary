//
//  postings.c
//  SearchLibrary
//
//  Created by huangjiaxiong on 2016/11/20.
//  Copyright © 2016年 huangjiaxiong. All rights reserved.
//
#include <stdio.h>
#include "postings.h"
#include "util.h"

/**
 * 释放倒排列表
 * @param[in] pl 待释放的倒排列表中的首元素
 */
void freePostingsList(postingsList *pl)
{
    postingsList *a, *a2;
    LL_FOREACH_SAFE(pl, a, a2) {
        if (a->positions) {
            utarray_free(a->positions);
        }
        LL_DELETE(pl, a);
        free(a);
    }
}

/**
 * 释放倒排索引
 * @param[in] ii 指向倒排索引的指针
 */
void freeInvertedIndex(invertedIndexHash *ii)
{
    invertedIndexValue *cur;
    while (ii) {
        cur = ii;
        HASH_DEL(ii, cur);
        if (cur->postingsList) {
            freePostingsList(cur->postingsList);
        }
        free(cur);
    }
}

/**
 * 从env中获取关联到指定词元上的倒排列表
 * @param[in] env 存储着应用程序运行环境的结构体
 * @param[in] token 词元
 * @param[out] postings 获取到的倒排列表
 * @retval 0 成功
 * @retval -1 失败
 */
int fetchPostings(searchLibEnv *env, char *token, postingsList **postings) {
    int rc = 0;
    invertedIndexValue *iiEntry;
    HASH_FIND_STR(env->iiBuffer, token, iiEntry);
    if (!iiEntry) {
        rc = -1;
        *postings = NULL;
    } else {
        *postings = iiEntry->postingsList;
    }
    
    return rc;
}


/**
 * 获取将两个倒排列表合并后得到的倒排列表
 * @param[in] pa 要合并的倒排列表
 * @param[in] pb 要合并的倒排列表
 * @return 合并后的倒排列表
 *
 */
static postingsList * mergePostings(postingsList *pa, postingsList *pb)
{
    postingsList *ret = NULL, *p = NULL;
    /* 用pa和pb分别遍历base和to_be_added（参见函数merge_inverted_index）中的倒排列表中的元素， */
    /* 将二者连接成按文档编号升序排列的链表 */
    while (pa || pb) {
        postingsList *e;
        if (!pb || (pa && pa->documentId <= pb->documentId)) {
            e = pa;
            pa = pa->next;
        } else if (!pa || pa->documentId >= pb->documentId) {
            e = pb;
            pb = pb->next;
        } else {
            abort();
        }
        e->next = NULL;
        if (!ret) {
            ret = e;
        } else {
            p->next = e;
        }
        p = e;
    }
    return ret;
}


/**
 * 合并两个倒排索引
 * @param[in] base 合并后其中的元素会增多的倒排索引（合并目标）
 * @param[in] toBeAdded 合并后就被释放的倒排索引（合并源）
 *
 */
void mergeInvertedIndex(invertedIndexHash *base, invertedIndexHash *toBeAdded)
{
    invertedIndexValue *p, *temp;
    
    HASH_ITER(hh, toBeAdded, p, temp) {
        invertedIndexValue *t;
        HASH_DEL(toBeAdded, p);
        HASH_FIND_STR(base, p->token, t);
        if (t) {
            t->postingsList = mergePostings(t->postingsList, p->postingsList);
            t->docsCount += p->docsCount;
            free(p);
        } else {
            HASH_ADD_KEYPTR(hh, base, p->token, strlen(p->token), p);
        }
    }
}

/**
 * 打印倒排列表中的内容。用于调试
 * @param[in] postings 待打印的倒排列表
 */
void dumpPostingsList(const postingsList *postings)
{
    const postingsList *pl;
    LL_FOREACH(postings, pl) {
        printf(" doc_id %d posittions (", pl->documentId);
        if (pl->positions) {
            const int *p = NULL;
            while ((p = (const int *)utarray_next(pl->positions, p))) {
                printf("%d ", *p);
            }
        }
        printf(")\n");
    }
}


/**
 * 输出倒排索引的内容
 * @param[in] ii 指向倒排索引的指针
 */
void dumpInvertedIndex(invertedIndexHash *ii)
{
    invertedIndexValue *it;
    printf("InvertedIndex\n");
    for (it = ii; it != NULL; it = it->hh.next) {
        printf("TOKEN %s (%d):\n",  it->token, it->docsCount);
        if (it->postingsList) {
            printf("POSTINGS: [\n");
            dumpPostingsList(it->postingsList);
            puts("]");
        }
    }
    printf("\n");
}

