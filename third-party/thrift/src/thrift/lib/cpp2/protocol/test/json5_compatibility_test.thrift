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

// Backward compatibility test cases for JSON decoder.
// The JSON5 decoder should accept multiple representations for the same value
// to maintain compatibility with different serialization outputs.
// Test data is defined here so it can be shared between different languages.

include "thrift/lib/cpp2/protocol/test/json5_test.thrift"

package "facebook.com/thrift/json5"

struct CompatibilityTestCase {
  1: string name;
  2: list<string> inputs;
  3: json5_test.Example output;
}

// @lint-ignore-every THRIFTFORMAT
const list<CompatibilityTestCase> compatibilityTestCases = [
  // ── Bool Format Compatibility ────────────────────────────────────────────
  // Bools should accept both true/false bare literals and JSON strings.
  CompatibilityTestCase{
    name = "BoolTrue",
    inputs = [
      "{\"boolValue\": true}",
      "{\"boolValue\": \"true\"}",
    ],
    output = json5_test.Example{boolValue = true},
  },
  CompatibilityTestCase{
    name = "BoolFalse",
    inputs = [
      "{\"boolValue\": false}",
      "{\"boolValue\": \"false\"}",
    ],
    output = json5_test.Example{boolValue = false},
  },
  // ── Map Format Compatibility ──────────────────────────────────────────────
  // Map should accept both object form {k: v} and array form [{"key": k, "value": v}]
  CompatibilityTestCase{
    name = "Map",
    inputs = [
      "{\"i32AsKey\": {\"1\": 2, \"3\": 4}}",
      "{\"i32AsKey\": [{\"key\": 1, \"value\": 2}, {\"key\": 3, \"value\": 4}]}",
    ],
    output = json5_test.Example{i32AsKey = {1: 2, 3: 4}},
  },
  // ── Integer Format Compatibility ──────────────────────────────────────────
  // Integers should accept both JSON numbers and JSON strings.
  CompatibilityTestCase{
    name = "Integer42",
    inputs = [
      "{\"i64Value\": 42}",
      "{\"i64Value\": \"42\"}",
    ],
    output = json5_test.Example{i64Value = 42},
  },
  CompatibilityTestCase{
    name = "IntegerNegative",
    inputs = [
      "{\"i64Value\": \"-123\"}",
    ],
    output = json5_test.Example{i64Value = -123},
  },
  // ── Float Format Compatibility ────────────────────────────────────────────
  // Floats should accept both JSON numbers and JSON strings.
  CompatibilityTestCase{
    name = "Float314",
    inputs = [
      "{\"floatValue\": 3.14}",
      "{\"floatValue\": \"3.14\"}",
    ],
    output = json5_test.Example{floatValue = 3.14},
  },
  CompatibilityTestCase{
    name = "DoublePi",
    inputs = [
      "{\"doubleValue\": 3.14159265358979}",
      "{\"doubleValue\": \"3.14159265358979\"}",
    ],
    output = json5_test.Example{doubleValue = 3.14159265358979},
  },
  CompatibilityTestCase{
    name = "FloatFromInteger",
    inputs = [
      "{\"floatValue\": 42}",
      "{\"floatValue\": \"42\"}",
    ],
    output = json5_test.Example{floatValue = 42.0},
  },
  // ── Enum Format Compatibility ─────────────────────────────────────────────
  // Enum should accept "enum-name (enum-value)", "enum-name", "(enum-value)"
  // and raw integer.
  CompatibilityTestCase{
    name = "EnumOne",
    inputs = [
      "{\"enumValue\": \"ONE (1)\"}",
      "{\"enumValue\": \"ONE(1)\"}",
      "{\"enumValue\": \"ONE \\t (1)\"}",
      "{\"enumValue\": \"ONE\"}",
      "{\"enumValue\": \"(1)\"}",
      "{\"enumValue\": \"1\"}",
      "{\"enumValue\": 1}",
      "{\"enumValue\": 0x1}",
    ],
    output = json5_test.Example{enumValue = 1},
  },
  CompatibilityTestCase{
    name = "EnumTwo",
    inputs = [
      "{\"enumValue\": \"TWO (2)\"}",
      "{\"enumValue\": \"(2)\"}",
      "{\"enumValue\": 0x2}",
    ],
    output = json5_test.Example{enumValue = 2},
  },
  CompatibilityTestCase{
    name = "EnumNegativeOne",
    inputs = [
      "{\"enumValue\": \"NEGATIVE_ONE (-1)\"}",
      "{\"enumValue\": \"NEGATIVE_ONE\"}",
      "{\"enumValue\": \"(-1)\"}",
      "{\"enumValue\": \"-1\"}",
      "{\"enumValue\": -1}",
      "{\"enumValue\": -0x1}",
    ],
    output = json5_test.Example{enumValue = -1},
  },
  CompatibilityTestCase{
    name = "EnumUnregistered",
    inputs = [
      "{\"enumValue\": \"(-5)\"}",
      "{\"enumValue\": \"SomeName (-5)\"}",
      "{\"enumValue\": -5}",
    ],
    output = json5_test.Example{enumValue = -5},
  },
  CompatibilityTestCase{
    name = "EnumDefault",
    inputs = [
      "{\"enumValue\": \"DEFAULT (0)\"}",
      "{\"enumValue\": \"(0)\"}",
      "{\"enumValue\": 0x0}",
    ],
    output = json5_test.Example{enumValue = 0},
  },
  // ── Enum as Map Key Compatibility ─────────────────────────────────────────
  // Enum as map key should accept "enum-name (enum-value)", "enum-name" and
  // "(enum-value)"
  CompatibilityTestCase{
    name = "EnumAsKey",
    inputs = [
      "{\"enumAsKey\": {\"ONE (1)\": \"TWO (2)\"}}",
      "{\"enumAsKey\": {\"ONE\": \"TWO\"}}",
      "{\"enumAsKey\": {\"(1)\": \"(2)\"}}",
      "{\"enumAsKey\": {\"1\": \"2\"}}",
    ],
    output = json5_test.Example{enumAsKey = {1: 2}},
  },
  // ── Binary Encoding Compatibility ─────────────────────────────────────────
  // Binary should accept utf-8, base64, and base64url object forms.
  CompatibilityTestCase{
    name = "Binary",
    inputs = [
      "{binaryValue: {base64url: \"fl9-IQ==\"}}",
      "{binaryValue: {base64url: \"fl9-IQ\"}}",
      "{binaryValue: {base64: \"fl9+IQ==\"}}",
      "{binaryValue: {base64: \"fl9+IQ\"}}",
      "{binaryValue: \"fl9-IQ==\"}",
      "{binaryValue: \"fl9-IQ\"}",
      "{binaryValue: \"fl9+IQ==\"}",
      "{binaryValue: \"fl9+IQ\"}",
      "{binaryValue: {\"utf-8\": \"~_~!\"}}",
    ],
    output = json5_test.Example{binaryValue = "~_~!"},
  },
  // ── Bool as Map Key Compatibility ─────────────────────────────────────────
  // Bool keys in maps are encoded as strings "true" or "false"
  CompatibilityTestCase{
    name = "BoolAsKeyTrue",
    inputs = [
      "{\"boolAsKey\": {\"true\": 42}}",
    ],
    output = json5_test.Example{boolAsKey = {1: 42}},
  },
  CompatibilityTestCase{
    name = "BoolAsKeyFalse",
    inputs = [
      "{\"boolAsKey\": {\"false\": 42}}",
    ],
    output = json5_test.Example{boolAsKey = {0: 42}},
  },
  CompatibilityTestCase{
    name = "BoolAsKeyBoth",
    inputs = [
      "{\"boolAsKey\": {\"true\": 1, \"false\": 0}}",
    ],
    output = json5_test.Example{boolAsKey = {1: 1, 0: 0}},
  },
  // ── Field Identifier Format Compatibility ────────────────────────────────────
  // Accepts: "name", "name (id)", "(id)", "id".
  CompatibilityTestCase{
    name = "FieldIdentifierFormats",
    inputs = [
      "{\"boolValue\": true}",
      "{\"boolValue (30)\": true}",
      "{\"(30)\": true}",
      "{\"30\": true}",
    ],
    output = json5_test.Example{boolValue = true},
  },
  // ── Unknown Field Skipping Compatibility ────────────────────────────────────
  // Unknown fields should be silently skipped during deserialization.
  CompatibilityTestCase{
    name = "SkipUnknownFields",
    inputs = [
      "{\"i64Value\": 42, \"unknownStr\": \"hello\"}",
      "{\"i64Value\": 42, \"unknownStr (123)\": \"hello\"}",
      "{\"unknownNum\": 123, \"i64Value\": 42}",
      "{\"unknownBool\": true, \"i64Value\": 42}",
      "{\"unknownNull\": null, \"i64Value\": 42}",
      "{\"unknownObj\": {\"a\": 1, \"b\": [2, 3]}, \"i64Value\": 42}",
      "{\"unknownArr\": [1, \"two\", true], \"i64Value\": 42}",
      "{\"x\": 1, \"y\": \"s\", \"z\": [1], \"w\": {\"a\": 1}, \"i64Value\": 42}",
    ],
    output = json5_test.Example{i64Value = 42},
  },
  // ── String as Map Key Compatibility ────────────────────────────────────────
  // String-keyed maps should accept both object form and array form.
  CompatibilityTestCase{
    name = "StringAsKeyObjectForm",
    inputs = [
      "{\"stringAsKey\": {\"hello\": \"world\"}}",
      "{\"stringAsKey\": [{\"key\": \"hello\", \"value\": \"world\"}]}",
    ],
    output = json5_test.Example{stringAsKey = {"hello": "world"}},
  },
  // ── String Line Continuation Compatibility ──────────────────────────────────
  // A backslash followed by a literal newline (LF, CR, or CRLF) is a line
  // continuation: the backslash and newline are stripped from the result.
  CompatibilityTestCase{
    name = "StringLineContinuation",
    inputs = [
      "{\"stringValue\": \"hello, world\"}",
      "{\"stringValue\": \"hello, \\\nworld\"}",
      "{\"stringValue\": \"hello, \\\rworld\"}",
      "{\"stringValue\": \"hello, \\\r\nworld\"}",
    ],
    output = json5_test.Example{stringValue = "hello, world"},
  },
  // ── I64 as Map Key Compatibility ─────────────────────────────────────────────
  // I64-keyed maps should accept both array form and object form with string keys.
  CompatibilityTestCase{
    name = "I64AsKeyArrayForm",
    inputs = [
      "{\"i64AsKey\": [{\"key\": 42, \"value\": 1}]}",
      "{\"i64AsKey\": {\"42\": 1}}",
    ],
    output = json5_test.Example{i64AsKey = {42: 1}},
  },
  // ── Null Field Handling ──────────────────────────────────────────────────────
  // Null field values should be treated as absent (field not set).
  CompatibilityTestCase{
    name = "NullFieldsSkipped",
    inputs = [
      "{\"i64Value\": null, \"boolValue\": true, \"stringValue\": null}",
    ],
    output = json5_test.Example{boolValue = true},
  },
  // ── Single-Quoted String Compatibility ──────────────────────────────────────
  CompatibilityTestCase{
    name = "SingleQuotedString",
    inputs = [
      "{\"stringValue\": 'hello'}",
      "{'stringValue': 'hello'}",
    ],
    output = json5_test.Example{stringValue = "hello"},
  },
  // ── JSON5 Comments Compatibility ────────────────────────────────────────────
  CompatibilityTestCase{
    name = "Json5Comments",
    inputs = [
      "{/* comment */\"i64Value\": 42}",
      "{\"i64Value\": /* inline */ 42}",
      "{\"i64Value\": 42 // line comment\n}",
    ],
    output = json5_test.Example{i64Value = 42},
  },
  // ── Double from Integer Compatibility ───────────────────────────────────────
  CompatibilityTestCase{
    name = "DoubleFromInteger",
    inputs = [
      "{\"doubleValue\": 42}",
      "{\"doubleValue\": \"42\"}",
    ],
    output = json5_test.Example{doubleValue = 42.0},
  },
  // ── Union Compatibility ─────────────────────────────────────────────────────
  CompatibilityTestCase{
    name = "UnionEmpty",
    inputs = [
      "{\"unionValue\": {}}",
    ],
    output = json5_test.Example{unionValue = json5_test.ExampleUnion{}},
  },
];
