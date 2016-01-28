/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include <folly/Format.h>

#define IMPLEMENT_LOGLEVEL(LEVEL)                       \
  template<typename... Args>                            \
  void Logger::F##LEVEL(Args&&... args) {               \
    if (LogLevel < Log##LEVEL) return;                  \
    LEVEL(folly::sformat(std::forward<Args>(args)...)); \
  }

namespace HPHP {

IMPLEMENT_LOGLEVEL(Error)
IMPLEMENT_LOGLEVEL(Warning)
IMPLEMENT_LOGLEVEL(Info)
IMPLEMENT_LOGLEVEL(Verbose)

}

#undef IMPLEMENT_LOGLEVEL
