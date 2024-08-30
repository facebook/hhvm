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

#include <thrift/compiler/whisker/object.h>
#include <thrift/compiler/whisker/test/render_test_helpers.h>

namespace w = whisker::make;

namespace whisker {

class InterpolationTest : public RenderTest {};

// NOTE:
//   Whisker intentionally does not support HTML escaping.
//   This means that Whisker is not conformant with the
//   Ampersand and TripleMustache variants of the following tests.
//   They have been omitted but can be found in the original spec.
//
// https://github.com/mustache/spec/blob/v1.4.2/specs/interpolation.yml

// Mustache-free templates should render as-is.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/interpolation.yml#L28-L34
TEST_F(InterpolationTest, NoInterpolation) {
  EXPECT_EQ(
      "Hello from {Mustache}!\n",
      *render("Hello from {Mustache}!\n", w::map({})));
}

// Unadorned tags should interpolate content into the template.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/interpolation.yml#L36-L42
TEST_F(InterpolationTest, BasicInterpolation) {
  EXPECT_EQ(
      "Hello, world!\n",
      *render(
          "Hello, {{subject}}!\n", w::map({{"subject", w::string("world")}})));
}

// Interpolated tag output should not be re-interpolated.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/interpolation.yml#L44-L48
TEST_F(InterpolationTest, NoReInterpolation) {
  EXPECT_EQ(
      "{{planet}}: Earth",
      *render(
          "{{template}}: {{planet}}",
          w::map(
              {{"template", w::string("{{planet}}")},
               {"planet", w::string("Earth")}})));
}

// Integers should interpolate seamlessly.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/interpolation.yml#L74-L78
TEST_F(InterpolationTest, BasicIntegerInterpolation) {
  EXPECT_EQ(
      "\"85 miles an hour!\"",
      *render("\"{{mph}} miles an hour!\"", w::map({{"mph", w::i64(85)}})));
}

// Decimals should interpolate seamlessly with proper significance.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/interpolation.yml#L92-L96
TEST_F(InterpolationTest, BasicDecimalInterpolation) {
  strict_printable_types = diagnostic_level::debug;
  EXPECT_EQ(
      "\"1.21 jiggawatts!\"",
      *render("\"{{power}} jiggawatts!\"", w::map({{"power", w::f64(1.21)}})));
}

// Nulls should interpolate as the empty string.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/interpolation.yml#L110-L114
TEST_F(InterpolationTest, BasicNullInterpolation) {
  strict_printable_types = diagnostic_level::debug;
  EXPECT_EQ(
      "I () be seen!",
      *render("I ({{cannot}}) be seen!", w::map({{"cannot", w::null}})));
}

// Failed context lookups should default to empty strings.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/interpolation.yml#L130-L134
TEST_F(InterpolationTest, BasicContextMissInterpolation) {
  strict_undefined_variables = diagnostic_level::debug;
  strict_printable_types = diagnostic_level::debug;
  EXPECT_EQ("I () be seen!", *render("I ({{cannot}}) be seen!", w::map({})));
}

// Dotted names should be considered a form of shorthand for sections.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/interpolation.yml#L150-L154
TEST_F(InterpolationTest, DottedNamesBasicInterpolation) {
  EXPECT_EQ(
      "\"Joe\" == \"Joe\"",
      *render(
          "\"{{person.name}}\" == \"{{#person}}{{name}}{{/person}}\"",
          w::map({{"person", w::map({{"name", w::string("Joe")}})}})));
}

// Dotted names should be functional to any level of nesting.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/interpolation.yml#L168-L173
TEST_F(InterpolationTest, DottedNamesArbitraryDepth) {
  EXPECT_EQ(
      "\"Phil\" == \"Phil\"",
      *render(
          "\"{{a.b.c.d.e.name}}\" == \"Phil\"",
          w::map(
              {{"a",
                w::map(
                    {{"b",
                      w::map(
                          {{"c",
                            w::map(
                                {{"d",
                                  w::map(
                                      {{"e",
                                        w::map(
                                            {{"name",
                                              w::string(
                                                  "Phil")}})}})}})}})}})}})));
}

// Any falsey value prior to the last part of the name should yield ''.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/interpolation.yml#L175-L180
TEST_F(InterpolationTest, DottedNamesBrokenChains) {
  strict_undefined_variables = diagnostic_level::debug;
  strict_printable_types = diagnostic_level::debug;
  EXPECT_EQ(
      "\"\" == \"\"",
      *render("\"{{a.b.c}}\" == \"\"", w::map({{"a", w::map({})}})));
}

// Each part of a dotted name should resolve only against its parent.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/interpolation.yml#L182-L188
TEST_F(InterpolationTest, DottedNamesBrokenChainResolution) {
  strict_undefined_variables = diagnostic_level::debug;
  strict_printable_types = diagnostic_level::debug;
  EXPECT_EQ(
      "\"\" == \"\"",
      *render(
          "\"{{a.b.c.name}}\" == \"\"",
          w::map(
              {{"a", w::map({{"b", w::map({})}})},
               {"c", w::map({{"name", w::string("Jim")}})}})));
}

// The first part of a dotted name should resolve as any other name.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/interpolation.yml#L190-L196
TEST_F(InterpolationTest, DottedNamesInitialResolution) {
  EXPECT_EQ(
      "\"Phil\" == \"Phil\"",
      *render(
          "\"{{#a}}{{b.c.d.e.name}}{{/a}}\" == \"Phil\"",
          w::map(
              {{"a",
                w::map(
                    {{"b",
                      w::map(
                          {{"c",
                            w::map(
                                {{"d",
                                  w::map(
                                      {{"e",
                                        w::map(
                                            {{"name",
                                              w::string("Phil")}})}})}})}})}})},
               {"b",
                w::map(
                    {{"c",
                      w::map(
                          {{"d",
                            w::map(
                                {{"e",
                                  w::map(
                                      {{"name",
                                        w::string("Wrong")}})}})}})}})}})));
}

// Dotted names should be resolved against former resolutions.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/interpolation.yml#L198-L204
TEST_F(InterpolationTest, DottedNameContextPrecedence) {
  strict_undefined_variables = diagnostic_level::debug;
  strict_printable_types = diagnostic_level::debug;
  EXPECT_EQ(
      "",
      *render(
          "{{#a}}{{b.c}}{{/a}}",
          w::map(
              {{"a", w::map({{"b", w::map({})}})},
               {"b", w::map({{"c", w::string("ERROR")}})}})));
}

// Dotted names shall not be parsed as single, atomic keys.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/interpolation.yml#L206-L211
TEST_F(InterpolationTest, DottedNamesAreNeverSingleKeys) {
  strict_undefined_variables = diagnostic_level::debug;
  strict_printable_types = diagnostic_level::debug;
  EXPECT_EQ("", *render("{{a.b}}", w::map({{"a.b", w::string("c")}})));
}

// Dotted Names in a given context are unvavailable due to dot splitting.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/interpolation.yml#L213-L219
TEST_F(InterpolationTest, DottedNamesNoMasking) {
  strict_undefined_variables = diagnostic_level::debug;
  strict_printable_types = diagnostic_level::debug;
  EXPECT_EQ(
      "d",
      *render(
          "{{a.b}}",
          w::map(
              {{"a.b", w::string("c")},
               {"a", w::map({{"b", w::string("d")}})}})));
}

// Unadorned tags should interpolate content into the template.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/interpolation.yml#L223-L229
TEST_F(InterpolationTest, ImplicitIteratorsBasicInterpolation) {
  EXPECT_EQ("Hello, world!", *render("Hello, {{.}}!", w::string("world")));
}

// Integers should interpolate seamlessly.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/interpolation.yml#L255-L259
TEST_F(InterpolationTest, ImplicitIteratorsBasicIntegerInterpolation) {
  EXPECT_EQ("85 miles an hour!", *render("{{.}} miles an hour!", w::i64(85)));
}

// Interpolation should not alter surrounding whitespace.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/interpolation.yml#L263-L267
TEST_F(InterpolationTest, InterpolationSurroundingWhitespace) {
  EXPECT_EQ(
      "| --- |",
      *render("| {{string}} |", w::map({{"string", w::string("---")}})));
}

// Standalone interpolation should not alter surrounding whitespace.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/interpolation.yml#L281-L285
TEST_F(InterpolationTest, InterpolationStandalone) {
  EXPECT_EQ(
      "  ---\\n",
      *render("  {{string}}\\n", w::map({{"string", w::string("---")}})));
}

// Superfluous in-tag whitespace should be ignored.
//   https://github.com/mustache/spec/blob/v1.4.2/specs/interpolation.yml#L301-L305
TEST_F(InterpolationTest, InterpolationWithPadding) {
  EXPECT_EQ(
      "|---|",
      *render("|{{ string }}|", w::map({{"string", w::string("---")}})));
}

} // namespace whisker
