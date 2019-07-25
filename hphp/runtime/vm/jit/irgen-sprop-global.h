/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#ifndef incl_HPHP_JIT_IRGEN_SPROP_GLOBAL_H_
#define incl_HPHP_JIT_IRGEN_SPROP_GLOBAL_H_

#include "hphp/runtime/vm/jit/types.h"

namespace HPHP {

struct Class;
struct StringData;
struct TypeConstraint;

namespace jit {

struct SSATmp;
struct Block;

namespace irgen {

struct IRGS;

//////////////////////////////////////////////////////////////////////

struct ClsPropLookup {
  SSATmp* propPtr;
  const TypeConstraint* tc;
  Slot slot;
};

ClsPropLookup ldClsPropAddrKnown(IRGS&, const Class*, const StringData*, bool);
ClsPropLookup ldClsPropAddr(IRGS&, SSATmp*, SSATmp*, bool, bool, bool);

//////////////////////////////////////////////////////////////////////

}}}


#endif
