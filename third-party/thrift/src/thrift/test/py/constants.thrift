/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

namespace py thrift.test.py.constants

include "thrift/test/py/constants_include.thrift"

const string apostrophe = "'";
const string tripleApostrophe = "'''";
const string quotationMark = '"'; //" //fix syntax highlighting
const string quote = 'this is a "quote"';
const string backslash = "\\";
const string escaped_a = "\x61";

const map<string, string> escapeChars = {
  "apostrophe": "'",
  "quotationMark": '"',
  "backslash": "\\",
  "escaped_a": "\x61",
};

const map<string, i32> char2ascii = {"'": 39, '"': 34, "\\": 92, "\x61": 97};

const map<string, constants_include.Foo> str2struct = {
  "foo": {"bar": {"baz": "qux"}},
};
