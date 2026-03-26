#include "slider/savefile.h"

#include <fstream>
#include <system_error>

namespace slider {

static void SetError(std::string* out, const char* msg) {
  if (out) *out = msg;
}

bool SaveBoardStateToFile(const std::filesystem::path& path,
                          const BoardState& state,
                          std::string* error) {
  if (!state.IsValid()) {
    SetError(error, "Invalid board state");
    return false;
  }

  std::ofstream os(path, std::ios::binary);
  if (!os) {
    SetError(error, "Could not open file for writing");
    return false;
  }

  const std::string data = state.Serialize();
  os.write(data.data(), static_cast<std::streamsize>(data.size()));
  if (!os) {
    SetError(error, "Could not write file");
    return false;
  }

  return true;
}

std::optional<BoardState> LoadBoardStateFromFile(const std::filesystem::path& path,
                                                 const SaveFileOptions& options,
                                                 std::string* error) {
  std::error_code ec;
  const auto file_size = std::filesystem::file_size(path, ec);
  if (ec) {
    SetError(error, "Could not read file size");
    return std::nullopt;
  }

  if (static_cast<size_t>(file_size) > options.max_bytes) {
    SetError(error, "Save file too large");
    return std::nullopt;
  }

  std::ifstream is(path, std::ios::binary);
  if (!is) {
    SetError(error, "Could not open file for reading");
    return std::nullopt;
  }

  std::string content;
  content.resize(static_cast<size_t>(file_size));
  if (!content.empty()) {
    is.read(content.data(), static_cast<std::streamsize>(content.size()));
    if (!is) {
      SetError(error, "Could not read file");
      return std::nullopt;
    }
  }

  BoardState decoded = BoardState::Deserialize(content);
  if (!decoded.IsValid()) {
    SetError(error, "Invalid save file");
    return std::nullopt;
  }

  return decoded;
}

}  // namespace slider

