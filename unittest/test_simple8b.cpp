#include <vector>

#include "simple8b.h"
#include "util.h"

namespace FastPForLib {

TEST(Simple8bTest, DecodesWithUnknownLength) {
  Simple8b<false> codec;
  std::vector<uint32_t> in;
  for (uint32_t i = 0; i < 128; ++i) {
    in.push_back(i);
  }

  std::vector<uint32_t> decoded(in.size(), 0);
  verifyUnknownInputLengthDecode(codec, in, decoded);
}
} // namespace FastPForLib
