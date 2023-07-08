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

#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/op/Get.h>
#include <thrift/lib/cpp2/op/Patch.h>

namespace apache::thrift {
namespace {

// A callback to add a PatchOp to a Patch
template <class Patch>
using AddOp = std::function<void(Patch&)>;

template <class Patch>
using AddOps = std::vector<AddOp<Patch>>;

template <class Patch>
void testMergePatchOps(
    const AddOps<Patch>& addOpList, const typename Patch::value_type& value) {
  Patch mergedPatch, patchWithMultipleOps;
  auto v1 = value;
  for (const auto& addOp : addOpList) {
    Patch patch;
    addOp(patch);
    // v1 is the result when we apply each patch individually, e.g.,
    //
    //   p1.Method1()
    //   p1.apply(v1)
    //   p2.Method2()
    //   p2.apply(v1)
    //   p3.Method3()
    //   p3.apply(v1)
    patch.apply(v1);

    {
      // 1. Test whether applying mergedPatch patch is equivalent to applying
      // each patch individually, e.g., testing
      //
      //   p1.apply(v1)
      //   p2.apply(v1)
      //   p3.apply(v1)
      //
      // is equivalent to
      //
      //   Patch mergedPatch
      //   mergedPatch.merge(p1)
      //   mergedPatch.merge(p2)
      //   mergedPatch.merge(p3)
      //   mergedPatch.apply(v2)
      auto v2 = value;
      mergedPatch.merge(patch);
      mergedPatch.apply(v2);
      EXPECT_EQ(v1, v2);
    }

    {
      // 2. Test whether patch APIs have merge semantics. e.g., testing
      //
      //   p1.Method1()
      //   p1.apply(v1)
      //   p2.Method2()
      //   p2.apply(v1)
      //   p3.Method3()
      //   p3.apply(v1)
      //
      // is equivalent to
      //
      //   Patch patchWithMultipleOps
      //   patchWithMultipleOps.Method1()
      //   patchWithMultipleOps.Method2()
      //   patchWithMultipleOps.Method3()
      //   patchWithMultipleOps.apply(v2)
      auto v2 = value;
      addOp(patchWithMultipleOps);
      patchWithMultipleOps.apply(v2);
      EXPECT_EQ(v1, v2);
    }
  }
}

template <class Patch>
void pickMultipleOpsAndTest(
    const AddOps<Patch>& candidates,
    const std::vector<typename Patch::value_type>& values,
    const int numberOfOpToPick, // Ideally we want to pick as many as operations
                                // as possible, but the test would be slow if we
                                // pick too many operations.
    AddOps<Patch> pickedOps = {}) {
  if (numberOfOpToPick == 0) {
    for (const auto& v : values) {
      testMergePatchOps(pickedOps, v);
    }
    return;
  }

  for (auto op : candidates) {
    pickedOps.push_back(op);
    pickMultipleOpsAndTest(candidates, values, numberOfOpToPick - 1, pickedOps);
    pickedOps.pop_back();
  }
}

TEST(PatchMergeTest, BoolPatch) {
  AddOps<op::BoolPatch> ops;
  ops.push_back([](auto& patch) { patch = true; });
  ops.push_back([](auto& patch) { patch = false; });
  ops.push_back([](auto& patch) { patch.clear(); });
  ops.push_back([](auto& patch) { patch.invert(); });
  pickMultipleOpsAndTest(ops, {true, false}, 6);
}

} // namespace
} // namespace apache::thrift
