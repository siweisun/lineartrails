#ifndef ASCON_H_
#define ASCON_H_

#include <vector>
#include <array>
#include <random>

#include "layer.h"
#include "mask.h"
#include "statemask.h"
#include "step_linear.h"
#include "step_nonlinear.h"
#include "updatequeue.h"
#include "memory"
#include "lrucache.h"


struct AsconState : public StateMask<5,64> {
  AsconState();
  friend std::ostream& operator<<(std::ostream& stream, const AsconState& statemask);
  void print(std::ostream& stream);
  virtual AsconState* clone();
};


#define ROTR(x,n) (((x)>>(n))|((x)<<(64-(n))))
//#define ROTL(x,n) (((x)<<(n))|((x)>>(64-(n))))

//27,39,42,59,57, 2,13,63,23,31

template <unsigned round>
std::array<BitVector, 1> AsconSigma(std::array<BitVector, 1> in) {
  switch (round) {
    case 0: 
      return {in[0] ^ ROTR(in[0], 27) ^ ROTR(in[0], 39)};
    case 1: 
      return {in[0] ^ ROTR(in[0], 42) ^ ROTR(in[0], 59)};
    case 2: 
      return {in[0] ^ ROTR(in[0], 57) ^ ROTR(in[0],  2)};
    case 3: 
      return {in[0] ^ ROTR(in[0], 13) ^ ROTR(in[0], 63)};
    case 4: 
      return {in[0] ^ ROTR(in[0], 23) ^ ROTR(in[0], 31)};
    default: 
      return {0};
  }
}



struct AsconLinearLayer : public LinearLayer {
  AsconLinearLayer& operator=(const AsconLinearLayer& rhs);
  AsconLinearLayer();
  virtual AsconLinearLayer* clone();
  void Init();
  AsconLinearLayer(StateMaskBase *in, StateMaskBase *out);
  virtual bool Update(UpdatePos pos);
  int GetNumLayer();

  static const unsigned int word_size_ = { 64 };
  static const unsigned int words_per_step_ = { 1 };
  static const unsigned int linear_steps_ = { 5 };
  static const unsigned int cache_size_ = { 0x1000 };
  std::array<LinearStep<word_size_, words_per_step_>, linear_steps_> sigmas;
  static std::unique_ptr<
      LRU_Cache<WordMaskArray<word_size_, words_per_step_>,
          LinearStepUpdateInfo<word_size_, words_per_step_>>> cache_[linear_steps_];
};


struct AsconSboxLayer : public SboxLayer<5, 64> {
  AsconSboxLayer& operator=(const AsconSboxLayer& rhs);
  AsconSboxLayer();
  AsconSboxLayer(StateMaskBase *in, StateMaskBase *out);
  virtual AsconSboxLayer* clone();
  virtual bool Update(UpdatePos pos);
  Mask GetVerticalMask(int b, const StateMaskBase& s) const;
  void SetVerticalMask(int b, StateMaskBase& s, const Mask& mask);

 static const unsigned int cache_size_ = { 0x1000 };
 static std::unique_ptr<LRU_Cache<unsigned long long,NonlinearStepUpdateInfo>> cache_;
};



#endif // ASCON_H_
