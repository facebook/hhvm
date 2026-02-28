/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/coro/GtestHelpers.h>

#define CO_TEST_X_(test_suite_name, test_name, parent_class, parent_id)        \
  static_assert(sizeof(GTEST_STRINGIFY_(test_suite_name)) > 1,                 \
                "test_suite_name must not be empty");                          \
  static_assert(sizeof(GTEST_STRINGIFY_(test_name)) > 1,                       \
                "test_name must not be empty");                                \
  class GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)                     \
      : public parent_class {                                                  \
   public:                                                                     \
    GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)() = default;            \
    ~GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)() override = default;  \
    GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)                         \
    (const GTEST_TEST_CLASS_NAME_(test_suite_name, test_name) &) = delete;     \
    GTEST_TEST_CLASS_NAME_(test_suite_name, test_name) &operator=(             \
        const GTEST_TEST_CLASS_NAME_(test_suite_name,                          \
                                     test_name) &) = delete; /* NOLINT */      \
    GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)                         \
    (GTEST_TEST_CLASS_NAME_(test_suite_name, test_name) &&) noexcept = delete; \
    GTEST_TEST_CLASS_NAME_(test_suite_name, test_name) &operator=(             \
        GTEST_TEST_CLASS_NAME_(test_suite_name,                                \
                               test_name) &&) noexcept = delete; /* NOLINT */  \
                                                                               \
   private:                                                                    \
    void TestBody() override;                                                  \
    folly::coro::Task<void> co_TestBody();                                     \
    static ::testing::TestInfo *const test_info_ [[maybe_unused]];             \
  };                                                                           \
                                                                               \
  ::testing::TestInfo *const GTEST_TEST_CLASS_NAME_(test_suite_name,           \
                                                    test_name)::test_info_ =   \
      ::testing::internal::MakeAndRegisterTestInfo(                            \
          #test_suite_name,                                                    \
          #test_name,                                                          \
          nullptr,                                                             \
          nullptr,                                                             \
          ::testing::internal::CodeLocation(__FILE__, __LINE__),               \
          (parent_id),                                                         \
          ::testing::internal::SuiteApiResolver<                               \
              parent_class>::GetSetUpCaseOrSuite(__FILE__, __LINE__),          \
          ::testing::internal::SuiteApiResolver<                               \
              parent_class>::GetTearDownCaseOrSuite(__FILE__, __LINE__),       \
          new ::testing::internal::TestFactoryImpl<GTEST_TEST_CLASS_NAME_(     \
              test_suite_name, test_name)>);                                   \
  void GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)::TestBody() {        \
    try {                                                                      \
      folly::coro::blockingWait(co_TestBody(), getExecutor());                 \
    } catch (...) {                                                            \
      folly::detail::gtestLogCurrentException(GTEST_LOG_(ERROR));              \
      throw;                                                                   \
    }                                                                          \
  }                                                                            \
  folly::coro::Task<void> GTEST_TEST_CLASS_NAME_(test_suite_name,              \
                                                 test_name)::co_TestBody()

// Like CO_TEST_F, but gets a DrivableExecutor from test_suite_name::getExecutor
#define CO_TEST_F_X(test_fixture, test_name) \
  CO_TEST_X_(test_fixture,                   \
             test_name,                      \
             test_fixture,                   \
             ::testing::internal::GetTypeId<test_fixture>())

// Like CO_TEST_P, but gets a DrivableExecutor from test_suite_name::getExecutor
#define CO_TEST_P_X(test_suite_name, test_name)                                \
  class GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)                     \
      : public test_suite_name {                                               \
   public:                                                                     \
    GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)() {                     \
    }                                                                          \
    void TestBody() override;                                                  \
    folly::coro::Task<void> co_TestBody();                                     \
                                                                               \
   private:                                                                    \
    static int AddToRegistry() {                                               \
      ::testing::UnitTest::GetInstance()                                       \
          ->parameterized_test_registry()                                      \
          .GetTestSuitePatternHolder<test_suite_name>(                         \
              GTEST_STRINGIFY_(test_suite_name),                               \
              ::testing::internal::CodeLocation(__FILE__, __LINE__))           \
          ->AddTestPattern(                                                    \
              GTEST_STRINGIFY_(test_suite_name),                               \
              GTEST_STRINGIFY_(test_name),                                     \
              new ::testing::internal::TestMetaFactory<GTEST_TEST_CLASS_NAME_( \
                  test_suite_name, test_name)>(),                              \
              ::testing::internal::CodeLocation(__FILE__, __LINE__));          \
      return 0;                                                                \
    }                                                                          \
    static int gtest_registering_dummy_ [[maybe_unused]];                      \
    GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)                         \
    (const GTEST_TEST_CLASS_NAME_(test_suite_name, test_name) &) = delete;     \
    GTEST_TEST_CLASS_NAME_(test_suite_name, test_name) &operator=(             \
        const GTEST_TEST_CLASS_NAME_(test_suite_name,                          \
                                     test_name) &) = delete; /* NOLINT */      \
  };                                                                           \
  int GTEST_TEST_CLASS_NAME_(test_suite_name,                                  \
                             test_name)::gtest_registering_dummy_ =            \
      GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)::AddToRegistry();     \
  void GTEST_TEST_CLASS_NAME_(test_suite_name, test_name)::TestBody() {        \
    try {                                                                      \
      folly::coro::blockingWait(co_TestBody(), getExecutor());                 \
    } catch (...) {                                                            \
      folly::detail::gtestLogCurrentException(GTEST_LOG_(ERROR));              \
      throw;                                                                   \
    }                                                                          \
  }                                                                            \
  folly::coro::Task<void> GTEST_TEST_CLASS_NAME_(test_suite_name,              \
                                                 test_name)::co_TestBody()
