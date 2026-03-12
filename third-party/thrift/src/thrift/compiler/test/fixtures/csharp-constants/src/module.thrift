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

// Minimal fixture for testing C# constant code generation.
// This fixture intentionally excludes struct/union constants which require
// struct codegen (added in a later diff). Delete this fixture once the full
// constants fixture compiles cleanly with struct support.

package "test.dev/fixtures/csharp_constants"

namespace csharp Test.Fixtures.CsharpConstants

// Primitive constants
const i32 myInt = 1337;
const i64 myLong = 9223372036854775807;
const string name = "Mark Zuckerberg";
const double pi = 3.14159;
const bool enabled = true;
const bool disabled = false;

// Enum
enum Color {
  RED = 0,
  GREEN = 1,
  BLUE = 2,
}

const Color favoriteColor = GREEN;

// List constants
const list<i32> numbers = [1, 2, 3, 4, 5];
const list<string> names = ["Alice", "Bob", "Charlie"];
const list<Color> palette = [RED, GREEN, BLUE];

// Set constants
const set<i32> uniqueNumbers = [1, 2, 3];
const set<string> tags = ["alpha", "beta"];

// Map constants
const map<string, i32> ages = {"Alice": 30, "Bob": 25};
const map<i32, string> idToName = {1: "One", 2: "Two"};
const map<string, Color> colorLookup = {"red": RED, "green": GREEN};

// Empty collections
const list<i32> emptyList = [];
const set<string> emptySet = [];
const map<i32, i32> emptyMap = {};

// String escape sequences
const string apostrophe = "'";
const string quote = '"';
const string backslash = "\\";
const string newline = "\n";
const string tab = "\t";

// Binary constant
const binary binData = "a\x00z";

// Boundary values
const i64 maxLong = 9223372036854775807;
const i64 minLong = -9223372036854775808;
const double zero = 0.0;
