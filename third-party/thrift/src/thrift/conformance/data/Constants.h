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

#include <string>
#include <vector>

namespace apache::thrift::conformance::data {

constexpr auto kGoodDefNames = {
    "type",
    "Type",
    "other-type",
    "other_type",
    "4",
};

constexpr auto kBadDefNames = {
    "ty%20pe",
    "/type",
    "type?",
    "type?a=",
    "type?a=b",
    "type#",
    "type#1",
    "type@",
    "type@1",
};

constexpr auto kGoodPackageNames = {
    "foo.com/my",
    "foo.com/m_y",
    "foo.com/m-y",
    "foo-bar.com/my",
    "foo.com/my/type",
    "1.2/3",
};

constexpr auto kBadPackageNames = {
    "my",
    "foo/my",
    "Foo.com/my",
    "foo.Com/my",
    "foo.com/My",
    "foo.com:42/my",
    "foo%20.com/my",
    "foo.com/m%20y",
    "@foo.com/my",
    ":@foo.com/my",
    ":foo.com/my",
    "user@foo.com/my",
    "user:pass@foo.com/my",
    "fbthrift://foo.com/my",
    ".com/my",
    "foo./my",
    "./my",
    "/my",
    "foo.com/m#y",
    "foo.com/m@y",
    "foo.com/my/ty@pe",
    "foo_bar.com/my",
};

// Generator functions for creating full URI examples.
std::vector<std::string> genGoodDefUris();
std::vector<std::string> genBadDefUris();

} // namespace apache::thrift::conformance::data
