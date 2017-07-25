(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
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
type classref_id = int
(* Conventionally this is "A_" followed by an integer *)
type adata_id = string

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

(* When using the PassX instructions we need to emit the right kind *)
module PassByRefKind = struct
  type t = AllowCell | WarnOnCell | ErrorOnCell
end

module MemberOpMode = struct

  type t =
  | ModeNone
  | Warn
  | Define
  | Unset

  let to_string op =
  match op with
  | ModeNone -> "None"
  | Warn -> "Warn"
  | Define -> "Define"
  | Unset -> "Unset"

end (* of MemberOpMode *)

module QueryOp = struct
  type t =
  | CGet
  | CGetQuiet
  | Isset
  | Empty

  let to_string op =
  match op with
  | CGet -> "CGet"
  | CGetQuiet -> "CGetQuiet"
  | Isset -> "Isset"
  | Empty -> "Empty"

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
  | ET of Litstr.id
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
  | UnboxR
  | UnboxRNop
  | RGetCNop

type instruct_lit_const =
  | NYI of string
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
  | NewMIArray of int (* capacity hint *)
  | NewMSArray of int (* capacity hint *)
  | NewLikeArrayL of local_id * int (* capacity hint *)
  | NewPackedArray of int
  | NewStructArray of Litstr.id list
  | NewVecArray of int
  | NewKeysetArray of int
  | NewPair
  | AddElemC
  | AddElemV
  | AddNewElemC
  | AddNewElemV
  | NewCol of CollectionType.t
  | ColFromArray of CollectionType.t
  | MapAddElemC
  | Cns of const_id
  | CnsE of const_id
  | CnsU of const_id * Litstr.id
  | ClsCns of const_id * classref_id
  | ClsCnsD of const_id * class_id
  | File
  | Dir
  | Method
  | NameA

type instruct_operator =
  | Concat
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
  | InstanceOf
  | InstanceOfD of class_id
  | Print
  | Clone
  | Exit
  | Fatal of FatalOp.t

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
  | SSwitch of (Litstr.id * Label.t) list
  | RetC
  | RetV
  | Unwind
  | Throw

type iterator_list = (bool * Iterator.t) list

type instruct_special_flow =
  | Continue of int
  | Break of int

type instruct_get =
  | CGetL of local_id
  | CGetQuietL of local_id
  | CGetL2 of local_id
  | CGetL3 of local_id
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
  | OpScalar (* Int or Dbl or Str or Bool *)
  | OpKeyset
  | OpDict
  | OpVec

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
  | FPushFunc of num_params
  | FPushFuncD of num_params * function_id
  | FPushFuncU of num_params * function_id * Litstr.id
  | FPushObjMethod of num_params * Ast.og_null_flavor
  | FPushObjMethodD of num_params * method_id * Ast.og_null_flavor
  | FPushClsMethod of num_params * classref_id
  | FPushClsMethodF of num_params * classref_id
  | FPushClsMethodD of num_params * method_id * class_id
  | FPushCtor of num_params * classref_id
  | FPushCtorD of num_params * class_id
  | FPushCtorI of num_params * classref_id
  | DecodeCufIter of num_params * Label.t
  | FPushCufIter of num_params * Iterator.t
  | FPushCuf of num_params
  | FPushCufF of num_params
  | FPushCufSafe of num_params
  | CufSafeArray
  | CufSafeReturn
  | FPassC of param_num
  | FPassCW of param_num
  | FPassCE of param_num
  | FPassV of param_num
  | FPassVNop of param_num
  | FPassR of param_num
  | FPassL of param_num * local_id
  | FPassN of param_num
  | FPassG of param_num
  | FPassS of param_num * classref_id
  | FCall of num_params
  | FCallD of num_params * class_id * function_id
  | FCallArray
  | FCallAwait of num_params * class_id * function_id
  | FCallUnpack of num_params
  | FCallBuiltin of num_params * num_params * Litstr.id

type instruct_base =
  | BaseNC of stack_index * MemberOpMode.t
  | BaseNL of local_id * MemberOpMode.t
  | FPassBaseNC of param_num * stack_index
  | FPassBaseNL of param_num * local_id
  | BaseGC of stack_index * MemberOpMode.t
  | BaseGL of local_id * MemberOpMode.t
  | FPassBaseGC of param_num * stack_index
  | FPassBaseGL of param_num * local_id
  | BaseSC of stack_index * classref_id
  | BaseSL of local_id * stack_index
  | BaseL of local_id * MemberOpMode.t
  | FPassBaseL of param_num * local_id
  | BaseC of stack_index
  | BaseR of stack_index
  | BaseH
  | Dim of MemberOpMode.t * MemberKey.t
  | FPassDim of param_num * MemberKey.t

type instruct_final =
  | QueryM of num_params * QueryOp.t * MemberKey.t
  | VGetM of num_params * MemberKey.t
  | FPassM of param_num * num_params * MemberKey.t
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
  | WIterInit of Iterator.t * Label.t * local_id
  | WIterInitK of Iterator.t * Label.t * local_id * local_id
  | MIterInit of Iterator.t * Label.t * local_id
  | MIterInitK of Iterator.t * Label.t * local_id * local_id
  | IterNext of Iterator.t * Label.t * local_id
  | IterNextK of Iterator.t * Label.t * local_id * local_id
  | WIterNext of Iterator.t * Label.t * local_id
  | WIterNextK of Iterator.t * Label.t * local_id * local_id
  | MIterNext of Iterator.t * Label.t * local_id
  | MIterNextK of Iterator.t * Label.t * local_id * local_id
  | IterFree of Iterator.t
  | MIterFree of Iterator.t
  | CIterFree of Iterator.t
  | IterBreak of Label.t * (bool * Iterator.t) list

type instruct_include_eval_define =
  | Incl
  | InclOnce
  | Req
  | ReqOnce
  | ReqDoc
  | Eval
  | AliasCls of Litstr.id * Litstr.id
  | DefFunc of function_num
  | DefCls of class_num
  | DefClsNop of class_num
  | DefCns of const_id
  | DefTypeAlias of typedef_num

type bare_this_op =
  | Notice
  | NoNotice

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
  | StaticLocCheck of local_id * Litstr.id
  | StaticLocDef of local_id * Litstr.id
  | StaticLocInit of local_id * Litstr.id
  | Catch
  | OODeclExists of class_kind
  | VerifyParamType of param_id
  | VerifyRetTypeC
  | VerifyRetTypeV
  | Self of classref_id
  | Parent of classref_id
  | LateBoundCls of classref_id
  | ClsRefName of classref_id
  | NativeImpl
  | IncStat of int * int (* counter id, value *)
  | AKExists
  | CreateCl of num_params * class_num
  | Idx
  | ArrayIdx
  | AssertRATL of local_id * repo_auth_type
  | AssertRATStk of stack_index * repo_auth_type
  | BreakTraceHint
  | Silence of local_id * op_silence
  | GetMemoKeyL of local_id
  | VarEnvDynCall
  | IsUninit
  | CGetCUNop
  | UGetCUNop
  | MemoSet of int * local_id * int
  | MemoGet of int * local_id * int
  | IsMemoType
  | MaybeMemoType

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

type gen_delegation =
  | ContAssignDelegate of Iterator.t
  | ContEnterDelegate
  | YieldFromDelegate of Iterator.t * Label.t
  | ContUnsetDelegate of free_iterator * Iterator.t

type async_functions =
  | WHResult
  | Await

type instruct_try =
  | TryCatchBegin
  | TryCatchMiddle
  | TryCatchEnd
  | TryCatchLegacyBegin of Label.t
  | TryCatchLegacyEnd
  | TryFaultBegin of Label.t
  | TryFaultEnd

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
  | IAsync of async_functions
  | IGenerator of gen_creation_execution
  | IGenDelegation of gen_delegation
  | IIncludeEvalDefine of instruct_include_eval_define
