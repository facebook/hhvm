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

#pragma once

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <thrift/compiler/diagnostic.h>

/*
 * Some helper functions for testing like teaching gtest how to print values
 * https://google.github.io/googletest/advanced.html#teaching-googletest-how-to-print-your-values
 */

namespace apache::thrift::compiler {

// Define `PrintTo()` function for `diagnostic`, so that gtest checks will
// print readable `diagnostic` rather than to dump the bytes.
inline void PrintTo(const diagnostic& d, std::ostream* os) {
  fmt::print(*os, "\n{}\n", d);
}

} // namespace apache::thrift::compiler
