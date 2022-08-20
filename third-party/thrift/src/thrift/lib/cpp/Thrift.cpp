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

#include <thrift/lib/cpp/Thrift.h>

#include <stdarg.h>
#include <stdio.h>

#include <folly/String.h>

namespace apache {
namespace thrift {

TOutput GlobalOutput;

void TOutput::printf(const char* message, ...) {
  // Try to reduce heap usage, even if printf is called rarely.
  static const int STACK_BUF_SIZE = 256;
  char stack_buf[STACK_BUF_SIZE];
  va_list ap;

  va_start(ap, message);
  int need = vsnprintf(stack_buf, STACK_BUF_SIZE, message, ap);
  va_end(ap);

  if (need < STACK_BUF_SIZE) {
    f_(stack_buf);
    return;
  }

  char* heap_buf = (char*)malloc((need + 1) * sizeof(char));
  if (heap_buf == nullptr) {
    // Malloc failed.  We might as well print the stack buffer.
    f_(stack_buf);
    return;
  }

  va_start(ap, message);
  int rval = vsnprintf(heap_buf, need + 1, message, ap);
  va_end(ap);
  // TODO(shigin): inform user
  if (rval != -1) {
    f_(heap_buf);
  }
  free(heap_buf);
}

void TOutput::perror(const char* message, int errno_copy) {
  std::string out = message + strerror_s(errno_copy);
  f_(out.c_str());
}

std::string TOutput::strerror_s(int errno_copy) {
  return folly::errnoStr(errno_copy);
}

TLibraryException::TLibraryException(const char* message, int errnoValue) {
  message_ = std::string(message) + ": " + TOutput::strerror_s(errnoValue);
}

} // namespace thrift
} // namespace apache
