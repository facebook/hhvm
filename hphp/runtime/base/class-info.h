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

#ifndef incl_HPHP_CLASS_INFO_H_
#define incl_HPHP_CLASS_INFO_H_

#include <utility>
#include <vector>

#include "hphp/runtime/base/type-array.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/type-variant.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class ClassInfoHook;

/**
 * Though called "ClassInfo", we consider global scope as a virtual "class".
 * Therefore, this is the place we store meta information of both global
 * functions and class methods and properties.
 */
struct ClassInfo {
  enum Attribute {                      //  class   prop   func  method param
    ParamCoerceModeNull    = (1 <<  0), //                  x      x
    IsRedeclared           = (1 <<  1), //    x             x
    IsVolatile             = (1 <<  2), //    x             x

    IsInterface            = (1 <<  3), //    x
    IsClosure              = (1 <<  3), //                  x      x
    IsAbstract             = (1 <<  4), //    x                    x
    IsFinal                = (1 <<  5), //    x                    x

    IsPublic               = (1 <<  6), //           x             x
    IsProtected            = (1 <<  7), //           x             x
    IsPrivate              = (1 <<  8), //           x             x
    IsStatic               = (1 <<  9), //           x             x
    IsCppAbstract          = (1 << 10), //    x
    HasCall                = IsPublic,  //    x
    AllowOverride          = IsPrivate, //                  x
    IsReference            = (1 << 11), //                  x      x     x
    // Unused              = (1 << 12),

    // need a non-zero number for const char * maps
    IsNothing              = (1 << 13),

    // Unused              = (1 << 14),

    IsCppSerializable      = (1 << 15), //    x
    HipHopSpecific         = (1 << 16), //    x             x

    VariableArguments      = (1 << 17), //                  x      x
    RefVariableArguments   = (1 << 18), //                  x      x

    FunctionIsFoldable     = (1 << 20), //                  x
    NoEffect               = (1 << 21), //                  x
    NoInjection            = (1 << 22), //                  x      x
    HasOptFunction         = (1 << 23), //                  x
    AllowIntercept         = (1 << 24), //                  x      x
    NoProfile              = (1 << 25), //                  x      x
    // Unused                (1 << 26),

    NoDefaultSweep         = (1 << 27), //    x
    IsSystem               = (1 << 28), //    x             x

    IsTrait                = (1 << 29), //    x
    ParamCoerceModeFalse   = (1 << 30), //                  x      x
    NoFCallBuiltin         = (1u << 31),//                  x      x
  };

public:
  /**
   * Return a list of declared classes.
   */
  static Array GetClasses() { return GetClassLike(IsInterface|IsTrait, 0); }

  /**
   * Return a list of declared interfaces.
   */
  static Array GetInterfaces() { return GetClassLike(IsInterface, IsInterface); }

  /**
   * Return a list of declared traits.
   */
  static Array GetTraits() { return GetClassLike(IsTrait, IsTrait); }

private:
  static Array GetClassLike(unsigned mask, unsigned value);
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_CLASS_INFO_H_
