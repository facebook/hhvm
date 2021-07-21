// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the \"hack\" directory of this source tree.
//
// @generated SignedSource<<09ea008979c0ec408bb182a058cb90e6>>


#pragma once



#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <ostream>
#include <new>
#include "hphp/hack/src/utils/ffi/ffi.h"
#include "hphp/hack/src/hhbc/hhbc_by_ref/hhbc_id.h"
#include "hphp/hack/src/hhbc/hhbc_by_ref/hhbc_label.h"
#include "hphp/hack/src/hhbc/hhbc_by_ref/hhbc_local.h"
#include "hphp/hack/src/hhbc/hhbc_by_ref/hhbc_runtime.h"


namespace HPHP {
namespace hackc {
namespace hhbc {
namespace ast {

enum class CheckStarted {
  IgnoreStarted,
  CheckStarted,
};

enum class CollectionType {
  Vector,
  Map,
  Set,
  Pair,
  ImmVector,
  ImmMap,
  ImmSet,
  Dict,
  Array,
  Keyset,
  Vec,
};

enum class FatalOp {
  Parse,
  Runtime,
  RuntimeOmitFrame,
};

enum class FreeIterator {
  IgnoreIter,
  FreeIter,
};

enum class HasGenericsOp {
  NoGenerics,
  MaybeGenerics,
  HasGenerics,
};

enum class InstructBasic {
  Nop,
  EntryNop,
  PopC,
  PopU,
  Dup,
};

enum class IsLogAsDynamicCallOp {
  LogAsDynamicCall,
  DontLogAsDynamicCall,
};

enum class MemberOpMode {
  ModeNone,
  Warn,
  Define,
  Unset,
  InOut,
};

enum class QueryOp {
  CGet,
  CGetQuiet,
  Isset,
  InOut,
};

enum class ReadOnlyOp {
  ReadOnly,
  Mutable,
  Any,
  CheckROCOW,
};

enum class SpecialClsRef {
  Static,
  Self_,
  Parent,
};

enum class Switchkind {
  Bounded,
  Unbounded,
};

enum class TypestructResolveOp {
  Resolve,
  DontResolve,
};

struct FcallFlags {
  uint8_t bits;

  explicit operator bool() const {
    return !!bits;
  }
  FcallFlags operator~() const {
    return {static_cast<decltype(bits)>(~bits)};
  }
  FcallFlags operator|(const FcallFlags& other) const {
    return {static_cast<decltype(bits)>(this->bits | other.bits)};
  }
  FcallFlags& operator|=(const FcallFlags& other) {
    *this = (*this | other);
    return *this;
  }
  FcallFlags operator&(const FcallFlags& other) const {
    return {static_cast<decltype(bits)>(this->bits & other.bits)};
  }
  FcallFlags& operator&=(const FcallFlags& other) {
    *this = (*this & other);
    return *this;
  }
  FcallFlags operator^(const FcallFlags& other) const {
    return {static_cast<decltype(bits)>(this->bits ^ other.bits)};
  }
  FcallFlags& operator^=(const FcallFlags& other) {
    *this = (*this ^ other);
    return *this;
  }
};
static const FcallFlags FcallFlags_HAS_UNPACK = FcallFlags{ /* .bits = */ (uint8_t)1 };
static const FcallFlags FcallFlags_HAS_GENERICS = FcallFlags{ /* .bits = */ (uint8_t)2 };
static const FcallFlags FcallFlags_LOCK_WHILE_UNWINDING = FcallFlags{ /* .bits = */ (uint8_t)4 };

using ParamNum = ptrdiff_t;

using StackIndex = ptrdiff_t;

using RecordNum = ptrdiff_t;

using TypedefNum = ptrdiff_t;

using ClassNum = ptrdiff_t;

using ConstNum = ptrdiff_t;

using ClassId = Type;

using FunctionId = Type;

using MethodId = Type;

using ConstId = Type;

using PropId = Type;

using NumParams = size_t;

using ByRefs = Slice<bool>;

struct FcallArgs {
  FcallFlags _0;
  NumParams _1;
  NumParams _2;
  ByRefs _3;
  Maybe<Label> _4;
  Maybe<Str> _5;
};

struct IterArgs {
  Id iter_id;
  Maybe<Local> key_id;
  Local val_id;
};

/// Conventionally this is "A_" followed by an integer
using AdataId = Str;

using ParamLocations = Slice<ptrdiff_t>;

struct MemberKey {
  enum class Tag {
    EC,
    EL,
    ET,
    EI,
    PC,
    PL,
    PT,
    QT,
    W,
  };

  struct EC_Body {
    StackIndex _0;
    ReadOnlyOp _1;
  };

  struct EL_Body {
    Local _0;
    ReadOnlyOp _1;
  };

  struct ET_Body {
    Str _0;
    ReadOnlyOp _1;
  };

  struct EI_Body {
    int64_t _0;
    ReadOnlyOp _1;
  };

  struct PC_Body {
    StackIndex _0;
    ReadOnlyOp _1;
  };

  struct PL_Body {
    Local _0;
    ReadOnlyOp _1;
  };

  struct PT_Body {
    PropId _0;
    ReadOnlyOp _1;
  };

  struct QT_Body {
    PropId _0;
    ReadOnlyOp _1;
  };

  Tag tag;
  union {
    EC_Body ec;
    EL_Body el;
    ET_Body et;
    EI_Body ei;
    PC_Body pc;
    PL_Body pl;
    PT_Body pt;
    QT_Body qt;
  };
};

struct InstructLitConst {
  enum class Tag {
    Null,
    True,
    False,
    NullUninit,
    Int,
    Double,
    String,
    LazyClass,
    /// Pseudo instruction that will get translated into appropraite literal
    /// bytecode, with possible reference to .adata *)
    TypedValue,
    Vec,
    Dict,
    Keyset,
    /// capacity hint
    NewDictArray,
    NewStructDict,
    NewVec,
    NewKeysetArray,
    NewPair,
    NewRecord,
    AddElemC,
    AddNewElemC,
    NewCol,
    ColFromArray,
    CnsE,
    ClsCns,
    ClsCnsD,
    ClsCnsL,
    File,
    Dir,
    Method,
    FuncCred,
  };

  struct Int_Body {
    int64_t _0;
  };

  struct Double_Body {
    Str _0;
  };

  struct String_Body {
    Str _0;
  };

  struct LazyClass_Body {
    ClassId _0;
  };

  struct TypedValue_Body {
    TypedValue _0;
  };

  struct Vec_Body {
    AdataId _0;
  };

  struct Dict_Body {
    AdataId _0;
  };

  struct Keyset_Body {
    AdataId _0;
  };

  struct NewDictArray_Body {
    ptrdiff_t _0;
  };

  struct NewStructDict_Body {
    Slice<Str> _0;
  };

  struct NewVec_Body {
    ptrdiff_t _0;
  };

  struct NewKeysetArray_Body {
    ptrdiff_t _0;
  };

  struct NewRecord_Body {
    ClassId _0;
    Slice<Str> _1;
  };

  struct NewCol_Body {
    CollectionType _0;
  };

  struct ColFromArray_Body {
    CollectionType _0;
  };

  struct CnsE_Body {
    ConstId _0;
  };

  struct ClsCns_Body {
    ConstId _0;
  };

  struct ClsCnsD_Body {
    ConstId _0;
    ClassId _1;
  };

  struct ClsCnsL_Body {
    Local _0;
  };

  Tag tag;
  union {
    Int_Body int_;
    Double_Body double_;
    String_Body string;
    LazyClass_Body lazy_class;
    TypedValue_Body typed_value;
    Vec_Body vec;
    Dict_Body dict;
    Keyset_Body keyset;
    NewDictArray_Body new_dict_array;
    NewStructDict_Body new_struct_dict;
    NewVec_Body new_vec;
    NewKeysetArray_Body new_keyset_array;
    NewRecord_Body new_record;
    NewCol_Body new_col;
    ColFromArray_Body col_from_array;
    CnsE_Body cns_e;
    ClsCns_Body cls_cns;
    ClsCnsD_Body cls_cns_d;
    ClsCnsL_Body cls_cns_l;
  };
};

struct InstructOperator {
  enum class Tag {
    Concat,
    ConcatN,
    Add,
    Sub,
    Mul,
    AddO,
    SubO,
    MulO,
    Div,
    Mod,
    Pow,
    Not,
    Same,
    NSame,
    Eq,
    Neq,
    Lt,
    Lte,
    Gt,
    Gte,
    Cmp,
    BitAnd,
    BitOr,
    BitXor,
    BitNot,
    Shl,
    Shr,
    CastBool,
    CastInt,
    CastDouble,
    CastString,
    CastVec,
    CastDict,
    CastKeyset,
    InstanceOf,
    InstanceOfD,
    IsLateBoundCls,
    IsTypeStructC,
    ThrowAsTypeStructException,
    CombineAndResolveTypeStruct,
    Print,
    Clone,
    Exit,
    Fatal,
    ResolveFunc,
    ResolveRFunc,
    ResolveMethCaller,
    ResolveObjMethod,
    ResolveClsMethod,
    ResolveClsMethodD,
    ResolveClsMethodS,
    ResolveRClsMethod,
    ResolveRClsMethodD,
    ResolveRClsMethodS,
    ResolveClass,
  };

  struct ConcatN_Body {
    ptrdiff_t _0;
  };

  struct InstanceOfD_Body {
    ClassId _0;
  };

  struct IsTypeStructC_Body {
    TypestructResolveOp _0;
  };

  struct CombineAndResolveTypeStruct_Body {
    ptrdiff_t _0;
  };

  struct Fatal_Body {
    FatalOp _0;
  };

  struct ResolveFunc_Body {
    FunctionId _0;
  };

  struct ResolveRFunc_Body {
    FunctionId _0;
  };

  struct ResolveMethCaller_Body {
    FunctionId _0;
  };

  struct ResolveClsMethod_Body {
    MethodId _0;
  };

  struct ResolveClsMethodD_Body {
    ClassId _0;
    MethodId _1;
  };

  struct ResolveClsMethodS_Body {
    SpecialClsRef _0;
    MethodId _1;
  };

  struct ResolveRClsMethod_Body {
    MethodId _0;
  };

  struct ResolveRClsMethodD_Body {
    ClassId _0;
    MethodId _1;
  };

  struct ResolveRClsMethodS_Body {
    SpecialClsRef _0;
    MethodId _1;
  };

  struct ResolveClass_Body {
    ClassId _0;
  };

  Tag tag;
  union {
    ConcatN_Body concat_n;
    InstanceOfD_Body instance_of_d;
    IsTypeStructC_Body is_type_struct_c;
    CombineAndResolveTypeStruct_Body combine_and_resolve_type_struct;
    Fatal_Body fatal;
    ResolveFunc_Body resolve_func;
    ResolveRFunc_Body resolve_r_func;
    ResolveMethCaller_Body resolve_meth_caller;
    ResolveClsMethod_Body resolve_cls_method;
    ResolveClsMethodD_Body resolve_cls_method_d;
    ResolveClsMethodS_Body resolve_cls_method_s;
    ResolveRClsMethod_Body resolve_r_cls_method;
    ResolveRClsMethodD_Body resolve_r_cls_method_d;
    ResolveRClsMethodS_Body resolve_r_cls_method_s;
    ResolveClass_Body resolve_class;
  };
};

struct InstructControlFlow {
  enum class Tag {
    Jmp,
    JmpNS,
    JmpZ,
    JmpNZ,
    /// bounded, base, offset vector
    Switch,
    /// litstr id / offset vector
    SSwitch,
    RetC,
    RetCSuspended,
    RetM,
    Throw,
  };

  struct Jmp_Body {
    Label _0;
  };

  struct JmpNS_Body {
    Label _0;
  };

  struct JmpZ_Body {
    Label _0;
  };

  struct JmpNZ_Body {
    Label _0;
  };

  struct Switch_Body {
    Switchkind _0;
    ptrdiff_t _1;
    BumpSliceMut<Label> _2;
  };

  struct SSwitch_Body {
    BumpSliceMut<Pair<Str, Label>> _0;
  };

  struct RetM_Body {
    NumParams _0;
  };

  Tag tag;
  union {
    Jmp_Body jmp;
    JmpNS_Body jmp_ns;
    JmpZ_Body jmp_z;
    JmpNZ_Body jmp_nz;
    Switch_Body switch_;
    SSwitch_Body s_switch;
    RetM_Body ret_m;
  };
};


extern "C" {

void no_call_compile_only_USED_TYPES_hhbc_ast(CheckStarted,
                                              FreeIterator,
                                              FcallFlags,
                                              ParamNum,
                                              StackIndex,
                                              RecordNum,
                                              TypedefNum,
                                              ClassNum,
                                              ConstNum,
                                              ClassId,
                                              FunctionId,
                                              MethodId,
                                              ConstId,
                                              PropId,
                                              FcallArgs,
                                              IterArgs,
                                              AdataId,
                                              ParamLocations,
                                              SpecialClsRef,
                                              MemberOpMode,
                                              QueryOp,
                                              CollectionType,
                                              FatalOp,
                                              MemberKey,
                                              InstructBasic,
                                              TypestructResolveOp,
                                              ReadOnlyOp,
                                              HasGenericsOp,
                                              IsLogAsDynamicCallOp,
                                              InstructLitConst,
                                              InstructOperator,
                                              Switchkind,
                                              InstructControlFlow);

} // extern "C"

} // namespace ast
} // namespace hhbc
} // namespace hackc
} // namespace HPHP
