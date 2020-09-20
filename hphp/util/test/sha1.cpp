#include <array>
#include <string>

#include <folly/portability/GTest.h>
#include <folly/Range.h>

#include "hphp/util/sha1.h"
#include "hphp/zend/zend-string.h"

using namespace HPHP;

SHA1 hash_content(folly::StringPiece content) {
  auto hashlen = 0;
  char* hash = string_sha1(content.data(), content.size(), false, hashlen);
  SCOPE_EXIT {
    free(hash);
  };
  return SHA1{folly::StringPiece{hash, static_cast<size_t>(hashlen)}};
}

TEST(Sha1, comparisons) {
  constexpr folly::StringPiece a = "teststringa";

  SHA1 asha = hash_content(a);
  EXPECT_EQ(asha, asha);

  constexpr folly::StringPiece b = "teststringb";
  SHA1 bsha = hash_content(b);
  EXPECT_EQ(bsha, bsha);

  // They're not equal and therefore one must be less than the other
  EXPECT_NE(asha, bsha);
  EXPECT_TRUE(asha < bsha || bsha < asha);

  // Their hex strings should not be equal
  EXPECT_NE(asha.toString(), bsha.toString());
}

TEST(Sha1, hexinverse) {
  constexpr folly::StringPiece a = "teststringa";
  SHA1 asha = hash_content(a);

  // Encode to hex
  auto ahex = asha.toString();

  // Decode from hex
  EXPECT_EQ(asha, SHA1{folly::StringPiece{ahex}});
}

TEST(Sha1, reasonablehash) {
  constexpr folly::StringPiece a = "teststringa";
  SHA1 asha = hash_content(a);

  // Verify the hash is really a 64-bit int
  EXPECT_NE(asha.hash() << 32, 0);
  EXPECT_NE(asha.hash() >> 32, 0);
}

TEST(Sha1, nboinverse) {
  constexpr folly::StringPiece a = "teststringa";
  SHA1 asha = hash_content(a);
  std::array<char, 20> anbo;
  asha.nbo(anbo.data());
  EXPECT_EQ(asha, SHA1(anbo.data(), anbo.size()));
}
