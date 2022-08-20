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

#ifndef _THRIFT_CONCURRENCY_EXCEPTION_H_
#define _THRIFT_CONCURRENCY_EXCEPTION_H_ 1

#include <exception>

#include <thrift/lib/cpp/Thrift.h>

namespace apache {
namespace thrift {
namespace concurrency {

class FOLLY_EXPORT NoSuchTaskException
    : public apache::thrift::TLibraryException {};

class FOLLY_EXPORT UncancellableTaskException
    : public apache::thrift::TLibraryException {};

class FOLLY_EXPORT InvalidArgumentException
    : public apache::thrift::TLibraryException {};

class FOLLY_EXPORT IllegalStateException
    : public apache::thrift::TLibraryException {
 public:
  IllegalStateException() {}
  IllegalStateException(const std::string& message)
      : TLibraryException(message) {}
};

class FOLLY_EXPORT TimedOutException
    : public apache::thrift::TLibraryException {
 public:
  TimedOutException() : TLibraryException("TimedOutException") {}
  TimedOutException(const std::string& message) : TLibraryException(message) {}
};

class FOLLY_EXPORT TooManyPendingTasksException
    : public apache::thrift::TLibraryException {
 public:
  TooManyPendingTasksException()
      : TLibraryException("TooManyPendingTasksException") {}
  TooManyPendingTasksException(const std::string& message)
      : TLibraryException(message) {}
};

class FOLLY_EXPORT SystemResourceException
    : public apache::thrift::TLibraryException {
 public:
  SystemResourceException() {}

  SystemResourceException(const std::string& message)
      : TLibraryException(message) {}
};

} // namespace concurrency
} // namespace thrift
} // namespace apache

#endif // #ifndef _THRIFT_CONCURRENCY_EXCEPTION_H_
