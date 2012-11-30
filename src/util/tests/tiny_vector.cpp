#include "util/tiny_vector.h"
#include <gtest/gtest.h>

namespace HPHP {

TEST(TinyVector, Basic) {
  TinyVector<int> v;

  auto fill = [&] {
    for (int i = 0; i < 10; ++i) {
      v.push_back(i);
    }
  };

  auto checkElems = [&] {
    for (auto& i : v) {
      EXPECT_EQ(i, v[i]);
    }
  };

  fill();
  EXPECT_EQ(10ul, v.size());
  checkElems();

  v.clear();
  EXPECT_EQ(0ul, v.size());

  fill();
  EXPECT_EQ(10ul, v.size());
  size_t size = v.size();
  for (size_t i = 0; i < size; ++i) {
    v.pop_back();
    checkElems();
    EXPECT_EQ(size - (i + 1), v.size());
  }

  EXPECT_EQ(0ul, v.size());
}

}
