(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type check_started =
  | IgnoreStarted
  | CheckStarted

type free_iterator =
  | IgnoreIter
  | FreeIter

type repo_auth_type = string (* see see runtime/base/repo-auth-type.h *)

type local_id = Local.t

type param_id =
  | Param_unnamed of int
  | Param_named of string

type param_num = int

type stack_index = int

type class_id = Hhbc_id.Class.t

type class_num = int

type record_num = int

type constant_num = int

type typedef_num = int

type function_id = Hhbc_id.Function.t

type method_id = Hhbc_id.Method.t

type const_id = Hhbc_id.Const.t

type prop_id = Hhbc_id.Prop.t

type num_params = int

type fcall_flags = {
  has_unpack: bool;
  has_generics: bool;
  supports_async_eager_return: bool;
  lock_while_unwinding: bool;
}

type by_refs = bool list

type fcall_args =
  fcall_flags
  * num_params
  * num_params
  * by_refs
  * Label.t option
  * string option

type iter_args = {
  iter_id: Iterator.t;
  key_id: local_id option;
  val_id: local_id;
}

type classref_id = int

(* Conventionally this is "A_" followed by an integer *)
type adata_id = string

module SpecialClsRef = struct
  type t =
    | Static
    | Self
    | Parent

  let to_string r =
    match r with
    | Static -> "Static"
    | Self -> "Self"
    | Parent -> "Parent"
end

(* of SpecialClsRef *)

module MemberOpMode = struct
  type t =
    | ModeNone
    | Warn
    | Define
    | Unset
    | InOut

  let to_string op =
    match op with
    | ModeNone -> "None"
    | Warn -> "Warn"
    | Define -> "Define"
    | Unset -> "Unset"
    | InOut -> "InOut"
end

(* of MemberOpMode *)

module QueryOp = struct
  type t =
    | CGet
    | CGetQuiet
    | Isset
    | InOut

  let to_string op =
    match op with
    | CGet -> "CGet"
    | CGetQuiet -> "CGetQuiet"
    | Isset -> "Isset"
    | InOut -> "InOut"
end

(* of QueryOp *)

module CollectionType = struct
  type t =
    | Vector
    | Map
    | Set
    | Pair
    | ImmVector
    | ImmMap
    | ImmSet
    | Dict
    | Array
    | Keyset
    | Vec

  let to_string = function
    | Vector -> "Vector"
    | Map -> "Map"
    | Set -> "Set"
    | Pair -> "Pair"
    | ImmVector -> "ImmVector"
    | ImmMap -> "ImmMap"
    | ImmSet -> "ImmSet"
    | Dict -> "dict"
    | Array -> "array"
    | Keyset -> "keyset"
    | Vec -> "vec"
end

(* of CollectionType *)

module FatalOp = struct
  type t =
    | Parse
    | Runtime
    | RuntimeOmitFrame

  let to_string op =
    match op with
    | Parse -> "Parse"
    | Runtime -> "Runtime"
    | RuntimeOmitFrame -> "RuntimeOmitFrame"
end

(* of FatalOp *)

module MemberKey = struct
  type t =
    | EC of stack_index
    | EL of local_id
    | ET of string
    | EI of int64
    | PC of stack_index
    | PL of local_id
    | PT of prop_id
    | QT of prop_id
    | W
end

(* Of MemberKey *)

type instruct_basic =
  | Nop
  | EntryNop
  | PopC
  | PopU
  | Dup

type typestruct_resolve_op =
  | Resolve
  | DontResolve

type has_generics_op =
  | NoGenerics
  | MaybeGenerics
  | HasGenerics

type is_log_as_dynamic_call_op =
  | LogAsDynamicCall
  | DontLogAsDynamicCall

type instruct_lit_const =
  | Null
  | True
  | False
  | NullUninit
  | Int of int64
  | Double of string
  | String of string
  (* Pseudo instruction that will get translated into appropraite literal
   * bytecode, with possible reference to .adata *)
  | TypedValue of Typed_value.t
  | Array of adata_id
  | Vec of adata_id
  | Dict of adata_id
  | Keyset of adata_id
  | NewArray of int (* capacity hint *)
  | NewMixedArray of int (* capacity hint *)
  | NewDictArray of int (* capacity hint *)
  | NewDArray of int (* capacity hint *)
  | NewLikeArrayL of local_id * int (* capacity hint *)
  | NewPackedArray of int
  | NewStructArray of string list
  | NewStructDArray of string list
  | NewStructDict of string list
  | NewVArray of int
  | NewVecArray of int
  | NewKeysetArray of int
  | NewPair
  | NewRecord of class_id * string list
  | NewRecordArray of class_id * string list
  | AddElemC
  | AddNewElemC
  | NewCol of CollectionType.t
  | ColFromArray of CollectionType.t
  | CnsE of const_id
  | ClsCns of const_id
  | ClsCnsD of const_id * class_id
  | File
  | Dir
  | Method
  | FuncCred

type instruct_operator =
  | Concat
  | ConcatN of int
  | Abs
  | Add
  | Sub
  | Mul
  | AddO
  | SubO
  | MulO
  | Div
  | Mod
  | Pow
  | Sqrt
  | Xor
  | Not
  | Same
  | NSame
  | Eq
  | Neq
  | Lt
  | Lte
  | Gt
  | Gte
  | Cmp
  | BitAnd
  | BitOr
  | BitXor
  | BitNot
  | Shl
  | Shr
  | Floor
  | Ceil
  | CastBool
  | CastInt
  | CastDouble
  | CastString
  | CastArray
  | CastVec
  | CastDict
  | CastKeyset
  | CastVArray
  | CastDArray
  | InstanceOf
  | InstanceOfD of class_id
  | IsLateBoundCls
  | IsTypeStructC of typestruct_resolve_op
  | ThrowAsTypeStructException
  | CombineAndResolveTypeStruct of int
  | Print
  | Clone
  | Exit
  | Fatal of FatalOp.t
  | ResolveFunc of function_id
  | ResolveMethCaller of function_id
  | ResolveObjMethod
  | ResolveClsMethod of method_id
  | ResolveClsMethodD of class_id * method_id
  | ResolveClsMethodS of SpecialClsRef.t * method_id

type switchkind =
  | Bounded
  | Unbounded

type instruct_control_flow =
  | Jmp of Label.t
  | JmpNS of Label.t
  | JmpZ of Label.t
  | JmpNZ of Label.t
  (* bounded, base, offset vector *)
  | Switch of switchkind * int * Label.t list
  (* litstr id / offset vector *)
  | SSwitch of (string * Label.t) list
  | RetC
  | RetCSuspended
  | RetM of num_params
  | Throw

type instruct_special_flow =
  | Continue of int
  | Break of int
  | Goto of string

type instruct_get =
  | CGetL of local_id
  | CGetQuietL of local_id
  | CGetL2 of local_id
  | CUGetL of local_id
  | PushL of local_id
  | CGetG
  | CGetS
  | ClassGetC
  | ClassGetTS

type istype_op =
  | OpNull
  | OpBool
  | OpInt
  | OpDbl
  | OpStr
  | OpArr
  | OpObj
  | OpRes
  | OpScalar (* Int or Dbl or Str or Bool *)
  | OpKeyset
  | OpDict
  | OpVec
  | OpArrLike (* Arr or Vec or Dict or Keyset *)
  | OpVArray
  | OpDArray
  | OpPHPArr (* Arr *)
  | OpClsMeth
  | OpFunc

type instruct_isset =
  | IssetC
  | IssetL of local_id
  | IsUnsetL of local_id
  | IssetG
  | IssetS
  | IsTypeC of istype_op
  | IsTypeL of local_id * istype_op

type setrange_op =
  | Forward
  | Reverse

type eq_op =
  | PlusEqual
  | MinusEqual
  | MulEqual
  | ConcatEqual
  | DivEqual
  | PowEqual
  | ModEqual
  | AndEqual
  | OrEqual
  | XorEqual
  | SlEqual
  | SrEqual
  | PlusEqualO
  | MinusEqualO
  | MulEqualO

type incdec_op =
  | PreInc
  | PostInc
  | PreDec
  | PostDec
  | PreIncO
  | PostIncO
  | PreDecO
  | PostDecO

type initprop_op =
  | Static
  | NonStatic

type instruct_mutator =
  | SetL of local_id
  (* PopL is put in mutators since it behaves as SetL + PopC *)
  | PopL of Local.t
  | SetG
  | SetS
  | SetOpL of local_id * eq_op
  | SetOpG of eq_op
  | SetOpS of eq_op
  | IncDecL of local_id * incdec_op
  | IncDecG of incdec_op
  | IncDecS of incdec_op
  | UnsetL of local_id
  | UnsetG
  | CheckProp of prop_id
  | InitProp of prop_id * initprop_op

type obj_null_flavor =
  | Obj_null_throws
  | Obj_null_safe

type instruct_call =
  | NewObj
  | NewObjR
  | NewObjD of class_id
  | NewObjRD of class_id
  | NewObjS of SpecialClsRef.t
  | FCall of fcall_args
  | FCallBuiltin of num_params * num_params * num_params * string
  | FCallClsMethod of fcall_args * is_log_as_dynamic_call_op
  | FCallClsMethodD of fcall_args * class_id * method_id
  | FCallClsMethodS of fcall_args * SpecialClsRef.t
  | FCallClsMethodSD of fcall_args * SpecialClsRef.t * method_id
  | FCallCtor of fcall_args
  | FCallFunc of fcall_args
  | FCallFuncD of fcall_args * function_id
  | FCallObjMethod of fcall_args * obj_null_flavor
  | FCallObjMethodD of fcall_args * obj_null_flavor * method_id

type instruct_base =
  | BaseGC of stack_index * MemberOpMode.t
  | BaseGL of local_id * MemberOpMode.t
  | BaseSC of stack_index * stack_index * MemberOpMode.t
  | BaseL of local_id * MemberOpMode.t
  | BaseC of stack_index * MemberOpMode.t
  | BaseH
  | Dim of MemberOpMode.t * MemberKey.t

type instruct_final =
  | QueryM of num_params * QueryOp.t * MemberKey.t
  | SetM of num_params * MemberKey.t
  | IncDecM of num_params * incdec_op * MemberKey.t
  | SetOpM of num_params * eq_op * MemberKey.t
  | UnsetM of num_params * MemberKey.t
  | SetRangeM of num_params * setrange_op * int

type instruct_iterator =
  | IterInit of iter_args * Label.t
  | IterNext of iter_args * Label.t
  | IterFree of Iterator.t

type instruct_include_eval_define =
  | Incl
  | InclOnce
  | Req
  | ReqOnce
  | ReqDoc
  | Eval
  | DefCls of class_num
  | DefClsNop of class_num
  | DefRecord of record_num
  | DefCns of constant_num
  | DefTypeAlias of typedef_num

type bare_this_op =
  | Notice
  | NoNotice
  | NeverNull

type class_kind =
  | KClass
  | KInterface
  | KTrait

type op_silence =
  | Start
  | End

type instruct_misc =
  | This
  | BareThis of bare_this_op
  | CheckThis
  | InitThisLoc of local_id
  | FuncNumArgs
  | ChainFaults
  | OODeclExists of class_kind
  | VerifyParamType of param_id
  | VerifyParamTypeTS of param_id
  | VerifyOutType of param_id
  | VerifyRetTypeC
  | VerifyRetTypeTS
  | Self
  | Parent
  | LateBoundCls
  | ClassName
  | RecordReifiedGeneric
  | CheckReifiedGenericMismatch
  | NativeImpl
  | AKExists
  | CreateCl of num_params * class_num
  | Idx
  | ArrayIdx
  | AssertRATL of local_id * repo_auth_type
  | AssertRATStk of stack_index * repo_auth_type
  | BreakTraceHint
  | Silence of local_id * op_silence
  | GetMemoKeyL of local_id
  | CGetCUNop
  | UGetCUNop
  | MemoGet of Label.t * (local_id * int) option
  | MemoGetEager of Label.t * Label.t * (local_id * int) option
  | MemoSet of (local_id * int) option
  | MemoSetEager of (local_id * int) option
  | LockObj
  | ThrowNonExhaustiveSwitch

type gen_creation_execution =
  | CreateCont
  | ContEnter
  | ContRaise
  | Yield
  | YieldK
  | ContCheck of check_started
  | ContValid
  | ContKey
  | ContGetReturn
  | ContCurrent

type gen_delegation =
  | ContAssignDelegate of Iterator.t
  | ContEnterDelegate
  | YieldFromDelegate of Iterator.t * Label.t
  | ContUnsetDelegate of free_iterator * Iterator.t

type async_functions =
  | WHResult
  | Await
  | AwaitAll of (Local.t * int) option

type instruct_try =
  | TryCatchBegin
  | TryCatchMiddle
  | TryCatchEnd

type srcloc = {
  line_begin: int;
  col_begin: int;
  line_end: int;
  col_end: int;
}

type instruct =
  | IBasic of instruct_basic
  | IIterator of instruct_iterator
  | ILitConst of instruct_lit_const
  | IOp of instruct_operator
  | IContFlow of instruct_control_flow
  | ISpecialFlow of instruct_special_flow
  | ICall of instruct_call
  | IMisc of instruct_misc
  | IGet of instruct_get
  | IMutator of instruct_mutator
  | IIsset of instruct_isset
  | IBase of instruct_base
  | IFinal of instruct_final
  | ILabel of Label.t
  | ITry of instruct_try
  | IComment of string
  | ISrcLoc of srcloc
  | IAsync of async_functions
  | IGenerator of gen_creation_execution
  | IGenDelegation of gen_delegation
  | IIncludeEvalDefine of instruct_include_eval_define
