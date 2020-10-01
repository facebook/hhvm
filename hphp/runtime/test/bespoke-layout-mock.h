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

inline Layout* makeDummyLayout(const std::string& name) {
  using ::testing::Mock;

  auto const ret = new Layout(name);
  Mock::AllowLeak(ret);
  return ret;
}


} // namespace testing
} // namespace bespoke
} // namespace HPHP

#endif // HPHP_RUNTIME_TEST_BESPOKE_LAYOUT_MOCK_H_
