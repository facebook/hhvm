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
#ifndef incl_HPHP_LOCALE_PORTABILITY_H_
#define incl_HPHP_LOCALE_PORTABILITY_H_

#include <locale.h>

#ifdef _MSC_VER
typedef _locale_t locale_t;

#define LC_ALL_MASK       LC_ALL
#define LC_COLLATE_MASK   LC_COLLATE
#define LC_CTYPE_MASK     LC_CTYPE
#define LC_MONETARY_MASK  LC_MONETARY
#define LC_NUMERIC_MASK   LC_NUMERIC
#define LC_TIME_MASK      LC_TIME

inline locale_t _current_locale() { return _get_current_locale(); }
#else
#include <langinfo.h>
#include <xlocale.h>
#endif

#endif
