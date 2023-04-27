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

#include <folly/CancellationToken.h>
#include <folly/portability/GTest.h>

using folly::CancellationSource;
using folly::CancellationToken;

TEST(CancellationSource, demo) {
  CancellationSource src;
  EXPECT_FALSE(src.isCancellationRequested());
  EXPECT_TRUE(src.canBeCancelled());

  CancellationToken token = src.getToken();
  EXPECT_FALSE(token.isCancellationRequested());

  src.requestCancellation();
  EXPECT_TRUE(src.isCancellationRequested());
  EXPECT_TRUE(token.isCancellationRequested());
}
