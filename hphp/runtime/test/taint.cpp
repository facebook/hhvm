// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.
/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifdef HHVM_TAINT

#include <gtest/gtest.h>

#include "hphp/runtime/vm/taint.h"

#include "hphp/runtime/base/attr.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/act-rec.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/func-emitter.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/vm-regs.h"
#include "hphp/util/trace.h"



namespace HPHP {
namespace taint {

struct TaintTest : public ::testing::Test {
  TaintTest() {
    SHA1 sha1, bcSha1;
    m_unitEmitter = std::make_unique<UnitEmitter>(
      sha1,
      bcSha1,
      m_funcTable,
      /* useGlobalIDs */ false);

    auto name = StringData::Make("func_emitter");
    m_funcEmitter = std::make_unique<FuncEmitter>(*m_unitEmitter, 0, 0, name);
    // Prime source location to avoid SEGFAULT.
    m_funcEmitter->recordSourceLocation(Location::Range(0, 0, 0, 1), 1);
  }

 protected:
  void SetUp() override {
    Configuration::get()->sources = {"__source"};
    Configuration::get()->sinks = {"__sink"};
    State::get()->reset();
  }

  Func* createFunc(const std::string& name) {
    auto func_name = StringData::MakeStatic(name);
    m_funcEmitter->name = func_name;
    m_funcEmitter->attrs = Attr::AttrNone;
    return m_funcEmitter->create(m_unit);
  }

  Native::FuncTable m_funcTable;
  std::unique_ptr<UnitEmitter> m_unitEmitter;
  std::unique_ptr<FuncEmitter> m_funcEmitter;
  Unit m_unit;
};

TEST_F(TaintTest, TestRetCSource) {
  ActRec act_rec;
  vmfp() = &act_rec;

  auto func = createFunc("__source");
  vmfp()->setFunc(func);

  State::get()->stack.push(kNoSource);

  PC pc = 0;
  iopRetC(pc);

  EXPECT_EQ(State::get()->stack.top(), kTestSource);
}

TEST_F(TaintTest, TestRetCNoSource) {
  ActRec act_rec;
  vmfp() = &act_rec;

  auto func = createFunc("__unrelated");
  vmfp()->setFunc(func);

  State::get()->stack.push(kNoSource);

  PC pc = 0;
  iopRetC(pc);

  EXPECT_EQ(State::get()->stack.top(), kNoSource);
}

TEST_F(TaintTest, TestFCallFuncDIssue) {
  ActRec act_rec;
  vmfp() = &act_rec;

  auto func = createFunc("__sink");
  vmfp()->setFunc(func);

  State::get()->stack.push(kTestSource);
  EXPECT_TRUE(State::get()->issues.empty());

  PC pc = 0;
  FCallArgs fca(
      FCallArgsBase::Flags::None,
      /* numArgs */ 0,
      /* numRets */ 0,
      /* inoutArgs */ nullptr,
      /* readonlyArgs */ nullptr,
      /* offset */ 0,
      /* context */ nullptr);
  iopFCallFuncD(
       /* retToJit */ false,
       pc,
       /* origpc */ pc,
       fca,
       /* id */ 0);

  EXPECT_EQ(State::get()->issues.size(), 1);
  EXPECT_EQ(State::get()->issues[0], (Issue{kTestSource, "__sink"}));
}

TEST_F(TaintTest, TestFCallFuncDNoIssue) {
  ActRec act_rec;
  vmfp() = &act_rec;

  auto func = createFunc("__sink");
  vmfp()->setFunc(func);

  State::get()->stack.push(kNoSource);
  EXPECT_TRUE(State::get()->issues.empty());

  PC pc = 0;
  FCallArgs fca(
      FCallArgsBase::Flags::None,
      /* numArgs */ 0,
      /* numRets */ 0,
      /* inoutArgs */ nullptr,
      /* readonlyArgs */ nullptr,
      /* offset */ 0,
      /* context */ nullptr);
  iopFCallFuncD(
       /* retToJit */ false,
       pc,
       /* origpc */ pc,
       fca,
       /* id */ 0);

  EXPECT_TRUE(State::get()->issues.empty());
}

} // namespace taint
} // namespace HPHP

#endif
