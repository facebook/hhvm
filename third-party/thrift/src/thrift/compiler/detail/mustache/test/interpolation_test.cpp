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

#include <thrift/compiler/detail/mustache/mstch.h>

using namespace apache::thrift;
// Mustache-free templates should render as-is.
TEST(InterpolationTEST, NoInterpolation) {
  EXPECT_EQ(
      "Hello from {Mustache}!\n",
      mstch::render("Hello from {Mustache}!\n", mstch::node()));
}
// Unadorned tags should interpolate content into the template.
TEST(InterpolationTEST, BasicInterpolation) {
  EXPECT_EQ(
      "Hello, world!\n",
      mstch::render(
          "Hello, {{subject}}!\n",
          mstch::map{{"subject", std::string("world")}}));
}
// Integers should interpolate seamlessly.
TEST(InterpolationTEST, BasicIntegerInterpolation) {
  EXPECT_EQ(
      "\"85 miles an hour!\"",
      mstch::render("\"{{mph}} miles an hour!\"", mstch::map{{"mph", 85}}));
}
// Decimals should interpolate seamlessly with proper significance.
TEST(InterpolationTEST, BasicDecimalInterpolation) {
  EXPECT_EQ(
      "\"1.21 jiggawatts!\"",
      mstch::render("\"{{power}} jiggawatts!\"", mstch::map{{"power", 1.21}}));
}
// Failed context lookups should default to empty strings.
TEST(InterpolationTEST, BasicContextMissInterpolation) {
  EXPECT_EQ(
      "I () be seen!", mstch::render("I ({{cannot}}) be seen!", mstch::node()));
}
// Dotted names should be considered a form of shorthand for sections.
TEST(InterpolationTEST, DottedNamesBasicInterpolation) {
  EXPECT_EQ(
      "\"Joe\" == \"Joe\"",
      mstch::render(
          "\"{{person.name}}\" == \"{{#person}}{{name}}{{/person}}\"",
          mstch::map{{"person", mstch::map{{"name", std::string("Joe")}}}}));
}
// Dotted names should be functional to any level of nesting.
TEST(InterpolationTEST, DottedNamesArbitraryDepth) {
  EXPECT_EQ(
      "\"Phil\" == \"Phil\"",
      mstch::render(
          "\"{{a.b.c.d.e.name}}\" == \"Phil\"",
          mstch::map{
              {"a",
               mstch::map{
                   {"b",
                    mstch::map{
                        {"c",
                         mstch::map{
                             {"d",
                              mstch::map{
                                  {"e",
                                   mstch::map{
                                       {"name",
                                        std::string("Phil")}}}}}}}}}}}}));
}
// Any falsey value prior to the last part of the name should yield ''.
TEST(InterpolationTEST, DottedNamesBrokenChains) {
  EXPECT_EQ(
      "\"\" == \"\"",
      mstch::render("\"{{a.b.c}}\" == \"\"", mstch::map{{"a", mstch::node()}}));
}
// Each part of a dotted name should resolve only against its parent.
TEST(InterpolationTEST, DottedNamesBrokenChainResolution) {
  EXPECT_EQ(
      "\"\" == \"\"",
      mstch::render(
          "\"{{a.b.c.name}}\" == \"\"",
          mstch::map{
              {"a", mstch::map{{"b", mstch::node()}}},
              {"c", mstch::map{{"name", std::string("Jim")}}}}));
}
// The first part of a dotted name should resolve as any other name.
TEST(InterpolationTEST, DottedNamesInitialResolution) {
  EXPECT_EQ(
      "\"Phil\" == \"Phil\"",
      mstch::render(
          "\"{{#a}}{{b.c.d.e.name}}{{/a}}\" == \"Phil\"",
          mstch::map{
              {"a",
               mstch::map{
                   {"b",
                    mstch::map{
                        {"c",
                         mstch::map{
                             {"d",
                              mstch::map{
                                  {"e",
                                   mstch::map{
                                       {"name", std::string("Phil")}}}}}}}}}}},
              {"b",
               mstch::map{
                   {"c",
                    mstch::map{
                        {"d",
                         mstch::map{
                             {"e",
                              mstch::map{
                                  {"name", std::string("Wrong")}}}}}}}}}}));
}
// Interpolation should not alter surrounding whitespace.
TEST(InterpolationTEST, InterpolationSurroundingWhitespace) {
  EXPECT_EQ(
      "| --- |",
      mstch::render(
          "| {{string}} |", mstch::map{{"string", std::string("---")}}));
}
// Standalone interpolation should not alter surrounding whitespace.
TEST(InterpolationTEST, InterpolationStandalone) {
  EXPECT_EQ(
      "  ---\\n",
      mstch::render(
          "  {{string}}\\n", mstch::map{{"string", std::string("---")}}));
}
// Superfluous in-tag whitespace should be ignored.
TEST(InterpolationTEST, InterpolationWithPadding) {
  EXPECT_EQ(
      "|---|",
      mstch::render(
          "|{{ string }}|", mstch::map{{"string", std::string("---")}}));
}
