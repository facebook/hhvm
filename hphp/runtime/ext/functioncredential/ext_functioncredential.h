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

#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/func.h"

namespace HPHP {

struct FunctionCredential {
  static ObjectData* newInstance(const Func* func);
  static const FunctionCredential* fromObject(const ObjectData* obj);

  static Class* classof();

  const Func* func() const {
    assertx(func_);
    return func_;
  }

private:
  const Func* func_;
};

}

