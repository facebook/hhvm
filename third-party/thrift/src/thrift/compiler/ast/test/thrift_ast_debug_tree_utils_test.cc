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

#include <memory>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "thrift/common/tree_printer.h"
#include "thrift/compiler/ast/t_program.h"
#include "thrift/compiler/ast/t_program_bundle.h"
#include "thrift/compiler/ast/thrift_ast_debug_tree_utils.h"
#include "thrift/compiler/parse/parse_ast.h"
#include "thrift/compiler/source_location.h"

namespace apache::thrift::compiler {

namespace {

/**
 * Normalizes memory addresses in the given input, to allow matching for
 * testing.
 *
 * Scans the input string and replaces all memory address patterns (e.g.,
 * `0x519000022b80`) with normalized addresses of the form `0xNORMALIZED_N`,
 * where N is a sequential index. Identical addresses in the input will map to
 * the same normalized address. The null address (0x0) is left unchanged.
 *
 * @param input The string containing memory addresses to normalize
 * @return A new string with all memory addresses normalized
 *
 * Example:
 *   Input:  "Object 0xabc123 references 0xdef456 and 0xabc123"
 *   Output: "Object 0xNORMALIZED_1 references 0xNORMALIZED_2 and
 * 0xNORMALIZED_1"
 */
std::string normalizeMemoryAddresses(const std::string& input) {
  std::string result;
  result.reserve(input.size());

  std::unordered_map<std::string, int> addressMap;
  int nextIndex = 1;

  std::size_t pos = 0;
  while (pos < input.size()) {
    // Look for the pattern "0x" followed by hex digits
    if (pos + 1 < input.size() && input[pos] == '0' && input[pos + 1] == 'x') {
      // Found the start of a memory address
      std::size_t addressStart = pos;
      std::size_t addressEnd = pos + 2; // Start after "0x"

      // Collect hex digits
      while (addressEnd < input.size() &&
             std::isxdigit(static_cast<unsigned char>(input[addressEnd]))) {
        ++addressEnd;
      }

      // Extract the full address (including @0x prefix)
      std::string address =
          input.substr(addressStart, addressEnd - addressStart);

      // Special handing for nullptr, which is consistent across runs.
      if (address == "0x0") {
        result += "0x0";
        pos = addressEnd;
        continue;
      }

      // Look up or assign an index for this address
      auto it = addressMap.find(address);
      int index;
      if (it == addressMap.end()) {
        index = nextIndex++;
        addressMap[address] = index;
      } else {
        index = it->second;
      }

      // Append the normalized address
      result += "0xNORMALIZED_";
      result += std::to_string(index);

      pos = addressEnd;
    } else {
      // Not a memory address, copy the character as-is
      result += input[pos];
      ++pos;
    }
  }

  return result;
}

std::string toNormalizedDebugTreeString(const t_program_bundle& programBundle) {
  return normalizeMemoryAddresses(
      apache::thrift::tree_printer::to_string(
          ThriftAstDebugTreeUtils::createTreeForProgramBundle(programBundle)));
}

} // namespace

TEST(AstTreePrinterTest, toNormalizedDebugTreeString) {
  const t_program_bundle kProgramBundle = []() {
    auto root_program = std::make_unique<t_program>(
        /*path=*/"my/test/path/virtual_test_file.thrift",
        /*full_path=*/"my/test/full_path/virtual_test_file.thrift");
    root_program->set_namespace("py3", "foo.bar");

    return t_program_bundle(std::move(root_program));
  }();

  EXPECT_EQ(
      toNormalizedDebugTreeString(kProgramBundle),
      normalizeMemoryAddresses(R"([t_program_bundle]
╰─ programs (size: 1)
   ╰─ programs[0] (root_program) [t_program] @0x51900001ae80
      ├─ (base) [t_named] @0x51900001ae80
      │  ├─ (base) [t_node] @0x51900001ae80
      │  │  ├─ src_range [source_range]
      │  │  │  ├─ begin (offset): 0
      │  │  │  ╰─ end (offset): 0
      │  │  ╰─ unstructured_annotations (size: 0)
      │  ├─ name: virtual_test_file
      │  ├─ scoped_name: virtual_test_file
      │  ├─ uri: 
      │  ├─ explicit_uri? false
      │  ├─ generated? false
      │  ├─ structured_annotations (size: 0)
      │  ├─ has_doc? false
      │  ├─ doc_range [source_range]
      │  │  ├─ begin (offset): 0
      │  │  ╰─ end (offset): 0
      │  ╰─ name_range: <absent>
      ├─ path: my/test/path/virtual_test_file.thrift
      ├─ full_path: my/test/full_path/virtual_test_file.thrift
      ├─ include_prefix: 
      ├─ package [t_package] @0x51900001af88
      │  ├─ name: 
      │  ├─ is_explicit? false
      │  ╰─ empty? true
      ├─ global_scope [t_global_scope] @0x512000000c50
      │  ├─ placeholder_typedefs (size: 0)
      │  ├─ resolution_mismatches (size: 0)
      │  ╰─ program_scopes (size: 0)
      ├─ program_scope [program_scope] @0x51900001b220
      ├─ includes (size: 0)
      ├─ included_programs (size: 0)
      ├─ includes_for_codegen (size: 0)
      ├─ namespaces (size: 1)
      │  ╰─ py3: foo.bar
      ├─ language_includes (size: 0)
      ├─ typedefs (size: 0)
      ├─ enums (size: 0)
      ├─ consts (size: 0)
      ├─ structs_and_unions (size: 0)
      ├─ exceptions (size: 0)
      ├─ services (size: 0)
      ├─ interactions (size: 0)
      ├─ definitions (size: 0)
      ├─ structured_definitions (size: 0)
      ╰─ type_instantiations (size: 0)
)"));
}

TEST(AstTreePrinterTest, ParseAndDebugPrint) {
  source_manager sourceManager(nullptr /* backend */);
  sourceManager.add_virtual_file(
      "my/virtual/test/path/ParseAndDebugPrint.thrift", R"(
include "thrift/annotation/thrift.thrift"

typedef i32 int

@thrift.Experimental
struct Foo {}

exception TestException {}

service MyService {
  void func1();

  i32 func2(Foo foo) throws (TestException ex);
}
)");
  diagnostics_engine diagnosticsEngine =
      diagnostics_engine::ignore_all(sourceManager);

  std::unique_ptr<t_program_bundle> programBundle = parse_ast(
      sourceManager,
      diagnosticsEngine,
      "my/virtual/test/path/ParseAndDebugPrint.thrift",
      parsing_params{});

  EXPECT_EQ(
      toNormalizedDebugTreeString(*programBundle),
      R"([t_program_bundle]
╰─ programs (size: 3)
   ├─ programs[0] (root_program) [t_program] @0xNORMALIZED_1
   │  ├─ (base) [t_named] @0xNORMALIZED_1
   │  │  ├─ (base) [t_node] @0xNORMALIZED_1
   │  │  │  ├─ src_range [source_range]
   │  │  │  │  ├─ begin (offset): 0
   │  │  │  │  ╰─ end (offset): 0
   │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  ├─ name: ParseAndDebugPrint
   │  │  ├─ scoped_name: ParseAndDebugPrint
   │  │  ├─ uri: 
   │  │  ├─ explicit_uri? false
   │  │  ├─ generated? false
   │  │  ├─ structured_annotations (size: 0)
   │  │  ├─ has_doc? false
   │  │  ├─ doc_range [source_range]
   │  │  │  ├─ begin (offset): 0
   │  │  │  ╰─ end (offset): 0
   │  │  ╰─ name_range: <absent>
   │  ├─ path: my/virtual/test/path/ParseAndDebugPrint.thrift
   │  ├─ full_path: my/virtual/test/path/ParseAndDebugPrint.thrift
   │  ├─ include_prefix: 
   │  ├─ package [t_package] @0xNORMALIZED_2
   │  │  ├─ name: 
   │  │  ├─ is_explicit? false
   │  │  ╰─ empty? true
   │  ├─ global_scope [t_global_scope] @0xNORMALIZED_3
   │  │  ├─ placeholder_typedefs (size: 1)
   │  │  │  ╰─ placeholder_typedefs[0] [t_placeholder_typedef] @0xNORMALIZED_4
   │  │  │     ╰─ (base) [t_typedef] @0xNORMALIZED_4
   │  │  │        ├─ (base) [t_type] @0xNORMALIZED_4
   │  │  │        │  ├─ (base) [t_named] @0xNORMALIZED_4
   │  │  │        │  │  ├─ (base) [t_node] @0xNORMALIZED_4
   │  │  │        │  │  │  ├─ src_range [source_range]
   │  │  │        │  │  │  │  ├─ begin (offset): 7257
   │  │  │        │  │  │  │  ╰─ end (offset): 7268
   │  │  │        │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │        │  │  ├─ name: RpcPriority
   │  │  │        │  │  ├─ scoped_name: thrift.RpcPriority
   │  │  │        │  │  ├─ uri: 
   │  │  │        │  │  ├─ explicit_uri? false
   │  │  │        │  │  ├─ generated? false
   │  │  │        │  │  ├─ structured_annotations (size: 0)
   │  │  │        │  │  ├─ has_doc? false
   │  │  │        │  │  ├─ doc_range [source_range]
   │  │  │        │  │  │  ├─ begin (offset): 0
   │  │  │        │  │  │  ╰─ end (offset): 0
   │  │  │        │  │  ╰─ name_range: <absent>
   │  │  │        │  ├─ full_name: thrift.RpcPriority
   │  │  │        │  ╰─ true_type: [t_type*] 0xNORMALIZED_5
   │  │  │        │     ╰─ full_name: thrift.RpcPriority
   │  │  │        ├─ type [t_type_ref]
   │  │  │        │  ├─ empty? false
   │  │  │        │  ├─ resolved? true
   │  │  │        │  ├─ src_range [source_range]
   │  │  │        │  │  ├─ begin (offset): 0
   │  │  │        │  │  ╰─ end (offset): 0
   │  │  │        │  ├─ type: [t_type*] 0xNORMALIZED_5
   │  │  │        │  │  ╰─ full_name: thrift.RpcPriority
   │  │  │        │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │        ╰─ typedef_kind: 2
   │  │  ├─ resolution_mismatches (size: 0)
   │  │  ╰─ program_scopes (size: 2)
   │  │     ├─ program_scope["scope"] (size: 1)
   │  │     │  ╰─ 0: [program_scope*] 0xNORMALIZED_6
   │  │     ╰─ program_scope["thrift"] (size: 1)
   │  │        ╰─ 0: [program_scope*] 0xNORMALIZED_7
   │  ├─ program_scope [program_scope] @0xNORMALIZED_8
   │  ├─ includes (size: 1)
   │  │  ╰─ includes[0] [t_include] @0xNORMALIZED_9
   │  │     ├─ (base) [t_node] @0xNORMALIZED_9
   │  │     │  ├─ src_range [source_range]
   │  │     │  │  ├─ begin (offset): 1
   │  │     │  │  ╰─ end (offset): 42
   │  │     │  ╰─ unstructured_annotations (size: 0)
   │  │     ├─ raw_path: thrift/annotation/thrift.thrift
   │  │     ├─ alias: <absent>
   │  │     ╰─ str_range [source_range]
   │  │        ├─ begin (offset): 9
   │  │        ╰─ end (offset): 42
   │  ├─ included_programs (size: 1)
   │  │  ╰─ included_program[0]:  [t_program*] 0xNORMALIZED_10
   │  │     ╰─ name: thrift
   │  ├─ includes_for_codegen (size: 0)
   │  ├─ language_includes (size: 0)
   │  ├─ typedefs (size: 1)
   │  │  ╰─ typedefs[0] [t_typedef] @0xNORMALIZED_11
   │  │     ├─ (base) [t_type] @0xNORMALIZED_11
   │  │     │  ├─ (base) [t_named] @0xNORMALIZED_11
   │  │     │  │  ├─ (base) [t_node] @0xNORMALIZED_11
   │  │     │  │  │  ├─ src_range [source_range]
   │  │     │  │  │  │  ├─ begin (offset): 44
   │  │     │  │  │  │  ╰─ end (offset): 59
   │  │     │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │     │  │  ├─ name: int
   │  │     │  │  ├─ scoped_name: ParseAndDebugPrint.int
   │  │     │  │  ├─ uri: 
   │  │     │  │  ├─ explicit_uri? true
   │  │     │  │  ├─ generated? false
   │  │     │  │  ├─ structured_annotations (size: 0)
   │  │     │  │  ├─ has_doc? false
   │  │     │  │  ├─ doc_range [source_range]
   │  │     │  │  │  ├─ begin (offset): 0
   │  │     │  │  │  ╰─ end (offset): 0
   │  │     │  │  ╰─ name_range [source_range]
   │  │     │  │     ├─ begin (offset): 56
   │  │     │  │     ╰─ end (offset): 59
   │  │     │  ├─ full_name: ParseAndDebugPrint.int
   │  │     │  ╰─ true_type: [t_type*] 0xNORMALIZED_12
   │  │     │     ╰─ full_name: i32
   │  │     ├─ type [t_type_ref]
   │  │     │  ├─ empty? false
   │  │     │  ├─ resolved? true
   │  │     │  ├─ src_range [source_range]
   │  │     │  │  ├─ begin (offset): 52
   │  │     │  │  ╰─ end (offset): 55
   │  │     │  ├─ type: [t_type*] 0xNORMALIZED_12
   │  │     │  │  ╰─ full_name: i32
   │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │     ╰─ typedef_kind: 0
   │  ├─ enums (size: 0)
   │  ├─ consts (size: 0)
   │  ├─ structs_and_unions (size: 1)
   │  │  ╰─ structs_and_unions[0] [t_structured] @0xNORMALIZED_13
   │  │     ├─ (base) [t_type] @0xNORMALIZED_13
   │  │     │  ├─ (base) [t_named] @0xNORMALIZED_13
   │  │     │  │  ├─ (base) [t_node] @0xNORMALIZED_13
   │  │     │  │  │  ├─ src_range [source_range]
   │  │     │  │  │  │  ├─ begin (offset): 61
   │  │     │  │  │  │  ╰─ end (offset): 95
   │  │     │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │     │  │  ├─ name: Foo
   │  │     │  │  ├─ scoped_name: ParseAndDebugPrint.Foo
   │  │     │  │  ├─ uri: 
   │  │     │  │  ├─ explicit_uri? false
   │  │     │  │  ├─ generated? false
   │  │     │  │  ├─ structured_annotations (size: 1)
   │  │     │  │  │  ╰─ structured_annotations[0] [t_const]
   │  │     │  │  │     ├─ (base) [t_named] @0xNORMALIZED_14
   │  │     │  │  │     │  ├─ (base) [t_node] @0xNORMALIZED_14
   │  │     │  │  │     │  │  ├─ src_range [source_range]
   │  │     │  │  │     │  │  │  ├─ begin (offset): 61
   │  │     │  │  │     │  │  │  ╰─ end (offset): 81
   │  │     │  │  │     │  │  ╰─ unstructured_annotations (size: 0)
   │  │     │  │  │     │  ├─ name: 
   │  │     │  │  │     │  ├─ scoped_name: ParseAndDebugPrint.
   │  │     │  │  │     │  ├─ uri: 
   │  │     │  │  │     │  ├─ explicit_uri? false
   │  │     │  │  │     │  ├─ generated? false
   │  │     │  │  │     │  ├─ structured_annotations (size: 0)
   │  │     │  │  │     │  ├─ has_doc? false
   │  │     │  │  │     │  ├─ doc_range [source_range]
   │  │     │  │  │     │  │  ├─ begin (offset): 0
   │  │     │  │  │     │  │  ╰─ end (offset): 0
   │  │     │  │  │     │  ╰─ name_range: <absent>
   │  │     │  │  │     ├─ type_ref [t_type_ref]
   │  │     │  │  │     │  ├─ empty? false
   │  │     │  │  │     │  ├─ resolved? true
   │  │     │  │  │     │  ├─ src_range [source_range]
   │  │     │  │  │     │  │  ├─ begin (offset): 61
   │  │     │  │  │     │  │  ╰─ end (offset): 81
   │  │     │  │  │     │  ├─ type: [t_type*] 0xNORMALIZED_15
   │  │     │  │  │     │  │  ╰─ full_name: thrift.Experimental
   │  │     │  │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │     │  │  │     ╰─ value [t_const_value*] 0xNORMALIZED_16
   │  │     │  │  │        ├─ src_range: <absent>
   │  │     │  │  │        ├─ is_empty? true
   │  │     │  │  │        ╰─ kind: map({})
   │  │     │  │  ├─ has_doc? false
   │  │     │  │  ├─ doc_range [source_range]
   │  │     │  │  │  ├─ begin (offset): 0
   │  │     │  │  │  ╰─ end (offset): 0
   │  │     │  │  ╰─ name_range [source_range]
   │  │     │  │     ├─ begin (offset): 89
   │  │     │  │     ╰─ end (offset): 92
   │  │     │  ├─ full_name: ParseAndDebugPrint.Foo
   │  │     │  ╰─ true_type: [t_type*] 0xNORMALIZED_13
   │  │     │     ╰─ full_name: ParseAndDebugPrint.Foo
   │  │     ╰─ fields (size: 0)
   │  ├─ exceptions (size: 1)
   │  │  ╰─ exceptions[0] [t_exception] @0xNORMALIZED_17
   │  │     ├─ (base) [t_structured] @0xNORMALIZED_17
   │  │     │  ├─ (base) [t_type] @0xNORMALIZED_17
   │  │     │  │  ├─ (base) [t_named] @0xNORMALIZED_17
   │  │     │  │  │  ├─ (base) [t_node] @0xNORMALIZED_17
   │  │     │  │  │  │  ├─ src_range [source_range]
   │  │     │  │  │  │  │  ├─ begin (offset): 97
   │  │     │  │  │  │  │  ╰─ end (offset): 123
   │  │     │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │     │  │  │  ├─ name: TestException
   │  │     │  │  │  ├─ scoped_name: ParseAndDebugPrint.TestException
   │  │     │  │  │  ├─ uri: 
   │  │     │  │  │  ├─ explicit_uri? false
   │  │     │  │  │  ├─ generated? false
   │  │     │  │  │  ├─ structured_annotations (size: 0)
   │  │     │  │  │  ├─ has_doc? false
   │  │     │  │  │  ├─ doc_range [source_range]
   │  │     │  │  │  │  ├─ begin (offset): 0
   │  │     │  │  │  │  ╰─ end (offset): 0
   │  │     │  │  │  ╰─ name_range [source_range]
   │  │     │  │  │     ├─ begin (offset): 107
   │  │     │  │  │     ╰─ end (offset): 120
   │  │     │  │  ├─ full_name: ParseAndDebugPrint.TestException
   │  │     │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_17
   │  │     │  │     ╰─ full_name: ParseAndDebugPrint.TestException
   │  │     │  ╰─ fields (size: 0)
   │  │     ├─ kind: 0
   │  │     ├─ blame: 0
   │  │     ├─ safety: 0
   │  │     ╰─ message_field: [t_field*] 0x0
   │  ├─ services (size: 1)
   │  │  ╰─ services[0] [t_service] @0xNORMALIZED_18
   │  │     ├─ (base) [t_interface] 0xNORMALIZED_18
   │  │     │  ├─ (base) [t_type] @0xNORMALIZED_18
   │  │     │  │  ├─ (base) [t_named] @0xNORMALIZED_18
   │  │     │  │  │  ├─ (base) [t_node] @0xNORMALIZED_18
   │  │     │  │  │  │  ├─ src_range [source_range]
   │  │     │  │  │  │  │  ├─ begin (offset): 125
   │  │     │  │  │  │  │  ╰─ end (offset): 211
   │  │     │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │     │  │  │  ├─ name: MyService
   │  │     │  │  │  ├─ scoped_name: ParseAndDebugPrint.MyService
   │  │     │  │  │  ├─ uri: 
   │  │     │  │  │  ├─ explicit_uri? false
   │  │     │  │  │  ├─ generated? false
   │  │     │  │  │  ├─ structured_annotations (size: 0)
   │  │     │  │  │  ├─ has_doc? false
   │  │     │  │  │  ├─ doc_range [source_range]
   │  │     │  │  │  │  ├─ begin (offset): 0
   │  │     │  │  │  │  ╰─ end (offset): 0
   │  │     │  │  │  ╰─ name_range [source_range]
   │  │     │  │  │     ├─ begin (offset): 133
   │  │     │  │  │     ╰─ end (offset): 142
   │  │     │  │  ├─ full_name: ParseAndDebugPrint.MyService
   │  │     │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_18
   │  │     │  │     ╰─ full_name: ParseAndDebugPrint.MyService
   │  │     │  ╰─ functions (size: 2)
   │  │     │     ├─ functions[0] [t_function] @0xNORMALIZED_19
   │  │     │     │  ├─ (base) [t_named] @0xNORMALIZED_19
   │  │     │     │  │  ├─ (base) [t_node] @0xNORMALIZED_19
   │  │     │     │  │  │  ├─ src_range [source_range]
   │  │     │     │  │  │  │  ├─ begin (offset): 147
   │  │     │     │  │  │  │  ╰─ end (offset): 160
   │  │     │     │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │     │     │  │  ├─ name: func1
   │  │     │     │  │  ├─ scoped_name: ParseAndDebugPrint.func1
   │  │     │     │  │  ├─ uri: 
   │  │     │     │  │  ├─ explicit_uri? false
   │  │     │     │  │  ├─ generated? false
   │  │     │     │  │  ├─ structured_annotations (size: 0)
   │  │     │     │  │  ├─ has_doc? false
   │  │     │     │  │  ├─ doc_range [source_range]
   │  │     │     │  │  │  ├─ begin (offset): 0
   │  │     │     │  │  │  ╰─ end (offset): 0
   │  │     │     │  │  ╰─ name_range [source_range]
   │  │     │     │  │     ├─ begin (offset): 152
   │  │     │     │  │     ╰─ end (offset): 157
   │  │     │     │  ├─ return_type [t_type_ref]
   │  │     │     │  │  ├─ empty? false
   │  │     │     │  │  ├─ resolved? true
   │  │     │     │  │  ├─ src_range [source_range]
   │  │     │     │  │  │  ├─ begin (offset): 147
   │  │     │     │  │  │  ╰─ end (offset): 151
   │  │     │     │  │  ├─ type: [t_type*] 0xNORMALIZED_20
   │  │     │     │  │  │  ╰─ full_name: void
   │  │     │     │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │     │     │  ├─ params [t_paramlist] @0xNORMALIZED_21
   │  │     │     │  │  ╰─ (base) [t_structured] @0xNORMALIZED_21
   │  │     │     │  │     ├─ (base) [t_type] @0xNORMALIZED_21
   │  │     │     │  │     │  ├─ (base) [t_named] @0xNORMALIZED_21
   │  │     │     │  │     │  │  ├─ (base) [t_node] @0xNORMALIZED_21
   │  │     │     │  │     │  │  │  ├─ src_range [source_range]
   │  │     │     │  │     │  │  │  │  ├─ begin (offset): 0
   │  │     │     │  │     │  │  │  │  ╰─ end (offset): 0
   │  │     │     │  │     │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │     │     │  │     │  │  ├─ name: func1_args
   │  │     │     │  │     │  │  ├─ scoped_name: ParseAndDebugPrint.func1_args
   │  │     │     │  │     │  │  ├─ uri: 
   │  │     │     │  │     │  │  ├─ explicit_uri? false
   │  │     │     │  │     │  │  ├─ generated? false
   │  │     │     │  │     │  │  ├─ structured_annotations (size: 0)
   │  │     │     │  │     │  │  ├─ has_doc? false
   │  │     │     │  │     │  │  ├─ doc_range [source_range]
   │  │     │     │  │     │  │  │  ├─ begin (offset): 0
   │  │     │     │  │     │  │  │  ╰─ end (offset): 0
   │  │     │     │  │     │  │  ╰─ name_range: <absent>
   │  │     │     │  │     │  ├─ full_name: ParseAndDebugPrint.func1_args
   │  │     │     │  │     │  ╰─ true_type: [t_type*] 0xNORMALIZED_21
   │  │     │     │  │     │     ╰─ full_name: ParseAndDebugPrint.func1_args
   │  │     │     │  │     ╰─ fields (size: 0)
   │  │     │     │  ├─ exceptions: N/A
   │  │     │     │  ├─ qualifier: 0
   │  │     │     │  ├─ sink_or_stream: N/A
   │  │     │     │  ├─ is_interaction_constructor? false
   │  │     │     │  ╰─ interaction [t_type_ref]
   │  │     │     │     ├─ empty? true
   │  │     │     │     ├─ resolved? false
   │  │     │     │     ├─ src_range [source_range]
   │  │     │     │     │  ├─ begin (offset): 0
   │  │     │     │     │  ╰─ end (offset): 0
   │  │     │     │     ├─ type: [t_type*] 0x0
   │  │     │     │     ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │     │     ╰─ functions[1] [t_function] @0xNORMALIZED_22
   │  │     │        ├─ (base) [t_named] @0xNORMALIZED_22
   │  │     │        │  ├─ (base) [t_node] @0xNORMALIZED_22
   │  │     │        │  │  ├─ src_range [source_range]
   │  │     │        │  │  │  ├─ begin (offset): 164
   │  │     │        │  │  │  ╰─ end (offset): 209
   │  │     │        │  │  ╰─ unstructured_annotations (size: 0)
   │  │     │        │  ├─ name: func2
   │  │     │        │  ├─ scoped_name: ParseAndDebugPrint.func2
   │  │     │        │  ├─ uri: 
   │  │     │        │  ├─ explicit_uri? false
   │  │     │        │  ├─ generated? false
   │  │     │        │  ├─ structured_annotations (size: 0)
   │  │     │        │  ├─ has_doc? false
   │  │     │        │  ├─ doc_range [source_range]
   │  │     │        │  │  ├─ begin (offset): 0
   │  │     │        │  │  ╰─ end (offset): 0
   │  │     │        │  ╰─ name_range [source_range]
   │  │     │        │     ├─ begin (offset): 168
   │  │     │        │     ╰─ end (offset): 173
   │  │     │        ├─ return_type [t_type_ref]
   │  │     │        │  ├─ empty? false
   │  │     │        │  ├─ resolved? true
   │  │     │        │  ├─ src_range [source_range]
   │  │     │        │  │  ├─ begin (offset): 164
   │  │     │        │  │  ╰─ end (offset): 167
   │  │     │        │  ├─ type: [t_type*] 0xNORMALIZED_12
   │  │     │        │  │  ╰─ full_name: i32
   │  │     │        │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │     │        ├─ params [t_paramlist] @0xNORMALIZED_23
   │  │     │        │  ╰─ (base) [t_structured] @0xNORMALIZED_23
   │  │     │        │     ├─ (base) [t_type] @0xNORMALIZED_23
   │  │     │        │     │  ├─ (base) [t_named] @0xNORMALIZED_23
   │  │     │        │     │  │  ├─ (base) [t_node] @0xNORMALIZED_23
   │  │     │        │     │  │  │  ├─ src_range [source_range]
   │  │     │        │     │  │  │  │  ├─ begin (offset): 0
   │  │     │        │     │  │  │  │  ╰─ end (offset): 0
   │  │     │        │     │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │     │        │     │  │  ├─ name: func2_args
   │  │     │        │     │  │  ├─ scoped_name: ParseAndDebugPrint.func2_args
   │  │     │        │     │  │  ├─ uri: 
   │  │     │        │     │  │  ├─ explicit_uri? false
   │  │     │        │     │  │  ├─ generated? false
   │  │     │        │     │  │  ├─ structured_annotations (size: 0)
   │  │     │        │     │  │  ├─ has_doc? false
   │  │     │        │     │  │  ├─ doc_range [source_range]
   │  │     │        │     │  │  │  ├─ begin (offset): 0
   │  │     │        │     │  │  │  ╰─ end (offset): 0
   │  │     │        │     │  │  ╰─ name_range: <absent>
   │  │     │        │     │  ├─ full_name: ParseAndDebugPrint.func2_args
   │  │     │        │     │  ╰─ true_type: [t_type*] 0xNORMALIZED_23
   │  │     │        │     │     ╰─ full_name: ParseAndDebugPrint.func2_args
   │  │     │        │     ╰─ fields (size: 1)
   │  │     │        │        ╰─ fields[0] [t_field] @0xNORMALIZED_24
   │  │     │        │           ├─ (base) [t_named] @0xNORMALIZED_24
   │  │     │        │           │  ├─ (base) [t_node] @0xNORMALIZED_24
   │  │     │        │           │  │  ├─ src_range [source_range]
   │  │     │        │           │  │  │  ├─ begin (offset): 174
   │  │     │        │           │  │  │  ╰─ end (offset): 181
   │  │     │        │           │  │  ╰─ unstructured_annotations (size: 0)
   │  │     │        │           │  ├─ name: foo
   │  │     │        │           │  ├─ scoped_name: foo
   │  │     │        │           │  ├─ uri: 
   │  │     │        │           │  ├─ explicit_uri? false
   │  │     │        │           │  ├─ generated? false
   │  │     │        │           │  ├─ structured_annotations (size: 0)
   │  │     │        │           │  ├─ has_doc? false
   │  │     │        │           │  ├─ doc_range [source_range]
   │  │     │        │           │  │  ├─ begin (offset): 0
   │  │     │        │           │  │  ╰─ end (offset): 0
   │  │     │        │           │  ╰─ name_range [source_range]
   │  │     │        │           │     ├─ begin (offset): 178
   │  │     │        │           │     ╰─ end (offset): 181
   │  │     │        │           ├─ qualifier: 0
   │  │     │        │           ├─ id: -1
   │  │     │        │           ├─ explicit_id: <absent>
   │  │     │        │           ├─ type [t_type_ref]
   │  │     │        │           │  ├─ empty? false
   │  │     │        │           │  ├─ resolved? true
   │  │     │        │           │  ├─ src_range [source_range]
   │  │     │        │           │  │  ├─ begin (offset): 174
   │  │     │        │           │  │  ╰─ end (offset): 177
   │  │     │        │           │  ├─ type: [t_type*] 0xNORMALIZED_13
   │  │     │        │           │  │  ╰─ full_name: ParseAndDebugPrint.Foo
   │  │     │        │           │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │     │        │           ├─ is_injected? false
   │  │     │        │           ╰─ default_value [t_const_value*] 0x0
   │  │     │        ├─ exceptions [t_throws] @0xNORMALIZED_25
   │  │     │        │  ╰─ (base) [t_structured] @0xNORMALIZED_25
   │  │     │        │     ├─ (base) [t_type] @0xNORMALIZED_25
   │  │     │        │     │  ├─ (base) [t_named] @0xNORMALIZED_25
   │  │     │        │     │  │  ├─ (base) [t_node] @0xNORMALIZED_25
   │  │     │        │     │  │  │  ├─ src_range [source_range]
   │  │     │        │     │  │  │  │  ├─ begin (offset): 0
   │  │     │        │     │  │  │  │  ╰─ end (offset): 0
   │  │     │        │     │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │     │        │     │  │  ├─ name: 
   │  │     │        │     │  │  ├─ scoped_name: 
   │  │     │        │     │  │  ├─ uri: 
   │  │     │        │     │  │  ├─ explicit_uri? false
   │  │     │        │     │  │  ├─ generated? false
   │  │     │        │     │  │  ├─ structured_annotations (size: 0)
   │  │     │        │     │  │  ├─ has_doc? false
   │  │     │        │     │  │  ├─ doc_range [source_range]
   │  │     │        │     │  │  │  ├─ begin (offset): 0
   │  │     │        │     │  │  │  ╰─ end (offset): 0
   │  │     │        │     │  │  ╰─ name_range: <absent>
   │  │     │        │     │  ├─ full_name: 
   │  │     │        │     │  ╰─ true_type: [t_type*] 0xNORMALIZED_25
   │  │     │        │     │     ╰─ full_name: 
   │  │     │        │     ╰─ fields (size: 1)
   │  │     │        │        ╰─ fields[0] [t_field] @0xNORMALIZED_26
   │  │     │        │           ├─ (base) [t_named] @0xNORMALIZED_26
   │  │     │        │           │  ├─ (base) [t_node] @0xNORMALIZED_26
   │  │     │        │           │  │  ├─ src_range [source_range]
   │  │     │        │           │  │  │  ├─ begin (offset): 191
   │  │     │        │           │  │  │  ╰─ end (offset): 207
   │  │     │        │           │  │  ╰─ unstructured_annotations (size: 0)
   │  │     │        │           │  ├─ name: ex
   │  │     │        │           │  ├─ scoped_name: ex
   │  │     │        │           │  ├─ uri: 
   │  │     │        │           │  ├─ explicit_uri? false
   │  │     │        │           │  ├─ generated? false
   │  │     │        │           │  ├─ structured_annotations (size: 0)
   │  │     │        │           │  ├─ has_doc? false
   │  │     │        │           │  ├─ doc_range [source_range]
   │  │     │        │           │  │  ├─ begin (offset): 0
   │  │     │        │           │  │  ╰─ end (offset): 0
   │  │     │        │           │  ╰─ name_range [source_range]
   │  │     │        │           │     ├─ begin (offset): 205
   │  │     │        │           │     ╰─ end (offset): 207
   │  │     │        │           ├─ qualifier: 0
   │  │     │        │           ├─ id: -1
   │  │     │        │           ├─ explicit_id: <absent>
   │  │     │        │           ├─ type [t_type_ref]
   │  │     │        │           │  ├─ empty? false
   │  │     │        │           │  ├─ resolved? true
   │  │     │        │           │  ├─ src_range [source_range]
   │  │     │        │           │  │  ├─ begin (offset): 191
   │  │     │        │           │  │  ╰─ end (offset): 204
   │  │     │        │           │  ├─ type: [t_type*] 0xNORMALIZED_17
   │  │     │        │           │  │  ╰─ full_name: ParseAndDebugPrint.TestException
   │  │     │        │           │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │     │        │           ├─ is_injected? false
   │  │     │        │           ╰─ default_value [t_const_value*] 0x0
   │  │     │        ├─ qualifier: 0
   │  │     │        ├─ sink_or_stream: N/A
   │  │     │        ├─ is_interaction_constructor? false
   │  │     │        ╰─ interaction [t_type_ref]
   │  │     │           ├─ empty? true
   │  │     │           ├─ resolved? false
   │  │     │           ├─ src_range [source_range]
   │  │     │           │  ├─ begin (offset): 0
   │  │     │           │  ╰─ end (offset): 0
   │  │     │           ├─ type: [t_type*] 0x0
   │  │     │           ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │     ├─ extends: N/A
   │  │     ╰─ extends_range [source_range]
   │  │        ├─ begin (offset): 0
   │  │        ╰─ end (offset): 0
   │  ├─ interactions (size: 0)
   │  ├─ definitions (size: 4)
   │  │  ├─ definitions[0]: [t_named*] 0xNORMALIZED_11
   │  │  │  ╰─ scoped_name: ParseAndDebugPrint.int
   │  │  ├─ definitions[1]: [t_named*] 0xNORMALIZED_13
   │  │  │  ╰─ scoped_name: ParseAndDebugPrint.Foo
   │  │  ├─ definitions[2]: [t_named*] 0xNORMALIZED_17
   │  │  │  ╰─ scoped_name: ParseAndDebugPrint.TestException
   │  │  ╰─ definitions[3]: [t_named*] 0xNORMALIZED_18
   │  │     ╰─ scoped_name: ParseAndDebugPrint.MyService
   │  ├─ structured_definitions (size: 2)
   │  │  ├─ structured_definitions[0]: [t_structured*] 0xNORMALIZED_13
   │  │  │  ╰─ name: Foo
   │  │  ╰─ structured_definitions[1]: [t_structured*] 0xNORMALIZED_17
   │  │     ╰─ name: TestException
   │  ╰─ type_instantiations (size: 0)
   ├─ programs[1] [t_program] @0xNORMALIZED_10
   │  ├─ (base) [t_named] @0xNORMALIZED_10
   │  │  ├─ (base) [t_node] @0xNORMALIZED_10
   │  │  │  ├─ src_range [source_range]
   │  │  │  │  ├─ begin (offset): 662
   │  │  │  │  ╰─ end (offset): 702
   │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  ├─ name: thrift
   │  │  ├─ scoped_name: thrift
   │  │  ├─ uri: 
   │  │  ├─ explicit_uri? false
   │  │  ├─ generated? false
   │  │  ├─ structured_annotations (size: 0)
   │  │  ├─ has_doc? false
   │  │  ├─ doc_range [source_range]
   │  │  │  ├─ begin (offset): 0
   │  │  │  ╰─ end (offset): 0
   │  │  ╰─ name_range: <absent>
   │  ├─ path: thrift/annotation/thrift.thrift
   │  ├─ full_path: thrift/annotation/thrift.thrift
   │  ├─ include_prefix: thrift/annotation/
   │  ├─ package [t_package] @0xNORMALIZED_27
   │  │  ├─ name: facebook.com/thrift/annotation
   │  │  ├─ is_explicit? true
   │  │  ╰─ empty? false
   │  ├─ global_scope [t_global_scope] @0xNORMALIZED_3
   │  │  ├─ placeholder_typedefs (size: 1)
   │  │  │  ╰─ placeholder_typedefs[0] [t_placeholder_typedef] @0xNORMALIZED_4
   │  │  │     ╰─ (base) [t_typedef] @0xNORMALIZED_4
   │  │  │        ├─ (base) [t_type] @0xNORMALIZED_4
   │  │  │        │  ├─ (base) [t_named] @0xNORMALIZED_4
   │  │  │        │  │  ├─ (base) [t_node] @0xNORMALIZED_4
   │  │  │        │  │  │  ├─ src_range [source_range]
   │  │  │        │  │  │  │  ├─ begin (offset): 7257
   │  │  │        │  │  │  │  ╰─ end (offset): 7268
   │  │  │        │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │        │  │  ├─ name: RpcPriority
   │  │  │        │  │  ├─ scoped_name: thrift.RpcPriority
   │  │  │        │  │  ├─ uri: 
   │  │  │        │  │  ├─ explicit_uri? false
   │  │  │        │  │  ├─ generated? false
   │  │  │        │  │  ├─ structured_annotations (size: 0)
   │  │  │        │  │  ├─ has_doc? false
   │  │  │        │  │  ├─ doc_range [source_range]
   │  │  │        │  │  │  ├─ begin (offset): 0
   │  │  │        │  │  │  ╰─ end (offset): 0
   │  │  │        │  │  ╰─ name_range: <absent>
   │  │  │        │  ├─ full_name: thrift.RpcPriority
   │  │  │        │  ╰─ true_type: [t_type*] 0xNORMALIZED_5
   │  │  │        │     ╰─ full_name: thrift.RpcPriority
   │  │  │        ├─ type [t_type_ref]
   │  │  │        │  ├─ empty? false
   │  │  │        │  ├─ resolved? true
   │  │  │        │  ├─ src_range [source_range]
   │  │  │        │  │  ├─ begin (offset): 0
   │  │  │        │  │  ╰─ end (offset): 0
   │  │  │        │  ├─ type: [t_type*] 0xNORMALIZED_5
   │  │  │        │  │  ╰─ full_name: thrift.RpcPriority
   │  │  │        │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │        ╰─ typedef_kind: 2
   │  │  ├─ resolution_mismatches (size: 0)
   │  │  ╰─ program_scopes (size: 2)
   │  │     ├─ program_scope["scope"] (size: 1)
   │  │     │  ╰─ 0: [program_scope*] 0xNORMALIZED_6
   │  │     ╰─ program_scope["thrift"] (size: 1)
   │  │        ╰─ 0: [program_scope*] 0xNORMALIZED_7
   │  ├─ program_scope [program_scope] @0xNORMALIZED_7
   │  ├─ includes (size: 1)
   │  │  ╰─ includes[0] [t_include] @0xNORMALIZED_28
   │  │     ├─ (base) [t_node] @0xNORMALIZED_28
   │  │     │  ├─ src_range [source_range]
   │  │     │  │  ├─ begin (offset): 620
   │  │     │  │  ╰─ end (offset): 660
   │  │     │  ╰─ unstructured_annotations (size: 0)
   │  │     ├─ raw_path: thrift/annotation/scope.thrift
   │  │     ├─ alias: <absent>
   │  │     ╰─ str_range [source_range]
   │  │        ├─ begin (offset): 628
   │  │        ╰─ end (offset): 660
   │  ├─ included_programs (size: 1)
   │  │  ╰─ included_program[0]:  [t_program*] 0xNORMALIZED_29
   │  │     ╰─ name: scope
   │  ├─ includes_for_codegen (size: 0)
   │  ├─ namespaces (size: 7)
   │  │  ├─ android: com.facebook.thrift.annotation_deprecated
   │  │  ├─ go: thrift.annotation.thrift
   │  │  ├─ hs: Facebook.Thrift.Annotation.Thrift
   │  │  ├─ java: com.facebook.thrift.annotation_deprecated
   │  │  ├─ js: thrift.annotation.thrift
   │  │  ├─ py: thrift.annotation.thrift
   │  │  ╰─ py.asyncio: facebook_thrift_asyncio.annotation.thrift
   │  ├─ language_includes (size: 0)
   │  ├─ typedefs (size: 0)
   │  ├─ enums (size: 1)
   │  │  ╰─ enums[0] [t_enum] @0xNORMALIZED_5
   │  │     ├─ (base) [t_type] @0xNORMALIZED_5
   │  │     │  ├─ (base) [t_named] @0xNORMALIZED_5
   │  │     │  │  ├─ (base) [t_node] @0xNORMALIZED_5
   │  │     │  │  │  ├─ src_range [source_range]
   │  │     │  │  │  │  ├─ begin (offset): 7278
   │  │     │  │  │  │  ╰─ end (offset): 7382
   │  │     │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │     │  │  ├─ name: RpcPriority
   │  │     │  │  ├─ scoped_name: thrift.RpcPriority
   │  │     │  │  ├─ uri: facebook.com/thrift/annotation/RpcPriority
   │  │     │  │  ├─ explicit_uri? false
   │  │     │  │  ├─ generated? false
   │  │     │  │  ├─ structured_annotations (size: 0)
   │  │     │  │  ├─ has_doc? false
   │  │     │  │  ├─ doc_range [source_range]
   │  │     │  │  │  ├─ begin (offset): 0
   │  │     │  │  │  ╰─ end (offset): 0
   │  │     │  │  ╰─ name_range [source_range]
   │  │     │  │     ├─ begin (offset): 7283
   │  │     │  │     ╰─ end (offset): 7294
   │  │     │  ├─ full_name: thrift.RpcPriority
   │  │     │  ╰─ true_type: [t_type*] 0xNORMALIZED_5
   │  │     │     ╰─ full_name: thrift.RpcPriority
   │  │     ├─ values (size: 5)
   │  │     │  ├─ values[0] [t_enum_value] @0xNORMALIZED_30
   │  │     │  │  ├─ (base) [t_named] @0xNORMALIZED_30
   │  │     │  │  │  ├─ (base) [t_node] @0xNORMALIZED_30
   │  │     │  │  │  │  ├─ src_range [source_range]
   │  │     │  │  │  │  │  ├─ begin (offset): 7299
   │  │     │  │  │  │  │  ╰─ end (offset): 7318
   │  │     │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │     │  │  │  ├─ name: HIGH_IMPORTANT
   │  │     │  │  │  ├─ scoped_name: HIGH_IMPORTANT
   │  │     │  │  │  ├─ uri: 
   │  │     │  │  │  ├─ explicit_uri? false
   │  │     │  │  │  ├─ generated? false
   │  │     │  │  │  ├─ structured_annotations (size: 0)
   │  │     │  │  │  ├─ has_doc? false
   │  │     │  │  │  ├─ doc_range [source_range]
   │  │     │  │  │  │  ├─ begin (offset): 0
   │  │     │  │  │  │  ╰─ end (offset): 0
   │  │     │  │  │  ╰─ name_range [source_range]
   │  │     │  │  │     ├─ begin (offset): 7299
   │  │     │  │  │     ╰─ end (offset): 7313
   │  │     │  │  ├─ value: 0
   │  │     │  │  ╰─ has_value: true
   │  │     │  ├─ values[1] [t_enum_value] @0xNORMALIZED_31
   │  │     │  │  ├─ (base) [t_named] @0xNORMALIZED_31
   │  │     │  │  │  ├─ (base) [t_node] @0xNORMALIZED_31
   │  │     │  │  │  │  ├─ src_range [source_range]
   │  │     │  │  │  │  │  ├─ begin (offset): 7321
   │  │     │  │  │  │  │  ╰─ end (offset): 7330
   │  │     │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │     │  │  │  ├─ name: HIGH
   │  │     │  │  │  ├─ scoped_name: HIGH
   │  │     │  │  │  ├─ uri: 
   │  │     │  │  │  ├─ explicit_uri? false
   │  │     │  │  │  ├─ generated? false
   │  │     │  │  │  ├─ structured_annotations (size: 0)
   │  │     │  │  │  ├─ has_doc? false
   │  │     │  │  │  ├─ doc_range [source_range]
   │  │     │  │  │  │  ├─ begin (offset): 0
   │  │     │  │  │  │  ╰─ end (offset): 0
   │  │     │  │  │  ╰─ name_range [source_range]
   │  │     │  │  │     ├─ begin (offset): 7321
   │  │     │  │  │     ╰─ end (offset): 7325
   │  │     │  │  ├─ value: 1
   │  │     │  │  ╰─ has_value: true
   │  │     │  ├─ values[2] [t_enum_value] @0xNORMALIZED_32
   │  │     │  │  ├─ (base) [t_named] @0xNORMALIZED_32
   │  │     │  │  │  ├─ (base) [t_node] @0xNORMALIZED_32
   │  │     │  │  │  │  ├─ src_range [source_range]
   │  │     │  │  │  │  │  ├─ begin (offset): 7333
   │  │     │  │  │  │  │  ╰─ end (offset): 7347
   │  │     │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │     │  │  │  ├─ name: IMPORTANT
   │  │     │  │  │  ├─ scoped_name: IMPORTANT
   │  │     │  │  │  ├─ uri: 
   │  │     │  │  │  ├─ explicit_uri? false
   │  │     │  │  │  ├─ generated? false
   │  │     │  │  │  ├─ structured_annotations (size: 0)
   │  │     │  │  │  ├─ has_doc? false
   │  │     │  │  │  ├─ doc_range [source_range]
   │  │     │  │  │  │  ├─ begin (offset): 0
   │  │     │  │  │  │  ╰─ end (offset): 0
   │  │     │  │  │  ╰─ name_range [source_range]
   │  │     │  │  │     ├─ begin (offset): 7333
   │  │     │  │  │     ╰─ end (offset): 7342
   │  │     │  │  ├─ value: 2
   │  │     │  │  ╰─ has_value: true
   │  │     │  ├─ values[3] [t_enum_value] @0xNORMALIZED_33
   │  │     │  │  ├─ (base) [t_named] @0xNORMALIZED_33
   │  │     │  │  │  ├─ (base) [t_node] @0xNORMALIZED_33
   │  │     │  │  │  │  ├─ src_range [source_range]
   │  │     │  │  │  │  │  ├─ begin (offset): 7350
   │  │     │  │  │  │  │  ╰─ end (offset): 7361
   │  │     │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │     │  │  │  ├─ name: NORMAL
   │  │     │  │  │  ├─ scoped_name: NORMAL
   │  │     │  │  │  ├─ uri: 
   │  │     │  │  │  ├─ explicit_uri? false
   │  │     │  │  │  ├─ generated? false
   │  │     │  │  │  ├─ structured_annotations (size: 0)
   │  │     │  │  │  ├─ has_doc? false
   │  │     │  │  │  ├─ doc_range [source_range]
   │  │     │  │  │  │  ├─ begin (offset): 0
   │  │     │  │  │  │  ╰─ end (offset): 0
   │  │     │  │  │  ╰─ name_range [source_range]
   │  │     │  │  │     ├─ begin (offset): 7350
   │  │     │  │  │     ╰─ end (offset): 7356
   │  │     │  │  ├─ value: 3
   │  │     │  │  ╰─ has_value: true
   │  │     │  ╰─ values[4] [t_enum_value] @0xNORMALIZED_34
   │  │     │     ├─ (base) [t_named] @0xNORMALIZED_34
   │  │     │     │  ├─ (base) [t_node] @0xNORMALIZED_34
   │  │     │     │  │  ├─ src_range [source_range]
   │  │     │     │  │  │  ├─ begin (offset): 7364
   │  │     │     │  │  │  ╰─ end (offset): 7380
   │  │     │     │  │  ╰─ unstructured_annotations (size: 0)
   │  │     │     │  ├─ name: BEST_EFFORT
   │  │     │     │  ├─ scoped_name: BEST_EFFORT
   │  │     │     │  ├─ uri: 
   │  │     │     │  ├─ explicit_uri? false
   │  │     │     │  ├─ generated? false
   │  │     │     │  ├─ structured_annotations (size: 0)
   │  │     │     │  ├─ has_doc? false
   │  │     │     │  ├─ doc_range [source_range]
   │  │     │     │  │  ├─ begin (offset): 0
   │  │     │     │  │  ╰─ end (offset): 0
   │  │     │     │  ╰─ name_range [source_range]
   │  │     │     │     ├─ begin (offset): 7364
   │  │     │     │     ╰─ end (offset): 7375
   │  │     │     ├─ value: 4
   │  │     │     ╰─ has_value: true
   │  │     ├─ unused: 113
   │  │     ╰─ consts (size: 5)
   │  │        ├─ consts[0] [t_const]
   │  │        │  ├─ (base) [t_named] @0xNORMALIZED_35
   │  │        │  │  ├─ (base) [t_node] @0xNORMALIZED_35
   │  │        │  │  │  ├─ src_range [source_range]
   │  │        │  │  │  │  ├─ begin (offset): 0
   │  │        │  │  │  │  ╰─ end (offset): 0
   │  │        │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │        │  │  ├─ name: HIGH_IMPORTANT
   │  │        │  │  ├─ scoped_name: thrift.HIGH_IMPORTANT
   │  │        │  │  ├─ uri: 
   │  │        │  │  ├─ explicit_uri? false
   │  │        │  │  ├─ generated? false
   │  │        │  │  ├─ structured_annotations (size: 0)
   │  │        │  │  ├─ has_doc? false
   │  │        │  │  ├─ doc_range [source_range]
   │  │        │  │  │  ├─ begin (offset): 0
   │  │        │  │  │  ╰─ end (offset): 0
   │  │        │  │  ╰─ name_range: <absent>
   │  │        │  ├─ type_ref [t_type_ref]
   │  │        │  │  ├─ empty? false
   │  │        │  │  ├─ resolved? true
   │  │        │  │  ├─ src_range [source_range]
   │  │        │  │  │  ├─ begin (offset): 0
   │  │        │  │  │  ╰─ end (offset): 0
   │  │        │  │  ├─ type: [t_type*] 0xNORMALIZED_12
   │  │        │  │  │  ╰─ full_name: i32
   │  │        │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │        │  ╰─ value [t_const_value*] 0xNORMALIZED_36
   │  │        │     ├─ src_range: <absent>
   │  │        │     ├─ is_empty? false
   │  │        │     ╰─ kind: integer
   │  │        ├─ consts[1] [t_const]
   │  │        │  ├─ (base) [t_named] @0xNORMALIZED_37
   │  │        │  │  ├─ (base) [t_node] @0xNORMALIZED_37
   │  │        │  │  │  ├─ src_range [source_range]
   │  │        │  │  │  │  ├─ begin (offset): 0
   │  │        │  │  │  │  ╰─ end (offset): 0
   │  │        │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │        │  │  ├─ name: HIGH
   │  │        │  │  ├─ scoped_name: thrift.HIGH
   │  │        │  │  ├─ uri: 
   │  │        │  │  ├─ explicit_uri? false
   │  │        │  │  ├─ generated? false
   │  │        │  │  ├─ structured_annotations (size: 0)
   │  │        │  │  ├─ has_doc? false
   │  │        │  │  ├─ doc_range [source_range]
   │  │        │  │  │  ├─ begin (offset): 0
   │  │        │  │  │  ╰─ end (offset): 0
   │  │        │  │  ╰─ name_range: <absent>
   │  │        │  ├─ type_ref [t_type_ref]
   │  │        │  │  ├─ empty? false
   │  │        │  │  ├─ resolved? true
   │  │        │  │  ├─ src_range [source_range]
   │  │        │  │  │  ├─ begin (offset): 0
   │  │        │  │  │  ╰─ end (offset): 0
   │  │        │  │  ├─ type: [t_type*] 0xNORMALIZED_12
   │  │        │  │  │  ╰─ full_name: i32
   │  │        │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │        │  ╰─ value [t_const_value*] 0xNORMALIZED_38
   │  │        │     ├─ src_range: <absent>
   │  │        │     ├─ is_empty? false
   │  │        │     ╰─ kind: integer
   │  │        ├─ consts[2] [t_const]
   │  │        │  ├─ (base) [t_named] @0xNORMALIZED_39
   │  │        │  │  ├─ (base) [t_node] @0xNORMALIZED_39
   │  │        │  │  │  ├─ src_range [source_range]
   │  │        │  │  │  │  ├─ begin (offset): 0
   │  │        │  │  │  │  ╰─ end (offset): 0
   │  │        │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │        │  │  ├─ name: IMPORTANT
   │  │        │  │  ├─ scoped_name: thrift.IMPORTANT
   │  │        │  │  ├─ uri: 
   │  │        │  │  ├─ explicit_uri? false
   │  │        │  │  ├─ generated? false
   │  │        │  │  ├─ structured_annotations (size: 0)
   │  │        │  │  ├─ has_doc? false
   │  │        │  │  ├─ doc_range [source_range]
   │  │        │  │  │  ├─ begin (offset): 0
   │  │        │  │  │  ╰─ end (offset): 0
   │  │        │  │  ╰─ name_range: <absent>
   │  │        │  ├─ type_ref [t_type_ref]
   │  │        │  │  ├─ empty? false
   │  │        │  │  ├─ resolved? true
   │  │        │  │  ├─ src_range [source_range]
   │  │        │  │  │  ├─ begin (offset): 0
   │  │        │  │  │  ╰─ end (offset): 0
   │  │        │  │  ├─ type: [t_type*] 0xNORMALIZED_12
   │  │        │  │  │  ╰─ full_name: i32
   │  │        │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │        │  ╰─ value [t_const_value*] 0xNORMALIZED_40
   │  │        │     ├─ src_range: <absent>
   │  │        │     ├─ is_empty? false
   │  │        │     ╰─ kind: integer
   │  │        ├─ consts[3] [t_const]
   │  │        │  ├─ (base) [t_named] @0xNORMALIZED_41
   │  │        │  │  ├─ (base) [t_node] @0xNORMALIZED_41
   │  │        │  │  │  ├─ src_range [source_range]
   │  │        │  │  │  │  ├─ begin (offset): 0
   │  │        │  │  │  │  ╰─ end (offset): 0
   │  │        │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │        │  │  ├─ name: NORMAL
   │  │        │  │  ├─ scoped_name: thrift.NORMAL
   │  │        │  │  ├─ uri: 
   │  │        │  │  ├─ explicit_uri? false
   │  │        │  │  ├─ generated? false
   │  │        │  │  ├─ structured_annotations (size: 0)
   │  │        │  │  ├─ has_doc? false
   │  │        │  │  ├─ doc_range [source_range]
   │  │        │  │  │  ├─ begin (offset): 0
   │  │        │  │  │  ╰─ end (offset): 0
   │  │        │  │  ╰─ name_range: <absent>
   │  │        │  ├─ type_ref [t_type_ref]
   │  │        │  │  ├─ empty? false
   │  │        │  │  ├─ resolved? true
   │  │        │  │  ├─ src_range [source_range]
   │  │        │  │  │  ├─ begin (offset): 0
   │  │        │  │  │  ╰─ end (offset): 0
   │  │        │  │  ├─ type: [t_type*] 0xNORMALIZED_12
   │  │        │  │  │  ╰─ full_name: i32
   │  │        │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │        │  ╰─ value [t_const_value*] 0xNORMALIZED_42
   │  │        │     ├─ src_range: <absent>
   │  │        │     ├─ is_empty? false
   │  │        │     ╰─ kind: integer
   │  │        ╰─ consts[4] [t_const]
   │  │           ├─ (base) [t_named] @0xNORMALIZED_43
   │  │           │  ├─ (base) [t_node] @0xNORMALIZED_43
   │  │           │  │  ├─ src_range [source_range]
   │  │           │  │  │  ├─ begin (offset): 0
   │  │           │  │  │  ╰─ end (offset): 0
   │  │           │  │  ╰─ unstructured_annotations (size: 0)
   │  │           │  ├─ name: BEST_EFFORT
   │  │           │  ├─ scoped_name: thrift.BEST_EFFORT
   │  │           │  ├─ uri: 
   │  │           │  ├─ explicit_uri? false
   │  │           │  ├─ generated? false
   │  │           │  ├─ structured_annotations (size: 0)
   │  │           │  ├─ has_doc? false
   │  │           │  ├─ doc_range [source_range]
   │  │           │  │  ├─ begin (offset): 0
   │  │           │  │  ╰─ end (offset): 0
   │  │           │  ╰─ name_range: <absent>
   │  │           ├─ type_ref [t_type_ref]
   │  │           │  ├─ empty? false
   │  │           │  ├─ resolved? true
   │  │           │  ├─ src_range [source_range]
   │  │           │  │  ├─ begin (offset): 0
   │  │           │  │  ╰─ end (offset): 0
   │  │           │  ├─ type: [t_type*] 0xNORMALIZED_12
   │  │           │  │  ╰─ full_name: i32
   │  │           │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │           ╰─ value [t_const_value*] 0xNORMALIZED_44
   │  │              ├─ src_range: <absent>
   │  │              ├─ is_empty? false
   │  │              ╰─ kind: integer
   │  ├─ consts (size: 0)
   │  ├─ structs_and_unions (size: 22)
   │  │  ├─ structs_and_unions[0] [t_structured] @0xNORMALIZED_15
   │  │  │  ├─ (base) [t_type] @0xNORMALIZED_15
   │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_15
   │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_15
   │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  ├─ begin (offset): 1348
   │  │  │  │  │  │  │  ╰─ end (offset): 1403
   │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  ├─ name: Experimental
   │  │  │  │  │  ├─ scoped_name: thrift.Experimental
   │  │  │  │  │  ├─ uri: facebook.com/thrift/annotation/Experimental
   │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  ├─ structured_annotations (size: 2)
   │  │  │  │  │  │  ├─ structured_annotations[0] [t_const]
   │  │  │  │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_45
   │  │  │  │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_45
   │  │  │  │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  │  │  │  ├─ begin (offset): 1348
   │  │  │  │  │  │  │  │  │  │  ╰─ end (offset): 1362
   │  │  │  │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │  │  │  ├─ name: 
   │  │  │  │  │  │  │  │  ├─ scoped_name: thrift.
   │  │  │  │  │  │  │  │  ├─ uri: 
   │  │  │  │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  │  │  │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │  │  │  ├─ has_doc? false
   │  │  │  │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  │  │  │  ├─ begin (offset): 0
   │  │  │  │  │  │  │  │  │  ╰─ end (offset): 0
   │  │  │  │  │  │  │  │  ╰─ name_range: <absent>
   │  │  │  │  │  │  │  ├─ type_ref [t_type_ref]
   │  │  │  │  │  │  │  │  ├─ empty? false
   │  │  │  │  │  │  │  │  ├─ resolved? true
   │  │  │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  │  │  ├─ begin (offset): 1348
   │  │  │  │  │  │  │  │  │  ╰─ end (offset): 1362
   │  │  │  │  │  │  │  │  ├─ type: [t_type*] 0xNORMALIZED_46
   │  │  │  │  │  │  │  │  │  ╰─ full_name: scope.Program
   │  │  │  │  │  │  │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │  │  ╰─ value [t_const_value*] 0xNORMALIZED_47
   │  │  │  │  │  │  │     ├─ src_range: <absent>
   │  │  │  │  │  │  │     ├─ is_empty? true
   │  │  │  │  │  │  │     ╰─ kind: map({})
   │  │  │  │  │  │  ╰─ structured_annotations[1] [t_const]
   │  │  │  │  │  │     ├─ (base) [t_named] @0xNORMALIZED_48
   │  │  │  │  │  │     │  ├─ (base) [t_node] @0xNORMALIZED_48
   │  │  │  │  │  │     │  │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  │  ├─ begin (offset): 1363
   │  │  │  │  │  │     │  │  │  ╰─ end (offset): 1380
   │  │  │  │  │  │     │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ name: 
   │  │  │  │  │  │     │  ├─ scoped_name: thrift.
   │  │  │  │  │  │     │  ├─ uri: 
   │  │  │  │  │  │     │  ├─ explicit_uri? false
   │  │  │  │  │  │     │  ├─ generated? false
   │  │  │  │  │  │     │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ has_doc? false
   │  │  │  │  │  │     │  ├─ doc_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 0
   │  │  │  │  │  │     │  │  ╰─ end (offset): 0
   │  │  │  │  │  │     │  ╰─ name_range: <absent>
   │  │  │  │  │  │     ├─ type_ref [t_type_ref]
   │  │  │  │  │  │     │  ├─ empty? false
   │  │  │  │  │  │     │  ├─ resolved? true
   │  │  │  │  │  │     │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 1363
   │  │  │  │  │  │     │  │  ╰─ end (offset): 1380
   │  │  │  │  │  │     │  ├─ type: [t_type*] 0xNORMALIZED_49
   │  │  │  │  │  │     │  │  ╰─ full_name: scope.Definition
   │  │  │  │  │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │     ╰─ value [t_const_value*] 0xNORMALIZED_50
   │  │  │  │  │  │        ├─ src_range: <absent>
   │  │  │  │  │  │        ├─ is_empty? true
   │  │  │  │  │  │        ╰─ kind: map({})
   │  │  │  │  │  ├─ has_doc? true
   │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  ├─ begin (offset): 1056
   │  │  │  │  │  │  ╰─ end (offset): 1347
   │  │  │  │  │  ╰─ name_range [source_range]
   │  │  │  │  │     ├─ begin (offset): 1388
   │  │  │  │  │     ╰─ end (offset): 1400
   │  │  │  │  ├─ full_name: thrift.Experimental
   │  │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_15
   │  │  │  │     ╰─ full_name: thrift.Experimental
   │  │  │  ╰─ fields (size: 0)
   │  │  ├─ structs_and_unions[1] [t_structured] @0xNORMALIZED_51
   │  │  │  ├─ (base) [t_type] @0xNORMALIZED_51
   │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_51
   │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_51
   │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  ├─ begin (offset): 2005
   │  │  │  │  │  │  │  ╰─ end (offset): 2461
   │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  ├─ name: ReserveIds
   │  │  │  │  │  ├─ scoped_name: thrift.ReserveIds
   │  │  │  │  │  ├─ uri: facebook.com/thrift/annotation/ReserveIds
   │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  ├─ structured_annotations (size: 2)
   │  │  │  │  │  │  ├─ structured_annotations[0] [t_const]
   │  │  │  │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_52
   │  │  │  │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_52
   │  │  │  │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  │  │  │  ├─ begin (offset): 2005
   │  │  │  │  │  │  │  │  │  │  ╰─ end (offset): 2022
   │  │  │  │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │  │  │  ├─ name: 
   │  │  │  │  │  │  │  │  ├─ scoped_name: thrift.
   │  │  │  │  │  │  │  │  ├─ uri: 
   │  │  │  │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  │  │  │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │  │  │  ├─ has_doc? false
   │  │  │  │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  │  │  │  ├─ begin (offset): 0
   │  │  │  │  │  │  │  │  │  ╰─ end (offset): 0
   │  │  │  │  │  │  │  │  ╰─ name_range: <absent>
   │  │  │  │  │  │  │  ├─ type_ref [t_type_ref]
   │  │  │  │  │  │  │  │  ├─ empty? false
   │  │  │  │  │  │  │  │  ├─ resolved? true
   │  │  │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  │  │  ├─ begin (offset): 2005
   │  │  │  │  │  │  │  │  │  ╰─ end (offset): 2022
   │  │  │  │  │  │  │  │  ├─ type: [t_type*] 0xNORMALIZED_53
   │  │  │  │  │  │  │  │  │  ╰─ full_name: scope.Structured
   │  │  │  │  │  │  │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │  │  ╰─ value [t_const_value*] 0xNORMALIZED_54
   │  │  │  │  │  │  │     ├─ src_range: <absent>
   │  │  │  │  │  │  │     ├─ is_empty? true
   │  │  │  │  │  │  │     ╰─ kind: map({})
   │  │  │  │  │  │  ╰─ structured_annotations[1] [t_const]
   │  │  │  │  │  │     ├─ (base) [t_named] @0xNORMALIZED_55
   │  │  │  │  │  │     │  ├─ (base) [t_node] @0xNORMALIZED_55
   │  │  │  │  │  │     │  │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  │  ├─ begin (offset): 2023
   │  │  │  │  │  │     │  │  │  ╰─ end (offset): 2034
   │  │  │  │  │  │     │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ name: 
   │  │  │  │  │  │     │  ├─ scoped_name: thrift.
   │  │  │  │  │  │     │  ├─ uri: 
   │  │  │  │  │  │     │  ├─ explicit_uri? false
   │  │  │  │  │  │     │  ├─ generated? false
   │  │  │  │  │  │     │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ has_doc? false
   │  │  │  │  │  │     │  ├─ doc_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 0
   │  │  │  │  │  │     │  │  ╰─ end (offset): 0
   │  │  │  │  │  │     │  ╰─ name_range: <absent>
   │  │  │  │  │  │     ├─ type_ref [t_type_ref]
   │  │  │  │  │  │     │  ├─ empty? false
   │  │  │  │  │  │     │  ├─ resolved? true
   │  │  │  │  │  │     │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 2023
   │  │  │  │  │  │     │  │  ╰─ end (offset): 2034
   │  │  │  │  │  │     │  ├─ type: [t_type*] 0xNORMALIZED_56
   │  │  │  │  │  │     │  │  ╰─ full_name: scope.Enum
   │  │  │  │  │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │     ╰─ value [t_const_value*] 0xNORMALIZED_57
   │  │  │  │  │  │        ├─ src_range: <absent>
   │  │  │  │  │  │        ├─ is_empty? true
   │  │  │  │  │  │        ╰─ kind: map({})
   │  │  │  │  │  ├─ has_doc? true
   │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  ├─ begin (offset): 1405
   │  │  │  │  │  │  ╰─ end (offset): 2004
   │  │  │  │  │  ╰─ name_range [source_range]
   │  │  │  │  │     ├─ begin (offset): 2042
   │  │  │  │  │     ╰─ end (offset): 2052
   │  │  │  │  ├─ full_name: thrift.ReserveIds
   │  │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_51
   │  │  │  │     ╰─ full_name: thrift.ReserveIds
   │  │  │  ╰─ fields (size: 2)
   │  │  │     ├─ fields[0] [t_field] @0xNORMALIZED_58
   │  │  │     │  ├─ (base) [t_named] @0xNORMALIZED_58
   │  │  │     │  │  ├─ (base) [t_node] @0xNORMALIZED_58
   │  │  │     │  │  │  ├─ src_range [source_range]
   │  │  │     │  │  │  │  ├─ begin (offset): 2102
   │  │  │     │  │  │  │  ╰─ end (offset): 2119
   │  │  │     │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │     │  │  ├─ name: ids
   │  │  │     │  │  ├─ scoped_name: ids
   │  │  │     │  │  ├─ uri: 
   │  │  │     │  │  ├─ explicit_uri? false
   │  │  │     │  │  ├─ generated? false
   │  │  │     │  │  ├─ structured_annotations (size: 0)
   │  │  │     │  │  ├─ has_doc? true
   │  │  │     │  │  ├─ doc_range [source_range]
   │  │  │     │  │  │  ├─ begin (offset): 2057
   │  │  │     │  │  │  ╰─ end (offset): 2099
   │  │  │     │  │  ╰─ name_range [source_range]
   │  │  │     │  │     ├─ begin (offset): 2115
   │  │  │     │  │     ╰─ end (offset): 2118
   │  │  │     │  ├─ qualifier: 0
   │  │  │     │  ├─ id: 1
   │  │  │     │  ├─ explicit_id: 1
   │  │  │     │  ├─ type [t_type_ref]
   │  │  │     │  │  ├─ empty? false
   │  │  │     │  │  ├─ resolved? true
   │  │  │     │  │  ├─ src_range [source_range]
   │  │  │     │  │  │  ├─ begin (offset): 2105
   │  │  │     │  │  │  ╰─ end (offset): 2114
   │  │  │     │  │  ├─ type: [t_type*] 0xNORMALIZED_59
   │  │  │     │  │  │  ╰─ full_name: list<i32>
   │  │  │     │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │     │  ├─ is_injected? false
   │  │  │     │  ╰─ default_value [t_const_value*] 0x0
   │  │  │     ╰─ fields[1] [t_field] @0xNORMALIZED_60
   │  │  │        ├─ (base) [t_named] @0xNORMALIZED_60
   │  │  │        │  ├─ (base) [t_node] @0xNORMALIZED_60
   │  │  │        │  │  ├─ src_range [source_range]
   │  │  │        │  │  │  ├─ begin (offset): 2432
   │  │  │        │  │  │  ╰─ end (offset): 2459
   │  │  │        │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │        │  ├─ name: id_ranges
   │  │  │        │  ├─ scoped_name: id_ranges
   │  │  │        │  ├─ uri: 
   │  │  │        │  ├─ explicit_uri? false
   │  │  │        │  ├─ generated? false
   │  │  │        │  ├─ structured_annotations (size: 0)
   │  │  │        │  ├─ has_doc? true
   │  │  │        │  ├─ doc_range [source_range]
   │  │  │        │  │  ├─ begin (offset): 2123
   │  │  │        │  │  ╰─ end (offset): 2429
   │  │  │        │  ╰─ name_range [source_range]
   │  │  │        │     ├─ begin (offset): 2449
   │  │  │        │     ╰─ end (offset): 2458
   │  │  │        ├─ qualifier: 0
   │  │  │        ├─ id: 2
   │  │  │        ├─ explicit_id: 2
   │  │  │        ├─ type [t_type_ref]
   │  │  │        │  ├─ empty? false
   │  │  │        │  ├─ resolved? true
   │  │  │        │  ├─ src_range [source_range]
   │  │  │        │  │  ├─ begin (offset): 2435
   │  │  │        │  │  ╰─ end (offset): 2448
   │  │  │        │  ├─ type: [t_type*] 0xNORMALIZED_61
   │  │  │        │  │  ╰─ full_name: map<i32, i32>
   │  │  │        │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │        ├─ is_injected? false
   │  │  │        ╰─ default_value [t_const_value*] 0x0
   │  │  ├─ structs_and_unions[2] [t_structured] @0xNORMALIZED_62
   │  │  │  ├─ (base) [t_type] @0xNORMALIZED_62
   │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_62
   │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_62
   │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  ├─ begin (offset): 2653
   │  │  │  │  │  │  │  ╰─ end (offset): 2782
   │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  ├─ name: RequiresBackwardCompatibility
   │  │  │  │  │  ├─ scoped_name: thrift.RequiresBackwardCompatibility
   │  │  │  │  │  ├─ uri: facebook.com/thrift/annotation/RequiresBackwardCompatibility
   │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  ├─ structured_annotations (size: 2)
   │  │  │  │  │  │  ├─ structured_annotations[0] [t_const]
   │  │  │  │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_63
   │  │  │  │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_63
   │  │  │  │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  │  │  │  ├─ begin (offset): 2653
   │  │  │  │  │  │  │  │  │  │  ╰─ end (offset): 2670
   │  │  │  │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │  │  │  ├─ name: 
   │  │  │  │  │  │  │  │  ├─ scoped_name: thrift.
   │  │  │  │  │  │  │  │  ├─ uri: 
   │  │  │  │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  │  │  │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │  │  │  ├─ has_doc? false
   │  │  │  │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  │  │  │  ├─ begin (offset): 0
   │  │  │  │  │  │  │  │  │  ╰─ end (offset): 0
   │  │  │  │  │  │  │  │  ╰─ name_range: <absent>
   │  │  │  │  │  │  │  ├─ type_ref [t_type_ref]
   │  │  │  │  │  │  │  │  ├─ empty? false
   │  │  │  │  │  │  │  │  ├─ resolved? true
   │  │  │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  │  │  ├─ begin (offset): 2653
   │  │  │  │  │  │  │  │  │  ╰─ end (offset): 2670
   │  │  │  │  │  │  │  │  ├─ type: [t_type*] 0xNORMALIZED_53
   │  │  │  │  │  │  │  │  │  ╰─ full_name: scope.Structured
   │  │  │  │  │  │  │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │  │  ╰─ value [t_const_value*] 0xNORMALIZED_64
   │  │  │  │  │  │  │     ├─ src_range: <absent>
   │  │  │  │  │  │  │     ├─ is_empty? true
   │  │  │  │  │  │  │     ╰─ kind: map({})
   │  │  │  │  │  │  ╰─ structured_annotations[1] [t_const]
   │  │  │  │  │  │     ├─ (base) [t_named] @0xNORMALIZED_65
   │  │  │  │  │  │     │  ├─ (base) [t_node] @0xNORMALIZED_65
   │  │  │  │  │  │     │  │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  │  ├─ begin (offset): 2671
   │  │  │  │  │  │     │  │  │  ╰─ end (offset): 2684
   │  │  │  │  │  │     │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ name: 
   │  │  │  │  │  │     │  ├─ scoped_name: thrift.
   │  │  │  │  │  │     │  ├─ uri: 
   │  │  │  │  │  │     │  ├─ explicit_uri? false
   │  │  │  │  │  │     │  ├─ generated? false
   │  │  │  │  │  │     │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ has_doc? false
   │  │  │  │  │  │     │  ├─ doc_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 0
   │  │  │  │  │  │     │  │  ╰─ end (offset): 0
   │  │  │  │  │  │     │  ╰─ name_range: <absent>
   │  │  │  │  │  │     ├─ type_ref [t_type_ref]
   │  │  │  │  │  │     │  ├─ empty? false
   │  │  │  │  │  │     │  ├─ resolved? true
   │  │  │  │  │  │     │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 2671
   │  │  │  │  │  │     │  │  ╰─ end (offset): 2684
   │  │  │  │  │  │     │  ├─ type: [t_type*] 0xNORMALIZED_15
   │  │  │  │  │  │     │  │  ╰─ full_name: thrift.Experimental
   │  │  │  │  │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │     ╰─ value [t_const_value*] 0xNORMALIZED_66
   │  │  │  │  │  │        ├─ src_range: <absent>
   │  │  │  │  │  │        ├─ is_empty? true
   │  │  │  │  │  │        ╰─ kind: map({})
   │  │  │  │  │  ├─ has_doc? true
   │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  ├─ begin (offset): 2463
   │  │  │  │  │  │  ╰─ end (offset): 2592
   │  │  │  │  │  ╰─ name_range [source_range]
   │  │  │  │  │     ├─ begin (offset): 2719
   │  │  │  │  │     ╰─ end (offset): 2748
   │  │  │  │  ├─ full_name: thrift.RequiresBackwardCompatibility
   │  │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_62
   │  │  │  │     ╰─ full_name: thrift.RequiresBackwardCompatibility
   │  │  │  ╰─ fields (size: 1)
   │  │  │     ╰─ fields[0] [t_field] @0xNORMALIZED_67
   │  │  │        ├─ (base) [t_named] @0xNORMALIZED_67
   │  │  │        │  ├─ (base) [t_node] @0xNORMALIZED_67
   │  │  │        │  │  ├─ src_range [source_range]
   │  │  │        │  │  │  ├─ begin (offset): 2753
   │  │  │        │  │  │  ╰─ end (offset): 2780
   │  │  │        │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │        │  ├─ name: field_name
   │  │  │        │  ├─ scoped_name: field_name
   │  │  │        │  ├─ uri: 
   │  │  │        │  ├─ explicit_uri? false
   │  │  │        │  ├─ generated? false
   │  │  │        │  ├─ structured_annotations (size: 0)
   │  │  │        │  ├─ has_doc? false
   │  │  │        │  ├─ doc_range [source_range]
   │  │  │        │  │  ├─ begin (offset): 0
   │  │  │        │  │  ╰─ end (offset): 0
   │  │  │        │  ╰─ name_range [source_range]
   │  │  │        │     ├─ begin (offset): 2761
   │  │  │        │     ╰─ end (offset): 2771
   │  │  │        ├─ qualifier: 0
   │  │  │        ├─ id: 1
   │  │  │        ├─ explicit_id: 1
   │  │  │        ├─ type [t_type_ref]
   │  │  │        │  ├─ empty? false
   │  │  │        │  ├─ resolved? true
   │  │  │        │  ├─ src_range [source_range]
   │  │  │        │  │  ├─ begin (offset): 2756
   │  │  │        │  │  ╰─ end (offset): 2760
   │  │  │        │  ├─ type: [t_type*] 0xNORMALIZED_68
   │  │  │        │  │  ╰─ full_name: bool
   │  │  │        │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │        ├─ is_injected? false
   │  │  │        ╰─ default_value [t_const_value*] 0xNORMALIZED_69
   │  │  │           ├─ src_range [source_range]
   │  │  │           │  ├─ begin (offset): 2774
   │  │  │           │  ╰─ end (offset): 2779
   │  │  │           ├─ is_empty? false
   │  │  │           ╰─ kind: bool
   │  │  ├─ structs_and_unions[3] [t_structured] @0xNORMALIZED_70
   │  │  │  ├─ (base) [t_type] @0xNORMALIZED_70
   │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_70
   │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_70
   │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  ├─ begin (offset): 3563
   │  │  │  │  │  │  │  ╰─ end (offset): 3642
   │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  ├─ name: TerseWrite
   │  │  │  │  │  ├─ scoped_name: thrift.TerseWrite
   │  │  │  │  │  ├─ uri: facebook.com/thrift/annotation/TerseWrite
   │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  ├─ structured_annotations (size: 4)
   │  │  │  │  │  │  ├─ structured_annotations[0] [t_const]
   │  │  │  │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_71
   │  │  │  │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_71
   │  │  │  │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  │  │  │  ├─ begin (offset): 3563
   │  │  │  │  │  │  │  │  │  │  ╰─ end (offset): 3577
   │  │  │  │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │  │  │  ├─ name: 
   │  │  │  │  │  │  │  │  ├─ scoped_name: thrift.
   │  │  │  │  │  │  │  │  ├─ uri: 
   │  │  │  │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  │  │  │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │  │  │  ├─ has_doc? false
   │  │  │  │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  │  │  │  ├─ begin (offset): 0
   │  │  │  │  │  │  │  │  │  ╰─ end (offset): 0
   │  │  │  │  │  │  │  │  ╰─ name_range: <absent>
   │  │  │  │  │  │  │  ├─ type_ref [t_type_ref]
   │  │  │  │  │  │  │  │  ├─ empty? false
   │  │  │  │  │  │  │  │  ├─ resolved? true
   │  │  │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  │  │  ├─ begin (offset): 3563
   │  │  │  │  │  │  │  │  │  ╰─ end (offset): 3577
   │  │  │  │  │  │  │  │  ├─ type: [t_type*] 0xNORMALIZED_46
   │  │  │  │  │  │  │  │  │  ╰─ full_name: scope.Program
   │  │  │  │  │  │  │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │  │  ╰─ value [t_const_value*] 0xNORMALIZED_72
   │  │  │  │  │  │  │     ├─ src_range: <absent>
   │  │  │  │  │  │  │     ├─ is_empty? true
   │  │  │  │  │  │  │     ╰─ kind: map({})
   │  │  │  │  │  │  ├─ structured_annotations[1] [t_const]
   │  │  │  │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_73
   │  │  │  │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_73
   │  │  │  │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  │  │  │  ├─ begin (offset): 3578
   │  │  │  │  │  │  │  │  │  │  ╰─ end (offset): 3591
   │  │  │  │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │  │  │  ├─ name: 
   │  │  │  │  │  │  │  │  ├─ scoped_name: thrift.
   │  │  │  │  │  │  │  │  ├─ uri: 
   │  │  │  │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  │  │  │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │  │  │  ├─ has_doc? false
   │  │  │  │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  │  │  │  ├─ begin (offset): 0
   │  │  │  │  │  │  │  │  │  ╰─ end (offset): 0
   │  │  │  │  │  │  │  │  ╰─ name_range: <absent>
   │  │  │  │  │  │  │  ├─ type_ref [t_type_ref]
   │  │  │  │  │  │  │  │  ├─ empty? false
   │  │  │  │  │  │  │  │  ├─ resolved? true
   │  │  │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  │  │  ├─ begin (offset): 3578
   │  │  │  │  │  │  │  │  │  ╰─ end (offset): 3591
   │  │  │  │  │  │  │  │  ├─ type: [t_type*] 0xNORMALIZED_74
   │  │  │  │  │  │  │  │  │  ╰─ full_name: scope.Struct
   │  │  │  │  │  │  │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │  │  ╰─ value [t_const_value*] 0xNORMALIZED_75
   │  │  │  │  │  │  │     ├─ src_range: <absent>
   │  │  │  │  │  │  │     ├─ is_empty? true
   │  │  │  │  │  │  │     ╰─ kind: map({})
   │  │  │  │  │  │  ├─ structured_annotations[2] [t_const]
   │  │  │  │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_76
   │  │  │  │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_76
   │  │  │  │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  │  │  │  ├─ begin (offset): 3592
   │  │  │  │  │  │  │  │  │  │  ╰─ end (offset): 3608
   │  │  │  │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │  │  │  ├─ name: 
   │  │  │  │  │  │  │  │  ├─ scoped_name: thrift.
   │  │  │  │  │  │  │  │  ├─ uri: 
   │  │  │  │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  │  │  │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │  │  │  ├─ has_doc? false
   │  │  │  │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  │  │  │  ├─ begin (offset): 0
   │  │  │  │  │  │  │  │  │  ╰─ end (offset): 0
   │  │  │  │  │  │  │  │  ╰─ name_range: <absent>
   │  │  │  │  │  │  │  ├─ type_ref [t_type_ref]
   │  │  │  │  │  │  │  │  ├─ empty? false
   │  │  │  │  │  │  │  │  ├─ resolved? true
   │  │  │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  │  │  ├─ begin (offset): 3592
   │  │  │  │  │  │  │  │  │  ╰─ end (offset): 3608
   │  │  │  │  │  │  │  │  ├─ type: [t_type*] 0xNORMALIZED_77
   │  │  │  │  │  │  │  │  │  ╰─ full_name: scope.Exception
   │  │  │  │  │  │  │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │  │  ╰─ value [t_const_value*] 0xNORMALIZED_78
   │  │  │  │  │  │  │     ├─ src_range: <absent>
   │  │  │  │  │  │  │     ├─ is_empty? true
   │  │  │  │  │  │  │     ╰─ kind: map({})
   │  │  │  │  │  │  ╰─ structured_annotations[3] [t_const]
   │  │  │  │  │  │     ├─ (base) [t_named] @0xNORMALIZED_79
   │  │  │  │  │  │     │  ├─ (base) [t_node] @0xNORMALIZED_79
   │  │  │  │  │  │     │  │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  │  ├─ begin (offset): 3609
   │  │  │  │  │  │     │  │  │  ╰─ end (offset): 3621
   │  │  │  │  │  │     │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ name: 
   │  │  │  │  │  │     │  ├─ scoped_name: thrift.
   │  │  │  │  │  │     │  ├─ uri: 
   │  │  │  │  │  │     │  ├─ explicit_uri? false
   │  │  │  │  │  │     │  ├─ generated? false
   │  │  │  │  │  │     │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ has_doc? false
   │  │  │  │  │  │     │  ├─ doc_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 0
   │  │  │  │  │  │     │  │  ╰─ end (offset): 0
   │  │  │  │  │  │     │  ╰─ name_range: <absent>
   │  │  │  │  │  │     ├─ type_ref [t_type_ref]
   │  │  │  │  │  │     │  ├─ empty? false
   │  │  │  │  │  │     │  ├─ resolved? true
   │  │  │  │  │  │     │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 3609
   │  │  │  │  │  │     │  │  ╰─ end (offset): 3621
   │  │  │  │  │  │     │  ├─ type: [t_type*] 0xNORMALIZED_80
   │  │  │  │  │  │     │  │  ╰─ full_name: scope.Field
   │  │  │  │  │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │     ╰─ value [t_const_value*] 0xNORMALIZED_81
   │  │  │  │  │  │        ├─ src_range: <absent>
   │  │  │  │  │  │        ├─ is_empty? true
   │  │  │  │  │  │        ╰─ kind: map({})
   │  │  │  │  │  ├─ has_doc? true
   │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  ├─ begin (offset): 2826
   │  │  │  │  │  │  ╰─ end (offset): 3562
   │  │  │  │  │  ╰─ name_range [source_range]
   │  │  │  │  │     ├─ begin (offset): 3629
   │  │  │  │  │     ╰─ end (offset): 3639
   │  │  │  │  ├─ full_name: thrift.TerseWrite
   │  │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_70
   │  │  │  │     ╰─ full_name: thrift.TerseWrite
   │  │  │  ╰─ fields (size: 0)
   │  │  ├─ structs_and_unions[4] [t_structured] @0xNORMALIZED_82
   │  │  │  ├─ (base) [t_type] @0xNORMALIZED_82
   │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_82
   │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_82
   │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  ├─ begin (offset): 4548
   │  │  │  │  │  │  │  ╰─ end (offset): 4574
   │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  ├─ name: Box
   │  │  │  │  │  ├─ scoped_name: thrift.Box
   │  │  │  │  │  ├─ uri: facebook.com/thrift/annotation/Box
   │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  ├─ structured_annotations (size: 1)
   │  │  │  │  │  │  ╰─ structured_annotations[0] [t_const]
   │  │  │  │  │  │     ├─ (base) [t_named] @0xNORMALIZED_83
   │  │  │  │  │  │     │  ├─ (base) [t_node] @0xNORMALIZED_83
   │  │  │  │  │  │     │  │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  │  ├─ begin (offset): 4548
   │  │  │  │  │  │     │  │  │  ╰─ end (offset): 4560
   │  │  │  │  │  │     │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ name: 
   │  │  │  │  │  │     │  ├─ scoped_name: thrift.
   │  │  │  │  │  │     │  ├─ uri: 
   │  │  │  │  │  │     │  ├─ explicit_uri? false
   │  │  │  │  │  │     │  ├─ generated? false
   │  │  │  │  │  │     │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ has_doc? false
   │  │  │  │  │  │     │  ├─ doc_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 0
   │  │  │  │  │  │     │  │  ╰─ end (offset): 0
   │  │  │  │  │  │     │  ╰─ name_range: <absent>
   │  │  │  │  │  │     ├─ type_ref [t_type_ref]
   │  │  │  │  │  │     │  ├─ empty? false
   │  │  │  │  │  │     │  ├─ resolved? true
   │  │  │  │  │  │     │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 4548
   │  │  │  │  │  │     │  │  ╰─ end (offset): 4560
   │  │  │  │  │  │     │  ├─ type: [t_type*] 0xNORMALIZED_80
   │  │  │  │  │  │     │  │  ╰─ full_name: scope.Field
   │  │  │  │  │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │     ╰─ value [t_const_value*] 0xNORMALIZED_84
   │  │  │  │  │  │        ├─ src_range: <absent>
   │  │  │  │  │  │        ├─ is_empty? true
   │  │  │  │  │  │        ╰─ kind: map({})
   │  │  │  │  │  ├─ has_doc? true
   │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  ├─ begin (offset): 3644
   │  │  │  │  │  │  ╰─ end (offset): 4547
   │  │  │  │  │  ╰─ name_range [source_range]
   │  │  │  │  │     ├─ begin (offset): 4568
   │  │  │  │  │     ╰─ end (offset): 4571
   │  │  │  │  ├─ full_name: thrift.Box
   │  │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_82
   │  │  │  │     ╰─ full_name: thrift.Box
   │  │  │  ╰─ fields (size: 0)
   │  │  ├─ structs_and_unions[5] [t_structured] @0xNORMALIZED_85
   │  │  │  ├─ (base) [t_type] @0xNORMALIZED_85
   │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_85
   │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_85
   │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  ├─ begin (offset): 4733
   │  │  │  │  │  │  │  ╰─ end (offset): 4761
   │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  ├─ name: Mixin
   │  │  │  │  │  ├─ scoped_name: thrift.Mixin
   │  │  │  │  │  ├─ uri: facebook.com/thrift/annotation/Mixin
   │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  ├─ structured_annotations (size: 1)
   │  │  │  │  │  │  ╰─ structured_annotations[0] [t_const]
   │  │  │  │  │  │     ├─ (base) [t_named] @0xNORMALIZED_86
   │  │  │  │  │  │     │  ├─ (base) [t_node] @0xNORMALIZED_86
   │  │  │  │  │  │     │  │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  │  ├─ begin (offset): 4733
   │  │  │  │  │  │     │  │  │  ╰─ end (offset): 4745
   │  │  │  │  │  │     │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ name: 
   │  │  │  │  │  │     │  ├─ scoped_name: thrift.
   │  │  │  │  │  │     │  ├─ uri: 
   │  │  │  │  │  │     │  ├─ explicit_uri? false
   │  │  │  │  │  │     │  ├─ generated? false
   │  │  │  │  │  │     │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ has_doc? false
   │  │  │  │  │  │     │  ├─ doc_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 0
   │  │  │  │  │  │     │  │  ╰─ end (offset): 0
   │  │  │  │  │  │     │  ╰─ name_range: <absent>
   │  │  │  │  │  │     ├─ type_ref [t_type_ref]
   │  │  │  │  │  │     │  ├─ empty? false
   │  │  │  │  │  │     │  ├─ resolved? true
   │  │  │  │  │  │     │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 4733
   │  │  │  │  │  │     │  │  ╰─ end (offset): 4745
   │  │  │  │  │  │     │  ├─ type: [t_type*] 0xNORMALIZED_80
   │  │  │  │  │  │     │  │  ╰─ full_name: scope.Field
   │  │  │  │  │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │     ╰─ value [t_const_value*] 0xNORMALIZED_87
   │  │  │  │  │  │        ├─ src_range: <absent>
   │  │  │  │  │  │        ├─ is_empty? true
   │  │  │  │  │  │        ╰─ kind: map({})
   │  │  │  │  │  ├─ has_doc? true
   │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  ├─ begin (offset): 4576
   │  │  │  │  │  │  ╰─ end (offset): 4732
   │  │  │  │  │  ╰─ name_range [source_range]
   │  │  │  │  │     ├─ begin (offset): 4753
   │  │  │  │  │     ╰─ end (offset): 4758
   │  │  │  │  ├─ full_name: thrift.Mixin
   │  │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_85
   │  │  │  │     ╰─ full_name: thrift.Mixin
   │  │  │  ╰─ fields (size: 0)
   │  │  ├─ structs_and_unions[6] [t_structured] @0xNORMALIZED_88
   │  │  │  ├─ (base) [t_type] @0xNORMALIZED_88
   │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_88
   │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_88
   │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  ├─ begin (offset): 5121
   │  │  │  │  │  │  │  ╰─ end (offset): 5213
   │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  ├─ name: SerializeInFieldIdOrder
   │  │  │  │  │  ├─ scoped_name: thrift.SerializeInFieldIdOrder
   │  │  │  │  │  ├─ uri: facebook.com/thrift/annotation/SerializeInFieldIdOrder
   │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  ├─ structured_annotations (size: 2)
   │  │  │  │  │  │  ├─ structured_annotations[0] [t_const]
   │  │  │  │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_89
   │  │  │  │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_89
   │  │  │  │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  │  │  │  ├─ begin (offset): 5121
   │  │  │  │  │  │  │  │  │  │  ╰─ end (offset): 5134
   │  │  │  │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │  │  │  ├─ name: 
   │  │  │  │  │  │  │  │  ├─ scoped_name: thrift.
   │  │  │  │  │  │  │  │  ├─ uri: 
   │  │  │  │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  │  │  │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │  │  │  ├─ has_doc? false
   │  │  │  │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  │  │  │  ├─ begin (offset): 0
   │  │  │  │  │  │  │  │  │  ╰─ end (offset): 0
   │  │  │  │  │  │  │  │  ╰─ name_range: <absent>
   │  │  │  │  │  │  │  ├─ type_ref [t_type_ref]
   │  │  │  │  │  │  │  │  ├─ empty? false
   │  │  │  │  │  │  │  │  ├─ resolved? true
   │  │  │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  │  │  ├─ begin (offset): 5121
   │  │  │  │  │  │  │  │  │  ╰─ end (offset): 5134
   │  │  │  │  │  │  │  │  ├─ type: [t_type*] 0xNORMALIZED_74
   │  │  │  │  │  │  │  │  │  ╰─ full_name: scope.Struct
   │  │  │  │  │  │  │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │  │  ╰─ value [t_const_value*] 0xNORMALIZED_90
   │  │  │  │  │  │  │     ├─ src_range: <absent>
   │  │  │  │  │  │  │     ├─ is_empty? true
   │  │  │  │  │  │  │     ╰─ kind: map({})
   │  │  │  │  │  │  ╰─ structured_annotations[1] [t_const]
   │  │  │  │  │  │     ├─ (base) [t_named] @0xNORMALIZED_91
   │  │  │  │  │  │     │  ├─ (base) [t_node] @0xNORMALIZED_91
   │  │  │  │  │  │     │  │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  │  ├─ begin (offset): 5135
   │  │  │  │  │  │     │  │  │  ╰─ end (offset): 5148
   │  │  │  │  │  │     │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ name: 
   │  │  │  │  │  │     │  ├─ scoped_name: thrift.
   │  │  │  │  │  │     │  ├─ uri: 
   │  │  │  │  │  │     │  ├─ explicit_uri? false
   │  │  │  │  │  │     │  ├─ generated? false
   │  │  │  │  │  │     │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ has_doc? false
   │  │  │  │  │  │     │  ├─ doc_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 0
   │  │  │  │  │  │     │  │  ╰─ end (offset): 0
   │  │  │  │  │  │     │  ╰─ name_range: <absent>
   │  │  │  │  │  │     ├─ type_ref [t_type_ref]
   │  │  │  │  │  │     │  ├─ empty? false
   │  │  │  │  │  │     │  ├─ resolved? true
   │  │  │  │  │  │     │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 5135
   │  │  │  │  │  │     │  │  ╰─ end (offset): 5148
   │  │  │  │  │  │     │  ├─ type: [t_type*] 0xNORMALIZED_15
   │  │  │  │  │  │     │  │  ╰─ full_name: thrift.Experimental
   │  │  │  │  │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │     ╰─ value [t_const_value*] 0xNORMALIZED_92
   │  │  │  │  │  │        ├─ src_range: <absent>
   │  │  │  │  │  │        ├─ is_empty? true
   │  │  │  │  │  │        ╰─ kind: map({})
   │  │  │  │  │  ├─ has_doc? true
   │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  ├─ begin (offset): 4763
   │  │  │  │  │  │  ╰─ end (offset): 5120
   │  │  │  │  │  ╰─ name_range [source_range]
   │  │  │  │  │     ├─ begin (offset): 5187
   │  │  │  │  │     ╰─ end (offset): 5210
   │  │  │  │  ├─ full_name: thrift.SerializeInFieldIdOrder
   │  │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_88
   │  │  │  │     ╰─ full_name: thrift.SerializeInFieldIdOrder
   │  │  │  ╰─ fields (size: 0)
   │  │  ├─ structs_and_unions[7] [t_structured] @0xNORMALIZED_93
   │  │  │  ├─ (base) [t_type] @0xNORMALIZED_93
   │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_93
   │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_93
   │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  ├─ begin (offset): 5352
   │  │  │  │  │  │  │  ╰─ end (offset): 5385
   │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  ├─ name: BitmaskEnum
   │  │  │  │  │  ├─ scoped_name: thrift.BitmaskEnum
   │  │  │  │  │  ├─ uri: facebook.com/thrift/annotation/BitmaskEnum
   │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  ├─ structured_annotations (size: 1)
   │  │  │  │  │  │  ╰─ structured_annotations[0] [t_const]
   │  │  │  │  │  │     ├─ (base) [t_named] @0xNORMALIZED_94
   │  │  │  │  │  │     │  ├─ (base) [t_node] @0xNORMALIZED_94
   │  │  │  │  │  │     │  │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  │  ├─ begin (offset): 5352
   │  │  │  │  │  │     │  │  │  ╰─ end (offset): 5363
   │  │  │  │  │  │     │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ name: 
   │  │  │  │  │  │     │  ├─ scoped_name: thrift.
   │  │  │  │  │  │     │  ├─ uri: 
   │  │  │  │  │  │     │  ├─ explicit_uri? false
   │  │  │  │  │  │     │  ├─ generated? false
   │  │  │  │  │  │     │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ has_doc? false
   │  │  │  │  │  │     │  ├─ doc_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 0
   │  │  │  │  │  │     │  │  ╰─ end (offset): 0
   │  │  │  │  │  │     │  ╰─ name_range: <absent>
   │  │  │  │  │  │     ├─ type_ref [t_type_ref]
   │  │  │  │  │  │     │  ├─ empty? false
   │  │  │  │  │  │     │  ├─ resolved? true
   │  │  │  │  │  │     │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 5352
   │  │  │  │  │  │     │  │  ╰─ end (offset): 5363
   │  │  │  │  │  │     │  ├─ type: [t_type*] 0xNORMALIZED_56
   │  │  │  │  │  │     │  │  ╰─ full_name: scope.Enum
   │  │  │  │  │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │     ╰─ value [t_const_value*] 0xNORMALIZED_95
   │  │  │  │  │  │        ├─ src_range: <absent>
   │  │  │  │  │  │        ├─ is_empty? true
   │  │  │  │  │  │        ╰─ kind: map({})
   │  │  │  │  │  ├─ has_doc? true
   │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  ├─ begin (offset): 5215
   │  │  │  │  │  │  ╰─ end (offset): 5351
   │  │  │  │  │  ╰─ name_range [source_range]
   │  │  │  │  │     ├─ begin (offset): 5371
   │  │  │  │  │     ╰─ end (offset): 5382
   │  │  │  │  ├─ full_name: thrift.BitmaskEnum
   │  │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_93
   │  │  │  │     ╰─ full_name: thrift.BitmaskEnum
   │  │  │  ╰─ fields (size: 0)
   │  │  ├─ structs_and_unions[8] [t_structured] @0xNORMALIZED_96
   │  │  │  ├─ (base) [t_type] @0xNORMALIZED_96
   │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_96
   │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_96
   │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  ├─ begin (offset): 6632
   │  │  │  │  │  │  │  ╰─ end (offset): 6671
   │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  ├─ name: ExceptionMessage
   │  │  │  │  │  ├─ scoped_name: thrift.ExceptionMessage
   │  │  │  │  │  ├─ uri: facebook.com/thrift/annotation/ExceptionMessage
   │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  ├─ structured_annotations (size: 1)
   │  │  │  │  │  │  ╰─ structured_annotations[0] [t_const]
   │  │  │  │  │  │     ├─ (base) [t_named] @0xNORMALIZED_97
   │  │  │  │  │  │     │  ├─ (base) [t_node] @0xNORMALIZED_97
   │  │  │  │  │  │     │  │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  │  ├─ begin (offset): 6632
   │  │  │  │  │  │     │  │  │  ╰─ end (offset): 6644
   │  │  │  │  │  │     │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ name: 
   │  │  │  │  │  │     │  ├─ scoped_name: thrift.
   │  │  │  │  │  │     │  ├─ uri: 
   │  │  │  │  │  │     │  ├─ explicit_uri? false
   │  │  │  │  │  │     │  ├─ generated? false
   │  │  │  │  │  │     │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ has_doc? false
   │  │  │  │  │  │     │  ├─ doc_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 0
   │  │  │  │  │  │     │  │  ╰─ end (offset): 0
   │  │  │  │  │  │     │  ╰─ name_range: <absent>
   │  │  │  │  │  │     ├─ type_ref [t_type_ref]
   │  │  │  │  │  │     │  ├─ empty? false
   │  │  │  │  │  │     │  ├─ resolved? true
   │  │  │  │  │  │     │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 6632
   │  │  │  │  │  │     │  │  ╰─ end (offset): 6644
   │  │  │  │  │  │     │  ├─ type: [t_type*] 0xNORMALIZED_80
   │  │  │  │  │  │     │  │  ╰─ full_name: scope.Field
   │  │  │  │  │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │     ╰─ value [t_const_value*] 0xNORMALIZED_98
   │  │  │  │  │  │        ├─ src_range: <absent>
   │  │  │  │  │  │        ├─ is_empty? true
   │  │  │  │  │  │        ╰─ kind: map({})
   │  │  │  │  │  ├─ has_doc? true
   │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  ├─ begin (offset): 5387
   │  │  │  │  │  │  ╰─ end (offset): 6631
   │  │  │  │  │  ╰─ name_range [source_range]
   │  │  │  │  │     ├─ begin (offset): 6652
   │  │  │  │  │     ╰─ end (offset): 6668
   │  │  │  │  ├─ full_name: thrift.ExceptionMessage
   │  │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_96
   │  │  │  │     ╰─ full_name: thrift.ExceptionMessage
   │  │  │  ╰─ fields (size: 0)
   │  │  ├─ structs_and_unions[9] [t_structured] @0xNORMALIZED_99
   │  │  │  ├─ (base) [t_type] @0xNORMALIZED_99
   │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_99
   │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_99
   │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  ├─ begin (offset): 6818
   │  │  │  │  │  │  │  ╰─ end (offset): 6864
   │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  ├─ name: InternBox
   │  │  │  │  │  ├─ scoped_name: thrift.InternBox
   │  │  │  │  │  ├─ uri: facebook.com/thrift/annotation/InternBox
   │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  ├─ structured_annotations (size: 2)
   │  │  │  │  │  │  ├─ structured_annotations[0] [t_const]
   │  │  │  │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_100
   │  │  │  │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_100
   │  │  │  │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  │  │  │  ├─ begin (offset): 6818
   │  │  │  │  │  │  │  │  │  │  ╰─ end (offset): 6830
   │  │  │  │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │  │  │  ├─ name: 
   │  │  │  │  │  │  │  │  ├─ scoped_name: thrift.
   │  │  │  │  │  │  │  │  ├─ uri: 
   │  │  │  │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  │  │  │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │  │  │  ├─ has_doc? false
   │  │  │  │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  │  │  │  ├─ begin (offset): 0
   │  │  │  │  │  │  │  │  │  ╰─ end (offset): 0
   │  │  │  │  │  │  │  │  ╰─ name_range: <absent>
   │  │  │  │  │  │  │  ├─ type_ref [t_type_ref]
   │  │  │  │  │  │  │  │  ├─ empty? false
   │  │  │  │  │  │  │  │  ├─ resolved? true
   │  │  │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  │  │  ├─ begin (offset): 6818
   │  │  │  │  │  │  │  │  │  ╰─ end (offset): 6830
   │  │  │  │  │  │  │  │  ├─ type: [t_type*] 0xNORMALIZED_80
   │  │  │  │  │  │  │  │  │  ╰─ full_name: scope.Field
   │  │  │  │  │  │  │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │  │  ╰─ value [t_const_value*] 0xNORMALIZED_101
   │  │  │  │  │  │  │     ├─ src_range: <absent>
   │  │  │  │  │  │  │     ├─ is_empty? true
   │  │  │  │  │  │  │     ╰─ kind: map({})
   │  │  │  │  │  │  ╰─ structured_annotations[1] [t_const]
   │  │  │  │  │  │     ├─ (base) [t_named] @0xNORMALIZED_102
   │  │  │  │  │  │     │  ├─ (base) [t_node] @0xNORMALIZED_102
   │  │  │  │  │  │     │  │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  │  ├─ begin (offset): 6831
   │  │  │  │  │  │     │  │  │  ╰─ end (offset): 6844
   │  │  │  │  │  │     │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ name: 
   │  │  │  │  │  │     │  ├─ scoped_name: thrift.
   │  │  │  │  │  │     │  ├─ uri: 
   │  │  │  │  │  │     │  ├─ explicit_uri? false
   │  │  │  │  │  │     │  ├─ generated? false
   │  │  │  │  │  │     │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ has_doc? false
   │  │  │  │  │  │     │  ├─ doc_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 0
   │  │  │  │  │  │     │  │  ╰─ end (offset): 0
   │  │  │  │  │  │     │  ╰─ name_range: <absent>
   │  │  │  │  │  │     ├─ type_ref [t_type_ref]
   │  │  │  │  │  │     │  ├─ empty? false
   │  │  │  │  │  │     │  ├─ resolved? true
   │  │  │  │  │  │     │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 6831
   │  │  │  │  │  │     │  │  ╰─ end (offset): 6844
   │  │  │  │  │  │     │  ├─ type: [t_type*] 0xNORMALIZED_15
   │  │  │  │  │  │     │  │  ╰─ full_name: thrift.Experimental
   │  │  │  │  │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │     ╰─ value [t_const_value*] 0xNORMALIZED_103
   │  │  │  │  │  │        ├─ src_range: <absent>
   │  │  │  │  │  │        ├─ is_empty? true
   │  │  │  │  │  │        ╰─ kind: map({})
   │  │  │  │  │  ├─ has_doc? true
   │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  ├─ begin (offset): 6673
   │  │  │  │  │  │  ╰─ end (offset): 6817
   │  │  │  │  │  ╰─ name_range [source_range]
   │  │  │  │  │     ├─ begin (offset): 6852
   │  │  │  │  │     ╰─ end (offset): 6861
   │  │  │  │  ├─ full_name: thrift.InternBox
   │  │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_99
   │  │  │  │     ╰─ full_name: thrift.InternBox
   │  │  │  ╰─ fields (size: 0)
   │  │  ├─ structs_and_unions[10] [t_structured] @0xNORMALIZED_104
   │  │  │  ├─ (base) [t_type] @0xNORMALIZED_104
   │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_104
   │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_104
   │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  ├─ begin (offset): 6951
   │  │  │  │  │  │  │  ╰─ end (offset): 6986
   │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  ├─ name: Serial
   │  │  │  │  │  ├─ scoped_name: thrift.Serial
   │  │  │  │  │  ├─ uri: facebook.com/thrift/annotation/Serial
   │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  ├─ structured_annotations (size: 1)
   │  │  │  │  │  │  ╰─ structured_annotations[0] [t_const]
   │  │  │  │  │  │     ├─ (base) [t_named] @0xNORMALIZED_105
   │  │  │  │  │  │     │  ├─ (base) [t_node] @0xNORMALIZED_105
   │  │  │  │  │  │     │  │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  │  ├─ begin (offset): 6951
   │  │  │  │  │  │     │  │  │  ╰─ end (offset): 6969
   │  │  │  │  │  │     │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ name: 
   │  │  │  │  │  │     │  ├─ scoped_name: thrift.
   │  │  │  │  │  │     │  ├─ uri: 
   │  │  │  │  │  │     │  ├─ explicit_uri? false
   │  │  │  │  │  │     │  ├─ generated? false
   │  │  │  │  │  │     │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ has_doc? false
   │  │  │  │  │  │     │  ├─ doc_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 0
   │  │  │  │  │  │     │  │  ╰─ end (offset): 0
   │  │  │  │  │  │     │  ╰─ name_range: <absent>
   │  │  │  │  │  │     ├─ type_ref [t_type_ref]
   │  │  │  │  │  │     │  ├─ empty? false
   │  │  │  │  │  │     │  ├─ resolved? true
   │  │  │  │  │  │     │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 6951
   │  │  │  │  │  │     │  │  ╰─ end (offset): 6969
   │  │  │  │  │  │     │  ├─ type: [t_type*] 0xNORMALIZED_106
   │  │  │  │  │  │     │  │  ╰─ full_name: scope.Interaction
   │  │  │  │  │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │     ╰─ value [t_const_value*] 0xNORMALIZED_107
   │  │  │  │  │  │        ├─ src_range: <absent>
   │  │  │  │  │  │        ├─ is_empty? true
   │  │  │  │  │  │        ╰─ kind: map({})
   │  │  │  │  │  ├─ has_doc? true
   │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  ├─ begin (offset): 6866
   │  │  │  │  │  │  ╰─ end (offset): 6950
   │  │  │  │  │  ╰─ name_range [source_range]
   │  │  │  │  │     ├─ begin (offset): 6977
   │  │  │  │  │     ╰─ end (offset): 6983
   │  │  │  │  ├─ full_name: thrift.Serial
   │  │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_104
   │  │  │  │     ╰─ full_name: thrift.Serial
   │  │  │  ╰─ fields (size: 0)
   │  │  ├─ structs_and_unions[11] [t_structured] @0xNORMALIZED_108
   │  │  │  ├─ (base) [t_type] @0xNORMALIZED_108
   │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_108
   │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_108
   │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  ├─ begin (offset): 7071
   │  │  │  │  │  │  │  ╰─ end (offset): 7149
   │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  ├─ name: Uri
   │  │  │  │  │  ├─ scoped_name: thrift.Uri
   │  │  │  │  │  ├─ uri: facebook.com/thrift/annotation/Uri
   │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  ├─ structured_annotations (size: 3)
   │  │  │  │  │  │  ├─ structured_annotations[0] [t_const]
   │  │  │  │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_109
   │  │  │  │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_109
   │  │  │  │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  │  │  │  ├─ begin (offset): 7071
   │  │  │  │  │  │  │  │  │  │  ╰─ end (offset): 7082
   │  │  │  │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │  │  │  ├─ name: 
   │  │  │  │  │  │  │  │  ├─ scoped_name: thrift.
   │  │  │  │  │  │  │  │  ├─ uri: 
   │  │  │  │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  │  │  │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │  │  │  ├─ has_doc? false
   │  │  │  │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  │  │  │  ├─ begin (offset): 0
   │  │  │  │  │  │  │  │  │  ╰─ end (offset): 0
   │  │  │  │  │  │  │  │  ╰─ name_range: <absent>
   │  │  │  │  │  │  │  ├─ type_ref [t_type_ref]
   │  │  │  │  │  │  │  │  ├─ empty? false
   │  │  │  │  │  │  │  │  ├─ resolved? true
   │  │  │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  │  │  ├─ begin (offset): 7071
   │  │  │  │  │  │  │  │  │  ╰─ end (offset): 7082
   │  │  │  │  │  │  │  │  ├─ type: [t_type*] 0xNORMALIZED_56
   │  │  │  │  │  │  │  │  │  ╰─ full_name: scope.Enum
   │  │  │  │  │  │  │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │  │  ╰─ value [t_const_value*] 0xNORMALIZED_110
   │  │  │  │  │  │  │     ├─ src_range: <absent>
   │  │  │  │  │  │  │     ├─ is_empty? true
   │  │  │  │  │  │  │     ╰─ kind: map({})
   │  │  │  │  │  │  ├─ structured_annotations[1] [t_const]
   │  │  │  │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_111
   │  │  │  │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_111
   │  │  │  │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  │  │  │  ├─ begin (offset): 7083
   │  │  │  │  │  │  │  │  │  │  ╰─ end (offset): 7097
   │  │  │  │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │  │  │  ├─ name: 
   │  │  │  │  │  │  │  │  ├─ scoped_name: thrift.
   │  │  │  │  │  │  │  │  ├─ uri: 
   │  │  │  │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  │  │  │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │  │  │  ├─ has_doc? false
   │  │  │  │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  │  │  │  ├─ begin (offset): 0
   │  │  │  │  │  │  │  │  │  ╰─ end (offset): 0
   │  │  │  │  │  │  │  │  ╰─ name_range: <absent>
   │  │  │  │  │  │  │  ├─ type_ref [t_type_ref]
   │  │  │  │  │  │  │  │  ├─ empty? false
   │  │  │  │  │  │  │  │  ├─ resolved? true
   │  │  │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  │  │  ├─ begin (offset): 7083
   │  │  │  │  │  │  │  │  │  ╰─ end (offset): 7097
   │  │  │  │  │  │  │  │  ├─ type: [t_type*] 0xNORMALIZED_112
   │  │  │  │  │  │  │  │  │  ╰─ full_name: scope.Service
   │  │  │  │  │  │  │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │  │  ╰─ value [t_const_value*] 0xNORMALIZED_113
   │  │  │  │  │  │  │     ├─ src_range: <absent>
   │  │  │  │  │  │  │     ├─ is_empty? true
   │  │  │  │  │  │  │     ╰─ kind: map({})
   │  │  │  │  │  │  ╰─ structured_annotations[2] [t_const]
   │  │  │  │  │  │     ├─ (base) [t_named] @0xNORMALIZED_114
   │  │  │  │  │  │     │  ├─ (base) [t_node] @0xNORMALIZED_114
   │  │  │  │  │  │     │  │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  │  ├─ begin (offset): 7098
   │  │  │  │  │  │     │  │  │  ╰─ end (offset): 7115
   │  │  │  │  │  │     │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ name: 
   │  │  │  │  │  │     │  ├─ scoped_name: thrift.
   │  │  │  │  │  │     │  ├─ uri: 
   │  │  │  │  │  │     │  ├─ explicit_uri? false
   │  │  │  │  │  │     │  ├─ generated? false
   │  │  │  │  │  │     │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ has_doc? false
   │  │  │  │  │  │     │  ├─ doc_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 0
   │  │  │  │  │  │     │  │  ╰─ end (offset): 0
   │  │  │  │  │  │     │  ╰─ name_range: <absent>
   │  │  │  │  │  │     ├─ type_ref [t_type_ref]
   │  │  │  │  │  │     │  ├─ empty? false
   │  │  │  │  │  │     │  ├─ resolved? true
   │  │  │  │  │  │     │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 7098
   │  │  │  │  │  │     │  │  ╰─ end (offset): 7115
   │  │  │  │  │  │     │  ├─ type: [t_type*] 0xNORMALIZED_53
   │  │  │  │  │  │     │  │  ╰─ full_name: scope.Structured
   │  │  │  │  │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │     ╰─ value [t_const_value*] 0xNORMALIZED_115
   │  │  │  │  │  │        ├─ src_range: <absent>
   │  │  │  │  │  │        ├─ is_empty? true
   │  │  │  │  │  │        ╰─ kind: map({})
   │  │  │  │  │  ├─ has_doc? true
   │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  ├─ begin (offset): 6988
   │  │  │  │  │  │  ╰─ end (offset): 7070
   │  │  │  │  │  ╰─ name_range [source_range]
   │  │  │  │  │     ├─ begin (offset): 7123
   │  │  │  │  │     ╰─ end (offset): 7126
   │  │  │  │  ├─ full_name: thrift.Uri
   │  │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_108
   │  │  │  │     ╰─ full_name: thrift.Uri
   │  │  │  ╰─ fields (size: 1)
   │  │  │     ╰─ fields[0] [t_field] @0xNORMALIZED_116
   │  │  │        ├─ (base) [t_named] @0xNORMALIZED_116
   │  │  │        │  ├─ (base) [t_node] @0xNORMALIZED_116
   │  │  │        │  │  ├─ src_range [source_range]
   │  │  │        │  │  │  ├─ begin (offset): 7131
   │  │  │        │  │  │  ╰─ end (offset): 7147
   │  │  │        │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │        │  ├─ name: value
   │  │  │        │  ├─ scoped_name: value
   │  │  │        │  ├─ uri: 
   │  │  │        │  ├─ explicit_uri? false
   │  │  │        │  ├─ generated? false
   │  │  │        │  ├─ structured_annotations (size: 0)
   │  │  │        │  ├─ has_doc? false
   │  │  │        │  ├─ doc_range [source_range]
   │  │  │        │  │  ├─ begin (offset): 0
   │  │  │        │  │  ╰─ end (offset): 0
   │  │  │        │  ╰─ name_range [source_range]
   │  │  │        │     ├─ begin (offset): 7141
   │  │  │        │     ╰─ end (offset): 7146
   │  │  │        ├─ qualifier: 0
   │  │  │        ├─ id: 1
   │  │  │        ├─ explicit_id: 1
   │  │  │        ├─ type [t_type_ref]
   │  │  │        │  ├─ empty? false
   │  │  │        │  ├─ resolved? true
   │  │  │        │  ├─ src_range [source_range]
   │  │  │        │  │  ├─ begin (offset): 7134
   │  │  │        │  │  ╰─ end (offset): 7140
   │  │  │        │  ├─ type: [t_type*] 0xNORMALIZED_117
   │  │  │        │  │  ╰─ full_name: string
   │  │  │        │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │        ├─ is_injected? false
   │  │  │        ╰─ default_value [t_const_value*] 0x0
   │  │  ├─ structs_and_unions[12] [t_structured] @0xNORMALIZED_118
   │  │  │  ├─ (base) [t_type] @0xNORMALIZED_118
   │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_118
   │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_118
   │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  ├─ begin (offset): 7218
   │  │  │  │  │  │  │  ╰─ end (offset): 7277
   │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  ├─ name: Priority
   │  │  │  │  │  ├─ scoped_name: thrift.Priority
   │  │  │  │  │  ├─ uri: facebook.com/thrift/annotation/Priority
   │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  ├─ structured_annotations (size: 1)
   │  │  │  │  │  │  ╰─ structured_annotations[0] [t_const]
   │  │  │  │  │  │     ├─ (base) [t_named] @0xNORMALIZED_119
   │  │  │  │  │  │     │  ├─ (base) [t_node] @0xNORMALIZED_119
   │  │  │  │  │  │     │  │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  │  ├─ begin (offset): 7218
   │  │  │  │  │  │     │  │  │  ╰─ end (offset): 7233
   │  │  │  │  │  │     │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ name: 
   │  │  │  │  │  │     │  ├─ scoped_name: thrift.
   │  │  │  │  │  │     │  ├─ uri: 
   │  │  │  │  │  │     │  ├─ explicit_uri? false
   │  │  │  │  │  │     │  ├─ generated? false
   │  │  │  │  │  │     │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ has_doc? false
   │  │  │  │  │  │     │  ├─ doc_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 0
   │  │  │  │  │  │     │  │  ╰─ end (offset): 0
   │  │  │  │  │  │     │  ╰─ name_range: <absent>
   │  │  │  │  │  │     ├─ type_ref [t_type_ref]
   │  │  │  │  │  │     │  ├─ empty? false
   │  │  │  │  │  │     │  ├─ resolved? true
   │  │  │  │  │  │     │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 7218
   │  │  │  │  │  │     │  │  ╰─ end (offset): 7233
   │  │  │  │  │  │     │  ├─ type: [t_type*] 0xNORMALIZED_120
   │  │  │  │  │  │     │  │  ╰─ full_name: scope.Function
   │  │  │  │  │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │     ╰─ value [t_const_value*] 0xNORMALIZED_121
   │  │  │  │  │  │        ├─ src_range: <absent>
   │  │  │  │  │  │        ├─ is_empty? true
   │  │  │  │  │  │        ╰─ kind: map({})
   │  │  │  │  │  ├─ has_doc? true
   │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  ├─ begin (offset): 7151
   │  │  │  │  │  │  ╰─ end (offset): 7217
   │  │  │  │  │  ╰─ name_range [source_range]
   │  │  │  │  │     ├─ begin (offset): 7241
   │  │  │  │  │     ╰─ end (offset): 7249
   │  │  │  │  ├─ full_name: thrift.Priority
   │  │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_118
   │  │  │  │     ╰─ full_name: thrift.Priority
   │  │  │  ╰─ fields (size: 1)
   │  │  │     ╰─ fields[0] [t_field] @0xNORMALIZED_122
   │  │  │        ├─ (base) [t_named] @0xNORMALIZED_122
   │  │  │        │  ├─ (base) [t_node] @0xNORMALIZED_122
   │  │  │        │  │  ├─ src_range [source_range]
   │  │  │        │  │  │  ├─ begin (offset): 7254
   │  │  │        │  │  │  ╰─ end (offset): 7275
   │  │  │        │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │        │  ├─ name: level
   │  │  │        │  ├─ scoped_name: level
   │  │  │        │  ├─ uri: 
   │  │  │        │  ├─ explicit_uri? false
   │  │  │        │  ├─ generated? false
   │  │  │        │  ├─ structured_annotations (size: 0)
   │  │  │        │  ├─ has_doc? false
   │  │  │        │  ├─ doc_range [source_range]
   │  │  │        │  │  ├─ begin (offset): 0
   │  │  │        │  │  ╰─ end (offset): 0
   │  │  │        │  ╰─ name_range [source_range]
   │  │  │        │     ├─ begin (offset): 7269
   │  │  │        │     ╰─ end (offset): 7274
   │  │  │        ├─ qualifier: 0
   │  │  │        ├─ id: 1
   │  │  │        ├─ explicit_id: 1
   │  │  │        ├─ type [t_type_ref]
   │  │  │        │  ├─ empty? false
   │  │  │        │  ├─ resolved? true
   │  │  │        │  ├─ src_range [source_range]
   │  │  │        │  │  ├─ begin (offset): 7257
   │  │  │        │  │  ╰─ end (offset): 7268
   │  │  │        │  ├─ type: [t_type*] 0xNORMALIZED_5
   │  │  │        │  │  ╰─ full_name: thrift.RpcPriority
   │  │  │        │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │        ├─ is_injected? false
   │  │  │        ╰─ default_value [t_const_value*] 0x0
   │  │  ├─ structs_and_unions[13] [t_structured] @0xNORMALIZED_123
   │  │  │  ├─ (base) [t_type] @0xNORMALIZED_123
   │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_123
   │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_123
   │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  ├─ begin (offset): 7443
   │  │  │  │  │  │  │  ╰─ end (offset): 7536
   │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  ├─ name: DeprecatedUnvalidatedAnnotations
   │  │  │  │  │  ├─ scoped_name: thrift.DeprecatedUnvalidatedAnnotations
   │  │  │  │  │  ├─ uri: facebook.com/thrift/annotation/DeprecatedUnvalidatedAnnotations
   │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  ├─ structured_annotations (size: 1)
   │  │  │  │  │  │  ╰─ structured_annotations[0] [t_const]
   │  │  │  │  │  │     ├─ (base) [t_named] @0xNORMALIZED_124
   │  │  │  │  │  │     │  ├─ (base) [t_node] @0xNORMALIZED_124
   │  │  │  │  │  │     │  │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  │  ├─ begin (offset): 7443
   │  │  │  │  │  │     │  │  │  ╰─ end (offset): 7460
   │  │  │  │  │  │     │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ name: 
   │  │  │  │  │  │     │  ├─ scoped_name: thrift.
   │  │  │  │  │  │     │  ├─ uri: 
   │  │  │  │  │  │     │  ├─ explicit_uri? false
   │  │  │  │  │  │     │  ├─ generated? false
   │  │  │  │  │  │     │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ has_doc? false
   │  │  │  │  │  │     │  ├─ doc_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 0
   │  │  │  │  │  │     │  │  ╰─ end (offset): 0
   │  │  │  │  │  │     │  ╰─ name_range: <absent>
   │  │  │  │  │  │     ├─ type_ref [t_type_ref]
   │  │  │  │  │  │     │  ├─ empty? false
   │  │  │  │  │  │     │  ├─ resolved? true
   │  │  │  │  │  │     │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 7443
   │  │  │  │  │  │     │  │  ╰─ end (offset): 7460
   │  │  │  │  │  │     │  ├─ type: [t_type*] 0xNORMALIZED_49
   │  │  │  │  │  │     │  │  ╰─ full_name: scope.Definition
   │  │  │  │  │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │     ╰─ value [t_const_value*] 0xNORMALIZED_125
   │  │  │  │  │  │        ├─ src_range: <absent>
   │  │  │  │  │  │        ├─ is_empty? true
   │  │  │  │  │  │        ╰─ kind: map({})
   │  │  │  │  │  ├─ has_doc? true
   │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  ├─ begin (offset): 7384
   │  │  │  │  │  │  ╰─ end (offset): 7442
   │  │  │  │  │  ╰─ name_range [source_range]
   │  │  │  │  │     ├─ begin (offset): 7468
   │  │  │  │  │     ╰─ end (offset): 7500
   │  │  │  │  ├─ full_name: thrift.DeprecatedUnvalidatedAnnotations
   │  │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_123
   │  │  │  │     ╰─ full_name: thrift.DeprecatedUnvalidatedAnnotations
   │  │  │  ╰─ fields (size: 1)
   │  │  │     ╰─ fields[0] [t_field] @0xNORMALIZED_126
   │  │  │        ├─ (base) [t_named] @0xNORMALIZED_126
   │  │  │        │  ├─ (base) [t_node] @0xNORMALIZED_126
   │  │  │        │  │  ├─ src_range [source_range]
   │  │  │        │  │  │  ├─ begin (offset): 7505
   │  │  │        │  │  │  ╰─ end (offset): 7534
   │  │  │        │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │        │  ├─ name: items
   │  │  │        │  ├─ scoped_name: items
   │  │  │        │  ├─ uri: 
   │  │  │        │  ├─ explicit_uri? false
   │  │  │        │  ├─ generated? false
   │  │  │        │  ├─ structured_annotations (size: 0)
   │  │  │        │  ├─ has_doc? false
   │  │  │        │  ├─ doc_range [source_range]
   │  │  │        │  │  ├─ begin (offset): 0
   │  │  │        │  │  ╰─ end (offset): 0
   │  │  │        │  ╰─ name_range [source_range]
   │  │  │        │     ├─ begin (offset): 7528
   │  │  │        │     ╰─ end (offset): 7533
   │  │  │        ├─ qualifier: 0
   │  │  │        ├─ id: 1
   │  │  │        ├─ explicit_id: 1
   │  │  │        ├─ type [t_type_ref]
   │  │  │        │  ├─ empty? false
   │  │  │        │  ├─ resolved? true
   │  │  │        │  ├─ src_range [source_range]
   │  │  │        │  │  ├─ begin (offset): 7508
   │  │  │        │  │  ╰─ end (offset): 7527
   │  │  │        │  ├─ type: [t_type*] 0xNORMALIZED_127
   │  │  │        │  │  ╰─ full_name: map<string, string>
   │  │  │        │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │        ├─ is_injected? false
   │  │  │        ╰─ default_value [t_const_value*] 0x0
   │  │  ├─ structs_and_unions[14] [t_structured] @0xNORMALIZED_128
   │  │  │  ├─ (base) [t_type] @0xNORMALIZED_128
   │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_128
   │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_128
   │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  ├─ begin (offset): 7943
   │  │  │  │  │  │  │  ╰─ end (offset): 7994
   │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  ├─ name: AllowReservedIdentifier
   │  │  │  │  │  ├─ scoped_name: thrift.AllowReservedIdentifier
   │  │  │  │  │  ├─ uri: facebook.com/thrift/annotation/AllowReservedIdentifier
   │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  ├─ structured_annotations (size: 1)
   │  │  │  │  │  │  ╰─ structured_annotations[0] [t_const]
   │  │  │  │  │  │     ├─ (base) [t_named] @0xNORMALIZED_129
   │  │  │  │  │  │     │  ├─ (base) [t_node] @0xNORMALIZED_129
   │  │  │  │  │  │     │  │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  │  ├─ begin (offset): 7943
   │  │  │  │  │  │     │  │  │  ╰─ end (offset): 7960
   │  │  │  │  │  │     │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ name: 
   │  │  │  │  │  │     │  ├─ scoped_name: thrift.
   │  │  │  │  │  │     │  ├─ uri: 
   │  │  │  │  │  │     │  ├─ explicit_uri? false
   │  │  │  │  │  │     │  ├─ generated? false
   │  │  │  │  │  │     │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ has_doc? false
   │  │  │  │  │  │     │  ├─ doc_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 0
   │  │  │  │  │  │     │  │  ╰─ end (offset): 0
   │  │  │  │  │  │     │  ╰─ name_range: <absent>
   │  │  │  │  │  │     ├─ type_ref [t_type_ref]
   │  │  │  │  │  │     │  ├─ empty? false
   │  │  │  │  │  │     │  ├─ resolved? true
   │  │  │  │  │  │     │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 7943
   │  │  │  │  │  │     │  │  ╰─ end (offset): 7960
   │  │  │  │  │  │     │  ├─ type: [t_type*] 0xNORMALIZED_49
   │  │  │  │  │  │     │  │  ╰─ full_name: scope.Definition
   │  │  │  │  │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │     ╰─ value [t_const_value*] 0xNORMALIZED_130
   │  │  │  │  │  │        ├─ src_range: <absent>
   │  │  │  │  │  │        ├─ is_empty? true
   │  │  │  │  │  │        ╰─ kind: map({})
   │  │  │  │  │  ├─ has_doc? true
   │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  ├─ begin (offset): 7538
   │  │  │  │  │  │  ╰─ end (offset): 7942
   │  │  │  │  │  ╰─ name_range [source_range]
   │  │  │  │  │     ├─ begin (offset): 7968
   │  │  │  │  │     ╰─ end (offset): 7991
   │  │  │  │  ├─ full_name: thrift.AllowReservedIdentifier
   │  │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_128
   │  │  │  │     ╰─ full_name: thrift.AllowReservedIdentifier
   │  │  │  ╰─ fields (size: 0)
   │  │  ├─ structs_and_unions[15] [t_structured] @0xNORMALIZED_131
   │  │  │  ├─ (base) [t_type] @0xNORMALIZED_131
   │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_131
   │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_131
   │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  ├─ begin (offset): 7996
   │  │  │  │  │  │  │  ╰─ end (offset): 8042
   │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  ├─ name: AllowReservedFilename
   │  │  │  │  │  ├─ scoped_name: thrift.AllowReservedFilename
   │  │  │  │  │  ├─ uri: facebook.com/thrift/annotation/AllowReservedFilename
   │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  ├─ structured_annotations (size: 1)
   │  │  │  │  │  │  ╰─ structured_annotations[0] [t_const]
   │  │  │  │  │  │     ├─ (base) [t_named] @0xNORMALIZED_132
   │  │  │  │  │  │     │  ├─ (base) [t_node] @0xNORMALIZED_132
   │  │  │  │  │  │     │  │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  │  ├─ begin (offset): 7996
   │  │  │  │  │  │     │  │  │  ╰─ end (offset): 8010
   │  │  │  │  │  │     │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ name: 
   │  │  │  │  │  │     │  ├─ scoped_name: thrift.
   │  │  │  │  │  │     │  ├─ uri: 
   │  │  │  │  │  │     │  ├─ explicit_uri? false
   │  │  │  │  │  │     │  ├─ generated? false
   │  │  │  │  │  │     │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ has_doc? false
   │  │  │  │  │  │     │  ├─ doc_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 0
   │  │  │  │  │  │     │  │  ╰─ end (offset): 0
   │  │  │  │  │  │     │  ╰─ name_range: <absent>
   │  │  │  │  │  │     ├─ type_ref [t_type_ref]
   │  │  │  │  │  │     │  ├─ empty? false
   │  │  │  │  │  │     │  ├─ resolved? true
   │  │  │  │  │  │     │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 7996
   │  │  │  │  │  │     │  │  ╰─ end (offset): 8010
   │  │  │  │  │  │     │  ├─ type: [t_type*] 0xNORMALIZED_46
   │  │  │  │  │  │     │  │  ╰─ full_name: scope.Program
   │  │  │  │  │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │     ╰─ value [t_const_value*] 0xNORMALIZED_133
   │  │  │  │  │  │        ├─ src_range: <absent>
   │  │  │  │  │  │        ├─ is_empty? true
   │  │  │  │  │  │        ╰─ kind: map({})
   │  │  │  │  │  ├─ has_doc? false
   │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  ├─ begin (offset): 0
   │  │  │  │  │  │  ╰─ end (offset): 0
   │  │  │  │  │  ╰─ name_range [source_range]
   │  │  │  │  │     ├─ begin (offset): 8018
   │  │  │  │  │     ╰─ end (offset): 8039
   │  │  │  │  ├─ full_name: thrift.AllowReservedFilename
   │  │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_131
   │  │  │  │     ╰─ full_name: thrift.AllowReservedFilename
   │  │  │  ╰─ fields (size: 0)
   │  │  ├─ structs_and_unions[16] [t_structured] @0xNORMALIZED_134
   │  │  │  ├─ (base) [t_type] @0xNORMALIZED_134
   │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_134
   │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_134
   │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  ├─ begin (offset): 8168
   │  │  │  │  │  │  │  ╰─ end (offset): 8209
   │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  ├─ name: RuntimeAnnotation
   │  │  │  │  │  ├─ scoped_name: thrift.RuntimeAnnotation
   │  │  │  │  │  ├─ uri: facebook.com/thrift/annotation/RuntimeAnnotation
   │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  ├─ structured_annotations (size: 1)
   │  │  │  │  │  │  ╰─ structured_annotations[0] [t_const]
   │  │  │  │  │  │     ├─ (base) [t_named] @0xNORMALIZED_135
   │  │  │  │  │  │     │  ├─ (base) [t_node] @0xNORMALIZED_135
   │  │  │  │  │  │     │  │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  │  ├─ begin (offset): 8168
   │  │  │  │  │  │     │  │  │  ╰─ end (offset): 8181
   │  │  │  │  │  │     │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ name: 
   │  │  │  │  │  │     │  ├─ scoped_name: thrift.
   │  │  │  │  │  │     │  ├─ uri: 
   │  │  │  │  │  │     │  ├─ explicit_uri? false
   │  │  │  │  │  │     │  ├─ generated? false
   │  │  │  │  │  │     │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ has_doc? false
   │  │  │  │  │  │     │  ├─ doc_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 0
   │  │  │  │  │  │     │  │  ╰─ end (offset): 0
   │  │  │  │  │  │     │  ╰─ name_range: <absent>
   │  │  │  │  │  │     ├─ type_ref [t_type_ref]
   │  │  │  │  │  │     │  ├─ empty? false
   │  │  │  │  │  │     │  ├─ resolved? true
   │  │  │  │  │  │     │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 8168
   │  │  │  │  │  │     │  │  ╰─ end (offset): 8181
   │  │  │  │  │  │     │  ├─ type: [t_type*] 0xNORMALIZED_74
   │  │  │  │  │  │     │  │  ╰─ full_name: scope.Struct
   │  │  │  │  │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │     ╰─ value [t_const_value*] 0xNORMALIZED_136
   │  │  │  │  │  │        ├─ src_range: <absent>
   │  │  │  │  │  │        ├─ is_empty? true
   │  │  │  │  │  │        ╰─ kind: map({})
   │  │  │  │  │  ├─ has_doc? true
   │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  ├─ begin (offset): 8044
   │  │  │  │  │  │  ╰─ end (offset): 8167
   │  │  │  │  │  ╰─ name_range [source_range]
   │  │  │  │  │     ├─ begin (offset): 8189
   │  │  │  │  │     ╰─ end (offset): 8206
   │  │  │  │  ├─ full_name: thrift.RuntimeAnnotation
   │  │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_134
   │  │  │  │     ╰─ full_name: thrift.RuntimeAnnotation
   │  │  │  ╰─ fields (size: 0)
   │  │  ├─ structs_and_unions[17] [t_structured] @0xNORMALIZED_137
   │  │  │  ├─ (base) [t_type] @0xNORMALIZED_137
   │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_137
   │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_137
   │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  ├─ begin (offset): 9540
   │  │  │  │  │  │  │  ╰─ end (offset): 9586
   │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  ├─ name: AllowLegacyTypedefUri
   │  │  │  │  │  ├─ scoped_name: thrift.AllowLegacyTypedefUri
   │  │  │  │  │  ├─ uri: facebook.com/thrift/annotation/AllowLegacyTypedefUri
   │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  ├─ structured_annotations (size: 1)
   │  │  │  │  │  │  ╰─ structured_annotations[0] [t_const]
   │  │  │  │  │  │     ├─ (base) [t_named] @0xNORMALIZED_138
   │  │  │  │  │  │     │  ├─ (base) [t_node] @0xNORMALIZED_138
   │  │  │  │  │  │     │  │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  │  ├─ begin (offset): 9540
   │  │  │  │  │  │     │  │  │  ╰─ end (offset): 9554
   │  │  │  │  │  │     │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ name: 
   │  │  │  │  │  │     │  ├─ scoped_name: thrift.
   │  │  │  │  │  │     │  ├─ uri: 
   │  │  │  │  │  │     │  ├─ explicit_uri? false
   │  │  │  │  │  │     │  ├─ generated? false
   │  │  │  │  │  │     │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ has_doc? false
   │  │  │  │  │  │     │  ├─ doc_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 0
   │  │  │  │  │  │     │  │  ╰─ end (offset): 0
   │  │  │  │  │  │     │  ╰─ name_range: <absent>
   │  │  │  │  │  │     ├─ type_ref [t_type_ref]
   │  │  │  │  │  │     │  ├─ empty? false
   │  │  │  │  │  │     │  ├─ resolved? true
   │  │  │  │  │  │     │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 9540
   │  │  │  │  │  │     │  │  ╰─ end (offset): 9554
   │  │  │  │  │  │     │  ├─ type: [t_type*] 0xNORMALIZED_139
   │  │  │  │  │  │     │  │  ╰─ full_name: scope.Typedef
   │  │  │  │  │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │     ╰─ value [t_const_value*] 0xNORMALIZED_140
   │  │  │  │  │  │        ├─ src_range: <absent>
   │  │  │  │  │  │        ├─ is_empty? true
   │  │  │  │  │  │        ╰─ kind: map({})
   │  │  │  │  │  ├─ has_doc? true
   │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  ├─ begin (offset): 8211
   │  │  │  │  │  │  ╰─ end (offset): 9539
   │  │  │  │  │  ╰─ name_range [source_range]
   │  │  │  │  │     ├─ begin (offset): 9562
   │  │  │  │  │     ╰─ end (offset): 9583
   │  │  │  │  ├─ full_name: thrift.AllowLegacyTypedefUri
   │  │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_137
   │  │  │  │     ╰─ full_name: thrift.AllowLegacyTypedefUri
   │  │  │  ╰─ fields (size: 0)
   │  │  ├─ structs_and_unions[18] [t_structured] @0xNORMALIZED_141
   │  │  │  ├─ (base) [t_type] @0xNORMALIZED_141
   │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_141
   │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_141
   │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  ├─ begin (offset): 10830
   │  │  │  │  │  │  │  ╰─ end (offset): 10890
   │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  ├─ name: AllowUnsafeOptionalCustomDefaultValue
   │  │  │  │  │  ├─ scoped_name: thrift.AllowUnsafeOptionalCustomDefaultValue
   │  │  │  │  │  ├─ uri: facebook.com/thrift/annotation/AllowUnsafeOptionalCustomDefaultValue
   │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  ├─ structured_annotations (size: 1)
   │  │  │  │  │  │  ╰─ structured_annotations[0] [t_const]
   │  │  │  │  │  │     ├─ (base) [t_named] @0xNORMALIZED_142
   │  │  │  │  │  │     │  ├─ (base) [t_node] @0xNORMALIZED_142
   │  │  │  │  │  │     │  │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  │  ├─ begin (offset): 10830
   │  │  │  │  │  │     │  │  │  ╰─ end (offset): 10842
   │  │  │  │  │  │     │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ name: 
   │  │  │  │  │  │     │  ├─ scoped_name: thrift.
   │  │  │  │  │  │     │  ├─ uri: 
   │  │  │  │  │  │     │  ├─ explicit_uri? false
   │  │  │  │  │  │     │  ├─ generated? false
   │  │  │  │  │  │     │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ has_doc? false
   │  │  │  │  │  │     │  ├─ doc_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 0
   │  │  │  │  │  │     │  │  ╰─ end (offset): 0
   │  │  │  │  │  │     │  ╰─ name_range: <absent>
   │  │  │  │  │  │     ├─ type_ref [t_type_ref]
   │  │  │  │  │  │     │  ├─ empty? false
   │  │  │  │  │  │     │  ├─ resolved? true
   │  │  │  │  │  │     │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 10830
   │  │  │  │  │  │     │  │  ╰─ end (offset): 10842
   │  │  │  │  │  │     │  ├─ type: [t_type*] 0xNORMALIZED_80
   │  │  │  │  │  │     │  │  ╰─ full_name: scope.Field
   │  │  │  │  │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │     ╰─ value [t_const_value*] 0xNORMALIZED_143
   │  │  │  │  │  │        ├─ src_range: <absent>
   │  │  │  │  │  │        ├─ is_empty? true
   │  │  │  │  │  │        ╰─ kind: map({})
   │  │  │  │  │  ├─ has_doc? true
   │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  ├─ begin (offset): 9588
   │  │  │  │  │  │  ╰─ end (offset): 10829
   │  │  │  │  │  ╰─ name_range [source_range]
   │  │  │  │  │     ├─ begin (offset): 10850
   │  │  │  │  │     ╰─ end (offset): 10887
   │  │  │  │  ├─ full_name: thrift.AllowUnsafeOptionalCustomDefaultValue
   │  │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_141
   │  │  │  │     ╰─ full_name: thrift.AllowUnsafeOptionalCustomDefaultValue
   │  │  │  ╰─ fields (size: 0)
   │  │  ├─ structs_and_unions[19] [t_structured] @0xNORMALIZED_144
   │  │  │  ├─ (base) [t_type] @0xNORMALIZED_144
   │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_144
   │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_144
   │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  ├─ begin (offset): 11616
   │  │  │  │  │  │  │  ╰─ end (offset): 11678
   │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  ├─ name: AllowUnsafeUnionFieldCustomDefaultValue
   │  │  │  │  │  ├─ scoped_name: thrift.AllowUnsafeUnionFieldCustomDefaultValue
   │  │  │  │  │  ├─ uri: facebook.com/thrift/annotation/AllowUnsafeUnionFieldCustomDefaultValue
   │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  ├─ structured_annotations (size: 1)
   │  │  │  │  │  │  ╰─ structured_annotations[0] [t_const]
   │  │  │  │  │  │     ├─ (base) [t_named] @0xNORMALIZED_145
   │  │  │  │  │  │     │  ├─ (base) [t_node] @0xNORMALIZED_145
   │  │  │  │  │  │     │  │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  │  ├─ begin (offset): 11616
   │  │  │  │  │  │     │  │  │  ╰─ end (offset): 11628
   │  │  │  │  │  │     │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ name: 
   │  │  │  │  │  │     │  ├─ scoped_name: thrift.
   │  │  │  │  │  │     │  ├─ uri: 
   │  │  │  │  │  │     │  ├─ explicit_uri? false
   │  │  │  │  │  │     │  ├─ generated? false
   │  │  │  │  │  │     │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ has_doc? false
   │  │  │  │  │  │     │  ├─ doc_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 0
   │  │  │  │  │  │     │  │  ╰─ end (offset): 0
   │  │  │  │  │  │     │  ╰─ name_range: <absent>
   │  │  │  │  │  │     ├─ type_ref [t_type_ref]
   │  │  │  │  │  │     │  ├─ empty? false
   │  │  │  │  │  │     │  ├─ resolved? true
   │  │  │  │  │  │     │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 11616
   │  │  │  │  │  │     │  │  ╰─ end (offset): 11628
   │  │  │  │  │  │     │  ├─ type: [t_type*] 0xNORMALIZED_80
   │  │  │  │  │  │     │  │  ╰─ full_name: scope.Field
   │  │  │  │  │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │     ╰─ value [t_const_value*] 0xNORMALIZED_146
   │  │  │  │  │  │        ├─ src_range: <absent>
   │  │  │  │  │  │        ├─ is_empty? true
   │  │  │  │  │  │        ╰─ kind: map({})
   │  │  │  │  │  ├─ has_doc? true
   │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  ├─ begin (offset): 10892
   │  │  │  │  │  │  ╰─ end (offset): 11615
   │  │  │  │  │  ╰─ name_range [source_range]
   │  │  │  │  │     ├─ begin (offset): 11636
   │  │  │  │  │     ╰─ end (offset): 11675
   │  │  │  │  ├─ full_name: thrift.AllowUnsafeUnionFieldCustomDefaultValue
   │  │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_144
   │  │  │  │     ╰─ full_name: thrift.AllowUnsafeUnionFieldCustomDefaultValue
   │  │  │  ╰─ fields (size: 0)
   │  │  ├─ structs_and_unions[20] [t_structured] @0xNORMALIZED_147
   │  │  │  ├─ (base) [t_type] @0xNORMALIZED_147
   │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_147
   │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_147
   │  │  │  │  │  │  ├─ src_range [source_range]
   │  │  │  │  │  │  │  ├─ begin (offset): 12268
   │  │  │  │  │  │  │  ╰─ end (offset): 12324
   │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  ├─ name: AllowUnsafeRequiredFieldQualifier
   │  │  │  │  │  ├─ scoped_name: thrift.AllowUnsafeRequiredFieldQualifier
   │  │  │  │  │  ├─ uri: facebook.com/thrift/annotation/AllowUnsafeRequiredFieldQualifier
   │  │  │  │  │  ├─ explicit_uri? false
   │  │  │  │  │  ├─ generated? false
   │  │  │  │  │  ├─ structured_annotations (size: 1)
   │  │  │  │  │  │  ╰─ structured_annotations[0] [t_const]
   │  │  │  │  │  │     ├─ (base) [t_named] @0xNORMALIZED_148
   │  │  │  │  │  │     │  ├─ (base) [t_node] @0xNORMALIZED_148
   │  │  │  │  │  │     │  │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  │  ├─ begin (offset): 12268
   │  │  │  │  │  │     │  │  │  ╰─ end (offset): 12280
   │  │  │  │  │  │     │  │  ╰─ unstructured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ name: 
   │  │  │  │  │  │     │  ├─ scoped_name: thrift.
   │  │  │  │  │  │     │  ├─ uri: 
   │  │  │  │  │  │     │  ├─ explicit_uri? false
   │  │  │  │  │  │     │  ├─ generated? false
   │  │  │  │  │  │     │  ├─ structured_annotations (size: 0)
   │  │  │  │  │  │     │  ├─ has_doc? false
   │  │  │  │  │  │     │  ├─ doc_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 0
   │  │  │  │  │  │     │  │  ╰─ end (offset): 0
   │  │  │  │  │  │     │  ╰─ name_range: <absent>
   │  │  │  │  │  │     ├─ type_ref [t_type_ref]
   │  │  │  │  │  │     │  ├─ empty? false
   │  │  │  │  │  │     │  ├─ resolved? true
   │  │  │  │  │  │     │  ├─ src_range [source_range]
   │  │  │  │  │  │     │  │  ├─ begin (offset): 12268
   │  │  │  │  │  │     │  │  ╰─ end (offset): 12280
   │  │  │  │  │  │     │  ├─ type: [t_type*] 0xNORMALIZED_80
   │  │  │  │  │  │     │  │  ╰─ full_name: scope.Field
   │  │  │  │  │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │  │  │  │  │     ╰─ value [t_const_value*] 0xNORMALIZED_149
   │  │  │  │  │  │        ├─ src_range: <absent>
   │  │  │  │  │  │        ├─ is_empty? true
   │  │  │  │  │  │        ╰─ kind: map({})
   │  │  │  │  │  ├─ has_doc? true
   │  │  │  │  │  ├─ doc_range [source_range]
   │  │  │  │  │  │  ├─ begin (offset): 11680
   │  │  │  │  │  │  ╰─ end (offset): 12267
   │  │  │  │  │  ╰─ name_range [source_range]
   │  │  │  │  │     ├─ begin (offset): 12288
   │  │  │  │  │     ╰─ end (offset): 12321
   │  │  │  │  ├─ full_name: thrift.AllowUnsafeRequiredFieldQualifier
   │  │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_147
   │  │  │  │     ╰─ full_name: thrift.AllowUnsafeRequiredFieldQualifier
   │  │  │  ╰─ fields (size: 0)
   │  │  ╰─ structs_and_unions[21] [t_structured] @0xNORMALIZED_150
   │  │     ├─ (base) [t_type] @0xNORMALIZED_150
   │  │     │  ├─ (base) [t_named] @0xNORMALIZED_150
   │  │     │  │  ├─ (base) [t_node] @0xNORMALIZED_150
   │  │     │  │  │  ├─ src_range [source_range]
   │  │     │  │  │  │  ├─ begin (offset): 14327
   │  │     │  │  │  │  ╰─ end (offset): 14404
   │  │     │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │     │  │  ├─ name: AllowLegacyMissingUris
   │  │     │  │  ├─ scoped_name: thrift.AllowLegacyMissingUris
   │  │     │  │  ├─ uri: facebook.com/thrift/annotation/AllowLegacyMissingUris
   │  │     │  │  ├─ explicit_uri? false
   │  │     │  │  ├─ generated? false
   │  │     │  │  ├─ structured_annotations (size: 3)
   │  │     │  │  │  ├─ structured_annotations[0] [t_const]
   │  │     │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_151
   │  │     │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_151
   │  │     │  │  │  │  │  │  ├─ src_range [source_range]
   │  │     │  │  │  │  │  │  │  ├─ begin (offset): 14327
   │  │     │  │  │  │  │  │  │  ╰─ end (offset): 14341
   │  │     │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │     │  │  │  │  │  ├─ name: 
   │  │     │  │  │  │  │  ├─ scoped_name: thrift.
   │  │     │  │  │  │  │  ├─ uri: 
   │  │     │  │  │  │  │  ├─ explicit_uri? false
   │  │     │  │  │  │  │  ├─ generated? false
   │  │     │  │  │  │  │  ├─ structured_annotations (size: 0)
   │  │     │  │  │  │  │  ├─ has_doc? false
   │  │     │  │  │  │  │  ├─ doc_range [source_range]
   │  │     │  │  │  │  │  │  ├─ begin (offset): 0
   │  │     │  │  │  │  │  │  ╰─ end (offset): 0
   │  │     │  │  │  │  │  ╰─ name_range: <absent>
   │  │     │  │  │  │  ├─ type_ref [t_type_ref]
   │  │     │  │  │  │  │  ├─ empty? false
   │  │     │  │  │  │  │  ├─ resolved? true
   │  │     │  │  │  │  │  ├─ src_range [source_range]
   │  │     │  │  │  │  │  │  ├─ begin (offset): 14327
   │  │     │  │  │  │  │  │  ╰─ end (offset): 14341
   │  │     │  │  │  │  │  ├─ type: [t_type*] 0xNORMALIZED_46
   │  │     │  │  │  │  │  │  ╰─ full_name: scope.Program
   │  │     │  │  │  │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │     │  │  │  │  ╰─ value [t_const_value*] 0xNORMALIZED_152
   │  │     │  │  │  │     ├─ src_range: <absent>
   │  │     │  │  │  │     ├─ is_empty? true
   │  │     │  │  │  │     ╰─ kind: map({})
   │  │     │  │  │  ├─ structured_annotations[1] [t_const]
   │  │     │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_153
   │  │     │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_153
   │  │     │  │  │  │  │  │  ├─ src_range [source_range]
   │  │     │  │  │  │  │  │  │  ├─ begin (offset): 14342
   │  │     │  │  │  │  │  │  │  ╰─ end (offset): 14359
   │  │     │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
   │  │     │  │  │  │  │  ├─ name: 
   │  │     │  │  │  │  │  ├─ scoped_name: thrift.
   │  │     │  │  │  │  │  ├─ uri: 
   │  │     │  │  │  │  │  ├─ explicit_uri? false
   │  │     │  │  │  │  │  ├─ generated? false
   │  │     │  │  │  │  │  ├─ structured_annotations (size: 0)
   │  │     │  │  │  │  │  ├─ has_doc? false
   │  │     │  │  │  │  │  ├─ doc_range [source_range]
   │  │     │  │  │  │  │  │  ├─ begin (offset): 0
   │  │     │  │  │  │  │  │  ╰─ end (offset): 0
   │  │     │  │  │  │  │  ╰─ name_range: <absent>
   │  │     │  │  │  │  ├─ type_ref [t_type_ref]
   │  │     │  │  │  │  │  ├─ empty? false
   │  │     │  │  │  │  │  ├─ resolved? true
   │  │     │  │  │  │  │  ├─ src_range [source_range]
   │  │     │  │  │  │  │  │  ├─ begin (offset): 14342
   │  │     │  │  │  │  │  │  ╰─ end (offset): 14359
   │  │     │  │  │  │  │  ├─ type: [t_type*] 0xNORMALIZED_53
   │  │     │  │  │  │  │  │  ╰─ full_name: scope.Structured
   │  │     │  │  │  │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │     │  │  │  │  ╰─ value [t_const_value*] 0xNORMALIZED_154
   │  │     │  │  │  │     ├─ src_range: <absent>
   │  │     │  │  │  │     ├─ is_empty? true
   │  │     │  │  │  │     ╰─ kind: map({})
   │  │     │  │  │  ╰─ structured_annotations[2] [t_const]
   │  │     │  │  │     ├─ (base) [t_named] @0xNORMALIZED_155
   │  │     │  │  │     │  ├─ (base) [t_node] @0xNORMALIZED_155
   │  │     │  │  │     │  │  ├─ src_range [source_range]
   │  │     │  │  │     │  │  │  ├─ begin (offset): 14360
   │  │     │  │  │     │  │  │  ╰─ end (offset): 14371
   │  │     │  │  │     │  │  ╰─ unstructured_annotations (size: 0)
   │  │     │  │  │     │  ├─ name: 
   │  │     │  │  │     │  ├─ scoped_name: thrift.
   │  │     │  │  │     │  ├─ uri: 
   │  │     │  │  │     │  ├─ explicit_uri? false
   │  │     │  │  │     │  ├─ generated? false
   │  │     │  │  │     │  ├─ structured_annotations (size: 0)
   │  │     │  │  │     │  ├─ has_doc? false
   │  │     │  │  │     │  ├─ doc_range [source_range]
   │  │     │  │  │     │  │  ├─ begin (offset): 0
   │  │     │  │  │     │  │  ╰─ end (offset): 0
   │  │     │  │  │     │  ╰─ name_range: <absent>
   │  │     │  │  │     ├─ type_ref [t_type_ref]
   │  │     │  │  │     │  ├─ empty? false
   │  │     │  │  │     │  ├─ resolved? true
   │  │     │  │  │     │  ├─ src_range [source_range]
   │  │     │  │  │     │  │  ├─ begin (offset): 14360
   │  │     │  │  │     │  │  ╰─ end (offset): 14371
   │  │     │  │  │     │  ├─ type: [t_type*] 0xNORMALIZED_56
   │  │     │  │  │     │  │  ╰─ full_name: scope.Enum
   │  │     │  │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
   │  │     │  │  │     ╰─ value [t_const_value*] 0xNORMALIZED_156
   │  │     │  │  │        ├─ src_range: <absent>
   │  │     │  │  │        ├─ is_empty? true
   │  │     │  │  │        ╰─ kind: map({})
   │  │     │  │  ├─ has_doc? true
   │  │     │  │  ├─ doc_range [source_range]
   │  │     │  │  │  ├─ begin (offset): 12326
   │  │     │  │  │  ╰─ end (offset): 14326
   │  │     │  │  ╰─ name_range [source_range]
   │  │     │  │     ├─ begin (offset): 14379
   │  │     │  │     ╰─ end (offset): 14401
   │  │     │  ├─ full_name: thrift.AllowLegacyMissingUris
   │  │     │  ╰─ true_type: [t_type*] 0xNORMALIZED_150
   │  │     │     ╰─ full_name: thrift.AllowLegacyMissingUris
   │  │     ╰─ fields (size: 0)
   │  ├─ exceptions (size: 0)
   │  ├─ services (size: 0)
   │  ├─ interactions (size: 0)
   │  ├─ definitions (size: 23)
   │  │  ├─ definitions[0]: [t_named*] 0xNORMALIZED_15
   │  │  │  ╰─ scoped_name: thrift.Experimental
   │  │  ├─ definitions[1]: [t_named*] 0xNORMALIZED_51
   │  │  │  ╰─ scoped_name: thrift.ReserveIds
   │  │  ├─ definitions[2]: [t_named*] 0xNORMALIZED_62
   │  │  │  ╰─ scoped_name: thrift.RequiresBackwardCompatibility
   │  │  ├─ definitions[3]: [t_named*] 0xNORMALIZED_70
   │  │  │  ╰─ scoped_name: thrift.TerseWrite
   │  │  ├─ definitions[4]: [t_named*] 0xNORMALIZED_82
   │  │  │  ╰─ scoped_name: thrift.Box
   │  │  ├─ definitions[5]: [t_named*] 0xNORMALIZED_85
   │  │  │  ╰─ scoped_name: thrift.Mixin
   │  │  ├─ definitions[6]: [t_named*] 0xNORMALIZED_88
   │  │  │  ╰─ scoped_name: thrift.SerializeInFieldIdOrder
   │  │  ├─ definitions[7]: [t_named*] 0xNORMALIZED_93
   │  │  │  ╰─ scoped_name: thrift.BitmaskEnum
   │  │  ├─ definitions[8]: [t_named*] 0xNORMALIZED_96
   │  │  │  ╰─ scoped_name: thrift.ExceptionMessage
   │  │  ├─ definitions[9]: [t_named*] 0xNORMALIZED_99
   │  │  │  ╰─ scoped_name: thrift.InternBox
   │  │  ├─ definitions[10]: [t_named*] 0xNORMALIZED_104
   │  │  │  ╰─ scoped_name: thrift.Serial
   │  │  ├─ definitions[11]: [t_named*] 0xNORMALIZED_108
   │  │  │  ╰─ scoped_name: thrift.Uri
   │  │  ├─ definitions[12]: [t_named*] 0xNORMALIZED_118
   │  │  │  ╰─ scoped_name: thrift.Priority
   │  │  ├─ definitions[13]: [t_named*] 0xNORMALIZED_5
   │  │  │  ╰─ scoped_name: thrift.RpcPriority
   │  │  ├─ definitions[14]: [t_named*] 0xNORMALIZED_123
   │  │  │  ╰─ scoped_name: thrift.DeprecatedUnvalidatedAnnotations
   │  │  ├─ definitions[15]: [t_named*] 0xNORMALIZED_128
   │  │  │  ╰─ scoped_name: thrift.AllowReservedIdentifier
   │  │  ├─ definitions[16]: [t_named*] 0xNORMALIZED_131
   │  │  │  ╰─ scoped_name: thrift.AllowReservedFilename
   │  │  ├─ definitions[17]: [t_named*] 0xNORMALIZED_134
   │  │  │  ╰─ scoped_name: thrift.RuntimeAnnotation
   │  │  ├─ definitions[18]: [t_named*] 0xNORMALIZED_137
   │  │  │  ╰─ scoped_name: thrift.AllowLegacyTypedefUri
   │  │  ├─ definitions[19]: [t_named*] 0xNORMALIZED_141
   │  │  │  ╰─ scoped_name: thrift.AllowUnsafeOptionalCustomDefaultValue
   │  │  ├─ definitions[20]: [t_named*] 0xNORMALIZED_144
   │  │  │  ╰─ scoped_name: thrift.AllowUnsafeUnionFieldCustomDefaultValue
   │  │  ├─ definitions[21]: [t_named*] 0xNORMALIZED_147
   │  │  │  ╰─ scoped_name: thrift.AllowUnsafeRequiredFieldQualifier
   │  │  ╰─ definitions[22]: [t_named*] 0xNORMALIZED_150
   │  │     ╰─ scoped_name: thrift.AllowLegacyMissingUris
   │  ├─ structured_definitions (size: 22)
   │  │  ├─ structured_definitions[0]: [t_structured*] 0xNORMALIZED_15
   │  │  │  ╰─ name: Experimental
   │  │  ├─ structured_definitions[1]: [t_structured*] 0xNORMALIZED_51
   │  │  │  ╰─ name: ReserveIds
   │  │  ├─ structured_definitions[2]: [t_structured*] 0xNORMALIZED_62
   │  │  │  ╰─ name: RequiresBackwardCompatibility
   │  │  ├─ structured_definitions[3]: [t_structured*] 0xNORMALIZED_70
   │  │  │  ╰─ name: TerseWrite
   │  │  ├─ structured_definitions[4]: [t_structured*] 0xNORMALIZED_82
   │  │  │  ╰─ name: Box
   │  │  ├─ structured_definitions[5]: [t_structured*] 0xNORMALIZED_85
   │  │  │  ╰─ name: Mixin
   │  │  ├─ structured_definitions[6]: [t_structured*] 0xNORMALIZED_88
   │  │  │  ╰─ name: SerializeInFieldIdOrder
   │  │  ├─ structured_definitions[7]: [t_structured*] 0xNORMALIZED_93
   │  │  │  ╰─ name: BitmaskEnum
   │  │  ├─ structured_definitions[8]: [t_structured*] 0xNORMALIZED_96
   │  │  │  ╰─ name: ExceptionMessage
   │  │  ├─ structured_definitions[9]: [t_structured*] 0xNORMALIZED_99
   │  │  │  ╰─ name: InternBox
   │  │  ├─ structured_definitions[10]: [t_structured*] 0xNORMALIZED_104
   │  │  │  ╰─ name: Serial
   │  │  ├─ structured_definitions[11]: [t_structured*] 0xNORMALIZED_108
   │  │  │  ╰─ name: Uri
   │  │  ├─ structured_definitions[12]: [t_structured*] 0xNORMALIZED_118
   │  │  │  ╰─ name: Priority
   │  │  ├─ structured_definitions[13]: [t_structured*] 0xNORMALIZED_123
   │  │  │  ╰─ name: DeprecatedUnvalidatedAnnotations
   │  │  ├─ structured_definitions[14]: [t_structured*] 0xNORMALIZED_128
   │  │  │  ╰─ name: AllowReservedIdentifier
   │  │  ├─ structured_definitions[15]: [t_structured*] 0xNORMALIZED_131
   │  │  │  ╰─ name: AllowReservedFilename
   │  │  ├─ structured_definitions[16]: [t_structured*] 0xNORMALIZED_134
   │  │  │  ╰─ name: RuntimeAnnotation
   │  │  ├─ structured_definitions[17]: [t_structured*] 0xNORMALIZED_137
   │  │  │  ╰─ name: AllowLegacyTypedefUri
   │  │  ├─ structured_definitions[18]: [t_structured*] 0xNORMALIZED_141
   │  │  │  ╰─ name: AllowUnsafeOptionalCustomDefaultValue
   │  │  ├─ structured_definitions[19]: [t_structured*] 0xNORMALIZED_144
   │  │  │  ╰─ name: AllowUnsafeUnionFieldCustomDefaultValue
   │  │  ├─ structured_definitions[20]: [t_structured*] 0xNORMALIZED_147
   │  │  │  ╰─ name: AllowUnsafeRequiredFieldQualifier
   │  │  ╰─ structured_definitions[21]: [t_structured*] 0xNORMALIZED_150
   │  │     ╰─ name: AllowLegacyMissingUris
   │  ╰─ type_instantiations (size: 3)
   │     ├─ type_instantiations[0] [t_container] @0xNORMALIZED_59
   │     │  ╰─ (base) [t_type] @0xNORMALIZED_59
   │     │     ├─ (base) [t_named] @0xNORMALIZED_59
   │     │     │  ├─ (base) [t_node] @0xNORMALIZED_59
   │     │     │  │  ├─ src_range [source_range]
   │     │     │  │  │  ├─ begin (offset): 2105
   │     │     │  │  │  ╰─ end (offset): 2114
   │     │     │  │  ╰─ unstructured_annotations (size: 0)
   │     │     │  ├─ name: 
   │     │     │  ├─ scoped_name: 
   │     │     │  ├─ uri: 
   │     │     │  ├─ explicit_uri? false
   │     │     │  ├─ generated? false
   │     │     │  ├─ structured_annotations (size: 0)
   │     │     │  ├─ has_doc? false
   │     │     │  ├─ doc_range [source_range]
   │     │     │  │  ├─ begin (offset): 0
   │     │     │  │  ╰─ end (offset): 0
   │     │     │  ╰─ name_range: <absent>
   │     │     ├─ full_name: list<i32>
   │     │     ╰─ true_type: [t_type*] 0xNORMALIZED_59
   │     │        ╰─ full_name: list<i32>
   │     ├─ type_instantiations[1] [t_container] @0xNORMALIZED_61
   │     │  ╰─ (base) [t_type] @0xNORMALIZED_61
   │     │     ├─ (base) [t_named] @0xNORMALIZED_61
   │     │     │  ├─ (base) [t_node] @0xNORMALIZED_61
   │     │     │  │  ├─ src_range [source_range]
   │     │     │  │  │  ├─ begin (offset): 2435
   │     │     │  │  │  ╰─ end (offset): 2448
   │     │     │  │  ╰─ unstructured_annotations (size: 0)
   │     │     │  ├─ name: 
   │     │     │  ├─ scoped_name: 
   │     │     │  ├─ uri: 
   │     │     │  ├─ explicit_uri? false
   │     │     │  ├─ generated? false
   │     │     │  ├─ structured_annotations (size: 0)
   │     │     │  ├─ has_doc? false
   │     │     │  ├─ doc_range [source_range]
   │     │     │  │  ├─ begin (offset): 0
   │     │     │  │  ╰─ end (offset): 0
   │     │     │  ╰─ name_range: <absent>
   │     │     ├─ full_name: map<i32, i32>
   │     │     ╰─ true_type: [t_type*] 0xNORMALIZED_61
   │     │        ╰─ full_name: map<i32, i32>
   │     ╰─ type_instantiations[2] [t_container] @0xNORMALIZED_127
   │        ╰─ (base) [t_type] @0xNORMALIZED_127
   │           ├─ (base) [t_named] @0xNORMALIZED_127
   │           │  ├─ (base) [t_node] @0xNORMALIZED_127
   │           │  │  ├─ src_range [source_range]
   │           │  │  │  ├─ begin (offset): 7508
   │           │  │  │  ╰─ end (offset): 7527
   │           │  │  ╰─ unstructured_annotations (size: 0)
   │           │  ├─ name: 
   │           │  ├─ scoped_name: 
   │           │  ├─ uri: 
   │           │  ├─ explicit_uri? false
   │           │  ├─ generated? false
   │           │  ├─ structured_annotations (size: 0)
   │           │  ├─ has_doc? false
   │           │  ├─ doc_range [source_range]
   │           │  │  ├─ begin (offset): 0
   │           │  │  ╰─ end (offset): 0
   │           │  ╰─ name_range: <absent>
   │           ├─ full_name: map<string, string>
   │           ╰─ true_type: [t_type*] 0xNORMALIZED_127
   │              ╰─ full_name: map<string, string>
   ╰─ programs[2] [t_program] @0xNORMALIZED_29
      ├─ (base) [t_named] @0xNORMALIZED_29
      │  ├─ (base) [t_node] @0xNORMALIZED_29
      │  │  ├─ src_range [source_range]
      │  │  │  ├─ begin (offset): 1069
      │  │  │  ╰─ end (offset): 1109
      │  │  ╰─ unstructured_annotations (size: 0)
      │  ├─ name: scope
      │  ├─ scoped_name: scope
      │  ├─ uri: 
      │  ├─ explicit_uri? false
      │  ├─ generated? false
      │  ├─ structured_annotations (size: 0)
      │  ├─ has_doc? true
      │  ├─ doc_range [source_range]
      │  │  ├─ begin (offset): 630
      │  │  ╰─ end (offset): 1068
      │  ╰─ name_range: <absent>
      ├─ path: thrift/annotation/scope.thrift
      ├─ full_path: thrift/annotation/scope.thrift
      ├─ include_prefix: thrift/annotation/
      ├─ package [t_package] @0xNORMALIZED_157
      │  ├─ name: facebook.com/thrift/annotation
      │  ├─ is_explicit? true
      │  ╰─ empty? false
      ├─ global_scope [t_global_scope] @0xNORMALIZED_3
      │  ├─ placeholder_typedefs (size: 1)
      │  │  ╰─ placeholder_typedefs[0] [t_placeholder_typedef] @0xNORMALIZED_4
      │  │     ╰─ (base) [t_typedef] @0xNORMALIZED_4
      │  │        ├─ (base) [t_type] @0xNORMALIZED_4
      │  │        │  ├─ (base) [t_named] @0xNORMALIZED_4
      │  │        │  │  ├─ (base) [t_node] @0xNORMALIZED_4
      │  │        │  │  │  ├─ src_range [source_range]
      │  │        │  │  │  │  ├─ begin (offset): 7257
      │  │        │  │  │  │  ╰─ end (offset): 7268
      │  │        │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │        │  │  ├─ name: RpcPriority
      │  │        │  │  ├─ scoped_name: thrift.RpcPriority
      │  │        │  │  ├─ uri: 
      │  │        │  │  ├─ explicit_uri? false
      │  │        │  │  ├─ generated? false
      │  │        │  │  ├─ structured_annotations (size: 0)
      │  │        │  │  ├─ has_doc? false
      │  │        │  │  ├─ doc_range [source_range]
      │  │        │  │  │  ├─ begin (offset): 0
      │  │        │  │  │  ╰─ end (offset): 0
      │  │        │  │  ╰─ name_range: <absent>
      │  │        │  ├─ full_name: thrift.RpcPriority
      │  │        │  ╰─ true_type: [t_type*] 0xNORMALIZED_5
      │  │        │     ╰─ full_name: thrift.RpcPriority
      │  │        ├─ type [t_type_ref]
      │  │        │  ├─ empty? false
      │  │        │  ├─ resolved? true
      │  │        │  ├─ src_range [source_range]
      │  │        │  │  ├─ begin (offset): 0
      │  │        │  │  ╰─ end (offset): 0
      │  │        │  ├─ type: [t_type*] 0xNORMALIZED_5
      │  │        │  │  ╰─ full_name: thrift.RpcPriority
      │  │        │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
      │  │        ╰─ typedef_kind: 2
      │  ├─ resolution_mismatches (size: 0)
      │  ╰─ program_scopes (size: 2)
      │     ├─ program_scope["scope"] (size: 1)
      │     │  ╰─ 0: [program_scope*] 0xNORMALIZED_6
      │     ╰─ program_scope["thrift"] (size: 1)
      │        ╰─ 0: [program_scope*] 0xNORMALIZED_7
      ├─ program_scope [program_scope] @0xNORMALIZED_6
      ├─ includes (size: 0)
      ├─ included_programs (size: 0)
      ├─ includes_for_codegen (size: 0)
      ├─ namespaces (size: 6)
      │  ├─ android: com.facebook.thrift.annotation_deprecated
      │  ├─ go: thrift.annotation.scope
      │  ├─ java: com.facebook.thrift.annotation_deprecated
      │  ├─ js: thrift.annotation.scope
      │  ├─ py: thrift.annotation.scope
      │  ╰─ py.asyncio: facebook_thrift_asyncio.annotation.scope
      ├─ language_includes (size: 0)
      ├─ typedefs (size: 0)
      ├─ enums (size: 0)
      ├─ consts (size: 0)
      ├─ structs_and_unions (size: 20)
      │  ├─ structs_and_unions[0] [t_structured] @0xNORMALIZED_158
      │  │  ├─ (base) [t_type] @0xNORMALIZED_158
      │  │  │  ├─ (base) [t_named] @0xNORMALIZED_158
      │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_158
      │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  ├─ begin (offset): 1934
      │  │  │  │  │  │  ╰─ end (offset): 1954
      │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  ├─ name: Transitive
      │  │  │  │  ├─ scoped_name: scope.Transitive
      │  │  │  │  ├─ uri: facebook.com/thrift/annotation/Transitive
      │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  ├─ generated? false
      │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  ├─ has_doc? true
      │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  ├─ begin (offset): 1402
      │  │  │  │  │  ╰─ end (offset): 1933
      │  │  │  │  ╰─ name_range [source_range]
      │  │  │  │     ├─ begin (offset): 1941
      │  │  │  │     ╰─ end (offset): 1951
      │  │  │  ├─ full_name: scope.Transitive
      │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_158
      │  │  │     ╰─ full_name: scope.Transitive
      │  │  ╰─ fields (size: 0)
      │  ├─ structs_and_unions[1] [t_structured] @0xNORMALIZED_46
      │  │  ├─ (base) [t_type] @0xNORMALIZED_46
      │  │  │  ├─ (base) [t_named] @0xNORMALIZED_46
      │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_46
      │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  ├─ begin (offset): 2108
      │  │  │  │  │  │  ╰─ end (offset): 2125
      │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  ├─ name: Program
      │  │  │  │  ├─ scoped_name: scope.Program
      │  │  │  │  ├─ uri: facebook.com/thrift/annotation/Program
      │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  ├─ generated? false
      │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  ├─ has_doc? true
      │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  ├─ begin (offset): 1956
      │  │  │  │  │  ╰─ end (offset): 2107
      │  │  │  │  ╰─ name_range [source_range]
      │  │  │  │     ├─ begin (offset): 2115
      │  │  │  │     ╰─ end (offset): 2122
      │  │  │  ├─ full_name: scope.Program
      │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_46
      │  │  │     ╰─ full_name: scope.Program
      │  │  ╰─ fields (size: 0)
      │  ├─ structs_and_unions[2] [t_structured] @0xNORMALIZED_74
      │  │  ├─ (base) [t_type] @0xNORMALIZED_74
      │  │  │  ├─ (base) [t_named] @0xNORMALIZED_74
      │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_74
      │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  ├─ begin (offset): 2165
      │  │  │  │  │  │  ╰─ end (offset): 2181
      │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  ├─ name: Struct
      │  │  │  │  ├─ scoped_name: scope.Struct
      │  │  │  │  ├─ uri: facebook.com/thrift/annotation/Struct
      │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  ├─ generated? false
      │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  ├─ has_doc? true
      │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  ├─ begin (offset): 2127
      │  │  │  │  │  ╰─ end (offset): 2164
      │  │  │  │  ╰─ name_range [source_range]
      │  │  │  │     ├─ begin (offset): 2172
      │  │  │  │     ╰─ end (offset): 2178
      │  │  │  ├─ full_name: scope.Struct
      │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_74
      │  │  │     ╰─ full_name: scope.Struct
      │  │  ╰─ fields (size: 0)
      │  ├─ structs_and_unions[3] [t_structured] @0xNORMALIZED_159
      │  │  ├─ (base) [t_type] @0xNORMALIZED_159
      │  │  │  ├─ (base) [t_named] @0xNORMALIZED_159
      │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_159
      │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  ├─ begin (offset): 2220
      │  │  │  │  │  │  ╰─ end (offset): 2235
      │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  ├─ name: Union
      │  │  │  │  ├─ scoped_name: scope.Union
      │  │  │  │  ├─ uri: facebook.com/thrift/annotation/Union
      │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  ├─ generated? false
      │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  ├─ has_doc? true
      │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  ├─ begin (offset): 2183
      │  │  │  │  │  ╰─ end (offset): 2219
      │  │  │  │  ╰─ name_range [source_range]
      │  │  │  │     ├─ begin (offset): 2227
      │  │  │  │     ╰─ end (offset): 2232
      │  │  │  ├─ full_name: scope.Union
      │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_159
      │  │  │     ╰─ full_name: scope.Union
      │  │  ╰─ fields (size: 0)
      │  ├─ structs_and_unions[4] [t_structured] @0xNORMALIZED_77
      │  │  ├─ (base) [t_type] @0xNORMALIZED_77
      │  │  │  ├─ (base) [t_named] @0xNORMALIZED_77
      │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_77
      │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  ├─ begin (offset): 2278
      │  │  │  │  │  │  ╰─ end (offset): 2297
      │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  ├─ name: Exception
      │  │  │  │  ├─ scoped_name: scope.Exception
      │  │  │  │  ├─ uri: facebook.com/thrift/annotation/Exception
      │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  ├─ generated? false
      │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  ├─ has_doc? true
      │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  ├─ begin (offset): 2237
      │  │  │  │  │  ╰─ end (offset): 2277
      │  │  │  │  ╰─ name_range [source_range]
      │  │  │  │     ├─ begin (offset): 2285
      │  │  │  │     ╰─ end (offset): 2294
      │  │  │  ├─ full_name: scope.Exception
      │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_77
      │  │  │     ╰─ full_name: scope.Exception
      │  │  ╰─ fields (size: 0)
      │  ├─ structs_and_unions[5] [t_structured] @0xNORMALIZED_160
      │  │  ├─ (base) [t_type] @0xNORMALIZED_160
      │  │  │  ├─ (base) [t_named] @0xNORMALIZED_160
      │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_160
      │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  ├─ begin (offset): 2349
      │  │  │  │  │  │  ╰─ end (offset): 2374
      │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  ├─ name: ThrownException
      │  │  │  │  ├─ scoped_name: scope.ThrownException
      │  │  │  │  ├─ uri: facebook.com/thrift/annotation/ThrownException
      │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  ├─ generated? false
      │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  ├─ has_doc? true
      │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  ├─ begin (offset): 2299
      │  │  │  │  │  ╰─ end (offset): 2348
      │  │  │  │  ╰─ name_range [source_range]
      │  │  │  │     ├─ begin (offset): 2356
      │  │  │  │     ╰─ end (offset): 2371
      │  │  │  ├─ full_name: scope.ThrownException
      │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_160
      │  │  │     ╰─ full_name: scope.ThrownException
      │  │  ╰─ fields (size: 0)
      │  ├─ structs_and_unions[6] [t_structured] @0xNORMALIZED_80
      │  │  ├─ (base) [t_type] @0xNORMALIZED_80
      │  │  │  ├─ (base) [t_named] @0xNORMALIZED_80
      │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_80
      │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  ├─ begin (offset): 2440
      │  │  │  │  │  │  ╰─ end (offset): 2455
      │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  ├─ name: Field
      │  │  │  │  ├─ scoped_name: scope.Field
      │  │  │  │  ├─ uri: facebook.com/thrift/annotation/Field
      │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  ├─ generated? false
      │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  ├─ has_doc? true
      │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  ├─ begin (offset): 2376
      │  │  │  │  │  ╰─ end (offset): 2439
      │  │  │  │  ╰─ name_range [source_range]
      │  │  │  │     ├─ begin (offset): 2447
      │  │  │  │     ╰─ end (offset): 2452
      │  │  │  ├─ full_name: scope.Field
      │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_80
      │  │  │     ╰─ full_name: scope.Field
      │  │  ╰─ fields (size: 0)
      │  ├─ structs_and_unions[7] [t_structured] @0xNORMALIZED_139
      │  │  ├─ (base) [t_type] @0xNORMALIZED_139
      │  │  │  ├─ (base) [t_named] @0xNORMALIZED_139
      │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_139
      │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  ├─ begin (offset): 2496
      │  │  │  │  │  │  ╰─ end (offset): 2513
      │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  ├─ name: Typedef
      │  │  │  │  ├─ scoped_name: scope.Typedef
      │  │  │  │  ├─ uri: facebook.com/thrift/annotation/Typedef
      │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  ├─ generated? false
      │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  ├─ has_doc? true
      │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  ├─ begin (offset): 2457
      │  │  │  │  │  ╰─ end (offset): 2495
      │  │  │  │  ╰─ name_range [source_range]
      │  │  │  │     ├─ begin (offset): 2503
      │  │  │  │     ╰─ end (offset): 2510
      │  │  │  ├─ full_name: scope.Typedef
      │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_139
      │  │  │     ╰─ full_name: scope.Typedef
      │  │  ╰─ fields (size: 0)
      │  ├─ structs_and_unions[8] [t_structured] @0xNORMALIZED_112
      │  │  ├─ (base) [t_type] @0xNORMALIZED_112
      │  │  │  ├─ (base) [t_named] @0xNORMALIZED_112
      │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_112
      │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  ├─ begin (offset): 2554
      │  │  │  │  │  │  ╰─ end (offset): 2571
      │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  ├─ name: Service
      │  │  │  │  ├─ scoped_name: scope.Service
      │  │  │  │  ├─ uri: facebook.com/thrift/annotation/Service
      │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  ├─ generated? false
      │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  ├─ has_doc? true
      │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  ├─ begin (offset): 2515
      │  │  │  │  │  ╰─ end (offset): 2553
      │  │  │  │  ╰─ name_range [source_range]
      │  │  │  │     ├─ begin (offset): 2561
      │  │  │  │     ╰─ end (offset): 2568
      │  │  │  ├─ full_name: scope.Service
      │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_112
      │  │  │     ╰─ full_name: scope.Service
      │  │  ╰─ fields (size: 0)
      │  ├─ structs_and_unions[9] [t_structured] @0xNORMALIZED_106
      │  │  ├─ (base) [t_type] @0xNORMALIZED_106
      │  │  │  ├─ (base) [t_named] @0xNORMALIZED_106
      │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_106
      │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  ├─ begin (offset): 2616
      │  │  │  │  │  │  ╰─ end (offset): 2637
      │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  ├─ name: Interaction
      │  │  │  │  ├─ scoped_name: scope.Interaction
      │  │  │  │  ├─ uri: facebook.com/thrift/annotation/Interaction
      │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  ├─ generated? false
      │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  ├─ has_doc? true
      │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  ├─ begin (offset): 2573
      │  │  │  │  │  ╰─ end (offset): 2615
      │  │  │  │  ╰─ name_range [source_range]
      │  │  │  │     ├─ begin (offset): 2623
      │  │  │  │     ╰─ end (offset): 2634
      │  │  │  ├─ full_name: scope.Interaction
      │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_106
      │  │  │     ╰─ full_name: scope.Interaction
      │  │  ╰─ fields (size: 0)
      │  ├─ structs_and_unions[10] [t_structured] @0xNORMALIZED_120
      │  │  ├─ (base) [t_type] @0xNORMALIZED_120
      │  │  │  ├─ (base) [t_named] @0xNORMALIZED_120
      │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_120
      │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  ├─ begin (offset): 2679
      │  │  │  │  │  │  ╰─ end (offset): 2697
      │  │  │  │  │  ╰─ unstructured_annotations (size: 2)
      │  │  │  │  │     ├─ name: hack.name
      │  │  │  │  │     │  ├─ src_range [source_range]
      │  │  │  │  │     │  │  ├─ begin (offset): 0
      │  │  │  │  │     │  │  ╰─ end (offset): 0
      │  │  │  │  │     │  ╰─ value: TFunction
      │  │  │  │  │     ╰─ name: js.name
      │  │  │  │  │        ├─ src_range [source_range]
      │  │  │  │  │        │  ├─ begin (offset): 0
      │  │  │  │  │        │  ╰─ end (offset): 0
      │  │  │  │  │        ╰─ value: TFunction
      │  │  │  │  ├─ name: Function
      │  │  │  │  ├─ scoped_name: scope.Function
      │  │  │  │  ├─ uri: facebook.com/thrift/annotation/Function
      │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  ├─ generated? false
      │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  ├─ has_doc? true
      │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  ├─ begin (offset): 2639
      │  │  │  │  │  ╰─ end (offset): 2678
      │  │  │  │  ╰─ name_range [source_range]
      │  │  │  │     ├─ begin (offset): 2686
      │  │  │  │     ╰─ end (offset): 2694
      │  │  │  ├─ full_name: scope.Function
      │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_120
      │  │  │     ╰─ full_name: scope.Function
      │  │  ╰─ fields (size: 0)
      │  ├─ structs_and_unions[11] [t_structured] @0xNORMALIZED_161
      │  │  ├─ (base) [t_type] @0xNORMALIZED_161
      │  │  │  ├─ (base) [t_named] @0xNORMALIZED_161
      │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_161
      │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  ├─ begin (offset): 2823
      │  │  │  │  │  │  ╰─ end (offset): 2850
      │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  ├─ name: FunctionParameter
      │  │  │  │  ├─ scoped_name: scope.FunctionParameter
      │  │  │  │  ├─ uri: facebook.com/thrift/annotation/FunctionParameter
      │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  ├─ generated? false
      │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  ├─ has_doc? true
      │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  ├─ begin (offset): 2773
      │  │  │  │  │  ╰─ end (offset): 2822
      │  │  │  │  ╰─ name_range [source_range]
      │  │  │  │     ├─ begin (offset): 2830
      │  │  │  │     ╰─ end (offset): 2847
      │  │  │  ├─ full_name: scope.FunctionParameter
      │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_161
      │  │  │     ╰─ full_name: scope.FunctionParameter
      │  │  ╰─ fields (size: 0)
      │  ├─ structs_and_unions[12] [t_structured] @0xNORMALIZED_162
      │  │  ├─ (base) [t_type] @0xNORMALIZED_162
      │  │  │  ├─ (base) [t_named] @0xNORMALIZED_162
      │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_162
      │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  ├─ begin (offset): 2892
      │  │  │  │  │  │  ╰─ end (offset): 2911
      │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  ├─ name: EnumValue
      │  │  │  │  ├─ scoped_name: scope.EnumValue
      │  │  │  │  ├─ uri: facebook.com/thrift/annotation/EnumValue
      │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  ├─ generated? false
      │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  ├─ has_doc? true
      │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  ├─ begin (offset): 2852
      │  │  │  │  │  ╰─ end (offset): 2891
      │  │  │  │  ╰─ name_range [source_range]
      │  │  │  │     ├─ begin (offset): 2899
      │  │  │  │     ╰─ end (offset): 2908
      │  │  │  ├─ full_name: scope.EnumValue
      │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_162
      │  │  │     ╰─ full_name: scope.EnumValue
      │  │  ╰─ fields (size: 0)
      │  ├─ structs_and_unions[13] [t_structured] @0xNORMALIZED_163
      │  │  ├─ (base) [t_type] @0xNORMALIZED_163
      │  │  │  ├─ (base) [t_named] @0xNORMALIZED_163
      │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_163
      │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  ├─ begin (offset): 2950
      │  │  │  │  │  │  ╰─ end (offset): 2965
      │  │  │  │  │  ╰─ unstructured_annotations (size: 1)
      │  │  │  │  │     ╰─ name: hack.name
      │  │  │  │  │        ├─ src_range [source_range]
      │  │  │  │  │        │  ├─ begin (offset): 0
      │  │  │  │  │        │  ╰─ end (offset): 0
      │  │  │  │  │        ╰─ value: TConst
      │  │  │  │  ├─ name: Const
      │  │  │  │  ├─ scoped_name: scope.Const
      │  │  │  │  ├─ uri: facebook.com/thrift/annotation/Const
      │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  ├─ generated? false
      │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  ├─ has_doc? true
      │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  ├─ begin (offset): 2913
      │  │  │  │  │  ╰─ end (offset): 2949
      │  │  │  │  ╰─ name_range [source_range]
      │  │  │  │     ├─ begin (offset): 2957
      │  │  │  │     ╰─ end (offset): 2962
      │  │  │  ├─ full_name: scope.Const
      │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_163
      │  │  │     ╰─ full_name: scope.Const
      │  │  ╰─ fields (size: 0)
      │  ├─ structs_and_unions[14] [t_structured] @0xNORMALIZED_56
      │  │  ├─ (base) [t_type] @0xNORMALIZED_56
      │  │  │  ├─ (base) [t_named] @0xNORMALIZED_56
      │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_56
      │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  ├─ begin (offset): 3130
      │  │  │  │  │  │  ╰─ end (offset): 3144
      │  │  │  │  │  ╰─ unstructured_annotations (size: 1)
      │  │  │  │  │     ╰─ name: py3.hidden
      │  │  │  │  │        ├─ src_range [source_range]
      │  │  │  │  │        │  ├─ begin (offset): 0
      │  │  │  │  │        │  ╰─ end (offset): 0
      │  │  │  │  │        ╰─ value: 1
      │  │  │  │  ├─ name: Enum
      │  │  │  │  ├─ scoped_name: scope.Enum
      │  │  │  │  ├─ uri: facebook.com/thrift/annotation/Enum
      │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  ├─ generated? false
      │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  ├─ has_doc? false
      │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  ├─ begin (offset): 0
      │  │  │  │  │  ╰─ end (offset): 0
      │  │  │  │  ╰─ name_range [source_range]
      │  │  │  │     ├─ begin (offset): 3137
      │  │  │  │     ╰─ end (offset): 3141
      │  │  │  ├─ full_name: scope.Enum
      │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_56
      │  │  │     ╰─ full_name: scope.Enum
      │  │  ╰─ fields (size: 0)
      │  ├─ structs_and_unions[15] [t_structured] @0xNORMALIZED_53
      │  │  ├─ (base) [t_type] @0xNORMALIZED_53
      │  │  │  ├─ (base) [t_named] @0xNORMALIZED_53
      │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_53
      │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  ├─ begin (offset): 3243
      │  │  │  │  │  │  ╰─ end (offset): 3301
      │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  ├─ name: Structured
      │  │  │  │  ├─ scoped_name: scope.Structured
      │  │  │  │  ├─ uri: facebook.com/thrift/annotation/Structured
      │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  ├─ generated? false
      │  │  │  │  ├─ structured_annotations (size: 4)
      │  │  │  │  │  ├─ structured_annotations[0] [t_const]
      │  │  │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_164
      │  │  │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_164
      │  │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  │  ├─ begin (offset): 3243
      │  │  │  │  │  │  │  │  │  ╰─ end (offset): 3250
      │  │  │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ name: 
      │  │  │  │  │  │  │  ├─ scoped_name: scope.
      │  │  │  │  │  │  │  ├─ uri: 
      │  │  │  │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  │  │  │  ├─ generated? false
      │  │  │  │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ has_doc? false
      │  │  │  │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 0
      │  │  │  │  │  │  │  │  ╰─ end (offset): 0
      │  │  │  │  │  │  │  ╰─ name_range: <absent>
      │  │  │  │  │  │  ├─ type_ref [t_type_ref]
      │  │  │  │  │  │  │  ├─ empty? false
      │  │  │  │  │  │  │  ├─ resolved? true
      │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 3243
      │  │  │  │  │  │  │  │  ╰─ end (offset): 3250
      │  │  │  │  │  │  │  ├─ type: [t_type*] 0xNORMALIZED_74
      │  │  │  │  │  │  │  │  ╰─ full_name: scope.Struct
      │  │  │  │  │  │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
      │  │  │  │  │  │  ╰─ value [t_const_value*] 0xNORMALIZED_165
      │  │  │  │  │  │     ├─ src_range: <absent>
      │  │  │  │  │  │     ├─ is_empty? true
      │  │  │  │  │  │     ╰─ kind: map({})
      │  │  │  │  │  ├─ structured_annotations[1] [t_const]
      │  │  │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_166
      │  │  │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_166
      │  │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  │  ├─ begin (offset): 3251
      │  │  │  │  │  │  │  │  │  ╰─ end (offset): 3257
      │  │  │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ name: 
      │  │  │  │  │  │  │  ├─ scoped_name: scope.
      │  │  │  │  │  │  │  ├─ uri: 
      │  │  │  │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  │  │  │  ├─ generated? false
      │  │  │  │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ has_doc? false
      │  │  │  │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 0
      │  │  │  │  │  │  │  │  ╰─ end (offset): 0
      │  │  │  │  │  │  │  ╰─ name_range: <absent>
      │  │  │  │  │  │  ├─ type_ref [t_type_ref]
      │  │  │  │  │  │  │  ├─ empty? false
      │  │  │  │  │  │  │  ├─ resolved? true
      │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 3251
      │  │  │  │  │  │  │  │  ╰─ end (offset): 3257
      │  │  │  │  │  │  │  ├─ type: [t_type*] 0xNORMALIZED_159
      │  │  │  │  │  │  │  │  ╰─ full_name: scope.Union
      │  │  │  │  │  │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
      │  │  │  │  │  │  ╰─ value [t_const_value*] 0xNORMALIZED_167
      │  │  │  │  │  │     ├─ src_range: <absent>
      │  │  │  │  │  │     ├─ is_empty? true
      │  │  │  │  │  │     ╰─ kind: map({})
      │  │  │  │  │  ├─ structured_annotations[2] [t_const]
      │  │  │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_168
      │  │  │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_168
      │  │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  │  ├─ begin (offset): 3258
      │  │  │  │  │  │  │  │  │  ╰─ end (offset): 3268
      │  │  │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ name: 
      │  │  │  │  │  │  │  ├─ scoped_name: scope.
      │  │  │  │  │  │  │  ├─ uri: 
      │  │  │  │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  │  │  │  ├─ generated? false
      │  │  │  │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ has_doc? false
      │  │  │  │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 0
      │  │  │  │  │  │  │  │  ╰─ end (offset): 0
      │  │  │  │  │  │  │  ╰─ name_range: <absent>
      │  │  │  │  │  │  ├─ type_ref [t_type_ref]
      │  │  │  │  │  │  │  ├─ empty? false
      │  │  │  │  │  │  │  ├─ resolved? true
      │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 3258
      │  │  │  │  │  │  │  │  ╰─ end (offset): 3268
      │  │  │  │  │  │  │  ├─ type: [t_type*] 0xNORMALIZED_77
      │  │  │  │  │  │  │  │  ╰─ full_name: scope.Exception
      │  │  │  │  │  │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
      │  │  │  │  │  │  ╰─ value [t_const_value*] 0xNORMALIZED_169
      │  │  │  │  │  │     ├─ src_range: <absent>
      │  │  │  │  │  │     ├─ is_empty? true
      │  │  │  │  │  │     ╰─ kind: map({})
      │  │  │  │  │  ╰─ structured_annotations[3] [t_const]
      │  │  │  │  │     ├─ (base) [t_named] @0xNORMALIZED_170
      │  │  │  │  │     │  ├─ (base) [t_node] @0xNORMALIZED_170
      │  │  │  │  │     │  │  ├─ src_range [source_range]
      │  │  │  │  │     │  │  │  ├─ begin (offset): 3269
      │  │  │  │  │     │  │  │  ╰─ end (offset): 3280
      │  │  │  │  │     │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  │     │  ├─ name: 
      │  │  │  │  │     │  ├─ scoped_name: scope.
      │  │  │  │  │     │  ├─ uri: 
      │  │  │  │  │     │  ├─ explicit_uri? false
      │  │  │  │  │     │  ├─ generated? false
      │  │  │  │  │     │  ├─ structured_annotations (size: 0)
      │  │  │  │  │     │  ├─ has_doc? false
      │  │  │  │  │     │  ├─ doc_range [source_range]
      │  │  │  │  │     │  │  ├─ begin (offset): 0
      │  │  │  │  │     │  │  ╰─ end (offset): 0
      │  │  │  │  │     │  ╰─ name_range: <absent>
      │  │  │  │  │     ├─ type_ref [t_type_ref]
      │  │  │  │  │     │  ├─ empty? false
      │  │  │  │  │     │  ├─ resolved? true
      │  │  │  │  │     │  ├─ src_range [source_range]
      │  │  │  │  │     │  │  ├─ begin (offset): 3269
      │  │  │  │  │     │  │  ╰─ end (offset): 3280
      │  │  │  │  │     │  ├─ type: [t_type*] 0xNORMALIZED_158
      │  │  │  │  │     │  │  ╰─ full_name: scope.Transitive
      │  │  │  │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
      │  │  │  │  │     ╰─ value [t_const_value*] 0xNORMALIZED_171
      │  │  │  │  │        ├─ src_range: <absent>
      │  │  │  │  │        ├─ is_empty? true
      │  │  │  │  │        ╰─ kind: map({})
      │  │  │  │  ├─ has_doc? true
      │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  ├─ begin (offset): 3184
      │  │  │  │  │  ╰─ end (offset): 3242
      │  │  │  │  ╰─ name_range [source_range]
      │  │  │  │     ├─ begin (offset): 3288
      │  │  │  │     ╰─ end (offset): 3298
      │  │  │  ├─ full_name: scope.Structured
      │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_53
      │  │  │     ╰─ full_name: scope.Structured
      │  │  ╰─ fields (size: 0)
      │  ├─ structs_and_unions[16] [t_structured] @0xNORMALIZED_172
      │  │  ├─ (base) [t_type] @0xNORMALIZED_172
      │  │  │  ├─ (base) [t_named] @0xNORMALIZED_172
      │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_172
      │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  ├─ begin (offset): 3361
      │  │  │  │  │  │  ╰─ end (offset): 3414
      │  │  │  │  │  ╰─ unstructured_annotations (size: 1)
      │  │  │  │  │     ╰─ name: hack.name
      │  │  │  │  │        ├─ src_range [source_range]
      │  │  │  │  │        │  ├─ begin (offset): 0
      │  │  │  │  │        │  ╰─ end (offset): 0
      │  │  │  │  │        ╰─ value: TInterface
      │  │  │  │  ├─ name: Interface
      │  │  │  │  ├─ scoped_name: scope.Interface
      │  │  │  │  ├─ uri: facebook.com/thrift/annotation/Interface
      │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  ├─ generated? false
      │  │  │  │  ├─ structured_annotations (size: 3)
      │  │  │  │  │  ├─ structured_annotations[0] [t_const]
      │  │  │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_173
      │  │  │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_173
      │  │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  │  ├─ begin (offset): 3361
      │  │  │  │  │  │  │  │  │  ╰─ end (offset): 3369
      │  │  │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ name: 
      │  │  │  │  │  │  │  ├─ scoped_name: scope.
      │  │  │  │  │  │  │  ├─ uri: 
      │  │  │  │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  │  │  │  ├─ generated? false
      │  │  │  │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ has_doc? false
      │  │  │  │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 0
      │  │  │  │  │  │  │  │  ╰─ end (offset): 0
      │  │  │  │  │  │  │  ╰─ name_range: <absent>
      │  │  │  │  │  │  ├─ type_ref [t_type_ref]
      │  │  │  │  │  │  │  ├─ empty? false
      │  │  │  │  │  │  │  ├─ resolved? true
      │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 3361
      │  │  │  │  │  │  │  │  ╰─ end (offset): 3369
      │  │  │  │  │  │  │  ├─ type: [t_type*] 0xNORMALIZED_112
      │  │  │  │  │  │  │  │  ╰─ full_name: scope.Service
      │  │  │  │  │  │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
      │  │  │  │  │  │  ╰─ value [t_const_value*] 0xNORMALIZED_174
      │  │  │  │  │  │     ├─ src_range: <absent>
      │  │  │  │  │  │     ├─ is_empty? true
      │  │  │  │  │  │     ╰─ kind: map({})
      │  │  │  │  │  ├─ structured_annotations[1] [t_const]
      │  │  │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_175
      │  │  │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_175
      │  │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  │  ├─ begin (offset): 3370
      │  │  │  │  │  │  │  │  │  ╰─ end (offset): 3382
      │  │  │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ name: 
      │  │  │  │  │  │  │  ├─ scoped_name: scope.
      │  │  │  │  │  │  │  ├─ uri: 
      │  │  │  │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  │  │  │  ├─ generated? false
      │  │  │  │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ has_doc? false
      │  │  │  │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 0
      │  │  │  │  │  │  │  │  ╰─ end (offset): 0
      │  │  │  │  │  │  │  ╰─ name_range: <absent>
      │  │  │  │  │  │  ├─ type_ref [t_type_ref]
      │  │  │  │  │  │  │  ├─ empty? false
      │  │  │  │  │  │  │  ├─ resolved? true
      │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 3370
      │  │  │  │  │  │  │  │  ╰─ end (offset): 3382
      │  │  │  │  │  │  │  ├─ type: [t_type*] 0xNORMALIZED_106
      │  │  │  │  │  │  │  │  ╰─ full_name: scope.Interaction
      │  │  │  │  │  │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
      │  │  │  │  │  │  ╰─ value [t_const_value*] 0xNORMALIZED_176
      │  │  │  │  │  │     ├─ src_range: <absent>
      │  │  │  │  │  │     ├─ is_empty? true
      │  │  │  │  │  │     ╰─ kind: map({})
      │  │  │  │  │  ╰─ structured_annotations[2] [t_const]
      │  │  │  │  │     ├─ (base) [t_named] @0xNORMALIZED_177
      │  │  │  │  │     │  ├─ (base) [t_node] @0xNORMALIZED_177
      │  │  │  │  │     │  │  ├─ src_range [source_range]
      │  │  │  │  │     │  │  │  ├─ begin (offset): 3383
      │  │  │  │  │     │  │  │  ╰─ end (offset): 3394
      │  │  │  │  │     │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  │     │  ├─ name: 
      │  │  │  │  │     │  ├─ scoped_name: scope.
      │  │  │  │  │     │  ├─ uri: 
      │  │  │  │  │     │  ├─ explicit_uri? false
      │  │  │  │  │     │  ├─ generated? false
      │  │  │  │  │     │  ├─ structured_annotations (size: 0)
      │  │  │  │  │     │  ├─ has_doc? false
      │  │  │  │  │     │  ├─ doc_range [source_range]
      │  │  │  │  │     │  │  ├─ begin (offset): 0
      │  │  │  │  │     │  │  ╰─ end (offset): 0
      │  │  │  │  │     │  ╰─ name_range: <absent>
      │  │  │  │  │     ├─ type_ref [t_type_ref]
      │  │  │  │  │     │  ├─ empty? false
      │  │  │  │  │     │  ├─ resolved? true
      │  │  │  │  │     │  ├─ src_range [source_range]
      │  │  │  │  │     │  │  ├─ begin (offset): 3383
      │  │  │  │  │     │  │  ╰─ end (offset): 3394
      │  │  │  │  │     │  ├─ type: [t_type*] 0xNORMALIZED_158
      │  │  │  │  │     │  │  ╰─ full_name: scope.Transitive
      │  │  │  │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
      │  │  │  │  │     ╰─ value [t_const_value*] 0xNORMALIZED_178
      │  │  │  │  │        ├─ src_range: <absent>
      │  │  │  │  │        ├─ is_empty? true
      │  │  │  │  │        ╰─ kind: map({})
      │  │  │  │  ├─ has_doc? true
      │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  ├─ begin (offset): 3303
      │  │  │  │  │  ╰─ end (offset): 3360
      │  │  │  │  ╰─ name_range [source_range]
      │  │  │  │     ├─ begin (offset): 3402
      │  │  │  │     ╰─ end (offset): 3411
      │  │  │  ├─ full_name: scope.Interface
      │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_172
      │  │  │     ╰─ full_name: scope.Interface
      │  │  ╰─ fields (size: 0)
      │  ├─ structs_and_unions[17] [t_structured] @0xNORMALIZED_179
      │  │  ├─ (base) [t_type] @0xNORMALIZED_179
      │  │  │  ├─ (base) [t_named] @0xNORMALIZED_179
      │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_179
      │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  ├─ begin (offset): 3528
      │  │  │  │  │  │  ╰─ end (offset): 3609
      │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  ├─ name: RootDefinition
      │  │  │  │  ├─ scoped_name: scope.RootDefinition
      │  │  │  │  ├─ uri: facebook.com/thrift/annotation/RootDefinition
      │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  ├─ generated? false
      │  │  │  │  ├─ structured_annotations (size: 6)
      │  │  │  │  │  ├─ structured_annotations[0] [t_const]
      │  │  │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_180
      │  │  │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_180
      │  │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  │  ├─ begin (offset): 3528
      │  │  │  │  │  │  │  │  │  ╰─ end (offset): 3539
      │  │  │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ name: 
      │  │  │  │  │  │  │  ├─ scoped_name: scope.
      │  │  │  │  │  │  │  ├─ uri: 
      │  │  │  │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  │  │  │  ├─ generated? false
      │  │  │  │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ has_doc? false
      │  │  │  │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 0
      │  │  │  │  │  │  │  │  ╰─ end (offset): 0
      │  │  │  │  │  │  │  ╰─ name_range: <absent>
      │  │  │  │  │  │  ├─ type_ref [t_type_ref]
      │  │  │  │  │  │  │  ├─ empty? false
      │  │  │  │  │  │  │  ├─ resolved? true
      │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 3528
      │  │  │  │  │  │  │  │  ╰─ end (offset): 3539
      │  │  │  │  │  │  │  ├─ type: [t_type*] 0xNORMALIZED_53
      │  │  │  │  │  │  │  │  ╰─ full_name: scope.Structured
      │  │  │  │  │  │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
      │  │  │  │  │  │  ╰─ value [t_const_value*] 0xNORMALIZED_181
      │  │  │  │  │  │     ├─ src_range: <absent>
      │  │  │  │  │  │     ├─ is_empty? true
      │  │  │  │  │  │     ╰─ kind: map({})
      │  │  │  │  │  ├─ structured_annotations[1] [t_const]
      │  │  │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_182
      │  │  │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_182
      │  │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  │  ├─ begin (offset): 3540
      │  │  │  │  │  │  │  │  │  ╰─ end (offset): 3550
      │  │  │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ name: 
      │  │  │  │  │  │  │  ├─ scoped_name: scope.
      │  │  │  │  │  │  │  ├─ uri: 
      │  │  │  │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  │  │  │  ├─ generated? false
      │  │  │  │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ has_doc? false
      │  │  │  │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 0
      │  │  │  │  │  │  │  │  ╰─ end (offset): 0
      │  │  │  │  │  │  │  ╰─ name_range: <absent>
      │  │  │  │  │  │  ├─ type_ref [t_type_ref]
      │  │  │  │  │  │  │  ├─ empty? false
      │  │  │  │  │  │  │  ├─ resolved? true
      │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 3540
      │  │  │  │  │  │  │  │  ╰─ end (offset): 3550
      │  │  │  │  │  │  │  ├─ type: [t_type*] 0xNORMALIZED_172
      │  │  │  │  │  │  │  │  ╰─ full_name: scope.Interface
      │  │  │  │  │  │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
      │  │  │  │  │  │  ╰─ value [t_const_value*] 0xNORMALIZED_183
      │  │  │  │  │  │     ├─ src_range: <absent>
      │  │  │  │  │  │     ├─ is_empty? true
      │  │  │  │  │  │     ╰─ kind: map({})
      │  │  │  │  │  ├─ structured_annotations[2] [t_const]
      │  │  │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_184
      │  │  │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_184
      │  │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  │  ├─ begin (offset): 3551
      │  │  │  │  │  │  │  │  │  ╰─ end (offset): 3559
      │  │  │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ name: 
      │  │  │  │  │  │  │  ├─ scoped_name: scope.
      │  │  │  │  │  │  │  ├─ uri: 
      │  │  │  │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  │  │  │  ├─ generated? false
      │  │  │  │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ has_doc? false
      │  │  │  │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 0
      │  │  │  │  │  │  │  │  ╰─ end (offset): 0
      │  │  │  │  │  │  │  ╰─ name_range: <absent>
      │  │  │  │  │  │  ├─ type_ref [t_type_ref]
      │  │  │  │  │  │  │  ├─ empty? false
      │  │  │  │  │  │  │  ├─ resolved? true
      │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 3551
      │  │  │  │  │  │  │  │  ╰─ end (offset): 3559
      │  │  │  │  │  │  │  ├─ type: [t_type*] 0xNORMALIZED_139
      │  │  │  │  │  │  │  │  ╰─ full_name: scope.Typedef
      │  │  │  │  │  │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
      │  │  │  │  │  │  ╰─ value [t_const_value*] 0xNORMALIZED_185
      │  │  │  │  │  │     ├─ src_range: <absent>
      │  │  │  │  │  │     ├─ is_empty? true
      │  │  │  │  │  │     ╰─ kind: map({})
      │  │  │  │  │  ├─ structured_annotations[3] [t_const]
      │  │  │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_186
      │  │  │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_186
      │  │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  │  ├─ begin (offset): 3560
      │  │  │  │  │  │  │  │  │  ╰─ end (offset): 3565
      │  │  │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ name: 
      │  │  │  │  │  │  │  ├─ scoped_name: scope.
      │  │  │  │  │  │  │  ├─ uri: 
      │  │  │  │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  │  │  │  ├─ generated? false
      │  │  │  │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ has_doc? false
      │  │  │  │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 0
      │  │  │  │  │  │  │  │  ╰─ end (offset): 0
      │  │  │  │  │  │  │  ╰─ name_range: <absent>
      │  │  │  │  │  │  ├─ type_ref [t_type_ref]
      │  │  │  │  │  │  │  ├─ empty? false
      │  │  │  │  │  │  │  ├─ resolved? true
      │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 3560
      │  │  │  │  │  │  │  │  ╰─ end (offset): 3565
      │  │  │  │  │  │  │  ├─ type: [t_type*] 0xNORMALIZED_56
      │  │  │  │  │  │  │  │  ╰─ full_name: scope.Enum
      │  │  │  │  │  │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
      │  │  │  │  │  │  ╰─ value [t_const_value*] 0xNORMALIZED_187
      │  │  │  │  │  │     ├─ src_range: <absent>
      │  │  │  │  │  │     ├─ is_empty? true
      │  │  │  │  │  │     ╰─ kind: map({})
      │  │  │  │  │  ├─ structured_annotations[4] [t_const]
      │  │  │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_188
      │  │  │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_188
      │  │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  │  ├─ begin (offset): 3566
      │  │  │  │  │  │  │  │  │  ╰─ end (offset): 3572
      │  │  │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ name: 
      │  │  │  │  │  │  │  ├─ scoped_name: scope.
      │  │  │  │  │  │  │  ├─ uri: 
      │  │  │  │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  │  │  │  ├─ generated? false
      │  │  │  │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ has_doc? false
      │  │  │  │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 0
      │  │  │  │  │  │  │  │  ╰─ end (offset): 0
      │  │  │  │  │  │  │  ╰─ name_range: <absent>
      │  │  │  │  │  │  ├─ type_ref [t_type_ref]
      │  │  │  │  │  │  │  ├─ empty? false
      │  │  │  │  │  │  │  ├─ resolved? true
      │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 3566
      │  │  │  │  │  │  │  │  ╰─ end (offset): 3572
      │  │  │  │  │  │  │  ├─ type: [t_type*] 0xNORMALIZED_163
      │  │  │  │  │  │  │  │  ╰─ full_name: scope.Const
      │  │  │  │  │  │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
      │  │  │  │  │  │  ╰─ value [t_const_value*] 0xNORMALIZED_189
      │  │  │  │  │  │     ├─ src_range: <absent>
      │  │  │  │  │  │     ├─ is_empty? true
      │  │  │  │  │  │     ╰─ kind: map({})
      │  │  │  │  │  ╰─ structured_annotations[5] [t_const]
      │  │  │  │  │     ├─ (base) [t_named] @0xNORMALIZED_190
      │  │  │  │  │     │  ├─ (base) [t_node] @0xNORMALIZED_190
      │  │  │  │  │     │  │  ├─ src_range [source_range]
      │  │  │  │  │     │  │  │  ├─ begin (offset): 3573
      │  │  │  │  │     │  │  │  ╰─ end (offset): 3584
      │  │  │  │  │     │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  │     │  ├─ name: 
      │  │  │  │  │     │  ├─ scoped_name: scope.
      │  │  │  │  │     │  ├─ uri: 
      │  │  │  │  │     │  ├─ explicit_uri? false
      │  │  │  │  │     │  ├─ generated? false
      │  │  │  │  │     │  ├─ structured_annotations (size: 0)
      │  │  │  │  │     │  ├─ has_doc? false
      │  │  │  │  │     │  ├─ doc_range [source_range]
      │  │  │  │  │     │  │  ├─ begin (offset): 0
      │  │  │  │  │     │  │  ╰─ end (offset): 0
      │  │  │  │  │     │  ╰─ name_range: <absent>
      │  │  │  │  │     ├─ type_ref [t_type_ref]
      │  │  │  │  │     │  ├─ empty? false
      │  │  │  │  │     │  ├─ resolved? true
      │  │  │  │  │     │  ├─ src_range [source_range]
      │  │  │  │  │     │  │  ├─ begin (offset): 3573
      │  │  │  │  │     │  │  ╰─ end (offset): 3584
      │  │  │  │  │     │  ├─ type: [t_type*] 0xNORMALIZED_158
      │  │  │  │  │     │  │  ╰─ full_name: scope.Transitive
      │  │  │  │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
      │  │  │  │  │     ╰─ value [t_const_value*] 0xNORMALIZED_191
      │  │  │  │  │        ├─ src_range: <absent>
      │  │  │  │  │        ├─ is_empty? true
      │  │  │  │  │        ╰─ kind: map({})
      │  │  │  │  ├─ has_doc? true
      │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  ├─ begin (offset): 3468
      │  │  │  │  │  ╰─ end (offset): 3527
      │  │  │  │  ╰─ name_range [source_range]
      │  │  │  │     ├─ begin (offset): 3592
      │  │  │  │     ╰─ end (offset): 3606
      │  │  │  ├─ full_name: scope.RootDefinition
      │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_179
      │  │  │     ╰─ full_name: scope.RootDefinition
      │  │  ╰─ fields (size: 0)
      │  ├─ structs_and_unions[18] [t_structured] @0xNORMALIZED_49
      │  │  ├─ (base) [t_type] @0xNORMALIZED_49
      │  │  │  ├─ (base) [t_named] @0xNORMALIZED_49
      │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_49
      │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  ├─ begin (offset): 3657
      │  │  │  │  │  │  ╰─ end (offset): 3752
      │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  ├─ name: Definition
      │  │  │  │  ├─ scoped_name: scope.Definition
      │  │  │  │  ├─ uri: facebook.com/thrift/annotation/Definition
      │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  ├─ generated? false
      │  │  │  │  ├─ structured_annotations (size: 6)
      │  │  │  │  │  ├─ structured_annotations[0] [t_const]
      │  │  │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_192
      │  │  │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_192
      │  │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  │  ├─ begin (offset): 3657
      │  │  │  │  │  │  │  │  │  ╰─ end (offset): 3672
      │  │  │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ name: 
      │  │  │  │  │  │  │  ├─ scoped_name: scope.
      │  │  │  │  │  │  │  ├─ uri: 
      │  │  │  │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  │  │  │  ├─ generated? false
      │  │  │  │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ has_doc? false
      │  │  │  │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 0
      │  │  │  │  │  │  │  │  ╰─ end (offset): 0
      │  │  │  │  │  │  │  ╰─ name_range: <absent>
      │  │  │  │  │  │  ├─ type_ref [t_type_ref]
      │  │  │  │  │  │  │  ├─ empty? false
      │  │  │  │  │  │  │  ├─ resolved? true
      │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 3657
      │  │  │  │  │  │  │  │  ╰─ end (offset): 3672
      │  │  │  │  │  │  │  ├─ type: [t_type*] 0xNORMALIZED_179
      │  │  │  │  │  │  │  │  ╰─ full_name: scope.RootDefinition
      │  │  │  │  │  │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
      │  │  │  │  │  │  ╰─ value [t_const_value*] 0xNORMALIZED_193
      │  │  │  │  │  │     ├─ src_range: <absent>
      │  │  │  │  │  │     ├─ is_empty? true
      │  │  │  │  │  │     ╰─ kind: map({})
      │  │  │  │  │  ├─ structured_annotations[1] [t_const]
      │  │  │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_194
      │  │  │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_194
      │  │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  │  ├─ begin (offset): 3673
      │  │  │  │  │  │  │  │  │  ╰─ end (offset): 3679
      │  │  │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ name: 
      │  │  │  │  │  │  │  ├─ scoped_name: scope.
      │  │  │  │  │  │  │  ├─ uri: 
      │  │  │  │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  │  │  │  ├─ generated? false
      │  │  │  │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ has_doc? false
      │  │  │  │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 0
      │  │  │  │  │  │  │  │  ╰─ end (offset): 0
      │  │  │  │  │  │  │  ╰─ name_range: <absent>
      │  │  │  │  │  │  ├─ type_ref [t_type_ref]
      │  │  │  │  │  │  │  ├─ empty? false
      │  │  │  │  │  │  │  ├─ resolved? true
      │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 3673
      │  │  │  │  │  │  │  │  ╰─ end (offset): 3679
      │  │  │  │  │  │  │  ├─ type: [t_type*] 0xNORMALIZED_80
      │  │  │  │  │  │  │  │  ╰─ full_name: scope.Field
      │  │  │  │  │  │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
      │  │  │  │  │  │  ╰─ value [t_const_value*] 0xNORMALIZED_195
      │  │  │  │  │  │     ├─ src_range: <absent>
      │  │  │  │  │  │     ├─ is_empty? true
      │  │  │  │  │  │     ╰─ kind: map({})
      │  │  │  │  │  ├─ structured_annotations[2] [t_const]
      │  │  │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_196
      │  │  │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_196
      │  │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  │  ├─ begin (offset): 3680
      │  │  │  │  │  │  │  │  │  ╰─ end (offset): 3689
      │  │  │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ name: 
      │  │  │  │  │  │  │  ├─ scoped_name: scope.
      │  │  │  │  │  │  │  ├─ uri: 
      │  │  │  │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  │  │  │  ├─ generated? false
      │  │  │  │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ has_doc? false
      │  │  │  │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 0
      │  │  │  │  │  │  │  │  ╰─ end (offset): 0
      │  │  │  │  │  │  │  ╰─ name_range: <absent>
      │  │  │  │  │  │  ├─ type_ref [t_type_ref]
      │  │  │  │  │  │  │  ├─ empty? false
      │  │  │  │  │  │  │  ├─ resolved? true
      │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 3680
      │  │  │  │  │  │  │  │  ╰─ end (offset): 3689
      │  │  │  │  │  │  │  ├─ type: [t_type*] 0xNORMALIZED_120
      │  │  │  │  │  │  │  │  ╰─ full_name: scope.Function
      │  │  │  │  │  │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
      │  │  │  │  │  │  ╰─ value [t_const_value*] 0xNORMALIZED_197
      │  │  │  │  │  │     ├─ src_range: <absent>
      │  │  │  │  │  │     ├─ is_empty? true
      │  │  │  │  │  │     ╰─ kind: map({})
      │  │  │  │  │  ├─ structured_annotations[3] [t_const]
      │  │  │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_198
      │  │  │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_198
      │  │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  │  ├─ begin (offset): 3690
      │  │  │  │  │  │  │  │  │  ╰─ end (offset): 3708
      │  │  │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ name: 
      │  │  │  │  │  │  │  ├─ scoped_name: scope.
      │  │  │  │  │  │  │  ├─ uri: 
      │  │  │  │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  │  │  │  ├─ generated? false
      │  │  │  │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ has_doc? false
      │  │  │  │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 0
      │  │  │  │  │  │  │  │  ╰─ end (offset): 0
      │  │  │  │  │  │  │  ╰─ name_range: <absent>
      │  │  │  │  │  │  ├─ type_ref [t_type_ref]
      │  │  │  │  │  │  │  ├─ empty? false
      │  │  │  │  │  │  │  ├─ resolved? true
      │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 3690
      │  │  │  │  │  │  │  │  ╰─ end (offset): 3708
      │  │  │  │  │  │  │  ├─ type: [t_type*] 0xNORMALIZED_161
      │  │  │  │  │  │  │  │  ╰─ full_name: scope.FunctionParameter
      │  │  │  │  │  │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
      │  │  │  │  │  │  ╰─ value [t_const_value*] 0xNORMALIZED_199
      │  │  │  │  │  │     ├─ src_range: <absent>
      │  │  │  │  │  │     ├─ is_empty? true
      │  │  │  │  │  │     ╰─ kind: map({})
      │  │  │  │  │  ├─ structured_annotations[4] [t_const]
      │  │  │  │  │  │  ├─ (base) [t_named] @0xNORMALIZED_200
      │  │  │  │  │  │  │  ├─ (base) [t_node] @0xNORMALIZED_200
      │  │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  │  ├─ begin (offset): 3709
      │  │  │  │  │  │  │  │  │  ╰─ end (offset): 3719
      │  │  │  │  │  │  │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ name: 
      │  │  │  │  │  │  │  ├─ scoped_name: scope.
      │  │  │  │  │  │  │  ├─ uri: 
      │  │  │  │  │  │  │  ├─ explicit_uri? false
      │  │  │  │  │  │  │  ├─ generated? false
      │  │  │  │  │  │  │  ├─ structured_annotations (size: 0)
      │  │  │  │  │  │  │  ├─ has_doc? false
      │  │  │  │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 0
      │  │  │  │  │  │  │  │  ╰─ end (offset): 0
      │  │  │  │  │  │  │  ╰─ name_range: <absent>
      │  │  │  │  │  │  ├─ type_ref [t_type_ref]
      │  │  │  │  │  │  │  ├─ empty? false
      │  │  │  │  │  │  │  ├─ resolved? true
      │  │  │  │  │  │  │  ├─ src_range [source_range]
      │  │  │  │  │  │  │  │  ├─ begin (offset): 3709
      │  │  │  │  │  │  │  │  ╰─ end (offset): 3719
      │  │  │  │  │  │  │  ├─ type: [t_type*] 0xNORMALIZED_162
      │  │  │  │  │  │  │  │  ╰─ full_name: scope.EnumValue
      │  │  │  │  │  │  │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
      │  │  │  │  │  │  ╰─ value [t_const_value*] 0xNORMALIZED_201
      │  │  │  │  │  │     ├─ src_range: <absent>
      │  │  │  │  │  │     ├─ is_empty? true
      │  │  │  │  │  │     ╰─ kind: map({})
      │  │  │  │  │  ╰─ structured_annotations[5] [t_const]
      │  │  │  │  │     ├─ (base) [t_named] @0xNORMALIZED_202
      │  │  │  │  │     │  ├─ (base) [t_node] @0xNORMALIZED_202
      │  │  │  │  │     │  │  ├─ src_range [source_range]
      │  │  │  │  │     │  │  │  ├─ begin (offset): 3720
      │  │  │  │  │     │  │  │  ╰─ end (offset): 3731
      │  │  │  │  │     │  │  ╰─ unstructured_annotations (size: 0)
      │  │  │  │  │     │  ├─ name: 
      │  │  │  │  │     │  ├─ scoped_name: scope.
      │  │  │  │  │     │  ├─ uri: 
      │  │  │  │  │     │  ├─ explicit_uri? false
      │  │  │  │  │     │  ├─ generated? false
      │  │  │  │  │     │  ├─ structured_annotations (size: 0)
      │  │  │  │  │     │  ├─ has_doc? false
      │  │  │  │  │     │  ├─ doc_range [source_range]
      │  │  │  │  │     │  │  ├─ begin (offset): 0
      │  │  │  │  │     │  │  ╰─ end (offset): 0
      │  │  │  │  │     │  ╰─ name_range: <absent>
      │  │  │  │  │     ├─ type_ref [t_type_ref]
      │  │  │  │  │     │  ├─ empty? false
      │  │  │  │  │     │  ├─ resolved? true
      │  │  │  │  │     │  ├─ src_range [source_range]
      │  │  │  │  │     │  │  ├─ begin (offset): 3720
      │  │  │  │  │     │  │  ╰─ end (offset): 3731
      │  │  │  │  │     │  ├─ type: [t_type*] 0xNORMALIZED_158
      │  │  │  │  │     │  │  ╰─ full_name: scope.Transitive
      │  │  │  │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
      │  │  │  │  │     ╰─ value [t_const_value*] 0xNORMALIZED_203
      │  │  │  │  │        ├─ src_range: <absent>
      │  │  │  │  │        ├─ is_empty? true
      │  │  │  │  │        ╰─ kind: map({})
      │  │  │  │  ├─ has_doc? true
      │  │  │  │  ├─ doc_range [source_range]
      │  │  │  │  │  ├─ begin (offset): 3611
      │  │  │  │  │  ╰─ end (offset): 3656
      │  │  │  │  ╰─ name_range [source_range]
      │  │  │  │     ├─ begin (offset): 3739
      │  │  │  │     ╰─ end (offset): 3749
      │  │  │  ├─ full_name: scope.Definition
      │  │  │  ╰─ true_type: [t_type*] 0xNORMALIZED_49
      │  │  │     ╰─ full_name: scope.Definition
      │  │  ╰─ fields (size: 0)
      │  ╰─ structs_and_unions[19] [t_structured] @0xNORMALIZED_204
      │     ├─ (base) [t_type] @0xNORMALIZED_204
      │     │  ├─ (base) [t_named] @0xNORMALIZED_204
      │     │  │  ├─ (base) [t_node] @0xNORMALIZED_204
      │     │  │  │  ├─ src_range [source_range]
      │     │  │  │  │  ├─ begin (offset): 3869
      │     │  │  │  │  ╰─ end (offset): 3906
      │     │  │  │  ╰─ unstructured_annotations (size: 0)
      │     │  │  ├─ name: DisableSchemaConst
      │     │  │  ├─ scoped_name: scope.DisableSchemaConst
      │     │  │  ├─ uri: facebook.com/thrift/annotation/DisableSchemaConst
      │     │  │  ├─ explicit_uri? false
      │     │  │  ├─ generated? false
      │     │  │  ├─ structured_annotations (size: 1)
      │     │  │  │  ╰─ structured_annotations[0] [t_const]
      │     │  │  │     ├─ (base) [t_named] @0xNORMALIZED_205
      │     │  │  │     │  ├─ (base) [t_node] @0xNORMALIZED_205
      │     │  │  │     │  │  ├─ src_range [source_range]
      │     │  │  │     │  │  │  ├─ begin (offset): 3869
      │     │  │  │     │  │  │  ╰─ end (offset): 3877
      │     │  │  │     │  │  ╰─ unstructured_annotations (size: 0)
      │     │  │  │     │  ├─ name: 
      │     │  │  │     │  ├─ scoped_name: scope.
      │     │  │  │     │  ├─ uri: 
      │     │  │  │     │  ├─ explicit_uri? false
      │     │  │  │     │  ├─ generated? false
      │     │  │  │     │  ├─ structured_annotations (size: 0)
      │     │  │  │     │  ├─ has_doc? false
      │     │  │  │     │  ├─ doc_range [source_range]
      │     │  │  │     │  │  ├─ begin (offset): 0
      │     │  │  │     │  │  ╰─ end (offset): 0
      │     │  │  │     │  ╰─ name_range: <absent>
      │     │  │  │     ├─ type_ref [t_type_ref]
      │     │  │  │     │  ├─ empty? false
      │     │  │  │     │  ├─ resolved? true
      │     │  │  │     │  ├─ src_range [source_range]
      │     │  │  │     │  │  ├─ begin (offset): 3869
      │     │  │  │     │  │  ╰─ end (offset): 3877
      │     │  │  │     │  ├─ type: [t_type*] 0xNORMALIZED_46
      │     │  │  │     │  │  ╰─ full_name: scope.Program
      │     │  │  │     │  ╰─ unresolved_type: [t_placeholder_typedef*] 0x0
      │     │  │  │     ╰─ value [t_const_value*] 0xNORMALIZED_206
      │     │  │  │        ├─ src_range: <absent>
      │     │  │  │        ├─ is_empty? true
      │     │  │  │        ╰─ kind: map({})
      │     │  │  ├─ has_doc? true
      │     │  │  ├─ doc_range [source_range]
      │     │  │  │  ├─ begin (offset): 3753
      │     │  │  │  ╰─ end (offset): 3868
      │     │  │  ╰─ name_range [source_range]
      │     │  │     ├─ begin (offset): 3885
      │     │  │     ╰─ end (offset): 3903
      │     │  ├─ full_name: scope.DisableSchemaConst
      │     │  ╰─ true_type: [t_type*] 0xNORMALIZED_204
      │     │     ╰─ full_name: scope.DisableSchemaConst
      │     ╰─ fields (size: 0)
      ├─ exceptions (size: 0)
      ├─ services (size: 0)
      ├─ interactions (size: 0)
      ├─ definitions (size: 20)
      │  ├─ definitions[0]: [t_named*] 0xNORMALIZED_158
      │  │  ╰─ scoped_name: scope.Transitive
      │  ├─ definitions[1]: [t_named*] 0xNORMALIZED_46
      │  │  ╰─ scoped_name: scope.Program
      │  ├─ definitions[2]: [t_named*] 0xNORMALIZED_74
      │  │  ╰─ scoped_name: scope.Struct
      │  ├─ definitions[3]: [t_named*] 0xNORMALIZED_159
      │  │  ╰─ scoped_name: scope.Union
      │  ├─ definitions[4]: [t_named*] 0xNORMALIZED_77
      │  │  ╰─ scoped_name: scope.Exception
      │  ├─ definitions[5]: [t_named*] 0xNORMALIZED_160
      │  │  ╰─ scoped_name: scope.ThrownException
      │  ├─ definitions[6]: [t_named*] 0xNORMALIZED_80
      │  │  ╰─ scoped_name: scope.Field
      │  ├─ definitions[7]: [t_named*] 0xNORMALIZED_139
      │  │  ╰─ scoped_name: scope.Typedef
      │  ├─ definitions[8]: [t_named*] 0xNORMALIZED_112
      │  │  ╰─ scoped_name: scope.Service
      │  ├─ definitions[9]: [t_named*] 0xNORMALIZED_106
      │  │  ╰─ scoped_name: scope.Interaction
      │  ├─ definitions[10]: [t_named*] 0xNORMALIZED_120
      │  │  ╰─ scoped_name: scope.Function
      │  ├─ definitions[11]: [t_named*] 0xNORMALIZED_161
      │  │  ╰─ scoped_name: scope.FunctionParameter
      │  ├─ definitions[12]: [t_named*] 0xNORMALIZED_162
      │  │  ╰─ scoped_name: scope.EnumValue
      │  ├─ definitions[13]: [t_named*] 0xNORMALIZED_163
      │  │  ╰─ scoped_name: scope.Const
      │  ├─ definitions[14]: [t_named*] 0xNORMALIZED_56
      │  │  ╰─ scoped_name: scope.Enum
      │  ├─ definitions[15]: [t_named*] 0xNORMALIZED_53
      │  │  ╰─ scoped_name: scope.Structured
      │  ├─ definitions[16]: [t_named*] 0xNORMALIZED_172
      │  │  ╰─ scoped_name: scope.Interface
      │  ├─ definitions[17]: [t_named*] 0xNORMALIZED_179
      │  │  ╰─ scoped_name: scope.RootDefinition
      │  ├─ definitions[18]: [t_named*] 0xNORMALIZED_49
      │  │  ╰─ scoped_name: scope.Definition
      │  ╰─ definitions[19]: [t_named*] 0xNORMALIZED_204
      │     ╰─ scoped_name: scope.DisableSchemaConst
      ├─ structured_definitions (size: 20)
      │  ├─ structured_definitions[0]: [t_structured*] 0xNORMALIZED_158
      │  │  ╰─ name: Transitive
      │  ├─ structured_definitions[1]: [t_structured*] 0xNORMALIZED_46
      │  │  ╰─ name: Program
      │  ├─ structured_definitions[2]: [t_structured*] 0xNORMALIZED_74
      │  │  ╰─ name: Struct
      │  ├─ structured_definitions[3]: [t_structured*] 0xNORMALIZED_159
      │  │  ╰─ name: Union
      │  ├─ structured_definitions[4]: [t_structured*] 0xNORMALIZED_77
      │  │  ╰─ name: Exception
      │  ├─ structured_definitions[5]: [t_structured*] 0xNORMALIZED_160
      │  │  ╰─ name: ThrownException
      │  ├─ structured_definitions[6]: [t_structured*] 0xNORMALIZED_80
      │  │  ╰─ name: Field
      │  ├─ structured_definitions[7]: [t_structured*] 0xNORMALIZED_139
      │  │  ╰─ name: Typedef
      │  ├─ structured_definitions[8]: [t_structured*] 0xNORMALIZED_112
      │  │  ╰─ name: Service
      │  ├─ structured_definitions[9]: [t_structured*] 0xNORMALIZED_106
      │  │  ╰─ name: Interaction
      │  ├─ structured_definitions[10]: [t_structured*] 0xNORMALIZED_120
      │  │  ╰─ name: Function
      │  ├─ structured_definitions[11]: [t_structured*] 0xNORMALIZED_161
      │  │  ╰─ name: FunctionParameter
      │  ├─ structured_definitions[12]: [t_structured*] 0xNORMALIZED_162
      │  │  ╰─ name: EnumValue
      │  ├─ structured_definitions[13]: [t_structured*] 0xNORMALIZED_163
      │  │  ╰─ name: Const
      │  ├─ structured_definitions[14]: [t_structured*] 0xNORMALIZED_56
      │  │  ╰─ name: Enum
      │  ├─ structured_definitions[15]: [t_structured*] 0xNORMALIZED_53
      │  │  ╰─ name: Structured
      │  ├─ structured_definitions[16]: [t_structured*] 0xNORMALIZED_172
      │  │  ╰─ name: Interface
      │  ├─ structured_definitions[17]: [t_structured*] 0xNORMALIZED_179
      │  │  ╰─ name: RootDefinition
      │  ├─ structured_definitions[18]: [t_structured*] 0xNORMALIZED_49
      │  │  ╰─ name: Definition
      │  ╰─ structured_definitions[19]: [t_structured*] 0xNORMALIZED_204
      │     ╰─ name: DisableSchemaConst
      ╰─ type_instantiations (size: 0)
)");
}

} // namespace apache::thrift::compiler
