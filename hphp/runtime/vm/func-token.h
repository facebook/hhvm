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

#include <atomic>
#include <memory>

namespace HPHP {

struct Func;

struct FuncToken {

  ~FuncToken();

  static std::shared_ptr<FuncToken> get(const Func* func);
  static void setInvalid(const Func* func);

  const Func* getFunc() const {
    if (!isValid()) return nullptr;
    return m_func;
  }

 private:
  FuncToken(const Func* func, bool isValid = true);

  bool isValid() const {
    return m_isValid.load(std::memory_order_acquire);
  }

  const Func* m_func;
  std::atomic<bool> m_isValid;
  bool m_persistent;
};

}
