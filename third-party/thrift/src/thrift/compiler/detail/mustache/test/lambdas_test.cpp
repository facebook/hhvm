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

// A lambda's return value should be interpolated.
TEST(LambdasTEST, Interpolation) {
  EXPECT_EQ(
      "Hello, world!",
      mstch::render(
          "Hello, {{lambda}}!",
          mstch::map{{"lambda", mstch::lambda{[]() -> mstch::node {
                        return std::string{"world"};
                      }}}}));
}
// A lambda's return value should be parsed.
TEST(LambdasTEST, InterpolationExpansion) {
  EXPECT_EQ(
      "Hello, world!",
      mstch::render(
          "Hello, {{lambda}}!",
          mstch::map{
              {"planet", std::string("world")},
              {"lambda", mstch::lambda{[]() -> mstch::node {
                 return std::string{"{{planet}}"};
               }}}}));
}
// Interpolated lambdas should not be cached.
TEST(LambdasTEST, InterpolationMultipleCalls) {
  int32_t var = 0;
  EXPECT_EQ(
      "1 == 2 == 3",
      mstch::render(
          "{{lambda}} == {{lambda}} == {{lambda}}",
          mstch::map{{"lambda", mstch::lambda{[&var]() -> mstch::node {
                        return ++var;
                      }}}}));
}
// Lambdas used for sections should receive the raw section string.
TEST(LambdasTEST, Section) {
  EXPECT_EQ(
      "<yes>",
      mstch::render(
          "<{{#lambda}}{{x}}{{/lambda}}>",
          mstch::map{
              {"x", std::string("Error!")},
              {"lambda",
               mstch::lambda{[](const std::string& text) -> mstch::node {
                 return text == "{{x}}" ? std::string("yes")
                                        : std::string("no");
               }}}}));
}
// Lambdas used for sections should have their results parsed.
TEST(LambdasTEST, SectionExpansion) {
  EXPECT_EQ(
      "<-Earth->",
      mstch::render(
          "<{{#lambda}}-{{/lambda}}>",
          mstch::map{
              {"planet", std::string("Earth")},
              {"lambda",
               mstch::lambda{[](const std::string& text) -> mstch::node {
                 return text + "{{planet}}" + text;
               }}}}));
}
// Lambdas used for sections should not be cached.
TEST(LambdasTEST, SectionMultipleCalls) {
  EXPECT_EQ(
      "__FILE__ != __LINE__",
      mstch::render(
          "{{#lambda}}FILE{{/lambda}} != {{#lambda}}LINE{{/lambda}}",
          mstch::map{
              {"lambda",
               mstch::lambda{[](const std::string& text) -> mstch::node {
                 return "__" + text + "__";
               }}}}));
}
// Lambdas used for inverted sections should be considered truthy.
TEST(LambdasTEST, InvertedSection) {
  EXPECT_EQ(
      "<>",
      mstch::render(
          "<{{^lambda}}{{static}}{{/lambda}}>",
          mstch::map{{"lambda", mstch::lambda{[]() -> mstch::node {
                        return false;
                      }}}}));
}
