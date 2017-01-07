//
//  search.c
//  SearchLibrary
//
//  Created by huangjiaxiong on 2016/11/27.
//  Copyright © 2016年 huangjiaxiong. All rights reserved.
//
#include <string.h>
#include "search.h"
#include "token.h"
#include "postings.h"
#include "util.h"

/**
 * 根据得分比较两条检索结果
 * @param[in] a 检索结果a的数据
 * @param[in] b 检索结果b的数据
 * @return 得分的大小关系
 */
//static int search_results_score_desc_sort(searchResults *a, searchResults *b)
//{
//    return (b->score > a->score) ? 1 : (b->score < a->score) ? -1 : 0;
//}

/**
 * 比较出现过词元a和词元b的文档数
 * @param[in] a 词元a的数据
 * @param[in] b 词元b的数据
 * @return 文档数的大小关系
 */
static int query_token_value_docs_count_desc_sort(queryTokenValue *a,
                                       queryTokenValue *b)
{
    return b->docsCount - a->docsCount;
}


/**
 * 将文档添加到检索结果中
 * @param[in] results 指向检索结果的指针
 * @param[in] documentId 要添加的文档的编号
 */
static void addSearchResult(searchResults **results, const int documentId)
{
    searchResults *r;
    if (*results) {
        HASH_FIND_INT(*results, &documentId, r);
    } else {
        r = NULL;
    }
    if (!r) {
        if ((r = malloc(sizeof(searchResults)))) {
            r->documentId = documentId;
            r->score = 0;
            HASH_ADD_INT(*results, documentId, r);
        }
    }
}

/**
 * 进行短语检索
 * @param[in] queryTokens 从查询中提取出的词元信息
 * @param[in] docCursors 用于检索文档的游标的集合
 * @return 检索出的短语数
 */
static int searchPhrase(const queryTokenHash *queryTokens, docSearchCursor *docCursors)
{
    int nPositions = 0;
    const queryTokenValue *qt;
    phraseSearchCursor *cursors;
    
    /* 获取查询中词元的总数 */
    for (qt = queryTokens; qt; qt = qt->hh.next) {
        nPositions += qt->positionsCount;
    }
    
    if ((cursors = (phraseSearchCursor *)malloc(sizeof(phraseSearchCursor) * nPositions))) {
        int i, phraseCount = 0;
        phraseSearchCursor *cur;
        /* 初始化游标 */
        for (i = 0, cur = cursors, qt = queryTokens; qt; i++, qt = qt->hh.next) {
            int *pos = NULL;
            while ((pos = (int *)utarray_next(qt->postingsList->positions,
                                              pos))) {
                cur->base = *pos;
                cur->positions = docCursors[i].current->positions;
                cur->current = (int *)utarray_front(cur->positions);
                cur++;
            }
        }
        /* 检索短语 */
        while (cursors[0].current) {
            int relPosition, nextRelPosition;
            relPosition = nextRelPosition = *cursors[0].current -
            cursors[0].base;
            /* 对于除词元A以外的词元，不断地向后读取其出现位置，直到其偏移量不小于词元A的偏移量为止 */
            for (cur = cursors + 1, i = 1; i < nPositions; cur++, i++) {
                for (; cur->current
                     && (*cur->current - cur->base) < relPosition;
                     cur->current = (int *)utarray_next(cur->positions, cur->current)) {}
                
                if (!cur->current) { goto exit; }
                
                /* 对于除词元A以外的词元，若其偏移量不等于A的偏移量，就退出循环 */
                if ((*cur->current - cur->base) != relPosition) {
                    nextRelPosition = *cur->current - cur->base;
                    break;
                }
            }
            if (nextRelPosition > relPosition) {
                /* 不断向后读取，直到词元A的偏移量不小于nextRelPosition为止 */
                while (cursors[0].current &&
                       (*cursors[0].current - cursors[0].base) < nextRelPosition) {
                    cursors[0].current = (int *)utarray_next(
                                                             cursors[0].positions, cursors[0].current);
                }
            } else {
                /* 找到了短语 */
                phraseCount++;
                cursors->current = (int *)utarray_next(
                                                       cursors->positions, cursors->current);
            }
        }
    exit:
        free(cursors);
        return phraseCount;
    }
    return 0;
}

/**
 * 释放词元的出现位置列表
 * @param[in] list 待释放的出现位置列表的首元素
 */
void freeTokenPositionsList(tokenPositionsList *list)
{
    freePostingsList((postingsList *)list);
}


/**
 * 检索文档
 * @param[in] env 存储着应用程序运行环境的结构体
 * @param[in,out] results 检索结果
 * @param[in] tokens 从查询中提取出的词元信息
 */
void searchDocs(searchLibEnv *env, searchResults **results,
            queryTokenHash *tokens)
{
    int nTokens;
    docSearchCursor *cursors;
    
    if (!tokens) { return; }
    
    /* 按照文档频率的升序对tokens排序 */
    HASH_SORT(tokens, query_token_value_docs_count_desc_sort);
    nTokens = HASH_COUNT(tokens);
    if (nTokens && (cursors = (docSearchCursor *)calloc(sizeof(docSearchCursor), nTokens))) {
        int i;
        docSearchCursor *cur;
        queryTokenValue *token;
        for (i = 0, token = tokens; token; i++, token = token->hh.next) {
            if (token->postingsList == 0) {
                print_error("decode postings error!: %d\n", token->token_id);
                goto exit;
            }
            
            if (fetchPostings(env, token->token, &cursors[i].documents)) {
                print_error("fetch postings error!: %s\n", token->token);
                goto exit;
            }
            
            if (!cursors[i].documents) {
                goto exit;
            }
            
            cursors[i].current = cursors[i].documents;
        }
        
        while (cursors[0].current) {
            int docId, nextDocId = 0;
            docId = cursors[0].current->documentId;
            for (cur = cursors + 1, i = 1; i < nTokens; i++, cur++) {
                while (cur->current &&  cur->current->documentId < docId) {
                    cur->current = cur->current->next;
                }
                if (!cur->current) {goto exit;}
                if (cur->current->documentId != docId) {
                    nextDocId = cur->current->documentId;
                    break;
                }
            }
            
            if (nextDocId > 0) {
                while (cursors[0].current && cursors[0].current->documentId < nextDocId) {
                    cursors[0].current = cursors[0].current->next;
                }
            } else { //获取结果
               int phraseCount = -1;
               phraseCount = searchPhrase(tokens, cursors);
               if (phraseCount) {
                   addSearchResult(results, docId);
               }
                
               cursors[0].current = cursors[0].current->next;
            }
        }
        
    exit:
        for (i = 0; i < nTokens; i++) {
            if (cursors[i].documents) {
                freePostingsList(cursors[i].documents);
            }
        }
        free(cursors);
    }
 
    freeInvertedIndex(tokens);
}

/**
 * 从查询字符串中提取出词元的信息
 * @param[in] env 存储着应用程序运行环境的结构体
 * @param[in] text 查询字符串
 * @param[in] textLen 查询字符串的长度
 * @param[in,out] queryTokens 按词元编号存储位置信息序列的关联数组
 *                             若传入的是指向NULL的指针，则新建一个关联数组
 * @retval 0 成功
 * @retval -1 失败
 */
static int splitQueryToTokens(searchLibEnv *env,
                        const UTF32Char *text,
                        const unsigned int textLen,
                        queryTokenHash **queryTokens)
{
    return textToPostingsLists(env, 0, text, textLen, (invertedIndexHash **)queryTokens);

}

/**
 * 打印检索结果
 * @param[in] env 存储着应用程序运行环境的结构体
 * @param[in] results 检索结果
 */
void printSearchResults(searchLibEnv *env, searchResults *results)
{
    searchResults *result;
    int num = HASH_COUNT(results);
    printf("searchResults\n");
    for (result = results; result; result = result->hh.next) {
        printf("doc_id %d \n", result->documentId);
    }
    printf("total results %d \n", num);
}

/**
 * 进行全文检索
 * @param[in] env 存储着应用程序运行环境的结构体
 * @param[in] query 查询
 */
void search(searchLibEnv *env, const char *query)
{
    int query32Len;
    UTF32Char *query32;
    
    if (!utf8toutf32(query, (int)strlen(query), &query32, &query32Len)) {
        searchResults *results = NULL;
        
        if (query32Len < env->tokenLen) {
            print_error("too short query.");
        } else {
            queryTokenHash *queryTokens = NULL;
            splitQueryToTokens(env, query32, query32Len, &queryTokens);
            searchDocs(env, &results, queryTokens);
        }
        //打印检索结果
        printSearchResults(env, results);
        
        free(query32);
    }

}
