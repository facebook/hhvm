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

#include <thrift/lib/cpp/transport/TFDTransport.h>

#include <cerrno>
#include <exception>

#include <folly/portability/Unistd.h>

using namespace std;

namespace apache {
namespace thrift {
namespace transport {

void TFDTransport::close() {
  if (!isOpen()) {
    return;
  }

  int rv = ::close(fd_);
  int errno_copy = errno;
  fd_ = -1;
  // Have to check uncaught_exception because this is called in the destructor.
  if (rv < 0 && std::uncaught_exceptions() == 0) {
    throw TTransportException(
        TTransportException::UNKNOWN, "TFDTransport::close()", errno_copy);
  }
}

uint32_t TFDTransport::read(uint8_t* buf, uint32_t len) {
  unsigned int maxRetries = 5; // same as the TSocket default
  unsigned int retries = 0;
  while (true) {
    ssize_t rv = ::read(fd_, buf, len);
    if (rv < 0) {
      if (errno == EINTR && retries < maxRetries) {
        // If interrupted, try again
        ++retries;
        continue;
      }
      int errno_copy = errno;
      throw TTransportException(
          TTransportException::UNKNOWN, "TFDTransport::read()", errno_copy);
    }
    return rv;
  }
}

void TFDTransport::write(const uint8_t* buf, uint32_t len) {
  while (len > 0) {
    ssize_t rv = ::write(fd_, buf, len);

    if (rv < 0) {
      int errno_copy = errno;
      throw TTransportException(
          TTransportException::UNKNOWN, "TFDTransport::write()", errno_copy);
    } else if (rv == 0) {
      throw TTransportException(
          TTransportException::END_OF_FILE, "TFDTransport::write()");
    }

    buf += rv;
    len -= rv;
  }
}

} // namespace transport
} // namespace thrift
} // namespace apache
