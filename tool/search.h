/*
 * search.h
 *
 *  Created on: Jun 11, 2014
 *      Author: cdobraunig
 */

#ifndef SEARCH_H_
#define SEARCH_H_

#include <memory>
#include <chrono>
#include <random>
#include <cfloat>
#include <vector>
#include <assert.h>

#include "permutation.h"
#include "mask.h"

class Search {

 public:
  Search(PermutationBase &perm);
  void RandomSearch1(unsigned int iterations, std::function<int(int, int, int)> rating);
  void HeuristicSearch1(unsigned int iterations, std::vector<std::vector<std::array<int,2>>> weights, std::function<int(int, int, int)> rating, int try_one_box, bool count_active);


  PermutationBase *perm_;

};

#endif /* SEARCH_H_ */