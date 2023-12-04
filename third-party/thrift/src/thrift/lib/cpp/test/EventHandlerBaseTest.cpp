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

#include <thrift/lib/cpp/EventHandlerBase.h>

#include <folly/portability/GTest.h>

using namespace std;
using namespace folly;
using namespace apache::thrift;

class lulz : public exception {
 public:
  explicit lulz(string message) noexcept : message_(std::move(message)) {}
  const char* what() const noexcept override { return message_.c_str(); }

 private:
  string message_;
};

namespace {

class EventHandler : public TProcessorEventHandler {
 public:
  string ex_type;
  string ex_what;
  void userException(
      void*,
      const char*,
      const std::string& ex_type,
      const std::string& ex_what) override {
    this->ex_type = ex_type;
    this->ex_what = ex_what;
  }
};

template <class E>
exception_ptr to_eptr(const E& e) {
  try {
    throw e;
  } catch (E&) {
    return current_exception();
  }
}

class TProcessorEventHandlerTest : public testing::Test {};

class TClientTester : public TClientBase {};

} // namespace

TEST_F(TProcessorEventHandlerTest, with_full_wrapped_eptr) {
  auto wrap = exception_wrapper(lulz("hello"));
  auto eptr = wrap.to_exception_ptr();
  EXPECT_TRUE(wrap.has_exception_ptr());
  EXPECT_EQ("lulz", wrap.class_name().toStdString());
  EXPECT_EQ("lulz: hello", wrap.what().toStdString());

  EventHandler eh;
  eh.userExceptionWrapped(nullptr, nullptr, false, wrap);
  EXPECT_EQ("lulz", eh.ex_type);
  EXPECT_EQ("lulz: hello", eh.ex_what);
}

TEST_F(TProcessorEventHandlerTest, with_wrap_surprise) {
  auto wrap = exception_wrapper(lulz("hello"));
  EXPECT_EQ("lulz", wrap.class_name().toStdString());
  EXPECT_EQ("lulz: hello", wrap.what().toStdString());

  EventHandler eh;
  eh.userExceptionWrapped(nullptr, nullptr, false, wrap);
  EXPECT_EQ("lulz", eh.ex_type);
  EXPECT_EQ("lulz: hello", eh.ex_what);
}

TEST_F(TProcessorEventHandlerTest, with_wrap_declared) {
  auto wrap = exception_wrapper(lulz("hello"));
  EXPECT_EQ("lulz", wrap.class_name().toStdString());
  EXPECT_EQ("lulz: hello", wrap.what().toStdString());

  EventHandler eh;
  eh.userExceptionWrapped(nullptr, nullptr, true, wrap);
  EXPECT_EQ("lulz", eh.ex_type);
  EXPECT_EQ("hello", eh.ex_what);
}

TEST_F(TProcessorEventHandlerTest, registerProcessorHandler) {
  auto countProcessor = [](const auto& h) {
    return std::count(
        TProcessorBase::getHandlers().begin(),
        TProcessorBase::getHandlers().end(),
        h);
  };
  auto countClient = [](const auto& h) {
    TClientTester tct;
    return std::count(
        tct.getEventHandlers().begin(), tct.getEventHandlers().end(), h);
  };

  auto h = std::make_shared<EventHandler>();
  EXPECT_FALSE(countProcessor(h));
  EXPECT_FALSE(countClient(h));
  TProcessorBase::addProcessorEventHandler(h);
  EXPECT_TRUE(countProcessor(h));
  EXPECT_FALSE(countClient(h));
  TClientBase::addClientEventHandler(h);
  EXPECT_TRUE(countProcessor(h));
  EXPECT_TRUE(countClient(h));
  TProcessorBase::removeProcessorEventHandler(h);
  EXPECT_FALSE(countProcessor(h));
  EXPECT_TRUE(countClient(h));
  TClientBase::removeClientEventHandler(h);
  EXPECT_FALSE(countProcessor(h));
  EXPECT_FALSE(countClient(h));
}

TEST_F(TProcessorEventHandlerTest, registrationActivity) {
  auto count = []() { return TProcessorBase::getHandlers().size(); };

  auto h1 = std::make_shared<EventHandler>();
  auto h2 = std::make_shared<EventHandler>();
  auto h3 = std::make_shared<EventHandler>();

  EXPECT_EQ(count(), 0);
  TProcessorBase::addProcessorEventHandler(h1);
  EXPECT_EQ(count(), 1);
  TProcessorBase::removeProcessorEventHandler(h1);
  EXPECT_EQ(count(), 0);
  TProcessorBase::addProcessorEventHandler(h1);
  EXPECT_EQ(count(), 1);
  TProcessorBase::addProcessorEventHandler(h2);
  EXPECT_EQ(count(), 2);
  TProcessorBase::removeProcessorEventHandler(h1);
  EXPECT_EQ(count(), 1);
  TProcessorBase::addProcessorEventHandler(h3);
  EXPECT_EQ(count(), 2);
  TProcessorBase::removeProcessorEventHandler(h3);
  EXPECT_EQ(count(), 1);
  TProcessorBase::removeProcessorEventHandler(h2);
  EXPECT_EQ(count(), 0);
}
