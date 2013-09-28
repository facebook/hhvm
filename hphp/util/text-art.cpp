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

#include "hphp/util/text-art.h"

namespace HPHP { namespace Util { namespace TextArt {
///////////////////////////////////////////////////////////////////////////////

bool s_use_utf8 = true;

const char *get_box_drawing_char(BoxDrawing name) {
  if (s_use_utf8) {
    switch (name) {
      case LightDownAndRight:     return "\xe2\x94\x8c";
      case LightHorizontal:       return "\xe2\x94\x80";
      case LightDownAndLeft:      return "\xe2\x94\x90";
      case LightVerticalAndRight: return "\xe2\x94\x9c";
      case LightVertical:         return "\xe2\x94\x82";
      case LightVerticalAndLeft:  return "\xe2\x94\xa4";
      case LightUpAndRight:       return "\xe2\x94\x94";
      case LightUpAndLeft:        return "\xe2\x94\x98";
    }
  } else {
    switch (name) {
      case LightDownAndRight:     return "+";
      case LightHorizontal:       return "-";
      case LightDownAndLeft:      return "+";
      case LightVerticalAndRight: return "+";
      case LightVertical:         return "|";
      case LightVerticalAndLeft:  return "+";
      case LightUpAndRight:       return "+";
      case LightUpAndLeft:        return "+";
    }
  }
  assert(false);
  return "";
}

///////////////////////////////////////////////////////////////////////////////
}}}
