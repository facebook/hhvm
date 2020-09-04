/*
  +----------------------------------------------------------------------+
  | HipHop for PHP                                                       |
  +----------------------------------------------------------------------+
  | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
*/

#ifndef HPHP_RUNTIME_TEST_BESPOKE_LAYOUT_MOCK_H_
#define HPHP_RUNTIME_TEST_BESPOKE_LAYOUT_MOCK_H_

#include "hphp/runtime/base/bespoke/layout.h"

#include <folly/portability/GMock.h>

namespace HPHP{
namespace bespoke {
namespace testing {

struct MockLayout : Layout {
  MOCK_CONST_METHOD0(describe, std::string());

  MOCK_CONST_METHOD1(heapSize, size_t(const ArrayData* ad));
  MOCK_CONST_METHOD2(scan, void(const ArrayData* ad, type_scan::Scanner& scan));
  MOCK_CONST_METHOD2(escalateToVanilla, ArrayData*(const ArrayData*, const char* reason));

  MOCK_CONST_METHOD2(convertToUncounted, void(ArrayData*, DataWalker::PointerMap* seen));
  MOCK_CONST_METHOD1(releaseUncounted, void(ArrayData*));
  MOCK_CONST_METHOD1(release, void(ArrayData*));

  MOCK_CONST_METHOD1(isVectorData, bool(const ArrayData*));
  MOCK_CONST_METHOD2(getInt, TypedValue(const ArrayData*, int64_t));
  MOCK_CONST_METHOD2(getStr, TypedValue(const ArrayData*, const StringData*));
  MOCK_CONST_METHOD2(getKey, TypedValue(const ArrayData*, ssize_t pos));
  MOCK_CONST_METHOD2(getVal, TypedValue(const ArrayData*, ssize_t pos));
  MOCK_CONST_METHOD2(getIntPos, ssize_t(const ArrayData*, int64_t));
  MOCK_CONST_METHOD2(getStrPos, ssize_t(const ArrayData*, const StringData*));

  MOCK_CONST_METHOD2(lvalInt, arr_lval(ArrayData* ad, int64_t k));
  MOCK_CONST_METHOD2(lvalStr, arr_lval(ArrayData* ad, StringData* k));
  MOCK_CONST_METHOD3(setInt, ArrayData*(ArrayData*, int64_t k, TypedValue v));
  MOCK_CONST_METHOD3(setStr, ArrayData*(ArrayData*, StringData* k, TypedValue v));
  MOCK_CONST_METHOD2(removeInt, ArrayData*(ArrayData*, int64_t));
  MOCK_CONST_METHOD2(removeStr, ArrayData*(ArrayData*, const StringData*));

  MOCK_CONST_METHOD1(iterBegin, ssize_t(const ArrayData*));
  MOCK_CONST_METHOD1(iterLast, ssize_t(const ArrayData*));
  MOCK_CONST_METHOD1(iterEnd, ssize_t(const ArrayData*));
  MOCK_CONST_METHOD2(iterAdvance, ssize_t(const ArrayData*, ssize_t));
  MOCK_CONST_METHOD2(iterRewind, ssize_t(const ArrayData*, ssize_t));

  MOCK_CONST_METHOD2(append, ArrayData*(ArrayData*, TypedValue v));
  MOCK_CONST_METHOD2(prepend, ArrayData*(ArrayData*, TypedValue v));
  MOCK_CONST_METHOD2(merge, ArrayData*(ArrayData*, const ArrayData*));
  MOCK_CONST_METHOD2(pop, ArrayData*(ArrayData*, Variant&));
  MOCK_CONST_METHOD2(dequeue, ArrayData*(ArrayData*, Variant&));
  MOCK_CONST_METHOD1(renumber, ArrayData*(ArrayData*));

  MOCK_CONST_METHOD1(copy, ArrayData*(const ArrayData*));
  MOCK_CONST_METHOD2(toVArray, ArrayData*(ArrayData*, bool copy));
  MOCK_CONST_METHOD2(toDArray, ArrayData*(ArrayData*, bool copy));
  MOCK_CONST_METHOD2(toVec, ArrayData*(ArrayData*, bool copy));
  MOCK_CONST_METHOD2(toDict, ArrayData*(ArrayData*, bool copy));
  MOCK_CONST_METHOD2(toKeyset, ArrayData*(ArrayData*, bool copy));

  MOCK_CONST_METHOD2(setLegacyArrayInPlace, void(ArrayData*, bool legacy));
};

inline Layout* makeDummyLayout(const std::string& name) {
  auto ret = new MockLayout();
  using ::testing::Return;
  using ::testing::Mock;

  EXPECT_CALL(*ret, describe())
    .WillRepeatedly(Return(name));

  Mock::AllowLeak(ret);

  return ret;
}


} // namespace testing
} // namespace bespoke
} // namespace HPHP

#endif // HPHP_RUNTIME_TEST_BESPOKE_LAYOUT_MOCK_H_
