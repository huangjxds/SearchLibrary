//
//  main.c
//  SearchLibrary
//
//  Created by huangjiaxiong on 2016/11/9.
//  Copyright © 2016年 huangjiaxiong. All rights reserved.
//

#include <stdio.h>
#include "search_library.h"
#include "postings.h"
#include <string.h>
#include "utstring.h"

void testHash(invertedIndexHash **ii)
{
    invertedIndexValue *iiValue, *temp;
    int i;
    char *keys[3] = {"jay", "jay", "ken"};
  //  char *values[3] = {"jay", "tom", "ken"};
    for (i = 0; i < 3; i ++) {
        iiValue = (invertedIndexValue *)malloc(sizeof(invertedIndexValue));
        iiValue->docsCount = 0;
        char *key = keys[i];
      //  char *value = values[i];
        iiValue->token_id = i;
        strcpy(iiValue->token, key);
        HASH_FIND_STR(*ii, key, temp);
        if (temp != NULL) {
            printf("the key %s is exists in hash. \n", key);
            continue;
        }
        HASH_ADD_KEYPTR(hh, *ii, iiValue->token, strlen(iiValue->token), iiValue);
        printf ("insert item. key=%d,value=%s \n", iiValue->token_id, iiValue->token);
    }

}

int main(int argc, const char * argv[]) {
    
    searchLibEnv env;
    searchLibEnv *searchLibEnv = &env;
    slSearchInit(searchLibEnv);

    char *text = "小小罗13189230600";
    slAddDocument(searchLibEnv, 1, text);
    
    char *text1 = "陈小小罗姐15521779389";
    slAddDocument(searchLibEnv, 2, text1);
//
//    char *text2 = "小王";
//    slAddDocument(searchLibEnv, 3, text2);
    
    dumpInvertedIndex(searchLibEnv->iiBuffer);
    
    char *query = "155";
    slSearch(searchLibEnv, query);
    
    return 0;
}
