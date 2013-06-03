/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_UTIL_TEXT_ART_H_
#define incl_HPHP_UTIL_TEXT_ART_H_

#include "hphp/util/base.h"

namespace HPHP { namespace Util { namespace TextArt {
///////////////////////////////////////////////////////////////////////////////

#define BOX_H  get_box_drawing_char(LightHorizontal)
#define BOX_V  get_box_drawing_char(LightVertical)
#define BOX_UL get_box_drawing_char(LightDownAndRight)
#define BOX_UR get_box_drawing_char(LightDownAndLeft)
#define BOX_VL get_box_drawing_char(LightVerticalAndRight)
#define BOX_VR get_box_drawing_char(LightVerticalAndLeft)
#define BOX_LL get_box_drawing_char(LightUpAndRight)
#define BOX_LR get_box_drawing_char(LightUpAndLeft)

enum BoxDrawing {
  LightDownAndRight,
  LightHorizontal,
  LightDownAndLeft,
  LightVerticalAndRight,
  LightVertical,
  LightVerticalAndLeft,
  LightUpAndRight,
  LightUpAndLeft,
};

const char *get_box_drawing_char(BoxDrawing name);

extern bool s_use_utf8;

///////////////////////////////////////////////////////////////////////////////
}}}

#endif // incl_HPHP_UTIL_TEXT_ART_H_
