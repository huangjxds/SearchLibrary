//
//  token.c
//  SearchLibrary
//
//  Created by huangjiaxiong on 2016/11/27.
//  Copyright © 2016年 huangjiaxiong. All rights reserved.
//

#include "token.h"
#include "postings.h"

/**
 * 检查输入的字符（UTF-32）是否不属于索引对象
 * @param[in] ustr 输入的字符（UTF-32）
 * @return 是否是空白字符
 * @retval 0 不是空白字符
 * @retval 1 是空白字符
 */
static int isIgnoredChar(const UTF32Char ustr) {
    switch (ustr) {
        case ' ': case '\f': case '\n': case '\r': case '\t': case '\v':
        case '!': case '"': case '#': case '$': case '%': case '&':
        case '\'': case '(': case ')': case '*': case '+': case ',':
        case '-': case '.': case '/':
        case ':': case ';': case '<': case '=': case '>': case '?': case '@':
        case '[': case '\\': case ']': case '^': case '_': case '`':
        case '{': case '|': case '}': case '~':
        case 0x3000: /* 全角空格 */
        case 0x3001: /* 、 */
        case 0x3002: /* 。 */
        case 0xFF08: /* （ */
        case 0xFF09: /* ） */
        case 0xFF01: /* ！ */
        case 0xFF0C: /* ， */
        case 0xFF1A: /* ： */
        case 0xFF1B: /* ； */
        case 0xFF1F: /* ? */
            return 1;
        default:
            return 0;
    }
}

/**
 * 将输入的字符串分割为N-gram
 * @param[in] ustr 输入的字符串（UTF-8）
 * @param[in] ustr_end 输入的字符串中最后一个字符的位置
 * @param[in] n N-gram中N的取值。建议将其设为大于1的值
 * @param[out] start 词元的起始位置
 * @return 分割出来的词元的长度
 */
static int ngramNext(const UTF32Char *ustr, const UTF32Char *ustr_end, unsigned int n, const UTF32Char **start)
{
    int i;
    const UTF32Char *p;
    
    /* 读取时跳过文本开头的空格等字符 */
    for (; ustr < ustr_end && isIgnoredChar(*ustr); ustr++) {
    }
    
    /* 不断取出最多包含n个字符的词元，直到遇到不属于索引对象的字符或到达了字符串的尾部 */
    for (i = 0, p = ustr; i < n && p < ustr_end && !isIgnoredChar(*p); i++, p++) {
    }
    
    *start = ustr;
    return (int)(p - ustr);
}

/**
 * 为invertedIndexValue分配存储空间并对其进行初始化
 * @param[in] token 词元
 * @param[in] docsCount 包含该词元的文档数
 * @return 生成的invertedIndexValue
 */
static invertedIndexValue * createNewInvertedIndex(const char *token,
                                                   const unsigned int tokenSize,
                                                   int docsCount)
{
    invertedIndexValue *iiEntry;
    
    iiEntry = malloc(sizeof(invertedIndexValue));
    if (!iiEntry) {
        print_error("cannot allocate memory for an inverted index.");
        return NULL;
    }
    iiEntry->positionsCount = 0;
    iiEntry->postingsList = NULL;
    iiEntry->token = (char *)malloc(tokenSize);
    strcpy(iiEntry->token, token);
    iiEntry->docsCount = docsCount;
    return iiEntry;
}



/**
 * 为倒排列表分配存储空间并对其进行并初始化
 * @param[in] documentId 文档编号
 * @return 生成的倒排列表
 */
static postingsList * createNewPostingsPist(int documentId)
{
    postingsList *pl;
    
    pl = malloc(sizeof(postingsList));
    if (!pl) {
        print_error("cannot allocate memory for a postings list.");
        return NULL;
    }
    pl->documentId = documentId;
    pl->positionsCount = 1;
    utarray_new(pl->positions, &ut_int_icd);
    
    return pl;
}

/**
 * 为传入的词元创建倒排列表
 * @param[in] documentId 文档编号 当documentId为0，是为查询的字符串创建倒排列表
 * @param[in] token 词元（UTF-8）
 * @param[in] tokenSize 词元的长度（以字节为单位）
 * @param[in] position 词元出现的位置
 * @param[in,out] postings 倒排列表的数组
 * @retval 0 成功
 * @retval -1 失败
 */

int tokenToPostingsList(searchLibEnv *env,
                        const int documentId,
                        const char *token,
                        const unsigned int tokenSize,
                        const int position,
                        invertedIndexHash **postings)
{
    
    postingsList *pList;
    invertedIndexValue *iiEntry;
    
    if (*postings) {
        HASH_FIND_STR(*postings, token, iiEntry);
    } else {
        iiEntry = NULL;
    }
    
    if (iiEntry) {
        pList = iiEntry->postingsList;
        pList->positionsCount ++;
    } else {
        int docsCount;
        if (documentId == 0) { //如果是在为查询的字符串创建倒排列表，从已有的例排文件中获取词元的 docsCount
           invertedIndexValue *tokenValue;
           HASH_FIND_STR(env->iiBuffer, token, tokenValue);
           docsCount = tokenValue ? tokenValue->docsCount : 0;
        } else {
           docsCount = 1;
        }
        
        iiEntry = createNewInvertedIndex(token, tokenSize, docsCount);
        if (!iiEntry) return -1;
        HASH_ADD_KEYPTR(hh, *postings, iiEntry->token, tokenSize, iiEntry);
        pList = createNewPostingsPist(documentId);
        if (!pList) return -1;
        LL_APPEND(iiEntry->postingsList, pList);
    }
    
    utarray_push_back(pList->positions, &position);
    iiEntry->positionsCount ++;
    
    return 0;
}


/**
 * 为构成文档内容的字符串建立倒排列表的集合
 * @param[in] env 存储着应用程序运行环境的结构体
 * @param[in] documentId 文档编号。为0时表示把要查询的关键词作为处理对象
 * @param[in] text 输入的字符串
 * @param[in] textLen 输入的字符串的长度
 * @param[in,out] postings 倒排列表的数组（也可视作是指向小倒排索引的指针）。若传入的指针指向了NULL，
 *                         则表示要新建一个倒排列表的数组（小倒排索引）。若传入的指针指向了之前就已经存在的倒排列表的数组，
 *                         则表示要添加元素
 * @retval 0 成功
 * @retval -1 失败
 */
int textToPostingsLists(searchLibEnv *env,
                        const int documentId,
                        const UTF32Char *text,
                        const int textLen,
                        invertedIndexHash **postings)
{
    invertedIndexHash *bufferPostings = NULL;
    int tLen, position = 0;
    int tokenLen = env->tokenLen;
    const UTF32Char *t = text, *tEnd = text + textLen;
    
    for (; (tLen = ngramNext(t, tEnd, tokenLen, &t)); t ++, position ++) {
        if (tLen >= tokenLen || documentId) {
            int retval = 0;
            int tokenSize;
            char token[tokenLen];
            utf32toutf8(t, tLen, token, &tokenSize);
            retval = tokenToPostingsList(env, documentId, token, tokenSize, position, &bufferPostings);
            if (retval) {return retval;}
        }
    }
    
    if (*postings) {
        mergeInvertedIndex(*postings, bufferPostings);
    } else {
        *postings = bufferPostings;
    }
    
    return 0;
}
