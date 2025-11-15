#include <vector>

#include "simple16.h"
#include "util.h"

namespace FastPForLib {

TEST(Simple16Test, DecodesWithUnknownLength) {
  Simple16<false> codec;
  std::vector<uint32_t> in;
  for (uint32_t i = 0; i < 128; ++i) {
    in.push_back(i);
  }

  // Simple16 may overrun the output buffer regardless of `n`, so a headroom of
  // 28 (the maximum number of elements in a single pack) is added.
  std::vector<uint32_t> decoded(in.size() + 28, 0);
  verifyUnknownInputLengthDecode(codec, in, decoded);
}
} // namespace FastPForLib
