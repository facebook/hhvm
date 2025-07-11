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

#include <folly/coro/Task.h>

#include <folly/coro/BlockingWait.h>
#include <folly/executors/GlobalExecutor.h>
#include <folly/futures/Future.h>
#include <folly/portability/GTest.h>

using namespace folly::coro;

Task<int> co_generateFortyTwo() {
  co_return 42;
}

Task<int> co_answerToLife() {
  int answer = co_await co_generateFortyTwo();
  co_return answer;
}

TEST(Task, demo) {
  auto executor = folly::getGlobalCPUExecutor().get();
  int answer = blockingWait(co_withExecutor(executor, co_answerToLife()));

  EXPECT_EQ(answer, 42);

  auto t1 = makeTask(1);
  auto t2 = makeTask(2);
  t1.swap(t2);

  EXPECT_EQ(blockingWait(co_withExecutor(executor, std::move(t1))), 2);
  EXPECT_EQ(blockingWait(co_withExecutor(executor, std::move(t2))), 1);

  auto answerFuture = co_answerToLife().semi().via(executor).then(
      [](folly::Try<int> semiResult) { EXPECT_EQ(semiResult.value(), 42); });

  folly::collectAll(std::move(answerFuture));

  auto voidReturnTask = makeTask();
  EXPECT_NO_THROW(
      blockingWait(co_withExecutor(executor, std::move(voidReturnTask))));

  auto errorYieldingTask = makeErrorTask<void>(
      folly::make_exception_wrapper<std::logic_error>("not really"));
  EXPECT_THROW(blockingWait(std::move(errorYieldingTask)), std::logic_error);

  auto taskifiedTry = makeResultTask(folly::Try<int>(10));
  EXPECT_EQ(
      blockingWait(co_withExecutor(executor, std::move(taskifiedTry))), 10);
}
