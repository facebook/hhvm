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

#ifndef HPHP_RUNTIME_TEST_BESPOKE_LAYOUT_MOCK_H_
#define HPHP_RUNTIME_TEST_BESPOKE_LAYOUT_MOCK_H_

#include "hphp/runtime/base/bespoke/layout.h"
#include "hphp/runtime/vm/jit/irgen.h"

#include <folly/portability/GMock.h>

namespace HPHP{
namespace bespoke {
namespace testing {

struct MockLayout : public Layout {
  MockLayout(const std::string& description): Layout(description) {}

  virtual SSATmp* emitGet(
      IRGS& env, SSATmp* arr, SSATmp* key, Block* taken) const override {
    return nullptr;
  }

  virtual SSATmp* emitElem(
      IRGS& env, SSATmp* lval, SSATmp* key, bool) const override {
    return nullptr;
  }

  virtual SSATmp* emitSet(
      IRGS& env, SSATmp* arr, SSATmp* key, SSATmp* val) const override {
    return nullptr;
  }

  virtual SSATmp* emitAppend(
    IRGS& env, SSATmp* arr, SSATmp* val) const override {
    return nullptr;
  }

  virtual SSATmp* emitIterFirstPos(IRGS& env, SSATmp* arr) const override {
    return nullptr;
  }

  virtual SSATmp* emitIterLastPos(IRGS& env, SSATmp* arr) const override {
    return nullptr;
  }

  virtual SSATmp* emitIterPos(
      IRGS& env, SSATmp* arr, SSATmp* idx) const override {
    return nullptr;
  }

  virtual SSATmp* emitIterAdvancePos(
      IRGS& env, SSATmp* arr, SSATmp* pos) const override {
    return nullptr;
  }

  virtual SSATmp* emitIterElm(
      IRGS& env, SSATmp* arr, SSATmp* pos) const override {
    return nullptr;
  }

  virtual SSATmp* emitIterGetKey(
      IRGS& env, SSATmp* arr, SSATmp* elm) const override {
    return nullptr;
  }

  virtual SSATmp* emitIterGetVal(
      IRGS& env, SSATmp* arr, SSATmp* elm) const override {
    return nullptr;
  }
};

inline Layout* makeDummyLayout(const std::string& name) {
  using ::testing::Mock;

  auto const ret = new MockLayout(name);
  Mock::AllowLeak(ret);
  return ret;
}


} // namespace testing
} // namespace bespoke
} // namespace HPHP

#endif // HPHP_RUNTIME_TEST_BESPOKE_LAYOUT_MOCK_H_
