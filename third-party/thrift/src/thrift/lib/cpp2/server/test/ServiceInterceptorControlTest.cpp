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

#include <folly/portability/GTest.h>

#include <folly/String.h>
#include <thrift/lib/cpp2/async/InterceptorFlags.h>
#include <thrift/lib/cpp2/server/ServiceInterceptorControl.h>

using namespace ::testing;

namespace apache::thrift {

namespace {

struct TestControl {
  explicit TestControl(std::string moduleName, std::string interceptorName)
      : name{}, controller{name} {
    name.setName(std::move(moduleName), std::move(interceptorName));
  }

  ServiceInterceptorQualifiedName name;
  ServiceInterceptorControl controller;
};

struct TestControllers {
  TestControl m0I0 = TestControl("M0", "I0");
  TestControl m0I1 = TestControl("M0", "I1");
  TestControl m1I0 = TestControl("M1", "I0");
  TestControl m2I1 = TestControl("M2", "I1");
};

class ServiceInterceptorControlTest : public Test {
 protected:
  void setFlagValue(const std::string& value) {
    THRIFT_FLAG_SET_MOCK(disabled_service_interceptors, value);
  }

  void expectDisabled(const TestControl& testController) {
    EXPECT_TRUE(testController.controller.isDisabled()) << fmt::format(
        "{} is not disabled, but should be", testController.name.get());
  }

  void expectNotDisabled(const TestControl& testController) {
    EXPECT_FALSE(testController.controller.isDisabled()) << fmt::format(
        "{} is disabled, but should not be", testController.name.get());
  }

  TestControllers testControllers;
};

} // namespace

TEST_F(ServiceInterceptorControlTest, DefaultEmptyDisabledInterceptors) {
  expectNotDisabled(testControllers.m0I0);
  expectNotDisabled(testControllers.m0I1);
  expectNotDisabled(testControllers.m1I0);
  expectNotDisabled(testControllers.m2I1);
}

TEST_F(ServiceInterceptorControlTest, SingleDisabledInterceptor) {
  setFlagValue("M0.I0");
  expectDisabled(testControllers.m0I0);
  expectNotDisabled(testControllers.m0I1);
  expectNotDisabled(testControllers.m1I0);
  expectNotDisabled(testControllers.m2I1);
}

TEST_F(ServiceInterceptorControlTest, MultipleDisabledInterceptors) {
  setFlagValue("M0.I0,M1.I0");
  expectDisabled(testControllers.m0I0);
  expectNotDisabled(testControllers.m0I1);
  expectDisabled(testControllers.m1I0);
  expectNotDisabled(testControllers.m2I1);
}

TEST_F(ServiceInterceptorControlTest, UpdateDisabledInterceptors) {
  setFlagValue("M0.I0,M1.I0");
  expectDisabled(testControllers.m0I0);
  expectNotDisabled(testControllers.m0I1);
  expectDisabled(testControllers.m1I0);
  expectNotDisabled(testControllers.m2I1);

  setFlagValue("M0.I1,M2.I1");
  expectNotDisabled(testControllers.m0I0);
  expectDisabled(testControllers.m0I1);
  expectNotDisabled(testControllers.m1I0);
  expectDisabled(testControllers.m2I1);
}

TEST_F(ServiceInterceptorControlTest, DisableModule) {
  setFlagValue("M0");
  expectDisabled(testControllers.m0I0);
  expectDisabled(testControllers.m0I1);
  expectNotDisabled(testControllers.m1I0);
  expectNotDisabled(testControllers.m2I1);
}

TEST_F(ServiceInterceptorControlTest, DisableModuleAndInterceptor) {
  setFlagValue("M2,M0.I0");
  expectDisabled(testControllers.m0I0);
  expectNotDisabled(testControllers.m0I1);
  expectNotDisabled(testControllers.m1I0);
  expectDisabled(testControllers.m2I1);
}

} // namespace apache::thrift
