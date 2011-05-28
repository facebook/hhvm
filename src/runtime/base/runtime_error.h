/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#ifndef __HPHP_RUNTIME_ERROR_H__
#define __HPHP_RUNTIME_ERROR_H__

#include <string>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ErrorConstants {
public:
  enum ErrorModes {
    ERROR = 1,
    WARNING = 2,
    PARSE = 4, // not supported
    NOTICE = 8,
    CORE_ERROR = 16, // not supported
    CORE_WARNING = 32, // not supported
    COMPILE_ERROR = 64, // not supported
    COMPILE_WARNING = 128, // not supported
    USER_ERROR = 256,
    USER_WARNING = 512,
    USER_NOTICE = 1024,
    STRICT = 2048,
    RECOVERABLE_ERROR = 4096,
    DEPRECATED = 8192,
    USER_DEPRECATED = 16384,

    /**
     * PHP's fatal errors cannot be fed into error handler. HipHop can. We
     * still need "ERROR" bit, so old PHP error handler can see this error.
     * The extra 24th bit will help people who want to find out if it's
     * a fatal error only HipHop throws or not.
     */
    FATAL_ERROR = ERROR | (1 << 24), // 16777217

    PHP_ALL = ERROR | WARNING | PARSE | NOTICE | CORE_ERROR | CORE_WARNING |
        COMPILE_ERROR | COMPILE_WARNING | USER_ERROR | USER_WARNING |
        USER_NOTICE | RECOVERABLE_ERROR | DEPRECATED | USER_DEPRECATED,

    HPHP_ALL = PHP_ALL | FATAL_ERROR
  };
};

void raise_error(const std::string &msg);
void raise_error(const char *fmt, ...);
void raise_recoverable_error(const std::string &msg);
void raise_recoverable_error(const char *fmt, ...);
void raise_strict_warning(const std::string &msg);
void raise_strict_warning(const char *fmt, ...);
void raise_warning(const std::string &msg);
void raise_warning(const char *fmt, ...);
void raise_notice(const std::string &msg);
void raise_notice(const char *fmt, ...);
void raise_debugging(const std::string &msg);
void raise_debugging(const char *fmt, ...);

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_RUNTIME_ERROR_H__

