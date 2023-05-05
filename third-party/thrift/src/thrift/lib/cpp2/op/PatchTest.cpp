/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/lib/cpp2/op/Patch.h>

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/op/Testing.h>

namespace apache {
namespace thrift {
namespace op {

TEST(PatchTest, BoolPatch) {
  BoolPatch patch;

  // Empty patch does nothing.
  test::expectPatch(patch, false, false);
  test::expectPatch(patch, true, true);

  // Inverting patch inverts.
  patch = BoolPatch::createInvert();
  test::expectPatch(patch, false, true, false);
  test::expectPatch(patch, true, false, true);

  // Double inverting patch does nothing.
  patch = !patch;
  test::expectPatch(patch, false, false);
  test::expectPatch(patch, true, true);

  // Assigning patch assigns.
  patch = true;
  test::expectPatch(patch, false, true);
  test::expectPatch(patch, true, true);
  patch = !patch;
  test::expectPatch(patch, false, false);
  test::expectPatch(patch, true, false);
}

template <typename NPatch>
void testNumberPatch() {
  NPatch patch;

  // Empty patch does nothing.
  test::expectPatch(patch, {7}, 7);

  // Incrementing patch increments.
  patch = 1 + NPatch::createAdd(1);
  test::expectPatch(patch, {7}, 9, 11);
  patch.merge(NPatch::createSubtract(2));
  test::expectPatch(patch, {7}, 7);

  // Assigning patch assigns.
  patch = 2;
  test::expectPatch(patch, {7}, 2);
  patch -= 2;
  test::expectPatch(patch, {7}, 0);
}

TEST(PatchTest, NumberPatch) {
  testNumberPatch<BytePatch>();
  testNumberPatch<I16Patch>();
  testNumberPatch<I32Patch>();
  testNumberPatch<I64Patch>();
  testNumberPatch<FloatPatch>();
  testNumberPatch<DoublePatch>();
}

TEST(PatchTest, StringPatch) {
  StringPatch patch;

  // Empty patch does nothing.
  test::expectPatch(patch, {"hi"}, "hi");

  // Relative patch patches.
  patch = StringPatch::createPrepend("_");
  patch += "_";
  test::expectPatch(patch, {"hi"}, "_hi_", "__hi__");
  patch.prepend("$");
  patch.merge(StringPatch::createAppend("^"));
  test::expectPatch(patch, {"hi"}, "$_hi_^", "$_$_hi_^_^");

  // Assign patch assigns.
  patch = "bye";
  test::expectPatch(patch, {"hi"}, "bye");
  patch = "__" + std::move(patch) + "__";
  test::expectPatch(patch, {"hi"}, "__bye__");
  patch = "";
  test::expectPatch(patch, {"hi"}, "");
}

TEST(PatchTest, BinaryPatch) {
  BinaryPatch patch;

  auto makeIOBuf = [](folly::StringPiece value) {
    return folly::IOBuf::wrapBufferAsValue(value);
  };

  // Empty patch does nothing.
  test::expectPatch(patch, {makeIOBuf("hi")}, makeIOBuf("hi"));

  // Relative patch patches.
  patch = BinaryPatch::createPrepend(makeIOBuf("_"));
  patch += makeIOBuf("_");
  test::expectPatch(
      patch, {makeIOBuf("hi")}, makeIOBuf("_hi_"), makeIOBuf("__hi__"));
  patch.prepend(makeIOBuf("$"));
  patch.merge(BinaryPatch::createAppend(makeIOBuf("^")));
  test::expectPatch(
      patch, {makeIOBuf("hi")}, makeIOBuf("$_hi_^"), makeIOBuf("$_$_hi_^_^"));

  // Assign patch assigns.
  patch = makeIOBuf("bye");
  test::expectPatch(patch, {makeIOBuf("hi")}, makeIOBuf("bye"));
  patch = makeIOBuf("__") + std::move(patch) + makeIOBuf("__");
  test::expectPatch(patch, {makeIOBuf("hi")}, makeIOBuf("__bye__"));
  patch = makeIOBuf("");
  test::expectPatch(patch, {makeIOBuf("hi")}, makeIOBuf(""));

  BinaryPatch clearPatch;
  clearPatch.clear();
  test::expectPatch(clearPatch, {makeIOBuf("hi")}, makeIOBuf(""));
}

} // namespace op
} // namespace thrift
} // namespace apache
