#ifndef FASTPFORLIB_UTIL_H_
#define FASTPFORLIB_UTIL_H_

#include <vector>

#include "gtest/gtest.h"

namespace FastPForLib {

template <class Codec>
void verifyUnknownInputLengthDecode(Codec &codec,
                                    const std::vector<uint32_t> &in,
                                    std::vector<uint32_t> &decoded) {
  std::vector<uint32_t> encoded(in.size() * 2, 0);
  size_t encodedSize;
  codec.encodeArray(in.data(), in.size(), encoded.data(), encodedSize);
  encoded.resize(encodedSize);

  size_t n = in.size();
  const uint32_t *decodedUntil =
      codec.decodeArray(encoded.data(), 0, decoded.data(), n);
  decoded.resize(n);

  // Check that the decoded size matches the input size.
  EXPECT_EQ(n, in.size());

  // Check that the returned pointer matches the end of the encoded buffer.
  EXPECT_EQ(decodedUntil, encoded.data() + encodedSize);

  // Check each decoded element matches its corresponding input value.
  for (size_t i = 0; i < in.size(); ++i) {
    EXPECT_EQ(in[i], decoded[i]);
  }
}

} // namespace FastPForLib

#endif // FASTPFORLIB_UTIL_H_
