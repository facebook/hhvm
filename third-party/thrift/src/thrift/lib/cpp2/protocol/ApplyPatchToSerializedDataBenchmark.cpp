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
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/protocol/Object.h>
#include <thrift/lib/cpp2/protocol/Patch.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/lib/thrift/gen-cpp2/patch_types.h>

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

Object getPatchObj1And4(bool clear) {
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
  return patchObj;
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

Object getPatchObj2() {
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
  return patchObj;
}

// Testcase 3 uses an object with many fields and patch contain all the fields.
Object getObject3(int n) {
  Object obj;
  while (n--) {
    obj[static_cast<FieldId>(n)].emplace_i32(n);
  }
  return obj;
}

Object getPatchObj3(int n) {
  Value intPatch;
  intPatch.ensure_object()[FieldId{static_cast<int16_t>(op::PatchOp::Add)}] =
      asValueStruct<type::i32_t>(1);
  Value objPatch;
  while (n--) {
    objPatch.ensure_object()[static_cast<FieldId>(n)] = intPatch;
  }
  Object patchObj;
  patchObj[FieldId{static_cast<int16_t>(op::PatchOp::PatchAfter)}] = objPatch;
  return patchObj;
}

std::unique_ptr<folly::IOBuf> serialized1, serialized2, serialized3,
    serialized4;
Object patchObj1, patchObj2, patchObj3, patchObj4;

void init(int n) {
  serialized1 =
      serializeObject<apache::thrift::CompactProtocolWriter>(getObject1And4(n));
  patchObj1 = getPatchObj1And4(false);
  serialized2 =
      serializeObject<apache::thrift::CompactProtocolWriter>(getObject2(n));
  patchObj2 = getPatchObj2();
  serialized3 =
      serializeObject<apache::thrift::CompactProtocolWriter>(getObject3(n));
  patchObj3 = getPatchObj3(n);
  serialized4 =
      serializeObject<apache::thrift::CompactProtocolWriter>(getObject1And4(n));
  patchObj4 = getPatchObj1And4(true);
}

void runOriginalApproach(
    std::unique_ptr<folly::IOBuf>& serialized, const Object& patchObj) {
  Value value;
  value.emplace_object(parseObject<CompactProtocolReader>(*serialized));
  protocol::applyPatch(patchObj, value);
  protocol::serializeObject<CompactProtocolWriter>(value.as_object());
}

BENCHMARK(patch_few_small_fields_without_partial_deser) {
  runOriginalApproach(serialized1, patchObj1);
}

BENCHMARK(patch_few_small_fields_with_partial_deser) {
  applyPatchToSerializedData<type::StandardProtocol::Compact>(
      patchObj1, *serialized1);
}

BENCHMARK(patch_large_fields_without_partial_deser) {
  runOriginalApproach(serialized2, patchObj2);
}

BENCHMARK(patch_large_fields_with_partial_deser) {
  applyPatchToSerializedData<type::StandardProtocol::Compact>(
      patchObj2, *serialized2);
}

BENCHMARK(patch_all_small_fields_without_partial_deser) {
  runOriginalApproach(serialized3, patchObj3);
}

BENCHMARK(patch_all_small_fields_with_partial_deser) {
  applyPatchToSerializedData<type::StandardProtocol::Compact>(
      patchObj3, *serialized3);
}

BENCHMARK(patch_clear_large_fields_without_partial_deser) {
  runOriginalApproach(serialized4, patchObj4);
}

BENCHMARK(patch_clear_large_fields_with_partial_deser) {
  applyPatchToSerializedData<type::StandardProtocol::Compact>(
      patchObj4, *serialized4);
}
} // namespace apache::thrift::protocol

int main(int argc, char** argv) {
  folly::init(&argc, &argv);
  apache::thrift::protocol::init(10000);
  runBenchmarks();
  return 0;
}
