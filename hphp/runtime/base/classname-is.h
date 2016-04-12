/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_CLASSNAME_IS_H
#define incl_HPHP_CLASSNAME_IS_H

namespace HPHP {

/**
 * InstantStatic allows defining a static in-class variable that is
 * initialized during program startup, without actually needing to define
 * it anywhere. When defining the static, just specify its type (T), the
 * type that T's constructor will receive (TInit), and the name of the
 * function that will be called for construction (init). One copy of
 * static data is generated per T/init.
 */
template <class T, class TInit, TInit init()>
struct InstantStatic {
  static T value;
};

template <class T, class TInit, TInit init()>
T InstantStatic<T, TInit, init>::value(init());

#define CLASSNAME_IS(str)                                               \
  static const char *GetClassName() { return str; }                     \
  static const StaticString& classnameof() {                            \
    return InstantStatic<const StaticString, const char*, GetClassName> \
      ::value;                                                          \
  }
}

#endif
