//
//  token.h
//  SearchLibrary
//
//  Created by huangjiaxiong on 2016/11/27.
//  Copyright © 2016年 huangjiaxiong. All rights reserved.
//

#ifndef token_h
#define token_h

#include <stdio.h>
#include "search_library.h"

int textToPostingsLists(searchLibEnv *env,
                        const int documentId,
                        const UTF32Char *text,
                        const int textLen,
                        invertedIndexHash **postings);



#endif /* token_h */
