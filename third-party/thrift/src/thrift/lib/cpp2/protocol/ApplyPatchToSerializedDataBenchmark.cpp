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

#include <folly/Benchmark.h>
#include <folly/init/Init.h>
#include <thrift/lib/cpp2/patch/DynamicPatch.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>

using namespace folly;
using namespace std;

namespace apache::thrift::protocol {

// Testcase 1 uses an object with a large bool list and patch doesn't contain
// the field. Testcase 4 uses the same object and clears the field.
Object getObject1And4(int n) {
  Object obj;
  std::vector<Value> boolList;
  while (n--) {
    boolList.push_back(asValueStruct<type::bool_t>(n % 2));
  }
  obj[FieldId{1}].emplace_list(boolList);
  obj[FieldId{2}].emplace_i32(5);
  return obj;
}

DynamicPatch getPatch1And4(bool clear) {
  Value intPatch;
  intPatch.ensure_object()[FieldId{static_cast<int16_t>(op::PatchOp::Assign)}] =
      asValueStruct<type::i32_t>(1);
  Value objPatch;
  objPatch.ensure_object()[FieldId{2}] = intPatch;
  if (clear) {
    Value listPatch;
    listPatch
        .ensure_object()[FieldId{static_cast<int16_t>(op::PatchOp::Clear)}] =
        asValueStruct<type::bool_t>(true);
    objPatch.objectValue_ref().ensure()[FieldId{1}] = listPatch;
  }
  Object patchObj;
  patchObj[FieldId{static_cast<int16_t>(op::PatchOp::PatchAfter)}] = objPatch;
  return DynamicPatch::fromObject(std::move(patchObj));
}

// Testcase 2 uses an object with a large bool list and patch contains
// the field.
Object getObject2(int n) {
  Object obj;
  std::vector<Value> boolList;
  while (n--) {
    boolList.push_back(asValueStruct<type::bool_t>(n % 2));
  }
  obj[FieldId{1}].emplace_list(boolList);
  obj[FieldId{2}].emplace_i32(5);
  return obj;
}

DynamicPatch getPatch2() {
  Value intPatch;
  intPatch.ensure_object()[FieldId{static_cast<int16_t>(op::PatchOp::Assign)}] =
      asValueStruct<type::i32_t>(1);
  Value listPatch;
  listPatch.ensure_object()[FieldId{static_cast<int16_t>(op::PatchOp::Put)}]
      .emplace_list(std::vector<Value>{asValueStruct<type::bool_t>(true)});
  Value objPatch;
  objPatch.ensure_object()[FieldId{1}] = listPatch;
  objPatch.ensure_object()[FieldId{2}] = intPatch;
  Object patchObj;
  patchObj[FieldId{static_cast<int16_t>(op::PatchOp::PatchAfter)}] = objPatch;
  return DynamicPatch::fromObject(std::move(patchObj));
}

// Testcase 3 uses an object with many fields and patch contains all the fields.
Object getObject3(int n) {
  Object obj;
  while (n--) {
    obj[static_cast<FieldId>(n)].emplace_i32(n);
  }
  return obj;
}

DynamicPatch getPatch3(int n) {
  Value intPatch;
  intPatch.ensure_object()[FieldId{static_cast<int16_t>(op::PatchOp::Add)}] =
      asValueStruct<type::i32_t>(1);
  Value objPatch;
  while (n--) {
    objPatch.ensure_object()[static_cast<FieldId>(n)] = intPatch;
  }
  Object patchObj;
  patchObj[FieldId{static_cast<int16_t>(op::PatchOp::PatchAfter)}] = objPatch;
  return DynamicPatch::fromObject(std::move(patchObj));
}

Object getObjectWithMap(int n) {
  Object obj;
  auto& map = obj[FieldId{1}].emplace_map();
  while (n--) {
    auto v = asValueStruct<type::binary_t>(folly::to<string>(n));
    map[v] = v;
  }
  return obj;
}

// Testcase 5 uses an object with map<string, string> and patch contains some
// map elements with MapPatch::Put.
DynamicPatch getPatch5() {
  Object patchObj;
  auto& mapPatchObj =
      patchObj[FieldId{static_cast<int16_t>(op::PatchOp::PatchAfter)}]
          .emplace_object()[FieldId{1}]
          .emplace_object();

  auto& put =
      mapPatchObj[FieldId{static_cast<int16_t>(op::PatchOp::Put)}].ensure_map();
  put.emplace(
      asValueStruct<type::binary_t>("42"), asValueStruct<type::binary_t>("42"));
  put.emplace(
      asValueStruct<type::binary_t>("43"), asValueStruct<type::binary_t>("43"));
  return DynamicPatch::fromObject(std::move(patchObj));
}

// Testcase 6 uses an object with map<string, string> and patch contains all the
// map elements with MapPatch::Put.
DynamicPatch getPatch6(int n) {
  Object patchObj;
  auto& mapPatchObj =
      patchObj[FieldId{static_cast<int16_t>(op::PatchOp::PatchAfter)}]
          .emplace_object()[FieldId{1}]
          .emplace_object();
  while (n--) {
    auto v = asValueStruct<type::binary_t>(folly::to<string>(n));
    mapPatchObj[FieldId{static_cast<int16_t>(op::PatchOp::Put)}]
        .ensure_map()
        .emplace(v, v);
  }
  return DynamicPatch::fromObject(std::move(patchObj));
}

// Testcase 7 uses an object with map<string, string> and patch contains some
// map elements with MapPatch::PatchAfter.
DynamicPatch getPatch7() {
  Value assignPatchValue;
  assignPatchValue
      .emplace_object()[FieldId{static_cast<int16_t>(op::PatchOp::Assign)}] =
      asValueStruct<type::binary_t>("100");

  Object patchObj;
  auto& mapPatchObj =
      patchObj[FieldId{static_cast<int16_t>(op::PatchOp::PatchAfter)}]
          .emplace_object()[FieldId{1}]
          .emplace_object();

  auto& put =
      mapPatchObj[FieldId{static_cast<int16_t>(op::PatchOp::PatchAfter)}]
          .ensure_map();
  put.emplace(asValueStruct<type::binary_t>("42"), assignPatchValue);
  put.emplace(asValueStruct<type::binary_t>("43"), assignPatchValue);
  return DynamicPatch::fromObject(std::move(patchObj));
}

// Testcase 8 uses an object with map<string, string> and patch contains all the
// map elements with MapPatch::PatchAfter.
DynamicPatch getPatch8(int n) {
  Value assignPatchValue;
  assignPatchValue
      .emplace_object()[FieldId{static_cast<int16_t>(op::PatchOp::Assign)}] =
      asValueStruct<type::binary_t>("100");

  Object patchObj;
  auto& mapPatchObj =
      patchObj[FieldId{static_cast<int16_t>(op::PatchOp::PatchAfter)}]
          .emplace_object()[FieldId{1}]
          .emplace_object();
  while (n--) {
    auto v = asValueStruct<type::binary_t>(folly::to<string>(n));
    mapPatchObj[FieldId{static_cast<int16_t>(op::PatchOp::PatchAfter)}]
        .ensure_map()
        .emplace(v, assignPatchValue);
  }
  return DynamicPatch::fromObject(std::move(patchObj));
}

std::unique_ptr<folly::IOBuf> serialized1, serialized2, serialized3,
    serialized4, serialized5, serialized6, serialized7, serialized8;
DynamicPatch patch1, patch2, patch3, patch4, patch5, patch6, patch7, patch8;

void init(int n) {
  serialized1 =
      serializeObject<apache::thrift::CompactProtocolWriter>(getObject1And4(n));
  patch1 = getPatch1And4(false);
  serialized2 =
      serializeObject<apache::thrift::CompactProtocolWriter>(getObject2(n));
  patch2 = getPatch2();
  serialized3 =
      serializeObject<apache::thrift::CompactProtocolWriter>(getObject3(n));
  patch3 = getPatch3(n);
  serialized4 =
      serializeObject<apache::thrift::CompactProtocolWriter>(getObject1And4(n));
  patch4 = getPatch1And4(true);
  serialized5 = serializeObject<apache::thrift::CompactProtocolWriter>(
      getObjectWithMap(n));
  patch5 = getPatch5();
  serialized6 = serializeObject<apache::thrift::CompactProtocolWriter>(
      getObjectWithMap(n));
  patch6 = getPatch6(n);
  serialized7 = serializeObject<apache::thrift::CompactProtocolWriter>(
      getObjectWithMap(n));
  patch7 = getPatch7();
  serialized8 = serializeObject<apache::thrift::CompactProtocolWriter>(
      getObjectWithMap(n));
  patch8 = getPatch8(n);
}

BENCHMARK(few_small_fields) {
  patch1.applyToSerializedObjectWithoutExtractingMask<
      type::StandardProtocol::Compact>(*serialized1);
}

BENCHMARK_RELATIVE(few_small_fields_partial_deser) {
  patch1.applyToSerializedObject<type::StandardProtocol::Compact>(*serialized1);
}

BENCHMARK(large_fields) {
  patch2.applyToSerializedObjectWithoutExtractingMask<
      type::StandardProtocol::Compact>(*serialized2);
}

BENCHMARK_RELATIVE(large_fields_partial_deser) {
  patch2.applyToSerializedObject<type::StandardProtocol::Compact>(*serialized2);
}

BENCHMARK(all_small_fields) {
  patch3.applyToSerializedObjectWithoutExtractingMask<
      type::StandardProtocol::Compact>(*serialized3);
}

BENCHMARK_RELATIVE(all_small_fields_partial_deser) {
  patch3.applyToSerializedObject<type::StandardProtocol::Compact>(*serialized3);
}

BENCHMARK(clear_large_fields) {
  patch4.applyToSerializedObjectWithoutExtractingMask<
      type::StandardProtocol::Compact>(*serialized4);
}

BENCHMARK_RELATIVE(clear_large_fields_partial_deser) {
  patch4.applyToSerializedObject<type::StandardProtocol::Compact>(*serialized4);
}

BENCHMARK(few_map_elems_with_put) {
  patch5.applyToSerializedObjectWithoutExtractingMask<
      type::StandardProtocol::Compact>(*serialized5);
}

BENCHMARK_RELATIVE(few_map_elems_with_put_partial_deser) {
  patch5.applyToSerializedObject<type::StandardProtocol::Compact>(*serialized5);
}

BENCHMARK(all_map_elems_with_put) {
  patch6.applyToSerializedObjectWithoutExtractingMask<
      type::StandardProtocol::Compact>(*serialized6);
}

BENCHMARK_RELATIVE(all_map_elems_with_put_partial_deser) {
  patch6.applyToSerializedObject<type::StandardProtocol::Compact>(*serialized6);
}

BENCHMARK(few_map_elems_with_patch_after) {
  patch7.applyToSerializedObjectWithoutExtractingMask<
      type::StandardProtocol::Compact>(*serialized7);
}

BENCHMARK_RELATIVE(few_map_elems_with_patch_after_partial_deser) {
  patch7.applyToSerializedObject<type::StandardProtocol::Compact>(*serialized7);
}

BENCHMARK(all_map_elems_with_patch_after) {
  patch8.applyToSerializedObjectWithoutExtractingMask<
      type::StandardProtocol::Compact>(*serialized8);
}

BENCHMARK_RELATIVE(all_map_elems_with_patch_after_partial_deser) {
  patch8.applyToSerializedObject<type::StandardProtocol::Compact>(*serialized8);
}

} // namespace apache::thrift::protocol

int main(int argc, char** argv) {
  const folly::Init init(&argc, &argv);
  apache::thrift::protocol::init(10000);
  runBenchmarks();
  return 0;
}
