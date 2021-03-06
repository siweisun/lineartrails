/*
 * search.cpp
 *
 *  Created on: Jun 11, 2014
 *      Author: cdobraunig
 */

#include "search.h"


Search::Search(Permutation &perm)
    : perm_(&perm) {

}


/*
void Search::RandomSearch1(unsigned int iterations,
                           std::function<int(int, int, int)> rating) {
  std::unique_ptr<Permutation> working_copy, temp_copy;
  double best_prob = -DBL_MAX;
  working_copy.reset(perm_->clone());
  working_copy->checkchar();

  std::vector<SboxPos> active;
  std::vector<SboxPos> inactive;

  std::mt19937 generator(
      std::chrono::high_resolution_clock::now().time_since_epoch().count());

  for (unsigned int i = 0; i < iterations; ++i) {
    temp_copy.reset(working_copy->clone());
    temp_copy->SboxStatus(active, inactive);
    while (active.size() != 0 || inactive.size() != 0) {
      while (inactive.size() != 0) {
        std::uniform_int_distribution<int> guessbox(0, inactive.size() - 1);
        if (temp_copy->guessbestsbox(inactive[guessbox(generator)], rating)
            == false) {
          temp_copy.reset(working_copy->clone());
          active.clear();
          break;
        }
        temp_copy->SboxStatus(active, inactive);
      }

      while (active.size() != 0) {
        std::uniform_int_distribution<int> guessbox(0, active.size() - 1);
        if (temp_copy->guessbestsbox(active[guessbox(generator)], rating)
            == false) {
          temp_copy.reset(working_copy->clone());
          temp_copy->SboxStatus(active, inactive);
          break;
        }
        temp_copy->SboxStatus(active, inactive);
      }
    }
    double current_prob = temp_copy->GetProbability().bias;
    if (current_prob > best_prob) {
      best_prob = current_prob;
      std::cout << "iteration: " << i << std::endl;
      temp_copy->PrintWithProbability();
    }
  }
}

//Search without stack using weighted guesses
//TODO: Separate finding guess pos from actual guessing
void Search::HeuristicSearch1(unsigned int iterations, GuessWeights weights,
                              std::function<int(int, int, int)> rating,
                              int try_one_box, bool count_active) {

  std::unique_ptr<Permutation> working_copy;
  std::unique_ptr<Permutation> temp_copy;
  double best_prob = -DBL_MAX;
  working_copy.reset(perm_->clone());
  if (working_copy->checkchar() == false)
    return;

  std::array<std::vector<std::vector<SboxPos>>, 2> active_boxes;

  working_copy->SboxStatus(active_boxes[1], active_boxes[0]);

  size_t num_rounds = active_boxes[1].size();
  size_t num_settings = weights.size();

  std::mt19937 generator(
      std::chrono::high_resolution_clock::now().time_since_epoch().count());

  for (unsigned int i = 0; i < iterations; ++i) {
    bool fail = true;
    while (fail) {
      GuessWeights working_weights = weights;
      std::vector<float> total_weights;

      temp_copy.reset(working_copy->clone());
      for(auto& setting : weights) {
        assert(setting.size() == num_rounds);
        int total_weight = 0;
        for(auto& layer : setting)
        total_weight += layer[0] + layer[1];
        total_weights.push_back(total_weight);
      }

      size_t current_setting = 0;
      while(current_setting < num_settings) {
        fail = false;
        temp_copy->SboxStatus(active_boxes[1], active_boxes[0]);

        std::uniform_real_distribution<float> distr_setting(0,total_weights[current_setting]);
        int random_weight = distr_setting(generator);

        for(int active = 0; active <= 1; ++active) {
          for(size_t layer = 0; layer < working_weights[current_setting].size(); ++layer) {
            random_weight -= working_weights[current_setting][layer][active];
            if(random_weight < 1) {
              if(active_boxes[active][layer].size() == 0) {
                if(active_boxes[0][layer].size() == 0) {
                  total_weights[current_setting] -= working_weights[current_setting][layer][active];
                  working_weights[current_setting][layer][active] = 0;
                }
              }
              else {
                std::uniform_int_distribution<int> guessbox(0, active_boxes[active][layer].size() - 1);
                if (temp_copy->guessbestsboxrandom(active_boxes[active][layer][guessbox(generator)], rating, try_one_box) == false) {
                  current_setting = num_settings + 1;
                  fail = true;
                }
              }
              active = 2;
              break;
            }
          }
        }

        if(current_setting < num_settings)
        if(total_weights[current_setting] == 0)
        current_setting = current_setting + 1;
      }
    }
    double current_prob;
    if (count_active)
      current_prob = -temp_copy->GetActiveSboxes();
    else
      current_prob = temp_copy->GetProbability().bias;
    if (current_prob > best_prob) {
      best_prob = current_prob;
      std::cout << "iteration: " << i << std::endl;
      temp_copy->PrintWithProbability();
    }
  }
}

void Search::HeuristicSearch2(unsigned int iterations, GuessWeights weights,
                              std::function<int(int, int, int)> rating,
                              int try_one_box, bool count_active) {

  std::unique_ptr<Permutation> working_copy;
  std::unique_ptr<Permutation> temp_copy;
  double best_prob = -DBL_MAX;
  GuessMask guesses;
  SboxPos guessed_box(0, 0);
  bool active;

  working_copy.reset(perm_->clone());
  if (working_copy->checkchar() == false)
    return;

//  guesses.createMask(working_copy.get(), weights);
//
//  for(auto& pos : guesses.weighted_pos_){
//    std::cout << "(" << (int) pos.first.layer_ << ", " << (int) pos.first.pos_ << " : " << pos.second << ")";
//  }
//  std::cout << std::endl;

  for (unsigned int i = 0; i < iterations; ++i) {
    temp_copy.reset(working_copy->clone());
    guesses.createMask(temp_copy.get(), weights);
    while (guesses.getRandPos(guessed_box, active)) {
      if (temp_copy->guessbestsboxrandom(guessed_box, rating, try_one_box)
          == false) {
        temp_copy.reset(working_copy->clone());
      }
      guesses.createMask(temp_copy.get(), weights);
    }
    double current_prob;
    if (count_active)
      current_prob = -temp_copy->GetActiveSboxes();
    else
      current_prob = temp_copy->GetProbability().bias;
    if (current_prob > best_prob) {
      best_prob = current_prob;
      std::cout << "iteration: " << i << std::endl;
      temp_copy->PrintWithProbability();
    }

  }
}

void Search::HeuristicSearch3(unsigned int iterations, GuessWeights weights,
                              std::function<int(int, int, int)> rating,
                              int try_one_box, bool count_active) {

  std::unique_ptr<Permutation> working_copy;
  std::unique_ptr<Permutation> temp_copy;
  double best_prob = -DBL_MAX;
  GuessMask guesses;
  SboxPos guessed_box(0, 0);
  bool active;

  working_copy.reset(perm_->clone());
  if (working_copy->checkchar() == false) {
    std::cout << "Initial chackchar failed" << std::endl;
    return;
  }

//  guesses.createMask(working_copy.get(), weights);
//
//  for(auto& pos : guesses.weighted_pos_){
//    std::cout << "(" << (int) std::get<0>(pos).layer_ << ", " << (int) std::get<0>(pos).pos_ << " : " << std::get<1>(pos) << ")";
//  }
//  std::cout << std::endl;

  for (unsigned int i = 0; i < iterations; ++i) {
    temp_copy.reset(working_copy->clone());
    guesses.createMask(temp_copy.get(), weights);
    bool still_guesses = guesses.getRandPos(guessed_box, active);
    while (still_guesses) {
      if (active == temp_copy->isActive(guessed_box))
        if (temp_copy->guessbestsboxrandom(guessed_box, rating, try_one_box)
            == false) {
          temp_copy.reset(working_copy->clone());
          guesses.createMask(temp_copy.get(), weights);
        }
      if (guesses.getRandPos(guessed_box, active) == 0) {
        guesses.createMask(temp_copy.get(), weights);
        still_guesses = guesses.getRandPos(guessed_box, active);
      }

    }
    double current_prob;
    if (count_active)
      current_prob = -temp_copy->GetActiveSboxes();
    else
      current_prob = temp_copy->GetProbability().bias;
    if (current_prob > best_prob) {
      best_prob = current_prob;
      std::cout << "iteration: " << i << std::endl;
      temp_copy->PrintWithProbability();
    }

  }
}*/

void Search::StackSearch1(Commandlineparser& cl_param,
                          Configparser& config_param) {

  std::unique_ptr<Permutation> working_copy;
  std::stack<std::unique_ptr<Permutation>> char_stack;

  double best_prob = -DBL_MAX;
  GuessMask guesses;
  SboxPos guessed_box(0, 0);
  SboxPos backtrack_box(0, 0);
  bool backtrack;
  bool active;

  working_copy = config_param.getPermutation();
  if (working_copy->checkchar() == false) {
    std::cout << "Initial checkchar failed" << std::endl;
    return;
  }
  Settings settings = config_param.getSettings();

  auto start_count = std::chrono::system_clock::now();
  std::mt19937 generator(
      std::chrono::high_resolution_clock::now().time_since_epoch().count());
  std::uniform_real_distribution<float> push_stack_rand(0.0, 1.0);

//  guesses.createMask(working_copy.get(), weights);
//
//  for(auto& pos : guesses.weighted_pos_){
//    std::cout << "(" << (int) pos.first.layer_ << ", " << (int) pos.first.pos_ << " : " << pos.second << ")";
//  }
//  std::cout << std::endl;
  unsigned int interations = (unsigned int) cl_param.getIntParameter("-iter");
  int total_iterations = 0;
  int print_char = cl_param.getIntParameter("-S");
  for (unsigned int i = 0; i < interations; ++i) {
    char_stack.emplace(working_copy->clone());
    char_stack.emplace(working_copy->clone());
    backtrack = false;
    guesses.createMask(char_stack.top().get(), settings);
    unsigned int curr_credit = config_param.getCredits();
    while (guesses.getRandPos(guessed_box, active)) {
      total_iterations++;
      auto duration = std::chrono::duration_cast<std::chrono::seconds>(
          std::chrono::system_clock::now() - start_count);
      if (cl_param.getIntParameter("-I") > 0
          && duration.count() > cl_param.getIntParameter("-I")) {
        std::cout << "PRINT-INFO: total iterations: " << total_iterations
                  << ", stack size: " << char_stack.size() << ", credits: "
                  << curr_credit << ", restarts: " << i << std::endl;
        print_char--;
        if (print_char == 0) {
          char_stack.top()->print(std::cout);
          print_char = cl_param.getIntParameter("-S");
        }
        start_count = std::chrono::system_clock::now();
      }

      if (curr_credit == 0)
        break;

      if (backtrack)
        guessed_box = backtrack_box;

      unsigned int wbias = guesses.getSboxWeigthProb();
      unsigned int whamming = guesses.getSboxWeightHamming();
      //FIXME: get rid of the 10
      auto rating = [wbias, whamming] (int bias, int hw_in, int hw_out) {
        return wbias*std::abs(bias) +whamming*((10-hw_in)+(10-hw_out));
      };
      if (char_stack.top()->guessbestsboxrandom(
          guessed_box, rating, guesses.getAlternativeSboxGuesses())) {
//          std::cout << "worked " << char_stack.size() << std::endl;
//          char_stack.top()->print(std::cout);
        backtrack = false;
        if (push_stack_rand(generator) <= guesses.getPushStackProb())
          char_stack.emplace(char_stack.top()->clone());
      } else {
//          std::cout << "failed" << std::endl;
//                    char_stack.top()->print(std::cout);
        char_stack.pop();
        curr_credit--;
        backtrack = true;
        backtrack_box = guessed_box;
        if (char_stack.size() == 1)
          char_stack.emplace(working_copy->clone());
      }
      guesses.createMask(char_stack.top().get(), settings);
    }
    double current_prob;
    if (config_param.printActive())
      current_prob = -char_stack.top()->GetActiveSboxes();
    else
      current_prob = char_stack.top()->GetProbability();
    if (current_prob > best_prob && curr_credit > 0) {
      best_prob = current_prob;
      std::cout << "iteration: " << i << std::endl;
      char_stack.top()->PrintWithProbability();
    }
    while (char_stack.size())
      char_stack.pop();
  }
}

void Search::StackSearchKeccak(Commandlineparser& cl_param,
                          Configparser& config_param) {

  std::unique_ptr<Permutation> working_copy;
  std::stack<std::unique_ptr<Permutation>> char_stack;

  double best_prob = -DBL_MAX;
  GuessMask guesses;
  SboxPos guessed_box(0, 0);
  SboxPos backtrack_box(0, 0);
  bool backtrack;
  bool active;

  working_copy = config_param.getPermutation();
  if (working_copy->checkchar() == false) {
    std::cout << "Initial checkchar failed" << std::endl;
    return;
  }
  Settings settings = config_param.getSettings();

  auto start_count = std::chrono::system_clock::now();
  std::mt19937 generator(
      std::chrono::high_resolution_clock::now().time_since_epoch().count());
  std::uniform_real_distribution<float> push_stack_rand(0.0, 1.0);

//  guesses.createMask(working_copy.get(), weights);
//
//  for(auto& pos : guesses.weighted_pos_){
//    std::cout << "(" << (int) pos.first.layer_ << ", " << (int) pos.first.pos_ << " : " << pos.second << ")";
//  }
//  std::cout << std::endl;
  unsigned int interations = (unsigned int) cl_param.getIntParameter("-iter");
  int total_iterations = 0;
  int print_char = cl_param.getIntParameter("-S");
  for (unsigned int i = 0; i < interations; ++i) {
    char_stack.emplace(working_copy->clone());
    char_stack.emplace(working_copy->clone());
    backtrack = false;
    guesses.createMask(char_stack.top().get(), settings);
    unsigned int curr_credit = config_param.getCredits();
    while (guesses.getRandPos(guessed_box, active)) {
      total_iterations++;
      auto duration = std::chrono::duration_cast<std::chrono::seconds>(
          std::chrono::system_clock::now() - start_count);
      if (cl_param.getIntParameter("-I") > 0
          && duration.count() > cl_param.getIntParameter("-I")) {
        std::cout << "PRINT-INFO: total iterations: " << total_iterations
                  << ", stack size: " << char_stack.size() << ", credits: "
                  << curr_credit << ", restarts: " << i << std::endl;
        print_char--;
        if (print_char == 0) {
          char_stack.top()->print(std::cout);
          print_char = cl_param.getIntParameter("-S");
        }
        start_count = std::chrono::system_clock::now();
      }

      if (curr_credit == 0)
        break;

      if (backtrack)
        guessed_box = backtrack_box;

      unsigned int wbias = guesses.getSboxWeigthProb();
      unsigned int whamming = guesses.getSboxWeightHamming();
      //FIXME: get rid of the 10
      auto rating = [wbias, whamming] (int bias, int hw_in, int hw_out) {
        return wbias * std::abs(bias) + whamming * ((10 - hw_in) + (10 - hw_out));
      };
      if (char_stack.top()->guessbestsboxrandom(guessed_box, rating, guesses.getAlternativeSboxGuesses())) {
//          std::cout << "worked " << char_stack.size() << std::endl;
//          char_stack.top()->print(std::cout);
        backtrack = false;
        if (push_stack_rand(generator) <= guesses.getPushStackProb())
          char_stack.emplace(char_stack.top()->clone());
      } else {
//          std::cout << "failed" << std::endl;
//                    char_stack.top()->print(std::cout);
        char_stack.pop();
        curr_credit--;
        backtrack = true;
        backtrack_box = guessed_box;
        if (char_stack.size() == 1)
          char_stack.emplace(working_copy->clone());
      }
      guesses.createMask(char_stack.top().get(), settings);
    }
    double current_prob;
    current_prob = KeccakProb(char_stack);
    if (current_prob > best_prob && curr_credit > 0) {
      best_prob = current_prob;
      std::cout << "iteration: " << i << std::endl;
      std::cout << "bias without last round: " << best_prob << std::endl;
      char_stack.top()->PrintWithProbability();
    }
    while (char_stack.size())
      char_stack.pop();
  }
}

double Search::KeccakProb(std::stack<std::unique_ptr<Permutation>>& char_stack) {
  double prob = 0.0;
  double temp_prob;

  for (unsigned i = 0; i < char_stack.top()->sbox_layers_.size() - 1; ++i) {
    temp_prob = char_stack.top()->sbox_layers_[i]->GetProbability();
    prob += temp_prob;
  }

  prob += char_stack.top()->sbox_layers_.size() - 2;

  return prob;
}

