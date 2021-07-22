// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the \"hack\" directory of this source tree.
//
// @generated SignedSource<<f902e96b7689087f3b65c11c42c485f7>>


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

enum class BareThisOp {
  Notice,
  NoNotice,
  NeverNull,
};

enum class CheckStarted {
  IgnoreStarted,
  CheckStarted,
};

enum class ClassKind {
  Class,
  Interface,
  Trait,
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

enum class EqOp {
  PlusEqual,
  MinusEqual,
  MulEqual,
  ConcatEqual,
  DivEqual,
  PowEqual,
  ModEqual,
  AndEqual,
  OrEqual,
  XorEqual,
  SlEqual,
  SrEqual,
  PlusEqualO,
  MinusEqualO,
  MulEqualO,
};

enum class FatalOp {
  Parse,
  Runtime,
  RuntimeOmitFrame,
};

enum class IncdecOp {
  PreInc,
  PostInc,
  PreDec,
  PostDec,
  PreIncO,
  PostIncO,
  PreDecO,
  PostDecO,
};

enum class InitpropOp {
  Static,
  NonStatic,
};

enum class InstructBasic {
  Nop,
  EntryNop,
  PopC,
  PopU,
  Dup,
};

enum class InstructIncludeEvalDefine {
  Incl,
  InclOnce,
  Req,
  ReqOnce,
  ReqDoc,
  Eval,
};

enum class InstructTry {
  TryCatchBegin,
  TryCatchMiddle,
  TryCatchEnd,
};

enum class IsLogAsDynamicCallOp {
  LogAsDynamicCall,
  DontLogAsDynamicCall,
};

enum class IstypeOp {
  OpNull,
  OpBool,
  OpInt,
  OpDbl,
  OpStr,
  OpObj,
  OpRes,
  OpScalar,
  /// Int or Dbl or Str or Bool
  OpKeyset,
  OpDict,
  OpVec,
  OpArrLike,
  /// Arr or Vec or Dict or Keyset
  OpClsMeth,
  OpFunc,
  OpLegacyArrLike,
  OpClass,
};

enum class MemberOpMode {
  ModeNone,
  Warn,
  Define,
  Unset,
  InOut,
};

enum class ObjNullFlavor {
  NullThrows,
  NullSafe,
};

enum class OpSilence {
  Start,
  End,
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

enum class SetrangeOp {
  Forward,
  Reverse,
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

struct IterArgs {
  Id iter_id;
  Maybe<Local> key_id;
  Local val_id;
};

struct InstructIterator {
  enum class Tag {
    IterInit,
    IterNext,
    IterFree,
  };

  struct IterInit_Body {
    IterArgs _0;
    Label _1;
  };

  struct IterNext_Body {
    IterArgs _0;
    Label _1;
  };

  struct IterFree_Body {
    Id _0;
  };

  Tag tag;
  union {
    IterInit_Body iter_init;
    IterNext_Body iter_next;
    IterFree_Body iter_free;
  };
};

using ClassId = Type;

/// Conventionally this is "A_" followed by an integer
using AdataId = Str;

using ConstId = Type;

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

using FunctionId = Type;

using MethodId = Type;

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

using NumParams = size_t;

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

struct InstructSpecialFlow {
  enum class Tag {
    Continue,
    Break,
    Goto,
  };

  struct Continue_Body {
    ptrdiff_t _0;
  };

  struct Break_Body {
    ptrdiff_t _0;
  };

  struct Goto_Body {
    Str _0;
  };

  Tag tag;
  union {
    Continue_Body continue_;
    Break_Body break_;
    Goto_Body goto_;
  };
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

using ByRefs = Slice<bool>;

struct FcallArgs {
  FcallFlags _0;
  NumParams _1;
  NumParams _2;
  ByRefs _3;
  Maybe<Label> _4;
  Maybe<Str> _5;
};

struct InstructCall {
  enum class Tag {
    NewObj,
    NewObjR,
    NewObjD,
    NewObjRD,
    NewObjS,
    FCall,
    FCallClsMethod,
    FCallClsMethodD,
    FCallClsMethodS,
    FCallClsMethodSD,
    FCallCtor,
    FCallFunc,
    FCallFuncD,
    FCallObjMethod,
    FCallObjMethodD,
  };

  struct NewObjD_Body {
    ClassId _0;
  };

  struct NewObjRD_Body {
    ClassId _0;
  };

  struct NewObjS_Body {
    SpecialClsRef _0;
  };

  struct FCall_Body {
    FcallArgs _0;
  };

  struct FCallClsMethod_Body {
    FcallArgs _0;
    IsLogAsDynamicCallOp _1;
  };

  struct FCallClsMethodD_Body {
    FcallArgs _0;
    ClassId _1;
    MethodId _2;
  };

  struct FCallClsMethodS_Body {
    FcallArgs _0;
    SpecialClsRef _1;
  };

  struct FCallClsMethodSD_Body {
    FcallArgs _0;
    SpecialClsRef _1;
    MethodId _2;
  };

  struct FCallCtor_Body {
    FcallArgs _0;
  };

  struct FCallFunc_Body {
    FcallArgs _0;
  };

  struct FCallFuncD_Body {
    FcallArgs _0;
    FunctionId _1;
  };

  struct FCallObjMethod_Body {
    FcallArgs _0;
    ObjNullFlavor _1;
  };

  struct FCallObjMethodD_Body {
    FcallArgs _0;
    ObjNullFlavor _1;
    MethodId _2;
  };

  Tag tag;
  union {
    NewObjD_Body new_obj_d;
    NewObjRD_Body new_obj_rd;
    NewObjS_Body new_obj_s;
    FCall_Body f_call;
    FCallClsMethod_Body f_call_cls_method;
    FCallClsMethodD_Body f_call_cls_method_d;
    FCallClsMethodS_Body f_call_cls_method_s;
    FCallClsMethodSD_Body f_call_cls_method_sd;
    FCallCtor_Body f_call_ctor;
    FCallFunc_Body f_call_func;
    FCallFuncD_Body f_call_func_d;
    FCallObjMethod_Body f_call_obj_method;
    FCallObjMethodD_Body f_call_obj_method_d;
  };
};

struct ParamId {
  enum class Tag {
    ParamUnnamed,
    ParamNamed,
  };

  struct ParamUnnamed_Body {
    ptrdiff_t _0;
  };

  struct ParamNamed_Body {
    Str _0;
  };

  Tag tag;
  union {
    ParamUnnamed_Body param_unnamed;
    ParamNamed_Body param_named;
  };
};

using ClassNum = ptrdiff_t;

/// see runtime/base/repo-auth-type.h
using RepoAuthType = Str;

using StackIndex = ptrdiff_t;

struct InstructMisc {
  enum class Tag {
    This,
    BareThis,
    CheckThis,
    FuncNumArgs,
    ChainFaults,
    OODeclExists,
    VerifyParamType,
    VerifyParamTypeTS,
    VerifyOutType,
    VerifyRetTypeC,
    VerifyRetTypeTS,
    Self_,
    Parent,
    LateBoundCls,
    ClassName,
    LazyClassFromClass,
    RecordReifiedGeneric,
    CheckReifiedGenericMismatch,
    NativeImpl,
    AKExists,
    CreateCl,
    Idx,
    ArrayIdx,
    ArrayMarkLegacy,
    ArrayUnmarkLegacy,
    AssertRATL,
    AssertRATStk,
    BreakTraceHint,
    Silence,
    GetMemoKeyL,
    CGetCUNop,
    UGetCUNop,
    MemoGet,
    MemoGetEager,
    MemoSet,
    MemoSetEager,
    LockObj,
    ThrowNonExhaustiveSwitch,
    RaiseClassStringConversionWarning,
  };

  struct BareThis_Body {
    BareThisOp _0;
  };

  struct OODeclExists_Body {
    ClassKind _0;
  };

  struct VerifyParamType_Body {
    ParamId _0;
  };

  struct VerifyParamTypeTS_Body {
    ParamId _0;
  };

  struct VerifyOutType_Body {
    ParamId _0;
  };

  struct CreateCl_Body {
    NumParams _0;
    ClassNum _1;
  };

  struct AssertRATL_Body {
    Local _0;
    RepoAuthType _1;
  };

  struct AssertRATStk_Body {
    StackIndex _0;
    RepoAuthType _1;
  };

  struct Silence_Body {
    Local _0;
    OpSilence _1;
  };

  struct GetMemoKeyL_Body {
    Local _0;
  };

  struct MemoGet_Body {
    Label _0;
    Maybe<Pair<Local, ptrdiff_t>> _1;
  };

  struct MemoGetEager_Body {
    Label _0;
    Label _1;
    Maybe<Pair<Local, ptrdiff_t>> _2;
  };

  struct MemoSet_Body {
    Maybe<Pair<Local, ptrdiff_t>> _0;
  };

  struct MemoSetEager_Body {
    Maybe<Pair<Local, ptrdiff_t>> _0;
  };

  Tag tag;
  union {
    BareThis_Body bare_this;
    OODeclExists_Body oo_decl_exists;
    VerifyParamType_Body verify_param_type;
    VerifyParamTypeTS_Body verify_param_type_ts;
    VerifyOutType_Body verify_out_type;
    CreateCl_Body create_cl;
    AssertRATL_Body assert_ratl;
    AssertRATStk_Body assert_rat_stk;
    Silence_Body silence;
    GetMemoKeyL_Body get_memo_key_l;
    MemoGet_Body memo_get;
    MemoGetEager_Body memo_get_eager;
    MemoSet_Body memo_set;
    MemoSetEager_Body memo_set_eager;
  };
};

struct InstructGet {
  enum class Tag {
    CGetL,
    CGetQuietL,
    CGetL2,
    CUGetL,
    PushL,
    CGetG,
    CGetS,
    ClassGetC,
    ClassGetTS,
  };

  struct CGetL_Body {
    Local _0;
  };

  struct CGetQuietL_Body {
    Local _0;
  };

  struct CGetL2_Body {
    Local _0;
  };

  struct CUGetL_Body {
    Local _0;
  };

  struct PushL_Body {
    Local _0;
  };

  struct CGetS_Body {
    ReadOnlyOp _0;
  };

  Tag tag;
  union {
    CGetL_Body c_get_l;
    CGetQuietL_Body c_get_quiet_l;
    CGetL2_Body c_get_l2;
    CUGetL_Body cu_get_l;
    PushL_Body push_l;
    CGetS_Body c_get_s;
  };
};

using PropId = Type;

struct InstructMutator {
  enum class Tag {
    SetL,
    /// PopL is put in mutators since it behaves as SetL + PopC
    PopL,
    SetG,
    SetS,
    SetOpL,
    SetOpG,
    SetOpS,
    IncDecL,
    IncDecG,
    IncDecS,
    UnsetL,
    UnsetG,
    CheckProp,
    InitProp,
  };

  struct SetL_Body {
    Local _0;
  };

  struct PopL_Body {
    Local _0;
  };

  struct SetS_Body {
    ReadOnlyOp _0;
  };

  struct SetOpL_Body {
    Local _0;
    EqOp _1;
  };

  struct SetOpG_Body {
    EqOp _0;
  };

  struct SetOpS_Body {
    EqOp _0;
  };

  struct IncDecL_Body {
    Local _0;
    IncdecOp _1;
  };

  struct IncDecG_Body {
    IncdecOp _0;
  };

  struct IncDecS_Body {
    IncdecOp _0;
  };

  struct UnsetL_Body {
    Local _0;
  };

  struct CheckProp_Body {
    PropId _0;
  };

  struct InitProp_Body {
    PropId _0;
    InitpropOp _1;
  };

  Tag tag;
  union {
    SetL_Body set_l;
    PopL_Body pop_l;
    SetS_Body set_s;
    SetOpL_Body set_op_l;
    SetOpG_Body set_op_g;
    SetOpS_Body set_op_s;
    IncDecL_Body inc_dec_l;
    IncDecG_Body inc_dec_g;
    IncDecS_Body inc_dec_s;
    UnsetL_Body unset_l;
    CheckProp_Body check_prop;
    InitProp_Body init_prop;
  };
};

struct InstructIsset {
  enum class Tag {
    IssetC,
    IssetL,
    IssetG,
    IssetS,
    IsUnsetL,
    IsTypeC,
    IsTypeL,
  };

  struct IssetL_Body {
    Local _0;
  };

  struct IsUnsetL_Body {
    Local _0;
  };

  struct IsTypeC_Body {
    IstypeOp _0;
  };

  struct IsTypeL_Body {
    Local _0;
    IstypeOp _1;
  };

  Tag tag;
  union {
    IssetL_Body isset_l;
    IsUnsetL_Body is_unset_l;
    IsTypeC_Body is_type_c;
    IsTypeL_Body is_type_l;
  };
};

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

struct InstructBase {
  enum class Tag {
    BaseGC,
    BaseGL,
    BaseSC,
    BaseL,
    BaseC,
    BaseH,
    Dim,
  };

  struct BaseGC_Body {
    StackIndex _0;
    MemberOpMode _1;
  };

  struct BaseGL_Body {
    Local _0;
    MemberOpMode _1;
  };

  struct BaseSC_Body {
    StackIndex _0;
    StackIndex _1;
    MemberOpMode _2;
    ReadOnlyOp _3;
  };

  struct BaseL_Body {
    Local _0;
    MemberOpMode _1;
  };

  struct BaseC_Body {
    StackIndex _0;
    MemberOpMode _1;
  };

  struct Dim_Body {
    MemberOpMode _0;
    MemberKey _1;
  };

  Tag tag;
  union {
    BaseGC_Body base_gc;
    BaseGL_Body base_gl;
    BaseSC_Body base_sc;
    BaseL_Body base_l;
    BaseC_Body base_c;
    Dim_Body dim;
  };
};

struct InstructFinal {
  enum class Tag {
    QueryM,
    SetM,
    IncDecM,
    SetOpM,
    UnsetM,
    SetRangeM,
  };

  struct QueryM_Body {
    NumParams _0;
    QueryOp _1;
    MemberKey _2;
  };

  struct SetM_Body {
    NumParams _0;
    MemberKey _1;
  };

  struct IncDecM_Body {
    NumParams _0;
    IncdecOp _1;
    MemberKey _2;
  };

  struct SetOpM_Body {
    NumParams _0;
    EqOp _1;
    MemberKey _2;
  };

  struct UnsetM_Body {
    NumParams _0;
    MemberKey _1;
  };

  struct SetRangeM_Body {
    NumParams _0;
    ptrdiff_t _1;
    SetrangeOp _2;
  };

  Tag tag;
  union {
    QueryM_Body query_m;
    SetM_Body set_m;
    IncDecM_Body inc_dec_m;
    SetOpM_Body set_op_m;
    UnsetM_Body unset_m;
    SetRangeM_Body set_range_m;
  };
};

struct Srcloc {
  ptrdiff_t line_begin;
  ptrdiff_t col_begin;
  ptrdiff_t line_end;
  ptrdiff_t col_end;
};

struct AsyncFunctions {
  enum class Tag {
    WHResult,
    Await,
    AwaitAll,
  };

  struct AwaitAll_Body {
    Maybe<Pair<Local, ptrdiff_t>> _0;
  };

  Tag tag;
  union {
    AwaitAll_Body await_all;
  };
};

struct GenCreationExecution {
  enum class Tag {
    CreateCont,
    ContEnter,
    ContRaise,
    Yield,
    YieldK,
    ContCheck,
    ContValid,
    ContKey,
    ContGetReturn,
    ContCurrent,
  };

  struct ContCheck_Body {
    CheckStarted _0;
  };

  Tag tag;
  union {
    ContCheck_Body cont_check;
  };
};

struct Instruct {
  enum class Tag {
    IBasic,
    IIterator,
    ILitConst,
    IOp,
    IContFlow,
    ISpecialFlow,
    ICall,
    IMisc,
    IGet,
    IMutator,
    IIsset,
    IBase,
    IFinal,
    ILabel,
    ITry,
    IComment,
    ISrcLoc,
    IAsync,
    IGenerator,
    IIncludeEvalDefine,
  };

  struct IBasic_Body {
    InstructBasic _0;
  };

  struct IIterator_Body {
    InstructIterator _0;
  };

  struct ILitConst_Body {
    InstructLitConst _0;
  };

  struct IOp_Body {
    InstructOperator _0;
  };

  struct IContFlow_Body {
    InstructControlFlow _0;
  };

  struct ISpecialFlow_Body {
    InstructSpecialFlow _0;
  };

  struct ICall_Body {
    InstructCall _0;
  };

  struct IMisc_Body {
    InstructMisc _0;
  };

  struct IGet_Body {
    InstructGet _0;
  };

  struct IMutator_Body {
    InstructMutator _0;
  };

  struct IIsset_Body {
    InstructIsset _0;
  };

  struct IBase_Body {
    InstructBase _0;
  };

  struct IFinal_Body {
    InstructFinal _0;
  };

  struct ILabel_Body {
    Label _0;
  };

  struct ITry_Body {
    InstructTry _0;
  };

  struct IComment_Body {
    Str _0;
  };

  struct ISrcLoc_Body {
    Srcloc _0;
  };

  struct IAsync_Body {
    AsyncFunctions _0;
  };

  struct IGenerator_Body {
    GenCreationExecution _0;
  };

  struct IIncludeEvalDefine_Body {
    InstructIncludeEvalDefine _0;
  };

  Tag tag;
  union {
    IBasic_Body i_basic;
    IIterator_Body i_iterator;
    ILitConst_Body i_lit_const;
    IOp_Body i_op;
    IContFlow_Body i_cont_flow;
    ISpecialFlow_Body i_special_flow;
    ICall_Body i_call;
    IMisc_Body i_misc;
    IGet_Body i_get;
    IMutator_Body i_mutator;
    IIsset_Body i_isset;
    IBase_Body i_base;
    IFinal_Body i_final;
    ILabel_Body i_label;
    ITry_Body i_try;
    IComment_Body i_comment;
    ISrcLoc_Body i_src_loc;
    IAsync_Body i_async;
    IGenerator_Body i_generator;
    IIncludeEvalDefine_Body i_include_eval_define;
  };
};


extern "C" {

void no_call_compile_only_USED_TYPES_hhbc_ast(Instruct);

} // extern "C"

} // namespace ast
} // namespace hhbc
} // namespace hackc
} // namespace HPHP
