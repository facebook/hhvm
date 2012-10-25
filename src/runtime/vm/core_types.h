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

/*
 * These may be used to provide a little more self-documentation about
 * whether execution stack typed values are assumed to be cells or
 * vars.  (See bytecode.specification for details.)
 */
typedef TypedValue Cell;
typedef TypedValue Var;

/*
 * Non-enumerated version of type for referring to opcodes or the
 * bytecode stream.  (Use the enum Op in hhbc.h for an enumerated
 * version.)
 */
typedef uint8_t Opcode;

/*
 * Id type for various components of a unit that have to have unique
 * identifiers.  For example, function ids, class ids, string literal
 * ids.
 */
typedef int Id;
const Id kInvalidId = Id(-1);

// Bytecode offsets.  Used for both absolute offsets and relative
// offsets.
typedef int32_t Offset;

/*
 * Various fields in the VM's runtime have indexes that are addressed
 * using this "slot" type.  For example: methods, properties, class
 * constants.
 *
 * No slot value greater than or equal to kInvalidSlot will actually
 * be used for one of these.
 */
typedef uint32_t Slot;
const Slot kInvalidSlot = Slot(-1);

// Special types that are not relevant to the runtime as a whole.
// The order for public/protected/private matters in numerous places.
//
// Attr unions are directly stored as integers in .hhbc repositories, so
// incompatible changes here require a schema version bump.
//
// AttrTrait on a method means that the method is NOT a constructor,
// even though it may look like one
//
// AttrNoOverride (WholeProgram only) on a class means its not extended
// and on a method means that no extending class defines the method.
//
// AttrVariadicByRef indicates a function is a builtin that takes
// variadic arguments, where the arguments are either by ref or
// optionally by ref.  (It is equivalent to having ClassInfo's
// (RefVariableArguments | MixedVariableArguments).)
//
// AttrMayUseVV indicates that a function may need to use a VarEnv or
// varargs (aka extraArgs) at run time.
enum Attr {
  AttrNone      = 0,             // class  property  method  //
  AttrReference = (1 << 0),      //                     X    //
  AttrPublic    = (1 << 1),      //            X        X    //
  AttrProtected = (1 << 2),      //            X        X    //
  AttrPrivate   = (1 << 3),      //            X        X    //
  AttrStatic    = (1 << 4),      //            X        X    //
  AttrAbstract  = (1 << 5),      //    X                X    //
  AttrFinal     = (1 << 6),      //    X                X    //
  AttrInterface = (1 << 7),      //    X                     //
  AttrTrait     = (1 << 8),      //    X                X    //
  AttrNoInjection = (1 << 9),    //                     X    //
  AttrUnique    = (1 << 10),     //    X                X    //
  AttrDynamicInvoke = (1 << 11), //                     X    //
  AttrNoExpandTrait = (1 << 12), //    X                     //
  AttrNoOverride= (1 << 13),     //    X                X    //
  AttrClone     = (1 << 14),     //                     X    //
  AttrVariadicByRef = (1 << 15), //                     X    //
  AttrMayUseVV  = (1 << 16),     //                     X    //
  AttrPersistent= (1 << 17)      //    X                X    //
};

static inline Attr operator|(Attr a, Attr b) { return Attr((int)a | (int)b); }

static inline const char * attrToVisibilityStr(Attr attr) {
  return (attr & AttrPrivate)   ? "private"   :
         (attr & AttrProtected) ? "protected" : "public";
}

} }

#endif
