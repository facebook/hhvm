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
type collection_type = string
type rel_offset = int
type property_name = string
type member_op_mode = int
type query_op = int
type member_key = int
type iter_vec = int
type check_started = bool
type free_iterator = int
type repo_auth_type = string (* see see runtime/base/repo-auth-type.h *)


type instruct_basic =
  | Nop
  | EntryNop
  | PopA
  | PopC
  | PopV
  | PopR
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
  | Double of float
  | String of int
  | Array of int
  | Vec of int (* scalar vec id *)
  | Dict of int (* scalar dict id *)
  | Keyset of int (* scalar keyset id *)
  | NewArray of int (* capacity hint *)
  | NewMixedArray of int (* capacity hint *)
  | NewDictArray of int (* capacity hint *)
  | NewMIArray of int (* capacity hint *)
  | NewMSArray of int (* capacity hint *)
  | NewLikeArrayL of int (* local variable id *) * int (* capacity hint *)
  | NewPackedArray of int
  | NewStructArray of int list
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
  | Cns of int (* litstr id *)
  | CnsE of int (* litstr id *)
  | CnsU of int * int (* litstr fallback *) (* litstr id *)
  | ClsCns of int (* litstr id *)
  | ClsCnssD of int * int (* litstr id, litstr id *)
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
  | CostDouble
  | CastString
  | CostArray
  | CastObject
  | CastVec
  | CastDict
  | CastKeyset
  | InstanceOf
  | InstanceOfD
  | Print
  | Clone
  | Exit
  | Fatal

type switchkind =
  | Bounded
  | Unbounded

type instruct_control_flow =
  | Jmp of rel_offset
  | JmpNS of rel_offset
  | JmpZ of rel_offset
  | JmpNZ of rel_offset
  | Switch of switchkind * int * int (* bounded, base, offset vector *)
  | SSwitch of int (* litstr id / offset vector *)
  | RetC
  | RetV
  | Unwind
  | Throw

type instuct_get =
  | CGetL of int (* local variable id *)
  | CGetQuietL of int (* local variable id *)
  | CGetL2 of int (* local variable id *)
  | CGetL3 of int (* local variable id *)
  | CUGetL of int (* local variable id *)
  | PushL of int (* local variable id *)
  | CGetN
  | CGetQuietN
  | CGetG
  | CGetQuietG
  | CGetS
  | VGetN
  | VGetG
  | VGetS
  | AGetC
  | AGetL of int (* local variable id *)

type operand =
  | OpNull
  | OpBool
  | OpInt
  | OpDbl
  | OpStr
  | OpArr
  | OpObj
  | OpScalar (* Int or Dbl or Str or Bool *)

type instuct_isset_emty_type_querying =
  | IssetC
  | IssetL of int (* local variable id *)
  | IssetN
  | IssetG
  | IssetS
  | EmptyL of int (* local variable id *)
  | EmptyN
  | EmptyG
  | EmptyS
  | IsTypeC of operand
  | IsTypeL of int * operand (* local variable id, operand *)

type instruct_mutator =
  | SetL of int (* local variable id *)
  | SetN
  | SetG
  | SetS
  | SetOpL of int * operand (* local variable id, operand *)
  | SetOpN of operand
  | SetOpG of operand
  | SetOpS of operand
  | IncDecl of int * operand (* local variable id, operand *)
  | IncDecN of operand
  | IncDecG of operand
  | IncDecS of operand
  | BindL of int (* local variable id *)
  | BindN
  | BindG
  | BindS
  | UnsetN
  | UnsetG
  | CheckProp of property_name
  | InitProp of property_name * operand

type instruct_call =
  | FPushFunc of int (* num params *)
  | FPushFuncD of int * int (* num params, litstr id *)
  | FPushFuncU of int * int * int (* num params, litstr id, litstr fallback, litstr fallback *)
  | FPushObjMethod of int (* num params *)
  | FPushObjMethodD of int * int (* num params, litstr id *)
  | FPushClsMethod of int (* num params *)
  | FPushClsMethodF of int (* num params *)
  | FPushClsMethodD of int * int * int (* num params, litstr id, litstr id *)
  | FPushCtor of int (* num params *)
  | FPushCtorD of int * int (* num params, litstr id *)
  | FpushCtorI of int * int (* num params, class id *)
  | DecodeCufIter of int * rel_offset (* iterator id, rel offset *)
  | FPushCufIter of int * int (* num params, iterator id *)
  | FPushCuf of int (* num params *)
  | FPushCufF of int (* num params *)
  | FPushCufSafe of int (* num params *)
  | CufSafeArray
  | CufSafeReturn
  | FPassC of int (* param id *)
  | FPassCW of int (* param id *)
  | FPassCE of int (* param id *)
  | FPassV of int (* param id *)
  | FPassVNop of int (* param id *)
  | FPassL of int * int (* param id, local variable id *)
  | FPassN of int (* param id *)
  | FPassG of int (* param id *)
  | FPassS of int (* param id *)
  | FCall of int (* param id *)
  | FCallD of int * int * int (* num params, class name id, function name id *)
  | FCallArray
  | FCallAwait of int * int * int (* num params, class name id, function name id *)
  | FCallUnpack of int (* num params *)
  | FCallBuiltin of int * int * int (* total params, passed, litstr id *)

type op_member_base =
  | BaseC
  | BaseR
  | BaseL of int (* local variable id *)
  | BaseLW of int (* local variable id *)
  | BaseLD of int (* local variable id *)
  | BaseNC
  | BaseNL of int (* local variable id *)
  | BaseNCW
  | BaseNLW of int (* local variable id *)
  | BaseNCD
  | BaseNLD of int (* local variable id *)
  | BaseGC
  | BaseGL of int (* local variable id *)
  | BaseGCW
  | BaseGLW of int (* local variable id *)
  | BaseGCD
  | BaseGLD of int (* local variable id *)
  | BaseSC
  | BaseSL of int (* local variable id *)
  | BaseH

type op_member_intermediate =
  | ElemC
  | ElemL of int (* local variable id *)
  | ElemCW
  | ElemLW of int (* local variable id *)
  | ElemCD
  | ElemLD of int (* local variable id *)
  | ElemCU
  | ElemLU of int (* local variable id *)
  | NewElem
  | PropC
  | PropL of int (* local variable id *)
  | PropCW
  | PropLW of int (* local variable id *)
  | PropCD
  | PropLD of int (* local variable id *)
  | PropCU
  | PropLU of int (* local variable id *)

type op_member_final =
  | CGutElemC
  | CGetElemL of int (* local variable id *)
  | VGetElemC
  | VGetElemL of int (* local variable id *)
  | IssetElemC
  | IssetElemL of int (* local variable id *)
  | EmptyElemC
  | EmptyElemL of int (* local variable id *)
  | SetElemC
  | SetElemL of int (* local variable id *)
  | SetOpElemC of operand
  | SetOpElemL of operand * int (* operand, local variable id *)
  | IncDecElemC of operand
  | IncDecElemL of operand * int (* operand, local variable id *)
  | BindElemC
  | BindElemL of int (* local variable id *)
  | UnsetElemC
  | UnsetElemL of int (* local variable id *)
  | VGetNewElem
  | SetNewElem
  | SetOpNewElem of operand
  | IncDecNewElem of operand
  | BindNewElem
  | CGetPropC
  | CGetPropL of int (* local variable id *)
  | VGetPropC
  | VGetPropL of int (* local variable id *)
  | IssetPropC
  | IssetPropL of int (* local variable id *)
  | EmptyPropC
  | EmptyPropL of int (* local variable id *)
  | SetPropC
  | SetPropL of int (* local variable id *)
  | SetOpPropC of operand
  | SetOpPropL of operand * int (* operand, local variable id *)
  | IncDecPropC of operand
  | IncDecPropL of operand * int (* operand, local variable id *)
  | BindPropC
  | BindPropL of int (* local variable id *)
  | UnsetPropC
  | UnsetPropL of int (* local variable id *)

type op_base =
  | BaseNC of int * member_op_mode (* stack index, member op index *)
  | BaseNL of int * member_op_mode (* local id, member op index *)
  | FPassBaseNC of int * int (* param id, stack index *)
  | FPassBaseNL of int * int (* param id, local id *)
  | BaseGC of int * member_op_mode (* stack index, member op mode *)
  | BaseGL of int * member_op_mode (* local id, member op mode *)
  | FPassBaseGC of int * int (* param id, stack index *)
  | FPassBaseGL of int * int (* param id, local id *)
  | BaseSC of int * int (* stack index, stack index *)
  | BaseSL of int * int (* local id, stack index *)
  | BaseL of int * member_op_mode (* local id, member op mode *)
  | FPassBaseL of int * int (* param id, local id *)
  | BaseC of int (* stack index *)
  | BaseR of int (* stack index *)
  | BaseH

type op_final =
  | QueryM of int * query_op * member_key (* stack count, query op, member key *)
  | VGetM of int * member_key (* stack count, member key *)
  | FPassM of int * int * member_key (* param id, stack count, member key *)
  | SetM of int * member_key (* stack count, member key *)
  | IncDecM of int * operand * member_key (* statuc count, operand, member key *)
  | SetOpM of int  * operand * member_key (* stack caunt, operand, member key *)
  | BindM of int * member_key (* stack count, member key *)
  | UnsetM of int * member_key (* stack count, member key *)
  | SetWithRefLML of int * int (* local id, local id *)
  | SetWithRefRML of int (* local id *)

type instruct_iterator =
  | IterInit of int * rel_offset * int (* iterator id, rel offset, local id *)
  | IterInitK of int * rel_offset * int * int (* iterator id, rel offset, local id, local id *)
  | WIterInit of int * rel_offset * int (* iterator id, rel offset, local id *)
  | WIterInitK of int * rel_offset * int * int (* iterator id, rel offset, local id, local id *)
  | MIterInit of int * rel_offset * int (* iterator id, rel offset, local id *)
  | MIterInitK of int * rel_offset * int * int (* iterator id, rel offset, local id, local id *)
  | IterNext of int * rel_offset * int (* iterator id, rel offset, local id *)
  | IterNextK of int * rel_offset * int * int (* iterator id, rel offset, local id, local id *)
  | WIterNext of int * rel_offset * int (* iterator id, rel offset, local id *)
  | WIterNextK of int * rel_offset * int * int (* iterator id, rel offset, local id, local id *)
  | MIterNext of int * rel_offset * int (* iterator id, rel offset, local id *)
  | MIterNextK of int * rel_offset * int * int (* iterator id, rel offset, local id, local id *)
  | IterFree of int (* iterator id *)
  | MIterFree of int (* iterator id *)
  | CIterFree of int (* iterator id *)
  | IterBreak of rel_offset * iter_vec

type instruct_include_eval_define =
  | Incl
  | InclOnce
  | Req
  | ReqOnce
  | ReqDoc
  | Eval
  | DefFunc of int (* function id *)
  | DefCls of int (* class id *)
  | DefClsNop of int (* class id *)
  | DefCns of int (* litstr id *)
  | DefTypeAlias of int (* litstr id *)

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
  | InitThisLoc of int (* local variable id *)
  | StaticLoc of int * int (* local variable id, litstr id *)
  | StaticLocInit of int * int (* local variable id, litstr id *)
  | Catch
  | OODeclExists of class_kind
  | VerifyParamType of int (* parameter id *)
  | VerifyRetTypeC
  | VerifyRetTypeV
  | Self
  | Parent
  | IncStat of int * int (* counter id, value *)
  | AKExists
  | CreateCl of int * int (* num args * class id *)
  | Idx
  | ArrayIdx
  | AssertRATL of int * repo_auth_type (* local id, repo auth type *)
  | AssertRATStk of int * repo_auth_type (* stack offset, repo auth type *)
  | BreakTraceHint
  | Silence of int * op_silence (* local id, Start|End *)
  | GetMemoKey
  | VarEnvDynCall

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

type gen_delegtion =
  | ContAssignDelegate
  | ContEnterDelegate
  | YieldFromDelegate
  | ContUnsetDelegate of free_iterator

type async_functions =
  | WHResult
  | Await

type instruct =
  | IBasic of instruct_basic
  | ILitConst of instruct_lit_const
  | IOp of instruct_operator
  | IContFlow of instruct_control_flow
  | ICall of instruct_call

type fun_def = {
  fun_name    : int;
  fun_litstr  : (int, string) Hashtbl.t;
  fun_body    : instruct list;
}

type hhas_prog = {
  hhas_fun: fun_def list;
  hhas_litstr: (int, string) Hashtbl.t;
}
