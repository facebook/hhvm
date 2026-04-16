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

// Negative test cases for JSON decoder validation.
// The JSON5 decoder should reject these malformed or type-mismatched inputs.
// Test data is defined here so it can be shared between different languages.

package "facebook.com/thrift/json5"

struct NegativeTestCase {
  1: string name;
  2: string json;
}

// @lint-ignore-every THRIFTFORMAT
const list<NegativeTestCase> enumValidationNegativeCases = [
  NegativeTestCase{name = "NameValueMismatch", json = "{\"enumValue\": \"ONE (2)\"}"},
  NegativeTestCase{name = "NameValueMismatchNegative", json = "{\"enumValue\": \"ONE (-1)\"}"},
  NegativeTestCase{name = "InvalidIdentifier", json = "{\"enumValue\": \"$ONE\"}"},
  NegativeTestCase{name = "InvalidName", json = "{\"enumValue\": \"INVALID_ENUM\"}"},
  NegativeTestCase{name = "InvalidNameWithValue", json = "{\"enumValue\": \"INVALID_ENUM (1)\"}"},
  NegativeTestCase{name = "ExtraWhitespace", json = "{\"enumValue\": \"ONE \"}"},
  NegativeTestCase{name = "ExtraWhitespace2", json = "{\"enumValue\": \"1 \"}"},
  NegativeTestCase{name = "MissingCloseParen", json = "{\"enumValue\": \"ONE (1\"}"},
  NegativeTestCase{name = "EmptyParentheses", json = "{\"enumValue\": \"ONE ()\"}"},
  NegativeTestCase{name = "ReversedNameValue", json = "{\"enumValue\": \"1 (ONE)\"}"},
  NegativeTestCase{name = "OnlyOpenParen", json = "{\"enumValue\": \"ONE (\"}"},
  NegativeTestCase{name = "OnlyCloseParen", json = "{\"enumValue\": \"ONE )\"}"},
  NegativeTestCase{name = "ParenNotAtEnd", json = "{\"enumValue\": \"ONE (1) extra\"}"},
  NegativeTestCase{name = "NonIntegerInParens", json = "{\"enumValue\": \"ONE (abc)\"}"},
  NegativeTestCase{name = "Float", json = "{\"enumValue\": 1.0}"},
  NegativeTestCase{name = "FloatInString", json = "{\"enumValue\": \"1.0\"}"},
  NegativeTestCase{name = "FloatInParens", json = "{\"enumValue\": \"ONE (1.0)\"}"},
  NegativeTestCase{name = "KeyMismatch", json = "{\"enumAsKey\": {\"ONE (2)\": \"TWO (2)\"}}"},
  NegativeTestCase{name = "KeyInvalidName", json = "{\"enumAsKey\": {\"INVALID\": \"TWO\"}}"},
  NegativeTestCase{name = "ValueMismatch", json = "{\"enumAsKey\": {\"ONE (1)\": \"TWO (1)\"}}"},
  NegativeTestCase{name = "ValueInvalidName", json = "{\"enumAsKey\": {\"ONE (1)\": \"INVALID (2)\"}}"},
];

const list<NegativeTestCase> typeValidationNegativeCases = [
  NegativeTestCase{name = "InvalidBinaryKey", json = "{\"binaryValue\": {\"invalid\": \"data\"}}"},
  NegativeTestCase{name = "ListGotString", json = "{\"listValue\": \"not_an_array\"}"},
  NegativeTestCase{name = "ListGotObject", json = "{\"listValue\": {\"nested\": \"item1\"}}"},
  NegativeTestCase{name = "IntegerInvalidBool", json = "{\"i64Value\": true}"},
  NegativeTestCase{name = "IntegerInvalidFloat", json = "{\"i64Value\": 0.0}"},
  NegativeTestCase{name = "IntegerInvalidString", json = "{\"i64Value\": \"123_not_a_number\"}"},
  NegativeTestCase{name = "IntegerEmptyString", json = "{\"i64Value\": \"\"}"},
  NegativeTestCase{name = "FloatInvalidBool", json = "{\"floatValue\": true}"},
  NegativeTestCase{name = "FloatInvalidString", json = "{\"floatValue\": \"123_not_a_float\"}"},
  NegativeTestCase{name = "FloatEmptyString", json = "{\"floatValue\": \"\"}"},
  NegativeTestCase{name = "FloatPrecisionLoss", json = "{\"floatValue\": 123456789}"},
  NegativeTestCase{name = "FloatPrecisionLossString", json = "{\"floatValue\": \"123456789\"}"},
  NegativeTestCase{name = "BoolGotNumber", json = "{\"boolValue\": 1}"},
  NegativeTestCase{name = "BoolKeyInvalidString", json = "{\"boolAsKey\": {\"yes\": 42}}"},
  NegativeTestCase{name = "BoolKeyNumericString", json = "{\"boolAsKey\": {\"1\": 42}}"},
];

const list<NegativeTestCase> formatValidationNegativeCases = [
  NegativeTestCase{name = "MapMissingKey", json = "{\"i32AsKey\": [{\"value\": 2}]}"},
  NegativeTestCase{name = "MapMissingValue", json = "{\"i32AsKey\": [{\"key\": 1}]}"},
  NegativeTestCase{name = "MapExtraElement", json = "{\"i32AsKey\": [{\"key\": 1, \"value\": 2, \"foo\": 3}]}"},
  NegativeTestCase{name = "MissingClosingBrace", json = "{\"i64Value\": 42"},
  NegativeTestCase{name = "ExtraComma", json = "{\"i64Value\": 42,,}"},
  NegativeTestCase{name = "EmptyInput", json = ""},
  NegativeTestCase{name = "ExtraContent", json = "{}garbage"},
  NegativeTestCase{name = "FieldNameNotFound", json = "{\"NonExist (30)\": true}"},
  NegativeTestCase{name = "FieldIdConflict", json = "{\"boolValue (31)\": true}"},
];

const list<NegativeTestCase> overflowValidationNegativeCases = [
  // byte overflow (range: -128 to 127)
  NegativeTestCase{name = "BytePositiveOverflow", json = "{\"byteValue\": 128}"},
  NegativeTestCase{name = "ByteNegativeOverflow", json = "{\"byteValue\": -129}"},
  // i16 overflow (range: -32768 to 32767)
  NegativeTestCase{name = "I16PositiveOverflow", json = "{\"i16Value\": 32768}"},
  NegativeTestCase{name = "I16NegativeOverflow", json = "{\"i16Value\": -32769}"},
  // i32 overflow (range: -2147483648 to 2147483647)
  NegativeTestCase{name = "I32PositiveOverflow", json = "{\"i32Value\": 2147483648}"},
  NegativeTestCase{name = "I32NegativeOverflow", json = "{\"i32Value\": -2147483649}"},
  // i64 overflow (range: -9223372036854775808 to 9223372036854775807)
  NegativeTestCase{name = "I64StringPositiveOverflow", json = "{\"i64Value\": \"9223372036854775808\"}"},
  NegativeTestCase{name = "I64StringNegativeOverflow", json = "{\"i64Value\": \"-9223372036854775809\"}"},
  // enum overflow (range: -2147483648 to 2147483647)
  NegativeTestCase{name = "EnumPositiveOverflow1", json = "{\"enumValue\": 2147483648}"},
  NegativeTestCase{name = "EnumPositiveOverflow2", json = "{\"enumValue\": \"2147483648\"}"},
  NegativeTestCase{name = "EnumPositiveOverflow3", json = "{\"enumValue\": \"(2147483648)\"}"},
  NegativeTestCase{name = "EnumNegativeOverflow1", json = "{\"enumValue\": -2147483649}"},
  NegativeTestCase{name = "EnumNegativeOverflow2", json = "{\"enumValue\": \"-2147483649\"}"},
  NegativeTestCase{name = "EnumNegativeOverflow3", json = "{\"enumValue\": \"(-2147483649)\"}"},
];
