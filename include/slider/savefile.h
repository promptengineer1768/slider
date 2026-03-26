#ifndef SLIDER_CORE_SAVEFILE_H_
#define SLIDER_CORE_SAVEFILE_H_

#include "slider/state.h"

#include <cstddef>
#include <filesystem>
#include <optional>
#include <utility>
#include <string>

namespace slider {

struct SaveFileOptions {
  // Maximum save file size accepted by the loader.
  size_t max_bytes = 1024 * 10;  // 10KB
};

// Pair-return save API: `first` indicates success, `second` carries an error.
using SaveBoardStateResult = std::pair<bool, std::string>;
using LoadBoardStateResult = std::pair<std::optional<BoardState>, std::string>;

SaveBoardStateResult SaveBoardStateToFile(const std::filesystem::path& path,
                                          const BoardState& state);

LoadBoardStateResult LoadBoardStateFromFile(const std::filesystem::path& path,
                                            const SaveFileOptions& options);

}  // namespace slider

#endif  // SLIDER_CORE_SAVEFILE_H_
