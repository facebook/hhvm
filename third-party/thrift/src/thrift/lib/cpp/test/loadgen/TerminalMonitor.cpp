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

#include <thrift/lib/cpp/test/loadgen/TerminalMonitor.h>

#include <folly/portability/Unistd.h>

#include <sys/ioctl.h>

namespace apache {
namespace thrift {
namespace loadgen {

TerminalMonitor::TerminalMonitor() : screenHeight_(-1), linesPrinted_(0) {
  // Determine the screen height
  screenHeight_ = getScreenHeight();
}

void TerminalMonitor::initializeInfo() {
  linesPrinted_ = printHeader();
}

void TerminalMonitor::redisplay(uint64_t intervalUsec) {
  // When the headers run off the top of the screen, reprint them
  if (screenHeight_ > 0 && linesPrinted_ > static_cast<size_t>(screenHeight_)) {
    linesPrinted_ = printHeader();
  }

  linesPrinted_ += printInfo(intervalUsec);
}

int32_t TerminalMonitor::getScreenHeight() {
  if (!isatty(STDOUT_FILENO)) {
    return -1;
  }

  struct winsize size;
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) != 0) {
    return -1;
  }

  return size.ws_row;
}

} // namespace loadgen
} // namespace thrift
} // namespace apache
