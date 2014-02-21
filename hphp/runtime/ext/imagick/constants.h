/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_EXT_IMAGICK_CONSTANTS_H_
#define incl_HPHP_EXT_IMAGICK_CONSTANTS_H_

#include "hphp/runtime/base/base-includes.h"

namespace HPHP {

enum IMAGCIK_COLOR {
  IMAGICK_COLOR_BLACK = 0,
  IMAGICK_COLOR_BLUE,
  IMAGICK_COLOR_CYAN,
  IMAGICK_COLOR_GREEN,
  IMAGICK_COLOR_RED,
  IMAGICK_COLOR_YELLOW,
  IMAGICK_COLOR_MAGENTA,
  IMAGICK_COLOR_OPACITY,
  IMAGICK_COLOR_ALPHA,
  IMAGICK_COLOR_FUZZ
};

extern const StaticString
  s_r,
  s_g,
  s_b,
  s_a,
  s_hue,
  s_saturation,
  s_luminosity,
  s_Imagick,
  s_ImagickDraw,
  s_ImagickPixel,
  s_ImagickPixelIterator;

template<typename T>
ALWAYS_INLINE
bool registerImagickConstants(const StaticString& name, T value) {
  return Native::registerClassConstant<KindOfInt64>(s_Imagick.get(),
                                                    name.get(),
                                                    (int64_t)value);
}

void loadImagickConstants();

//////////////////////////////////////////////////////////////////////////////
} // namespace HPHP

#endif // incl_HPHP_EXT_IMAGICK_CONSTANTS_H_
