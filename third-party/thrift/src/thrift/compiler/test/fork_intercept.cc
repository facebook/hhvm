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

#include <pthread.h>
#include <folly/lang/Exception.h>
#include <thrift/compiler/test/fork_intercept.h>

bool fork_intercept::intercept = []() {
  bool installed = false;
#if FOLLY_HAVE_PTHREAD_ATFORK

  pthread_atfork(
      []() {
        if (!fork_intercept::get_intercept()) {
          // Call the real fork
          return;
        }
        folly::terminate_with<std::runtime_error>(
            "INTERCEPTED: Direct fork() override - intercepted fork() call\n");
      },
      nullptr,
      nullptr);
  installed = true;
#endif
  return installed;
}();
