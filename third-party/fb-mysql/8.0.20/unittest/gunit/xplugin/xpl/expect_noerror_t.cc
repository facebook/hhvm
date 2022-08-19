/* Copyright (c) 2015, 2018, Oracle and/or its affiliates. All rights reserved.

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

#include <gtest/gtest.h>
#include <sys/types.h>

#include "plugin/x/ngs/include/ngs/error_code.h"
#include "plugin/x/ngs/include/ngs/protocol/protocol_protobuf.h"
#include "plugin/x/src/expect/expect.h"
#include "plugin/x/src/expect/expect_stack.h"
#include "plugin/x/src/xpl_error.h"

static const int EXPECT_NO_ERROR = 1;

namespace xpl {
namespace test {
static ngs::Error_code success;

static ngs::Error_code simulate_instruction(Expectation_stack &xs, int8_t mid,
                                            ngs::Error_code fail) {
  ngs::Error_code err = xs.pre_client_stmt(mid);
  if (!err) {
    // execution would come here
    err = fail;
    xs.post_client_stmt(mid, fail);
  }
  return err;
}

static ngs::Error_code simulate_close(Expectation_stack &xs) {
  ngs::Error_code err =
      xs.pre_client_stmt(Mysqlx::ClientMessages::EXPECT_CLOSE);
  if (!err) {
    err = xs.close();
    xs.post_client_stmt(Mysqlx::ClientMessages::EXPECT_CLOSE, err);
  }
  return err;
}

static ngs::Error_code simulate_open(Expectation_stack &xs,
                                     const Mysqlx::Expect::Open &open) {
  ngs::Error_code err = xs.pre_client_stmt(Mysqlx::ClientMessages::EXPECT_OPEN);
  if (!err) {
    err = xs.open(open);
    xs.post_client_stmt(Mysqlx::ClientMessages::EXPECT_OPEN, err);
  }
  return err;
}

static Mysqlx::Expect::Open Inherit() {
  Mysqlx::Expect::Open open;
  open.set_op(Mysqlx::Expect::Open::EXPECT_CTX_COPY_PREV);
  return open;
}

static Mysqlx::Expect::Open Noerror() {
  Mysqlx::Expect::Open open;

  open.set_op(Mysqlx::Expect::Open::EXPECT_CTX_EMPTY);
  Mysqlx::Expect::Open::Condition *cond = open.mutable_cond()->Add();
  cond->set_condition_key(EXPECT_NO_ERROR);
  cond->set_op(Mysqlx::Expect::Open::Condition::EXPECT_OP_SET);
  return open;
}

static Mysqlx::Expect::Open Plain() {
  Mysqlx::Expect::Open open;

  open.set_op(Mysqlx::Expect::Open::EXPECT_CTX_EMPTY);
  return open;
}

static Mysqlx::Expect::Open Inherit_and_clear_noerror() {
  Mysqlx::Expect::Open open = Inherit();

  Mysqlx::Expect::Open::Condition *cond = open.mutable_cond()->Add();
  cond->set_condition_key(EXPECT_NO_ERROR);
  cond->set_op(Mysqlx::Expect::Open::Condition::EXPECT_OP_UNSET);

  return open;
}

static Mysqlx::Expect::Open Inherit_and_add_noerror() {
  Mysqlx::Expect::Open open = Inherit();

  Mysqlx::Expect::Open::Condition *cond = open.mutable_cond()->Add();
  cond->set_condition_key(EXPECT_NO_ERROR);
  cond->set_op(Mysqlx::Expect::Open::Condition::EXPECT_OP_SET);

  return open;
}

#define EXPECT_ok_cmd() \
  ASSERT_EQ(success, simulate_instruction(xs, 1, ngs::Error_code()))
#define EXPECT_error_cmd()                     \
  ASSERT_EQ(ngs::Error_code(1234, "whatever"), \
            simulate_instruction(xs, 2, ngs::Error_code(1234, "whatever")))
#define EXPECT_fail(error_code, exp)                                 \
  ASSERT_EQ(ngs::Error_code(error_code, "Expectation failed: " exp), \
            simulate_instruction(xs, 3, ngs::Error_code()))

#define EXPECT_open_ok(msg) ASSERT_EQ(success, simulate_open(xs, msg))
#define EXPECT_open_fail(msg, error_code, exp)                       \
  ASSERT_EQ(ngs::Error_code(error_code, "Expectation failed: " exp), \
            simulate_open(xs, msg))

#define EXPECT_close_ok() ASSERT_EQ(success, simulate_close(xs))
#define EXPECT_close_fail(error_code, exp)                           \
  ASSERT_EQ(ngs::Error_code(error_code, "Expectation failed: " exp), \
            simulate_close(xs))

TEST(expect, plain) {
  Expectation_stack xs;

  // open expect block
  EXPECT_open_ok(Plain());

  // ok command 1
  EXPECT_ok_cmd();
  EXPECT_ok_cmd();
  // error command 2
  EXPECT_error_cmd();

  // subsequent cmds succeed normally
  EXPECT_ok_cmd();
  EXPECT_error_cmd();

  // now close the block
  EXPECT_close_ok();

  EXPECT_ok_cmd();

  // close too much should fail
  EXPECT_EQ(
      ngs::Error_code(ER_X_EXPECT_NOT_OPEN, "Expect block currently not open"),
      xs.close());
}

TEST(expect, noerror) {
  Expectation_stack xs;

  // open expect block
  EXPECT_open_ok(Noerror());

  // ok command 1
  EXPECT_ok_cmd();
  // error command 2
  EXPECT_error_cmd();
  // now everything fails
  EXPECT_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
  EXPECT_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");

  // now close the block
  EXPECT_close_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");

  // now commands should succeed again
  EXPECT_ok_cmd();
}

TEST(expect, noerror_in_noerror) {
  Expectation_stack xs;

  // fail in the inner block
  EXPECT_open_ok(Noerror());
  EXPECT_ok_cmd();
  {
    EXPECT_open_ok(Noerror());
    EXPECT_ok_cmd();
    EXPECT_error_cmd();
    EXPECT_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
    EXPECT_close_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
  }
  EXPECT_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
  EXPECT_close_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");

  EXPECT_ok_cmd();
  EXPECT_EQ(
      ngs::Error_code(ER_X_EXPECT_NOT_OPEN, "Expect block currently not open"),
      xs.close());

  // fail in the outer block
  EXPECT_open_ok(Noerror());
  EXPECT_ok_cmd();
  EXPECT_error_cmd();
  {
    EXPECT_open_fail(Noerror(), ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
    EXPECT_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
    EXPECT_close_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
  }
  EXPECT_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
  EXPECT_close_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");

  EXPECT_ok_cmd();
  EXPECT_EQ(
      ngs::Error_code(ER_X_EXPECT_NOT_OPEN, "Expect block currently not open"),
      xs.close());

  // fail in inner block again
  EXPECT_open_ok(Noerror());
  EXPECT_ok_cmd();
  {
    EXPECT_open_ok(Inherit());
    EXPECT_ok_cmd();
    EXPECT_error_cmd();
    EXPECT_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
    EXPECT_close_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
  }
  EXPECT_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
  EXPECT_close_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");

  EXPECT_ok_cmd();
}

TEST(expect, plain_in_noerror) {
  Expectation_stack xs;

  // fail in the inner block
  EXPECT_open_ok(Noerror());
  EXPECT_ok_cmd();
  {
    EXPECT_open_ok(Plain());
    EXPECT_ok_cmd();
    EXPECT_error_cmd();
    EXPECT_ok_cmd();
    EXPECT_close_ok();
  }
  EXPECT_close_ok();

  EXPECT_ok_cmd();
  EXPECT_EQ(
      ngs::Error_code(ER_X_EXPECT_NOT_OPEN, "Expect block currently not open"),
      xs.close());

  // fail in the outer block
  EXPECT_open_ok(Noerror());
  EXPECT_ok_cmd();
  EXPECT_error_cmd();
  {
    EXPECT_open_fail(Plain(), ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
    EXPECT_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
    EXPECT_close_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
  }
  EXPECT_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
  EXPECT_close_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");

  EXPECT_ok_cmd();
  EXPECT_EQ(
      ngs::Error_code(ER_X_EXPECT_NOT_OPEN, "Expect block currently not open"),
      xs.close());

  // fail in inner block again
  EXPECT_open_ok(Noerror());
  EXPECT_ok_cmd();
  {
    EXPECT_open_ok(Inherit());
    EXPECT_ok_cmd();
    EXPECT_error_cmd();
    EXPECT_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
    EXPECT_close_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
  }
  EXPECT_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
  EXPECT_close_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");

  EXPECT_open_ok(Noerror());
  EXPECT_ok_cmd();
  {
    EXPECT_open_ok(Inherit_and_add_noerror());
    EXPECT_ok_cmd();
    EXPECT_error_cmd();
    EXPECT_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
    EXPECT_close_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
  }
  EXPECT_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
  EXPECT_close_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");

  EXPECT_open_ok(Noerror());
  EXPECT_ok_cmd();
  {
    EXPECT_open_ok(Inherit_and_clear_noerror());
    EXPECT_ok_cmd();
    EXPECT_error_cmd();
    EXPECT_ok_cmd();
    EXPECT_close_ok();
  }
  EXPECT_close_ok();

  EXPECT_ok_cmd();
}

TEST(expect, noerror_in_plain) {
  Expectation_stack xs;

  // fail in the inner block
  EXPECT_open_ok(Plain());
  EXPECT_ok_cmd();
  {
    EXPECT_open_ok(Noerror());
    EXPECT_ok_cmd();
    EXPECT_error_cmd();
    EXPECT_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
    EXPECT_close_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
  }
  EXPECT_ok_cmd();
  EXPECT_close_ok();

  EXPECT_ok_cmd();
  EXPECT_EQ(
      ngs::Error_code(ER_X_EXPECT_NOT_OPEN, "Expect block currently not open"),
      xs.close());

  // fail in the outer block
  EXPECT_open_ok(Plain());
  EXPECT_ok_cmd();
  EXPECT_error_cmd();
  {
    EXPECT_open_ok(Noerror());
    EXPECT_ok_cmd();
    EXPECT_close_ok();
  }
  EXPECT_ok_cmd();
  EXPECT_close_ok();

  EXPECT_ok_cmd();
  EXPECT_EQ(
      ngs::Error_code(ER_X_EXPECT_NOT_OPEN, "Expect block currently not open"),
      xs.close());

  // fail in inner block again
  EXPECT_open_ok(Plain());
  EXPECT_ok_cmd();
  {
    EXPECT_open_ok(Inherit());
    EXPECT_ok_cmd();
    EXPECT_error_cmd();
    EXPECT_ok_cmd();
    EXPECT_close_ok();
  }
  EXPECT_close_ok();

  EXPECT_open_ok(Plain());
  EXPECT_ok_cmd();
  {
    EXPECT_open_ok(Inherit_and_add_noerror());
    EXPECT_ok_cmd();
    EXPECT_error_cmd();
    EXPECT_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
    EXPECT_close_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
  }
  EXPECT_ok_cmd();
  EXPECT_close_ok();

  EXPECT_ok_cmd();
}

TEST(expect, nested_inheriting) {
  Expectation_stack xs;
  EXPECT_open_ok(Noerror());
  EXPECT_ok_cmd();
  {
    EXPECT_open_ok(Inherit_and_add_noerror());
    EXPECT_ok_cmd();
    EXPECT_error_cmd();
    EXPECT_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
    EXPECT_close_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
  }
  EXPECT_close_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");

  EXPECT_open_ok(Noerror());
  EXPECT_ok_cmd();
  {
    EXPECT_open_ok(Inherit_and_clear_noerror());
    EXPECT_ok_cmd();
    EXPECT_error_cmd();
    EXPECT_ok_cmd();
    EXPECT_close_ok();
  }
  EXPECT_close_ok();

  EXPECT_open_ok(Plain());
  EXPECT_ok_cmd();
  {
    EXPECT_open_ok(Inherit_and_add_noerror());
    EXPECT_ok_cmd();
    EXPECT_error_cmd();
    EXPECT_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
    EXPECT_close_fail(ER_X_EXPECT_NO_ERROR_FAILED, "no_error");
  }
  EXPECT_close_ok();

  EXPECT_open_ok(Plain());
  EXPECT_ok_cmd();
  {
    EXPECT_open_ok(Inherit_and_clear_noerror());
    EXPECT_ok_cmd();
    EXPECT_error_cmd();
    EXPECT_ok_cmd();
    EXPECT_close_ok();
  }
  EXPECT_close_ok();
}

TEST(expect, invalid) {
  {
    Expectation exp;
    EXPECT_EQ(
        ngs::Error_code(ER_X_EXPECT_BAD_CONDITION, "Unknown condition key"),
        exp.set(1234, "1"));
  }

  {
    Expectation exp;
    EXPECT_EQ(ngs::Error_code(), exp.set(1, ""));
    EXPECT_EQ(true, exp.fail_on_error());

    EXPECT_EQ(ngs::Error_code(), exp.set(1, "1"));
    EXPECT_EQ(true, exp.fail_on_error());

    EXPECT_EQ(ngs::Error_code(), exp.set(1, "0"));
    EXPECT_FALSE(exp.fail_on_error());

    EXPECT_EQ(ngs::Error_code(ER_X_EXPECT_BAD_CONDITION_VALUE,
                              "Invalid value 'bla' for expectation no_error"),
              exp.set(1, "bla"));
    EXPECT_FALSE(exp.fail_on_error());
  }
}

class Expect_surprise : public Expect_condition {
 public:
  Expect_surprise(const uint32_t k, const std::string &v)
      : Expect_condition(k, ""), m_surprise_value(v) {}

  Expect_condition_ptr clone() override {
    return Expect_condition_ptr{new Expect_surprise(key(), m_surprise_value)};
  }

  ngs::Error_code check_if_error() override {
    const std::string valid_value = "true";

    if (m_surprise_value == valid_value) return {};

    return ngs::Error_code(ER_X_EXPECT_NO_ERROR_FAILED, "");
  }

  void set(const std::string &flag) { m_surprise_value = flag; }

  const std::string &value() override { return m_surprise_value; }

 private:
  std::string m_surprise_value;
};

TEST(expect, condition) {
  Expectation expect;

  Expect_surprise *surp = new Expect_surprise(1234, "false");
  ASSERT_FALSE(expect.check_conditions());
  expect.add_condition(std::unique_ptr<Expect_condition>{surp});
  ASSERT_TRUE(expect.check_conditions());
  surp->set("true");
  ASSERT_FALSE(expect.check_conditions());
  surp->set("false");
  {
    Expectation copy(expect);

    ASSERT_TRUE(expect.check_conditions());
    ASSERT_TRUE(copy.check_conditions());
    expect.unset(1234, "");
    ASSERT_FALSE(expect.check_conditions());
    ASSERT_TRUE(copy.check_conditions());
  }
}

TEST(expect, condition_unset) {
  const uint32_t expect_key = 1234u;
  Expectation expect;

  Expect_surprise *surp[4];

  ASSERT_FALSE(expect.check_conditions());

  for (int i = 0; i < 4; ++i) {
    surp[i] = new Expect_surprise(expect_key, "true");

    expect.add_condition(std::unique_ptr<Expect_condition>{surp[i]});
  }

  ASSERT_FALSE(expect.check_conditions());

  surp[0]->set("false");
  ASSERT_TRUE(expect.check_conditions());

  const auto cond_value = surp[0]->value();
  expect.unset(surp[0]->key(), cond_value);
  ASSERT_FALSE(expect.check_conditions());
  surp[1]->set("false");
  ASSERT_TRUE(expect.check_conditions());

  expect.unset(expect_key, "");
  ASSERT_FALSE(expect.check_conditions());
}

}  // namespace test
}  // namespace xpl
