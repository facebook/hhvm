/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#include "hphp/runtime/base/locale.h"
#include "hphp/runtime/base/type-array.h"
#include "hphp/system/systemlib.h"

namespace HPHP {

struct HSLLocale : SystemLib::ClassLoader<"HH\\Lib\\_Private\\_Locale\\Locale"> {
  struct Ops;

  HSLLocale() = default;
  explicit HSLLocale(std::shared_ptr<Locale> loc);
  ~HSLLocale();

  void sweep();

  std::shared_ptr<Locale> get() const { return m_locale; }
  Ops* ops() const { assertx(m_ops); return m_ops; }
  Array __debugInfo() const;

  static Object newInstance(std::shared_ptr<Locale> loc);
  static HSLLocale* fromObject(const Object& obj);
 private:
  std::shared_ptr<Locale> m_locale;
  Ops* m_ops = nullptr;
};

} // namespace HPHP
