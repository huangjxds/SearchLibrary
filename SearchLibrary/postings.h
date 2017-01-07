//
//  postings.h
//  SearchLibrary
//
//  Created by huangjiaxiong on 2016/11/20.
//  Copyright © 2016年 huangjiaxiong. All rights reserved.
//

#ifndef postings_h
#define postings_h

#include "search_library.h"

void freeInvertedIndex(invertedIndexHash *ii);

void freePostingsList(postingsList *pl);

int fetchPostings(searchLibEnv *env, char *token, postingsList **postings);

void mergeInvertedIndex(invertedIndexHash *base, invertedIndexHash *toBeAdded);

void dumpInvertedIndex(invertedIndexHash *ii);

void dumpPostingsList(const postingsList *postings);

#endif /* postings_h */
