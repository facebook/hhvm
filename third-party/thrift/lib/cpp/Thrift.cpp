/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "thrift/lib/cpp/Thrift.h"
#include "thrift/lib/cpp/config.h"
#include <cstring>
#include <boost/lexical_cast.hpp>
#include <stdarg.h>
#include <stdio.h>

namespace apache { namespace thrift {

TOutput GlobalOutput;

void TOutput::printf(const char *message, ...) {
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

  char *heap_buf = (char*)malloc((need+1) * sizeof(char));
  if (heap_buf == nullptr) {
    // Malloc failed.  We might as well print the stack buffer.
    f_(stack_buf);
    return;
  }

  va_start(ap, message);
  int rval = vsnprintf(heap_buf, need+1, message, ap);
  va_end(ap);
  // TODO(shigin): inform user
  if (rval != -1) {
    f_(heap_buf);
  }
  free(heap_buf);
}

void TOutput::perror(const char *message, int errno_copy) {
  std::string out = message + strerror_s(errno_copy);
  f_(out.c_str());
}

std::string TOutput::strerror_s(int errno_copy) {
#ifndef HAVE_STRERROR_R
  return "errno = " + boost::lexical_cast<std::string>(errno_copy);
#else  // HAVE_STRERROR_R

  char b_errbuf[1024] = { '\0' };
#ifdef STRERROR_R_CHAR_P
  char *b_error = strerror_r(errno_copy, b_errbuf, sizeof(b_errbuf));
#else
  char *b_error = b_errbuf;
  int rv = strerror_r(errno_copy, b_errbuf, sizeof(b_errbuf));
  if (rv == -1) {
    // strerror_r failed.  omgwtfbbq.
    return "XSI-compliant strerror_r() failed with errno = " +
      boost::lexical_cast<std::string>(errno_copy);
  }
#endif
  // Can anyone prove that explicit cast is probably not necessary
  // to ensure that the string object is constructed before
  // b_error becomes invalid?
  return std::string(b_error);

#endif  // HAVE_STRERROR_R
}

TLibraryException::TLibraryException(const char* message, int errnoValue) {
  message_ = std::string(message) + ": " + TOutput::strerror_s(errnoValue);
}

}} // apache::thrift
