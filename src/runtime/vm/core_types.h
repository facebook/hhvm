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

#ifndef incl_VM_CORE_TYPES_H_
#define incl_VM_CORE_TYPES_H_

#include <runtime/base/complex_types.h>

namespace HPHP {
namespace VM {
///////////////////////////////////////////////////////////////////////////////

typedef TypedValue Cell;
typedef TypedValue Var;
typedef TypedValue Home;

typedef uint8_t Opcode;
typedef int Id;
typedef int Offset; // used for both absolute offsets and relative offsets
typedef int Line;

// Special types that are not relevant to the runtime as a whole.
static const DataType KindOfInvalid = DataType(-1);
static const DataType KindOfHome    = DataType(-2);
static const DataType KindOfClass   = DataType(-3);

// The order for public/protected/private matters in numerous places.
enum Attr {
  AttrNone      = 0,        // class  property  method //
  AttrReference = (1 << 0), //                    X    //
  AttrPublic    = (1 << 1), //           X        X    //
  AttrProtected = (1 << 2), //           X        X    //
  AttrPrivate   = (1 << 3), //           X        X    //
  AttrStatic    = (1 << 4), //           X        X    //
  AttrAbstract  = (1 << 5), //   X                X    //
  AttrFinal     = (1 << 6), //   X                X    //
  AttrInterface = (1 << 7), //   X                     //
  AttrTrait     = (1 << 8)  //   X                     //
};

} }

#endif
