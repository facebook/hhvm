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

#include <glog/logging.h>

#include <folly/Portability.h>
#include <folly/init/Init.h>
#include <folly/portability/GFlags.h>
#include <folly/portability/GTest.h>

/*
 * This is the recommended main function for all tests.
 * The Makefile links it into all of the test programs so that tests do not need
 * to - and indeed should typically not - define their own main() functions
 */
FOLLY_ATTR_WEAK int main(int argc, char** argv);

int main(int argc, char** argv) {
  // Enable glog logging to stderr by default.
  FLAGS_logtostderr = true;

  ::testing::InitGoogleTest(&argc, argv);
  folly::Init init(&argc, &argv);

  return RUN_ALL_TESTS();
}
