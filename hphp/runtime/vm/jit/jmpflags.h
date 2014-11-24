/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_JMPFLAGS_H_
#define incl_HPHP_JMPFLAGS_H_

namespace HPHP { namespace jit {

struct NormalizedInstruction;

//////////////////////////////////////////////////////////////////////

enum JmpFlags {
  JmpFlagNone        = 0,
  JmpFlagEndsRegion  = 1,
  JmpFlagNextIsMerge = 2,
};

inline JmpFlags operator|(JmpFlags f1, JmpFlags f2) {
  return static_cast<JmpFlags>(
    static_cast<unsigned>(f1) | static_cast<unsigned>(f2));
}

inline JmpFlags operator&(JmpFlags f1, JmpFlags f2) {
  return static_cast<JmpFlags>(
    static_cast<unsigned>(f1) & static_cast<unsigned>(f2));
}

JmpFlags instrJmpFlags(const NormalizedInstruction& ni);

//////////////////////////////////////////////////////////////////////

}}

#endif
