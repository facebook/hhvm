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
#pragma once

#include "hphp/util/optional.h"

namespace HPHP { namespace jit {

struct InterpOneData;
struct NormalizedInstruction;
struct Type;

namespace irgen {

struct IRGS;

//////////////////////////////////////////////////////////////////////

void interpOne(IRGS&);
void interpOne(IRGS&, int popped);
void interpOne(IRGS&, Type t, int popped);
void interpOne(IRGS&, Optional<Type>, int popped, int pushed,
               InterpOneData&);

//////////////////////////////////////////////////////////////////////

}}}
