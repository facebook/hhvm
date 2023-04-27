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

#include "hphp/runtime/base/locale.h"
#include "hphp/util/rds-local.h"
#include "hphp/util/thread-local.h"

#include <locale.h>
#include <langinfo.h>

namespace HPHP {

struct ThreadSafeLocaleHandler {
public:
  ThreadSafeLocaleHandler();
  ~ThreadSafeLocaleHandler();
  void reset();
  const char* actuallySetLocale(int category, const char* locale);
  struct lconv* localeconv();

  static std::shared_ptr<Locale> getRequestLocale();
  static void setRequestLocale(std::shared_ptr<Locale>);

private:
  void generate_LC_ALL_String();

  std::string m_lc_all;
  std::shared_ptr<Locale> m_locale;
};

extern RDS_LOCAL(ThreadSafeLocaleHandler, g_thread_safe_locale_handler);

}
