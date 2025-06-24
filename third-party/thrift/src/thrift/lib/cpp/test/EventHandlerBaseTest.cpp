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

#include <gtest/gtest.h>

#include <thrift/lib/cpp2/runtime/Init.h>

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
      std::string_view,
      std::string_view ex_type,
      std::string_view ex_what) override {
    this->ex_type = std::string(ex_type);
    this->ex_what = std::string(ex_what);
  }
};

template <class E>
std::exception_ptr to_eptr(const E& e) {
  try {
    throw e;
  } catch (E&) {
    return std::current_exception();
  }
}

class TProcessorEventHandlerTest : public testing::Test {};

class TClientTester : public TClientBase {
 public:
  static std::size_t count() {
    TClientTester tct;
    return tct.getEventHandlers().size();
  }
  static std::size_t countOf(
      const std::shared_ptr<TProcessorEventHandler>& handler) {
    TClientTester tct;
    return std::count(
        tct.getEventHandlers().begin(), tct.getEventHandlers().end(), handler);
  }
};

} // namespace

TEST_F(TProcessorEventHandlerTest, with_full_wrapped_eptr) {
  auto wrap = exception_wrapper(lulz("hello"));
  auto eptr = wrap.to_exception_ptr();
  EXPECT_TRUE(wrap.has_exception_ptr());
  EXPECT_EQ("lulz", wrap.class_name().toStdString());
  EXPECT_EQ("lulz: hello", wrap.what().toStdString());

  EventHandler eh;
  eh.userExceptionWrapped(nullptr, "", false, wrap);
  EXPECT_EQ("lulz", eh.ex_type);
  EXPECT_EQ("lulz: hello", eh.ex_what);
}

TEST_F(TProcessorEventHandlerTest, with_wrap_surprise) {
  auto wrap = exception_wrapper(lulz("hello"));
  EXPECT_EQ("lulz", wrap.class_name().toStdString());
  EXPECT_EQ("lulz: hello", wrap.what().toStdString());

  EventHandler eh;
  eh.userExceptionWrapped(nullptr, "", false, wrap);
  EXPECT_EQ("lulz", eh.ex_type);
  EXPECT_EQ("lulz: hello", eh.ex_what);
}

TEST_F(TProcessorEventHandlerTest, with_wrap_declared) {
  auto wrap = exception_wrapper(lulz("hello"));
  EXPECT_EQ("lulz", wrap.class_name().toStdString());
  EXPECT_EQ("lulz: hello", wrap.what().toStdString());

  EventHandler eh;
  eh.userExceptionWrapped(nullptr, "", true, wrap);
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

  auto h = std::make_shared<EventHandler>();
  EXPECT_FALSE(countProcessor(h));
  EXPECT_FALSE(TClientTester::countOf(h));
  TProcessorBase::addProcessorEventHandler_deprecated(h);
  EXPECT_TRUE(countProcessor(h));
  EXPECT_FALSE(TClientTester::countOf(h));
  TClientBase::addClientEventHandler(h);
  EXPECT_TRUE(countProcessor(h));
  EXPECT_TRUE(TClientTester::countOf(h));
  TProcessorBase::removeProcessorEventHandler(h);
  EXPECT_FALSE(countProcessor(h));
  EXPECT_TRUE(TClientTester::countOf(h));
  TClientBase::removeClientEventHandler(h);
  EXPECT_FALSE(countProcessor(h));
  EXPECT_FALSE(TClientTester::countOf(h));
}

TEST_F(TProcessorEventHandlerTest, registrationActivity) {
  auto count = []() { return TProcessorBase::getHandlers().size(); };

  auto h1 = std::make_shared<EventHandler>();
  auto h2 = std::make_shared<EventHandler>();
  auto h3 = std::make_shared<EventHandler>();

  EXPECT_EQ(count(), 0);
  TProcessorBase::addProcessorEventHandler_deprecated(h1);
  EXPECT_EQ(count(), 1);
  TProcessorBase::removeProcessorEventHandler(h1);
  EXPECT_EQ(count(), 0);
  TProcessorBase::addProcessorEventHandler_deprecated(h1);
  EXPECT_EQ(count(), 1);
  TProcessorBase::addProcessorEventHandler_deprecated(h2);
  EXPECT_EQ(count(), 2);
  TProcessorBase::removeProcessorEventHandler(h1);
  EXPECT_EQ(count(), 1);
  TProcessorBase::addProcessorEventHandler_deprecated(h3);
  EXPECT_EQ(count(), 2);
  TProcessorBase::removeProcessorEventHandler(h3);
  EXPECT_EQ(count(), 1);
  TProcessorBase::removeProcessorEventHandler(h2);
  EXPECT_EQ(count(), 0);
}

TEST_F(TProcessorEventHandlerTest, RuntimeInitClientHandlers) {
  auto h1 = std::make_shared<EventHandler>();
  auto h2 = std::make_shared<EventHandler>();

  apache::thrift::runtime::InitOptions options;
  options.legacyClientEventHandlers.push_back(h1);
  apache::thrift::runtime::init(std::move(options));

  EXPECT_EQ(TClientTester::count(), 1);
  EXPECT_EQ(TClientTester::countOf(h1), 1);

  TClientBase::addClientEventHandler(h2);
  EXPECT_EQ(TClientTester::count(), 2);
  EXPECT_EQ(TClientTester::countOf(h1), 1);
  EXPECT_EQ(TClientTester::countOf(h2), 1);
}
