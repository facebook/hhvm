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

#pragma once

#include "hphp/util/assertions.h"

namespace HPHP {

struct StringData;

struct LazyClassData {
  LazyClassData() = default;

  static LazyClassData create(const StringData* name);

  const StringData* name() const {
    assertx(className);
    return className;
  }
private:
  explicit LazyClassData(const StringData* name);
  const StringData* className;
};

const StringData* lazyClassToStringHelper(const LazyClassData& lclass,
                                          const char* source);
}
