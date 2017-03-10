(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

(**
 * TODO (hgo): see within HHVM codebase what those types actually are *)
type property_name = string
type iter_vec = int
type check_started = bool
type free_iterator = int
type repo_auth_type = string (* see see runtime/base/repo-auth-type.h *)
type local_id = Local.t

type param_id =
  | Param_unnamed of int
  | Param_named of string
type param_num = int
type stack_index = int
type class_id = string
type function_id = string
type num_params = int

type collection_type = int

(* These are the three flavors of value that can live on the stack:
 *   C = cell
 *   R = ref
 *   A = classref
 *)
module Flavor = struct
  type t = Cell | Ref | Classref
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
  | Isset
  | Empty

  let to_string op =
  match op with
  | CGet -> "CGet"
  | Isset -> "Isset"
  | Empty -> "Empty"

end (* of QueryOp *)

module MemberKey = struct
  type t =
  | EC of stack_index
  | EL of local_id
  | ET of Litstr.id
  | EI of int64
  | PC of stack_index
  | PL of local_id
  | PT of Litstr.id
  | QT of Litstr.id
  | W
end (* Of MemberKey *)

type instruct_basic =
  | Nop
  | EntryNop
  | PopA
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
  | Null
  | True
  | False
  | NullUninit
  | Int of int64
  | Double of Litstr.id
  | String of Litstr.id
  | Array of int * instruct_lit_const list
  | Vec of int * instruct_lit_const list
  | Dict of int * instruct_lit_const list
  | Keyset of int * instruct_lit_const list
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
  | AddElemC
  | AddElemV
  | AddNewElemC
  | AddNewElemV
  | NewCol of collection_type
  | ColFromArray of collection_type
  | MapAddElemC
  | ColAddNewElemC
  | Cns of Litstr.id
  | CnsE of Litstr.id
  | CnsU of int * Litstr.id (* litstr fallback *)
  | ClsCns of Litstr.id
  | ClsCnssD of Litstr.id * Litstr.id
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
  | InstanceOfD of Litstr.id
  | Print
  | Clone
  | Exit
  | Fatal

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

type instruct_special_flow =
  | Continue of int * int  (* This will be rewritten *)
  | Break of int * int  (* This will be rewritten *)

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
  | CGetS
  | VGetN
  | VGetG
  | VGetS
  | AGetC
  | AGetL of local_id

type istype_op =
  | OpNull
  | OpBool
  | OpInt
  | OpDbl
  | OpStr
  | OpArr
  | OpObj
  | OpScalar (* Int or Dbl or Str or Bool *)

type instruct_isset =
  | IssetC
  | IssetL of local_id
  | IssetN
  | IssetG
  | IssetS
  | EmptyL of local_id
  | EmptyN
  | EmptyG
  | EmptyS
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
  | SetS
  | SetOpL of local_id * eq_op
  | SetOpN of eq_op
  | SetOpG of eq_op
  | SetOpS of eq_op
  | IncDecL of local_id * incdec_op
  | IncDecN of incdec_op
  | IncDecG of incdec_op
  | IncDecS of incdec_op
  | BindL of local_id
  | BindN
  | BindG
  | BindS
  | UnsetL of local_id
  | UnsetN
  | UnsetG
  | CheckProp of property_name
  | InitProp of property_name * initprop_op

type instruct_call =
  | FPushFunc of num_params
  | FPushFuncD of num_params * Litstr.id
  | FPushFuncU of num_params * Litstr.id * Litstr.id
  | FPushObjMethod of num_params
  | FPushObjMethodD of num_params * Litstr.id * Ast.og_null_flavor
  | FPushClsMethod of num_params
  | FPushClsMethodF of num_params
  | FPushClsMethodD of num_params * Litstr.id * Litstr.id
  | FPushCtor of num_params
  | FPushCtorD of num_params * Litstr.id
  | FPushCtorI of num_params * class_id
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
  | FPassS of param_num
  | FCall of num_params
  | FCallD of num_params * class_id * function_id
  | FCallArray
  | FCallAwait of num_params * class_id * function_id
  | FCallUnpack of num_params
  | FCallBuiltin of num_params * num_params * Litstr.id

type op_member_intermediate =
  | ElemC
  | ElemL of local_id
  | ElemCW
  | ElemLW of local_id
  | ElemCD
  | ElemLD of local_id
  | ElemCU
  | ElemLU of local_id
  | NewElem
  | PropC
  | PropL of local_id
  | PropCW
  | PropLW of local_id
  | PropCD
  | PropLD of local_id
  | PropCU
  | PropLU of local_id

type op_member_final =
  | CGutElemC
  | CGetElemL of local_id
  | VGetElemC
  | VGetElemL of local_id
  | IssetElemC
  | IssetElemL of local_id
  | EmptyElemC
  | EmptyElemL of local_id
  | SetElemC
  | SetElemL of local_id
  | SetOpElemC of eq_op
  | SetOpElemL of eq_op * local_id
  | IncDecElemC of incdec_op
  | IncDecElemL of incdec_op * local_id
  | BindElemC
  | BindElemL of local_id
  | UnsetElemC
  | UnsetElemL of local_id
  | VGetNewElem
  | SetNewElem
  | SetOpNewElem of eq_op
  | IncDecNewElem of incdec_op
  | BindNewElem
  | CGetPropC
  | CGetPropL of local_id
  | VGetPropC
  | VGetPropL of local_id
  | IssetPropC
  | IssetPropL of local_id
  | EmptyPropC
  | EmptyPropL of local_id
  | SetPropC
  | SetPropL of local_id
  | SetOpPropC of eq_op
  | SetOpPropL of eq_op * local_id
  | IncDecPropC of incdec_op
  | IncDecPropL of incdec_op * local_id
  | BindPropC
  | BindPropL of local_id
  | UnsetPropC
  | UnsetPropL of local_id

type instruct_base =
  | BaseNC of stack_index * MemberOpMode.t
  | BaseNL of local_id * MemberOpMode.t
  | FPassBaseNC of param_num * stack_index
  | FPassBaseNL of param_num * local_id
  | BaseGC of stack_index * MemberOpMode.t
  | BaseGL of local_id * MemberOpMode.t
  | FPassBaseGC of param_num * stack_index
  | FPassBaseGL of param_num * local_id
  | BaseSC of stack_index * stack_index
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
  | IterBreak of Label.t * iter_vec

type instruct_include_eval_define =
  | Incl
  | InclOnce
  | Req
  | ReqOnce
  | ReqDoc
  | Eval
  | DefFunc of function_id
  | DefCls of class_id
  | DefClsNop of class_id
  | DefCns of Litstr.id
  | DefTypeAlias of Litstr.id

type bare_this_op =
  | Notice
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
  | StaticLoc of local_id * Litstr.id
  | StaticLocInit of local_id * Litstr.id
  | Catch
  | OODeclExists of class_kind
  | VerifyParamType of param_id
  | VerifyRetTypeC
  | VerifyRetTypeV
  | Self
  | Parent
  | LateBoundCls
  | NativeImpl
  | IncStat of int * int (* counter id, value *)
  | AKExists
  | CreateCl of num_params * class_id
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
  | ConStarted
  | ContKey
  | ContGetReturn

type gen_delegation =
  | ContAssignDelegate
  | ContEnterDelegate
  | YieldFromDelegate
  | ContUnsetDelegate of free_iterator

type async_functions =
  | WHResult
  | Await

type instruct_try =
  | TryCatchBegin of Label.t
  | TryCatchEnd
  | TryFaultBegin of Label.t
  | TryFaultEnd

and instruct =
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
