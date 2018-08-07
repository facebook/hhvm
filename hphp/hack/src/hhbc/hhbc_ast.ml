(**
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
type function_num = int
type typedef_num = int
type function_id = Hhbc_id.Function.t
type method_id = Hhbc_id.Method.t
type const_id = Hhbc_id.Const.t
type prop_id = Hhbc_id.Prop.t
type num_params = int
type has_unpack = bool
type classref_id = int
(* Conventionally this is "A_" followed by an integer *)
type adata_id = string
type param_locations = int list

(* These are the three flavors of value that can live on the stack:
 *   C = cell
 *   V = ref
 *   R = return value
 *   F = function argument
 *   U = uninit
 * Note: Function argument and uninit are not added to the type
 *       as they are handled separately
 *)
module Flavor = struct
  type t = Cell | Ref | ReturnVal
end

(* Indicates whether a call site was annotated as pass-by-ref or not *)
type fpass_hint =
  | Any
  | Cell
  | Ref

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

end (* of SpecialClsRef *)

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

end (* of MemberOpMode *)

module QueryOp = struct
  type t =
  | CGet
  | CGetQuiet
  | Isset
  | Empty
  | InOut

  let to_string op =
  match op with
  | CGet -> "CGet"
  | CGetQuiet -> "CGetQuiet"
  | Isset -> "Isset"
  | Empty -> "Empty"
  | InOut -> "InOut"

end (* of QueryOp *)

module CollectionType = struct
  type t =
  | Vector
  | Map
  | Set
  | Pair
  | ImmVector
  | ImmMap
  | ImmSet

  let to_string = function
  | Vector      -> "Vector"
  | Map         -> "Map"
  | Set         -> "Set"
  | Pair        -> "Pair"
  | ImmVector   -> "ImmVector"
  | ImmMap      -> "ImmMap"
  | ImmSet      -> "ImmSet"

end (* of CollectionType *)

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

end (* of FatalOp *)

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
end (* Of MemberKey *)

type instruct_basic =
  | Nop
  | EntryNop
  | PopC
  | PopV
  | PopR
  | PopU
  | Dup
  | Box
  | Unbox
  | BoxR
  | BoxRNop
  | UnboxR
  | UnboxRNop
  | RGetCNop

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
  | AddElemC
  | AddElemV
  | AddNewElemC
  | AddNewElemV
  | NewCol of CollectionType.t
  | ColFromArray of CollectionType.t
  | Cns of const_id
  | CnsE of const_id
  | CnsU of const_id * string
  | ClsCns of const_id * classref_id
  | ClsCnsD of const_id * class_id
  | File
  | Dir
  | Method

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
  | CastObject
  | CastVec
  | CastDict
  | CastKeyset
  | CastVArray
  | CastDArray
  | InstanceOf
  | InstanceOfD of class_id
  | IsTypeStruct of adata_id
  | AsTypeStruct of adata_id
  | Print
  | Clone
  | Exit
  | Fatal of FatalOp.t
  | ResolveFunc of function_id
  | ResolveObjMethod
  | ResolveClsMethod

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
  | RetM of num_params
  | RetV
  | Unwind
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
  | CGetN
  | CGetQuietN
  | CGetG
  | CGetQuietG
  | CGetS of classref_id
  | VGetL of local_id
  | VGetN
  | VGetG
  | VGetS of classref_id
  | ClsRefGetL of local_id * classref_id
  | ClsRefGetC of classref_id

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

type instruct_isset =
  | IssetC
  | IssetL of local_id
  | IssetN
  | IssetG
  | IssetS of classref_id
  | EmptyL of local_id
  | EmptyN
  | EmptyG
  | EmptyS of classref_id
  | IsTypeC of istype_op
  | IsTypeL of local_id * istype_op

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
  | SetN
  | SetG
  | SetS of classref_id
  | SetOpL of local_id * eq_op
  | SetOpN of eq_op
  | SetOpG of eq_op
  | SetOpS of eq_op * classref_id
  | IncDecL of local_id * incdec_op
  | IncDecN of incdec_op
  | IncDecG of incdec_op
  | IncDecS of incdec_op * classref_id
  | BindL of local_id
  | BindN
  | BindG
  | BindS of classref_id
  | UnsetL of local_id
  | UnsetN
  | UnsetG
  | CheckProp of prop_id
  | InitProp of prop_id * initprop_op

type instruct_call =
  | FPushFunc of num_params * param_locations
  | FPushFuncD of num_params * function_id
  | FPushFuncU of num_params * function_id * string
  | FPushObjMethod of num_params * Ast.og_null_flavor * param_locations
  | FPushObjMethodD of num_params * method_id * Ast.og_null_flavor
  | FPushClsMethod of num_params * classref_id * param_locations
  | FPushClsMethodD of num_params * method_id * class_id
  | FPushClsMethodS of num_params * SpecialClsRef.t
  | FPushClsMethodSD of num_params * SpecialClsRef.t * method_id
  | FPushCtor of num_params * classref_id
  | FPushCtorD of num_params * class_id
  | FPushCtorI of num_params * classref_id
  | FPushCtorS of num_params * SpecialClsRef.t
  | DecodeCufIter of Iterator.t * Label.t
  | FPushCufIter of num_params * Iterator.t
  | FIsParamByRef of param_num * fpass_hint
  | FIsParamByRefCufIter of param_num * fpass_hint * Iterator.t
  | FThrowOnRefMismatch of bool list
  | FHandleRefMismatch of param_num * fpass_hint * string
  | FCall of num_params * has_unpack * num_params * class_id * function_id
  | FCallAwait of num_params * class_id * function_id
  | FCallBuiltin of num_params * num_params * string

type instruct_base =
  | BaseNC of stack_index * MemberOpMode.t
  | BaseNL of local_id * MemberOpMode.t
  | BaseGC of stack_index * MemberOpMode.t
  | BaseGL of local_id * MemberOpMode.t
  | BaseSC of stack_index * classref_id * MemberOpMode.t
  | BaseSL of local_id * stack_index * MemberOpMode.t
  | BaseL of local_id * MemberOpMode.t
  | BaseC of stack_index * MemberOpMode.t
  | BaseR of stack_index * MemberOpMode.t
  | BaseH
  | Dim of MemberOpMode.t * MemberKey.t

type instruct_final =
  | QueryM of num_params * QueryOp.t * MemberKey.t
  | VGetM of num_params * MemberKey.t
  | SetM of num_params * MemberKey.t
  | IncDecM of num_params * incdec_op * MemberKey.t
  | SetOpM of num_params  * eq_op * MemberKey.t
  | BindM of num_params * MemberKey.t
  | UnsetM of num_params * MemberKey.t
  | SetWithRefLML of local_id * local_id
  | SetWithRefRML of local_id

type instruct_iterator =
  | IterInit of Iterator.t * Label.t * local_id
  | IterInitK of Iterator.t * Label.t * local_id * local_id
  | LIterInit of Iterator.t * local_id * Label.t * local_id
  | LIterInitK of Iterator.t * local_id * Label.t * local_id * local_id
  | WIterInit of Iterator.t * Label.t * local_id
  | WIterInitK of Iterator.t * Label.t * local_id * local_id
  | MIterInit of Iterator.t * Label.t * local_id
  | MIterInitK of Iterator.t * Label.t * local_id * local_id
  | IterNext of Iterator.t * Label.t * local_id
  | IterNextK of Iterator.t * Label.t * local_id * local_id
  | LIterNext of Iterator.t * local_id * Label.t * local_id
  | LIterNextK of Iterator.t * local_id * Label.t * local_id * local_id
  | WIterNext of Iterator.t * Label.t * local_id
  | WIterNextK of Iterator.t * Label.t * local_id * local_id
  | MIterNext of Iterator.t * Label.t * local_id
  | MIterNextK of Iterator.t * Label.t * local_id * local_id
  | IterFree of Iterator.t
  | MIterFree of Iterator.t
  | CIterFree of Iterator.t
  | LIterFree of Iterator.t * local_id
  | IterBreak of Label.t * (bool * Iterator.t) list

type instruct_include_eval_define =
  | Incl
  | InclOnce
  | Req
  | ReqOnce
  | ReqDoc
  | Eval
  | AliasCls of string * string
  | DefFunc of function_num
  | DefCls of class_num
  | DefClsNop of class_num
  | DefCns of const_id
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
  | StaticLocCheck of local_id * string
  | StaticLocDef of local_id * string
  | StaticLocInit of local_id * string
  | Catch
  | ChainFaults
  | OODeclExists of class_kind
  | VerifyParamType of param_id
  | VerifyOutType of param_id
  | VerifyRetTypeC
  | VerifyRetTypeV
  | Self of classref_id
  | Parent of classref_id
  | LateBoundCls of classref_id
  | ClsRefName of classref_id
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
  | MemoSet of (local_id * int) option

type gen_creation_execution =
  | CreateCont
  | ContEnter
  | ContRaise
  | Yield
  | YieldK
  | ContCheck of check_started
  | ContValid
  | ContStarted
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
  | TryCatchLegacyBegin of Label.t
  | TryCatchLegacyEnd
  | TryFaultBegin of Label.t
  | TryFaultEnd

type srcloc = { line_begin:int; col_begin:int; line_end:int; col_end:int }
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
