/* Copyright (c) 2015, 2019, Oracle and/or its affiliates. All rights reserved.

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License, version 2.0,
 as published by the Free Software Foundation.

 This program is also distributed with certain software (including
 but not limited to OpenSSL) that is licensed under separate terms,
 as designated in a particular file or component or in included license
 documentation.  The authors of MySQL hereby grant you an additional
 permission to link the program and your derivative works with the
 separately licensed software that they have included with MySQL.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License, version 2.0, for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include "my_config.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "plugin/x/ngs/include/ngs/memory.h"
#include "plugin/x/src/xpl_error.h"
#include "unittest/gunit/xplugin/xpl/mock/capabilities.h"

const int NUMBER_OF_HANDLERS = 4;
const char *NAMES[NUMBER_OF_HANDLERS] = {"first", "second", "third", "fourth"};

using ::Mysqlx::Connection::Capabilities;
using ::Mysqlx::Connection::Capability;
using ::Mysqlx::Datatypes::Any;

namespace xpl {

namespace test {

using namespace ::testing;

class CapabilitiesConfiguratorTestSuite : public Test {
 public:
  typedef std::shared_ptr<StrictMock<Mock_capability_handler>> Mock_ptr;

  void SetUp() {
    for (int i = 0; i < NUMBER_OF_HANDLERS; ++i) {
      mock_handlers.push_back(
          std::make_shared<StrictMock<Mock_capability_handler>>());
    }
    std::for_each(std::begin(mock_handlers), std::end(mock_handlers),
                  expect_is_gettable<true>);

    std::vector<Capability_handler_ptr> handlers(mock_handlers.begin(),
                                                 mock_handlers.end());

    std::for_each(mock_handlers.begin(), mock_handlers.end(),
                  default_is_supported<false>);

    sut.reset(new Capabilities_configurator(handlers));
  }

  template <bool Result>
  static void expect_is_supported(Mock_ptr mock) {
    EXPECT_CALL(*mock, is_settable()).WillRepeatedly(Return(Result));
    EXPECT_CALL(*mock, is_gettable()).WillRepeatedly(Return(Result));
    EXPECT_CALL(*mock, is_supported_impl()).WillOnce(Return(Result));
  }

  template <bool Result>
  static void default_is_supported(Mock_ptr mock) {
    EXPECT_CALL(*mock, is_supported_impl()).WillRepeatedly(Return(Result));
  }

  static void expect_get_capability(Mock_ptr mock) {
    EXPECT_CALL(*mock, get_void(_));
  }

  static void expect_commit(Mock_ptr mock) {
    EXPECT_CALL(*mock, commit_void());
  }

  template <bool Result>
  static void expect_is_gettable(Mock_ptr mock) {
    EXPECT_CALL(*mock, is_gettable()).WillRepeatedly(Return(Result));
  }

  void assert_get(std::vector<Mock_ptr> &supported_handlers) {
    std::for_each(supported_handlers.begin(), supported_handlers.end(),
                  expect_is_supported<true>);
    std::for_each(supported_handlers.begin(), supported_handlers.end(),
                  expect_get_name(NAMES));
    std::for_each(supported_handlers.begin(), supported_handlers.end(),
                  expect_get_capability);

    ngs::Memory_instrumented<Capabilities>::Unique_ptr cap(sut->get());

    ASSERT_TRUE(nullptr != cap.get());
    ASSERT_EQ(static_cast<int>(supported_handlers.size()),
              cap->capabilities_size());

    for (std::size_t i = 0; i < supported_handlers.size(); i++) {
      ASSERT_EQ(NAMES[i], cap->capabilities(static_cast<int>(i)).name());
    }
  }

  Capability &add_capability(Capabilities &caps, const std::size_t mock_index) {
    Capability *cap = caps.add_capabilities();

    cap->set_name(NAMES[mock_index]);
    cap->mutable_value()->mutable_scalar()->set_v_signed_int(mock_index);

    return *cap;
  }

  Capability &add_capability_and_expect_it(Capabilities &caps,
                                           const std::size_t mock_index,
                                           const ngs::Error_code &set_result) {
    Capability &cap = add_capability(caps, mock_index);

    EXPECT_CALL(*mock_handlers[mock_index], is_settable())
        .WillRepeatedly(Return(true));
    EXPECT_CALL(*mock_handlers[mock_index], set_impl(Ref(cap.value())))
        .WillRepeatedly(Return(set_result));

    return cap;
  }

  struct expect_get_name {
    template <std::size_t ELEMENTS>
    expect_get_name(const char *(&names)[ELEMENTS])
        : m_names(names), m_elements(ELEMENTS), m_current(0) {}

    void operator()(Mock_ptr mock) {
      EXPECT_CALL(*mock, name()).WillOnce(Return(get_next_name()));
    }

    std::string get_next_name() {
      std::string result = m_names[m_current++];

      if (m_current >= m_elements) m_current = 0;

      return result;
    }

    const char **m_names;
    const std::size_t m_elements;
    std::size_t m_current;
  };

  struct default_get_name : expect_get_name {
    template <std::size_t ELEMENTS>
    default_get_name(const char *(&names)[ELEMENTS]) : expect_get_name(names) {}

    void operator()(Mock_ptr &mock) {
      EXPECT_CALL(*mock, name()).WillRepeatedly(Return(get_next_name()));
    }
  };

  std::vector<Mock_ptr> mock_handlers;

  std::unique_ptr<Capabilities_configurator> sut;
};

TEST_F(CapabilitiesConfiguratorTestSuite, get_doesNothing_whenEmpty) {
  std::vector<Mock_ptr> empty;

  assert_get(empty);
}

TEST_F(CapabilitiesConfiguratorTestSuite, get_returnsAllCapabilities) {
  assert_get(mock_handlers);
}

TEST_F(CapabilitiesConfiguratorTestSuite, get_returnsOnlySupportedCaps) {
  std::vector<Mock_ptr> supported_handlers;

  supported_handlers.push_back(mock_handlers[0]);
  supported_handlers.push_back(mock_handlers[NUMBER_OF_HANDLERS - 1]);

  assert_get(supported_handlers);
}

TEST_F(CapabilitiesConfiguratorTestSuite,
       prepareSet_errorErrorAndCommitDoesNothing_whenOneUnknownCapability) {
  std::unique_ptr<Capabilities> caps(new Capabilities());

  Capability *cap = caps->add_capabilities();

  cap->set_name("UNKNOWN");

  std::for_each(mock_handlers.begin(), mock_handlers.end(),
                default_get_name(NAMES));

  ASSERT_EQ(ER_X_CAPABILITY_NOT_FOUND, sut->prepare_set(*caps).error);

  sut->commit();
}

TEST_F(CapabilitiesConfiguratorTestSuite,
       prepare_set_fail_when_duplicated_capabilities) {
  std::unique_ptr<Capabilities> caps(new Capabilities());
  std::vector<Mock_ptr> supported_handlers;

  std::for_each(mock_handlers.begin(), mock_handlers.end(),
                default_get_name(NAMES));

  add_capability_and_expect_it(*caps, 0, {});
  add_capability_and_expect_it(*caps, 0, {});
  supported_handlers.push_back(mock_handlers[0]);

  ASSERT_EQ(ER_X_DUPLICATED_CAPABILITIES, sut->prepare_set(*caps).error);

  sut->commit();
}

TEST_F(CapabilitiesConfiguratorTestSuite,
       prepareSet_success_whenAllRequestedCapsSucceded) {
  std::unique_ptr<Capabilities> caps(new Capabilities());
  std::vector<Mock_ptr> supported_handlers;

  std::for_each(mock_handlers.begin(), mock_handlers.end(),
                default_get_name(NAMES));

  add_capability_and_expect_it(*caps, 0, {});
  supported_handlers.push_back(mock_handlers[0]);

  add_capability_and_expect_it(*caps, NUMBER_OF_HANDLERS - 1, {});
  supported_handlers.push_back(mock_handlers[NUMBER_OF_HANDLERS - 1]);

  ASSERT_FALSE(sut->prepare_set(*caps));

  std::for_each(supported_handlers.begin(), supported_handlers.end(),
                expect_commit);
  sut->commit();
}

TEST_F(CapabilitiesConfiguratorTestSuite,
       prepareSet_FailsAndCommitDoesNothing_whenAnyCapsFailsLast) {
  std::unique_ptr<Capabilities> caps(new Capabilities());
  std::vector<Mock_ptr> supported_handlers;

  std::for_each(mock_handlers.begin(), mock_handlers.end(),
                default_get_name(NAMES));

  add_capability_and_expect_it(*caps, 0, {});
  supported_handlers.push_back(mock_handlers[0]);

  add_capability_and_expect_it(
      *caps, NUMBER_OF_HANDLERS - 1,
      ngs::Error(ER_X_CAPABILITIES_PREPARE_FAILED, "fail"));
  supported_handlers.push_back(mock_handlers[NUMBER_OF_HANDLERS - 1]);

  ASSERT_EQ(ER_X_CAPABILITIES_PREPARE_FAILED, sut->prepare_set(*caps).error);

  sut->commit();
}

TEST_F(CapabilitiesConfiguratorTestSuite,
       prepareSet_FailsAndCommitDoesNothing_whenAnyCapsFailsFirst) {
  std::unique_ptr<Capabilities> caps(new Capabilities());
  std::vector<Mock_ptr> supported_handlers;

  std::for_each(mock_handlers.begin(), mock_handlers.end(),
                default_get_name(NAMES));

  add_capability_and_expect_it(
      *caps, 0, ngs::Error(ER_X_CAPABILITIES_PREPARE_FAILED, "fail"));
  supported_handlers.push_back(mock_handlers[0]);

  add_capability(*caps, NUMBER_OF_HANDLERS - 1);
  supported_handlers.push_back(mock_handlers[NUMBER_OF_HANDLERS - 1]);

  ASSERT_EQ(ER_X_CAPABILITIES_PREPARE_FAILED, sut->prepare_set(*caps).error);

  sut->commit();
}

}  // namespace test

}  // namespace xpl
