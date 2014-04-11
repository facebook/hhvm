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

#include "thrift/lib/cpp/util/FdUtils.h"

#include <fcntl.h>

namespace apache { namespace thrift { namespace util {

int setCloseOnExec(int fd, int value) {
  // Read the current flags
  int old_flags = fcntl(fd, F_GETFD, 0);

  // If reading the flags failed, return error indication now
  if (old_flags < 0)
    return -1;

  // Set just the flag we want to set
  int new_flags;
  if (value != 0)
    new_flags = old_flags | FD_CLOEXEC;
  else
    new_flags = old_flags & ~FD_CLOEXEC;

  // Store modified flag word in the descriptor
  return fcntl(fd, F_SETFD, new_flags);
}

}}} // apache::thrift::util
