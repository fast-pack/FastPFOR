#include <vector>

#include "gtest/gtest.h"

#include "codecs.h"
#include "vsencoding.h"

namespace FastPForLib {

// Decodes into a buffer that has guard words after the `nvalue` integers
// decodeArray may write, and checks that the guard words are untouched.
static void verifyDecodeStaysInBounds(const std::vector<uint32_t> &in) {
  vsencoding::VSEncodingBlocks codec;
  std::vector<uint32_t> encoded(in.size() * 2 + 64, 0);
  size_t encodedSize = encoded.size();
  codec.encodeArray(in.data(), in.size(), encoded.data(), encodedSize);

  const uint32_t guard = 0xDEADBEEF;
  std::vector<uint32_t> decoded(in.size() + 64, guard);
  size_t n = in.size();
  const uint32_t *decodedUntil =
      codec.decodeArray(encoded.data(), encodedSize, decoded.data(), n);

  EXPECT_EQ(n, in.size());
  EXPECT_EQ(decodedUntil, encoded.data() + encodedSize);
  for (size_t i = 0; i < in.size(); ++i) {
    EXPECT_EQ(in[i], decoded[i]);
  }
  size_t overrun = 0;
  for (size_t i = in.size(); i < decoded.size(); ++i) {
    if (decoded[i] != guard) {
      overrun = i - in.size() + 1;
    }
  }
  EXPECT_EQ(overrun, 0U) << "decoding " << in.size()
                         << " integers wrote past the output buffer";
}

// https://github.com/fast-pack/FastPFor/issues/128
TEST(VSEncodingTest, DecodeDoesNotWritePastOutput) {
  verifyDecodeStaysInBounds({1, 2, 3, 4, 5});

  // Up to 300 integers span many blocks and several descriptor words. From 17
  // integers on, the encoder used to read past __vseblocks_possLens.
  for (uint32_t length = 0; length <= 300; ++length) {
    std::vector<uint32_t> increasing(length);
    for (uint32_t i = 0; i < length; ++i) {
      increasing[i] = i * 7;
    }
    verifyDecodeStaysInBounds(increasing);
    verifyDecodeStaysInBounds(std::vector<uint32_t>(length, 0));
  }
}
} // namespace FastPForLib
