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

#include "hphp/runtime/base/bespoke-layout.h"

#include "hphp/runtime/base/bespoke/layout.h"
#include "hphp/runtime/base/bespoke/logging-array.h"
#include "hphp/runtime/base/bespoke-array.h"

namespace HPHP {

BespokeLayout BespokeLayout::LayoutFromIndex(uint16_t idx) {
  auto const layout = bespoke::layoutForIndex(idx);
  return BespokeLayout{layout};
}

std::string BespokeLayout::describe() const {
  return m_layout->describe();
}

uint16_t BespokeLayout::index() const {
  return m_layout->index();
}

BespokeLayout BespokeLayout::loggingLayout() {
  return BespokeLayout{bespoke::LoggingLayout::layout()};
}

}

