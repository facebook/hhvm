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

#include <gtest/gtest.h>

#include <thrift/lib/cpp2/test/gen-cpp2/ServiceNameA.h>
#include <thrift/lib/cpp2/test/gen-cpp2/ServiceNameB.h>
#include <thrift/lib/cpp2/test/gen-cpp2/ServiceNameC.h>

namespace apache::thrift::test {

TEST(ServiceNameTest, Basic) {
  apache::thrift::ServiceHandler<ServiceNameA> serviceA;
  EXPECT_EQ(serviceA.getName(), "ServiceNameA");
}

TEST(ServiceNameTest, Derived) {
  apache::thrift::ServiceHandler<ServiceNameB> serviceB;
  EXPECT_EQ(serviceB.getName(), "ServiceNameB");

  apache::thrift::ServiceHandler<ServiceNameC> serviceC;
  EXPECT_EQ(serviceC.getName(), "ServiceNameC");
}

TEST(ServiceNameTest, Override) {
  class ServiceNameX
      : public apache::thrift::ServiceHandler<ServiceNameC>,
        public virtual apache::thrift::ServiceHandler<ServiceNameB> {
   public:
    explicit ServiceNameX(std::string name) {
      setNameOverride(std::move(name));
    }
    ServiceNameX() : ServiceNameX("ServiceNameX") {}
  };
  ServiceNameX serviceX;
  EXPECT_EQ(serviceX.getName(), "ServiceNameX");

  class ServiceNameY : public ServiceNameX {
   public:
    ServiceNameY() { setNameOverride("ServiceNameY"); }
  };
  ServiceNameY serviceY;
  EXPECT_EQ(serviceY.getName(), "ServiceNameY");
}

} // namespace apache::thrift::test
