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
type local_id = int
type param_id = int
type iterator_id = int
type stack_index = int
type class_id = string
type function_id = string
type num_params = int

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
  | String of Litstr.id
  | Array of int
  | Vec of int (* scalar vec id *)
  | Dict of int (* scalar dict id *)
  | Keyset of int (* scalar keyset id *)
  | NewArray of int (* capacity hint *)
  | NewMixedArray of int (* capacity hint *)
  | NewDictArray of int (* capacity hint *)
  | NewMIArray of int (* capacity hint *)
  | NewMSArray of int (* capacity hint *)
  | NewLikeArrayL of local_id * int (* capacity hint *)
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
  | Switch of switchkind * int * int list (* bounded, base, offset vector *)
  | SSwitch of (Litstr.id * rel_offset) list (* litstr id / offset vector *)
  | RetC
  | RetV
  | Unwind
  | Throw

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
  | IssetL of local_id
  | IssetN
  | IssetG
  | IssetS
  | EmptyL of local_id
  | EmptyN
  | EmptyG
  | EmptyS
  | IsTypeC of operand
  | IsTypeL of local_id * operand

type instruct_mutator =
  | SetL of local_id
  | SetN
  | SetG
  | SetS
  | SetOpL of local_id * operand
  | SetOpN of operand
  | SetOpG of operand
  | SetOpS of operand
  | IncDecl of local_id * operand
  | IncDecN of operand
  | IncDecG of operand
  | IncDecS of operand
  | BindL of local_id
  | BindN
  | BindG
  | BindS
  | UnsetN
  | UnsetG
  | CheckProp of property_name
  | InitProp of property_name * operand

type instruct_call =
  | FPushFunc of num_params
  | FPushFuncD of num_params * Litstr.id
  | FPushFuncU of num_params * Litstr.id * Litstr.id
  | FPushObjMethod of num_params
  | FPushObjMethodD of num_params * Litstr.id
  | FPushClsMethod of num_params
  | FPushClsMethodF of num_params
  | FPushClsMethodD of num_params * Litstr.id * Litstr.id
  | FPushCtor of num_params
  | FPushCtorD of num_params * Litstr.id
  | FpushCtorI of num_params * class_id
  | DecodeCufIter of num_params * rel_offset
  | FPushCufIter of num_params * iterator_id
  | FPushCuf of num_params
  | FPushCufF of num_params
  | FPushCufSafe of num_params
  | CufSafeArray
  | CufSafeReturn
  | FPassC of param_id
  | FPassCW of param_id
  | FPassCE of param_id
  | FPassV of param_id
  | FPassVNop of param_id
  | FPassL of param_id * local_id
  | FPassN of param_id
  | FPassG of param_id
  | FPassS of param_id
  | FCall of param_id
  | FCallD of num_params * class_id * function_id
  | FCallArray
  | FCallAwait of num_params * class_id * function_id
  | FCallUnpack of num_params
  | FCallBuiltin of num_params * num_params * Litstr.id

type op_member_base =
  | BaseC
  | BaseR
  | BaseL of local_id
  | BaseLW of local_id
  | BaseLD of local_id
  | BaseNC
  | BaseNL of local_id
  | BaseNCW
  | BaseNLW of local_id
  | BaseNCD
  | BaseNLD of local_id
  | BaseGC
  | BaseGL of local_id
  | BaseGCW
  | BaseGLW of local_id
  | BaseGCD
  | BaseGLD of local_id
  | BaseSC
  | BaseSL of local_id
  | BaseH

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
  | SetOpElemC of operand
  | SetOpElemL of operand * local_id
  | IncDecElemC of operand
  | IncDecElemL of operand * local_id
  | BindElemC
  | BindElemL of local_id
  | UnsetElemC
  | UnsetElemL of local_id
  | VGetNewElem
  | SetNewElem
  | SetOpNewElem of operand
  | IncDecNewElem of operand
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
  | SetOpPropC of operand
  | SetOpPropL of operand * local_id
  | IncDecPropC of operand
  | IncDecPropL of operand * local_id
  | BindPropC
  | BindPropL of local_id
  | UnsetPropC
  | UnsetPropL of local_id

type op_base =
  | BaseNC of stack_index * member_op_mode
  | BaseNL of local_id * member_op_mode
  | FPassBaseNC of param_id * stack_index
  | FPassBaseNL of param_id * local_id
  | BaseGC of stack_index * member_op_mode
  | BaseGL of local_id * member_op_mode
  | FPassBaseGC of param_id * stack_index
  | FPassBaseGL of param_id * local_id
  | BaseSC of stack_index * stack_index
  | BaseSL of local_id * stack_index
  | BaseL of local_id * member_op_mode
  | FPassBaseL of param_id * local_id
  | BaseC of stack_index
  | BaseR of stack_index
  | BaseH

type op_final =
  | QueryM of num_params * query_op * member_key
  | VGetM of num_params * member_key
  | FPassM of param_id * num_params * member_key
  | SetM of num_params * member_key
  | IncDecM of num_params * operand * member_key
  | SetOpM of num_params  * operand * member_key
  | BindM of num_params * member_key
  | UnsetM of num_params * member_key
  | SetWithRefLML of local_id * local_id
  | SetWithRefRML of local_id

type instruct_iterator =
  | IterInit of iterator_id * rel_offset * local_id
  | IterInitK of iterator_id * rel_offset * local_id * local_id
  | WIterInit of iterator_id * rel_offset * local_id
  | WIterInitK of iterator_id * rel_offset * local_id * local_id
  | MIterInit of iterator_id * rel_offset * local_id
  | MIterInitK of iterator_id * rel_offset * local_id * local_id
  | IterNext of iterator_id * rel_offset * local_id
  | IterNextK of iterator_id * rel_offset * local_id * local_id
  | WIterNext of iterator_id * rel_offset * local_id
  | WIterNextK of iterator_id * rel_offset * local_id * local_id
  | MIterNext of iterator_id * rel_offset * local_id
  | MIterNextK of iterator_id * rel_offset * local_id * local_id
  | IterFree of iterator_id
  | MIterFree of iterator_id
  | CIterFree of iterator_id
  | IterBreak of rel_offset * iter_vec

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
  | IncStat of int * int (* counter id, value *)
  | AKExists
  | CreateCl of num_params * class_id
  | Idx
  | ArrayIdx
  | AssertRATL of local_id * repo_auth_type
  | AssertRATStk of stack_index * repo_auth_type
  | BreakTraceHint
  | Silence of local_id * op_silence
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

type fun_return_type = {
  param_name : Litstr.id;
}

type fun_def = {
  f_name          : Litstr.id;
  f_body          : instruct list;
  f_return_types  : fun_return_type list;
}

type class_def = {
  class_name      : Litstr.id;
  (* TODO declarations *)
}

type hhas_prog = {
  hhas_fun     : fun_def list;
  hhas_classes : class_def list;
}

let make hhas_fun =
  let hhas_classes = [] in (* TODO: Pass these in *)
  { hhas_fun; hhas_classes }
