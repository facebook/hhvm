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

#include <thrift/lib/cpp/transport/TSocket.h>

#include <folly/portability/GTest.h>

using namespace folly;
using namespace apache::thrift::transport;

TEST(TTransportException, invalidTTransportExceptionType) {
  TTransportException tex1;
  EXPECT_STREQ("TTransportException: Unknown transport exception", tex1.what());

  TTransportException tex2(TTransportException::TTransportExceptionType(24));
  EXPECT_STREQ(
      "TTransportException: (Invalid exception type '24')", tex2.what());

  TTransportException tex3(
      TTransportException::TTransportExceptionType(23), "This is my message");
  EXPECT_STREQ("This is my message", tex3.what());

  TTransportException tex4(
      TTransportException::TTransportExceptionType(22),
      "This is my second message",
      0);
  EXPECT_STREQ("This is my second message", tex4.what());

  TTransportException tex5(
      TTransportException::TTransportExceptionType(21),
      "This is my third message",
      1);
  EXPECT_STREQ(
      "This is my third message: Operation not permitted", tex5.what());

  // if we provide an empty string it doesn't take precedence
  TTransportException tex6(
      TTransportException::TTransportExceptionType(20), "");
  EXPECT_STREQ(
      "TTransportException: (Invalid exception type '20')", tex6.what());

  TTransportException tex7(
      TTransportException::TTransportExceptionType(19), "", 1);
  EXPECT_STREQ(
      "TTransportException: (Invalid exception type '19'): Operation not permitted",
      tex7.what());
}
