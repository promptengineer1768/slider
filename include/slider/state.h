#ifndef SLIDER_CORE_STATE_H_
#define SLIDER_CORE_STATE_H_

#include <vector>
#include <cstdint>
#include <string>

namespace slider {

class BoardState {
 public:
  BoardState() = default;
  BoardState(int size, const std::vector<int>& tiles);

  int GetSize() const { return size_; }
  const std::vector<int>& GetTiles() const { return tiles_; }
  int GetEmptyPos() const;
  
  bool IsSolved() const;
  bool IsValid() const;

  bool operator==(const BoardState& other) const;
  bool operator<(const BoardState& other) const; // For std::set/std::map

  std::string Serialize() const;
  static BoardState Deserialize(const std::string& data);

 private:
  int size_ = 0;
  std::vector<int> tiles_;
};

} // namespace slider

#endif // SLIDER_CORE_STATE_H_
