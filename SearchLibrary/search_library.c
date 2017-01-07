//
//  SearchLibrary.c
//  SearchLibrary
//
//  Created by huangjiaxiong on 2016/11/9.
//  Copyright © 2016年 huangjiaxiong. All rights reserved.
//
#include <stdio.h>
#include "search_library.h"
#include "sl_pinyincode.h"
#include "postings.h"
#include "token.h"
#include "search.h"

/* TRUE/FALSE */
#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif

static int pinyinCodeIndexSort[SL_PINYIN_NUMER];

// PinYinCode 按拼音排序后存入PinYinCodeIndexSort
void sortPinyinCodeIndex()
{
    int low = 0;
    int mid = 0;
    int high = 0;
    int i = 0;
    int j = 0;
    int val = 0;
    int exist = FALSE;
    int midIndex = 0;
    int sort[SL_PINYIN_NUMER];
    
    for (i = 0; i < SL_PINYIN_NUMER; i++) {
        low = 0;
        high = i - 1;
        exist = FALSE;
        while (low <= high) {
            mid = (low + high) >> 1;
            midIndex = sort[mid];
            val = strcmp(slPinyinCode[midIndex], slPinyinCode[i]);
            if (val < 0)
                low = mid + 1;
            else if (val > 0)
                high = mid - 1;
            else {
                exist = TRUE;
                break;
            }
        }
        if (exist)
            low = mid;
        
        for (j = i; j > low; j--)
            sort[j] = sort[j - 1];
        sort[j] = i;
    }
    
    for (int i = 0; i < SL_PINYIN_NUMER; i++)
        pinyinCodeIndexSort[sort[i]] = i;
}




void slAddDocument(searchLibEnv *env, const int documentId, const char *text)
{
    if (text) {
        UTF32Char *text32;
        int text32Len;
        unsigned int textSize;
        textSize = (unsigned int)strlen(text);
        
        /* 转换文档正文的字符编码 */
        if (!utf8toutf32(text, textSize, &text32, &text32Len)) {
            /* 为文档创建倒排列表 */
            textToPostingsLists(env, documentId, text32, text32Len, &env->iiBuffer);
            env->iiBufferCount ++;
            free(text32);
        }
        
        env->indexedCount ++;
    }
}



static void initEnv(searchLibEnv *env)
{
    memset(env, 0, sizeof(searchLibEnv));
    env->tokenLen = N_GRAM;
}



void slSearchInit(searchLibEnv *env)
{
    sortPinyinCodeIndex();
    initEnv(env);
}

void slSearch(searchLibEnv *env, const char *query)
{
    search(env, query);
}









