/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_SETLOCALE_H_
#define incl_HPHP_SETLOCALE_H_

#include <vector>
#include <string>
#include "hphp/util/locale-portability.h"
#include "hphp/util/thread-local.h"

namespace HPHP {

class ThreadSafeLocaleHandler {
private:
  typedef struct {
    int category;
    int category_mask;
    std::string category_str;
    std::string locale_str;
  } CategoryAndLocaleMap;

public:
  ThreadSafeLocaleHandler();
  ~ThreadSafeLocaleHandler();
  void reset();
  const char* actuallySetLocale(int category, const char* locale);
  struct lconv* localeconv();

private:
  void generate_LC_ALL_String();

  std::vector<CategoryAndLocaleMap> m_category_locale_map;
  locale_t m_locale;
};

extern DECLARE_THREAD_LOCAL(ThreadSafeLocaleHandler,
                            g_thread_safe_locale_handler);

}

#endif // incl_HPHP_SETLOCALE_H_
