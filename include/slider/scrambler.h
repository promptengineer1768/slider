#ifndef SLIDER_CORE_SCRAMBLER_H_
#define SLIDER_CORE_SCRAMBLER_H_

#include "slider/board.h"
#include <vector>

namespace slider {

class Scrambler {
 public:
  static std::vector<Direction> Scramble(Board& board, int max_moves = 100);
};

} // namespace slider

#endif // SLIDER_CORE_SCRAMBLER_H_
