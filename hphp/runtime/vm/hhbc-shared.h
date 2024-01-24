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

#include "hphp/runtime/base/header-kind-shared.h"

namespace HPHP {

#define FATAL_OPS                               \
  FATAL_OP(Runtime)                             \
  FATAL_OP(Parse)                               \
  FATAL_OP(RuntimeOmitFrame)

enum class FatalOp : uint8_t {
#define FATAL_OP(x) x,
  FATAL_OPS
#undef FATAL_OP
};

#define INITPROP_OPS    \
  INITPROP_OP(Static)   \
  INITPROP_OP(NonStatic)

enum class InitPropOp : uint8_t {
#define INITPROP_OP(op) op,
  INITPROP_OPS
#undef INITPROP_OP
};

#define ISTYPE_OPS                             \
  ISTYPE_OP(Null)                              \
  ISTYPE_OP(Bool)                              \
  ISTYPE_OP(Int)                               \
  ISTYPE_OP(Dbl)                               \
  ISTYPE_OP(Str)                               \
  ISTYPE_OP(Obj)                               \
  ISTYPE_OP(Res)                               \
  ISTYPE_OP(Scalar)                            \
  ISTYPE_OP(Keyset)                            \
  ISTYPE_OP(Dict)                              \
  ISTYPE_OP(Vec)                               \
  ISTYPE_OP(ArrLike)                           \
  ISTYPE_OP(ClsMeth)                           \
  ISTYPE_OP(Func)                              \
  ISTYPE_OP(LegacyArrLike)                     \
  ISTYPE_OP(Class)

enum class IsTypeOp : uint8_t {
#define ISTYPE_OP(op) op,
  ISTYPE_OPS
#undef ISTYPE_OP
};

#define SPECIAL_CLS_REFS                        \
  REF(SelfCls)                                  \
  REF(LateBoundCls)                             \
  REF(ParentCls)

enum class SpecialClsRef : uint8_t {
#define REF(name) name,
  SPECIAL_CLS_REFS
#undef REF
};

#define CLASS_GET_C_MODES                                           \
  MODE(Normal) /* Result of compiling expressions like $c::foo() */ \

enum class ClassGetCMode : uint8_t {
#define MODE(op) op,
  CLASS_GET_C_MODES
#undef MODE
};

#define M_OP_MODES                                 \
  MODE(None)                                       \
  MODE(Warn)                                       \
  MODE(Define)                                     \
  MODE(Unset)                                      \
  /* InOut mode restricts allowed bases to the
     array like types. */                          \
  MODE(InOut)

enum class MOpMode : uint8_t {
#define MODE(name) name,
  M_OP_MODES
#undef MODE
};

#define QUERY_M_OPS                               \
  OP(CGet)                                        \
  OP(CGetQuiet)                                   \
  OP(Isset)                                       \
  OP(InOut)

enum class QueryMOp : uint8_t {
#define OP(name) name,
  QUERY_M_OPS
#undef OP
};


#define TYPE_STRUCT_RESOLVE_OPS \
  OP(Resolve)                   \
  OP(DontResolve)

enum class TypeStructResolveOp : uint8_t {
#define OP(name) name,
  TYPE_STRUCT_RESOLVE_OPS
#undef OP
};

#define TYPE_STRUCT_ENFORCE_KINDS \
  KIND(Deep)                      \
  KIND(Shallow)

enum class TypeStructEnforceKind : uint8_t {
#define KIND(name) name,
  TYPE_STRUCT_ENFORCE_KINDS
#undef KIND
};

#define AS_TYPE_STRUCT_EXCEPTION_KINDS \
  KIND(Error)                           \
  KIND(Typehint)

enum class AsTypeStructExceptionKind : uint8_t {
#define KIND(name) name,
  AS_TYPE_STRUCT_EXCEPTION_KINDS
#undef KIND
};

#define IS_LOG_AS_DYNAMIC_CALL_OPS                  \
  IS_LOG_AS_DYNAMIC_CALL_OP(LogAsDynamicCall)       \
  IS_LOG_AS_DYNAMIC_CALL_OP(DontLogAsDynamicCall)

enum class IsLogAsDynamicCallOp : uint8_t {
#define IS_LOG_AS_DYNAMIC_CALL_OP(name) name,
  IS_LOG_AS_DYNAMIC_CALL_OPS
#undef IS_LOG_AS_DYNAMIC_CALL_OP
};

#define READONLY_OPS    \
  OP(Any)               \
  OP(Readonly)          \
  OP(Mutable)           \
  OP(CheckROCOW)        \
  OP(CheckMutROCOW)

enum class ReadonlyOp : uint8_t {
#define OP(name) name,
  READONLY_OPS
#undef OP
};

#define SET_RANGE_OPS \
  OP(Forward)         \
  OP(Reverse)

enum class SetRangeOp : uint8_t {
#define OP(name) name,
  SET_RANGE_OPS
#undef OP
};

#define CONT_CHECK_OPS                            \
  CONT_CHECK_OP(IgnoreStarted)                    \
  CONT_CHECK_OP(CheckStarted)

enum class ContCheckOp : uint8_t {
#define CONT_CHECK_OP(name) name,
  CONT_CHECK_OPS
#undef CONT_CHECK_OP
};

#define SWITCH_KINDS                            \
  KIND(Unbounded)                               \
  KIND(Bounded)

enum class SwitchKind : uint8_t {
#define KIND(x) x,
  SWITCH_KINDS
#undef KIND
};

#define OBJMETHOD_OPS                             \
  OBJMETHOD_OP(NullThrows)                        \
  OBJMETHOD_OP(NullSafe)

enum class ObjMethodOp : uint8_t {
#define OBJMETHOD_OP(x) x,
  OBJMETHOD_OPS
#undef OBJMETHOD_OP
};

#define SILENCE_OPS \
  SILENCE_OP(Start) \
  SILENCE_OP(End)

enum class SilenceOp : uint8_t {
#define SILENCE_OP(x) x,
  SILENCE_OPS
#undef SILENCE_OP
};


#define BARETHIS_OPS    \
  BARETHIS_OP(Notice)   \
  BARETHIS_OP(NoNotice) \
  BARETHIS_OP(NeverNull)

enum class BareThisOp : uint8_t {
#define BARETHIS_OP(x) x,
  BARETHIS_OPS
#undef BARETHIS_OP
};

#define INCDEC_OPS    \
  INCDEC_OP(PreInc)   \
  INCDEC_OP(PostInc)  \
  INCDEC_OP(PreDec)   \
  INCDEC_OP(PostDec)  \

enum class IncDecOp : uint8_t {
#define INCDEC_OP(incDecOp) incDecOp,
  INCDEC_OPS
#undef INCDEC_OP
};

// Each of the setop ops maps to a binary bytecode op. We have reasons
// for using distinct bitwise representations, though. This macro records
// their correspondence for mapping either direction.
#define SETOP_OPS \
  SETOP_OP(PlusEqual,   OpAdd) \
  SETOP_OP(MinusEqual,  OpSub) \
  SETOP_OP(MulEqual,    OpMul) \
  SETOP_OP(ConcatEqual, OpConcat) \
  SETOP_OP(DivEqual,    OpDiv) \
  SETOP_OP(PowEqual,    OpPow) \
  SETOP_OP(ModEqual,    OpMod) \
  SETOP_OP(AndEqual,    OpBitAnd) \
  SETOP_OP(OrEqual,     OpBitOr) \
  SETOP_OP(XorEqual,    OpBitXor) \
  SETOP_OP(SlEqual,     OpShl) \
  SETOP_OP(SrEqual,     OpShr)  \

enum class SetOpOp : uint8_t {
#define SETOP_OP(setOpOp, bcOp) setOpOp,
  SETOP_OPS
#undef SETOP_OP
};

#define OO_DECL_EXISTS_OPS                             \
  OO_DECL_EXISTS_OP(Class)                             \
  OO_DECL_EXISTS_OP(Interface)                         \
  OO_DECL_EXISTS_OP(Trait)

enum class OODeclExistsOp : uint8_t {
#define OO_DECL_EXISTS_OP(x) x,
  OO_DECL_EXISTS_OPS
#undef OO_DECL_EXISTS_OP
};


}
