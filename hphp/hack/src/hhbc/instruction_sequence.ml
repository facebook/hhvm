(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Core
open Hhbc_ast

(* The various from_X functions below take some kind of AST (expression,
 * statement, etc.) and produce what is logically a sequence of instructions.
 * This could simply be represented by a list, but then we would need to
 * use an accumulator to avoid the quadratic complexity associated with
 * repeated appending to a list. Instead, we simply build a tree of
 * instructions which can easily be flattened at the end.
 *)
type t =
| Instr_list of instruct list
| Instr_concat of t list
| Instr_try_fault of t * t

(* Some helper constructors *)
let instr x = Instr_list [x]
let instrs x = Instr_list x
let gather x = Instr_concat x
let empty = Instr_list []

let instr_iterinit iter_id label value =
  instr (IIterator (IterInit (iter_id, label, value)))
let instr_iterinitk id label key value =
  instr (IIterator (IterInitK (id, label, key, value)))
let instr_iternext id label value =
  instr (IIterator (IterNext (id, label, value)))
let instr_iternextk id label key value =
  instr (IIterator (IterNextK (id, label, key, value)))
let instr_iterfree id =
  instr (IIterator (IterFree id))

let instr_jmp label = instr (IContFlow (Jmp label))
let instr_jmpz label = instr (IContFlow (JmpZ label))
let instr_jmpnz label = instr (IContFlow (JmpNZ label))
let instr_jmpns label = instr (IContFlow (JmpNS label))
let instr_label label = instr (ILabel label)
let instr_continue level = instr (ISpecialFlow (Continue (level, level)))
let instr_break level = instr (ISpecialFlow (Break (level, level)))
let instr_unwind = instr (IContFlow Unwind)
let instr_false = instr (ILitConst False)
let instr_true = instr (ILitConst True)
let instr_eq = instr (IOp Eq)
let instr_retc = instr (IContFlow RetC)
let instr_null = instr (ILitConst Null)
let instr_catch = instr (IMisc Catch)
let instr_dup = instr (IBasic Dup)
let instr_instanceofd s = instr (IOp (InstanceOfD s))
let instr_instanceof = instr (IOp InstanceOf)
let instr_int i = instr (ILitConst (Int (Int64.of_int i)))
let instr_int_of_string litstr =
  instr (ILitConst (Int (Int64.of_string litstr)))
let instr_double litstr = instr (ILitConst (Double litstr))
let instr_string litstr = instr (ILitConst (String litstr))
let instr_this = instr (IMisc This)
let instr_istypec op = instr (IIsset (IsTypeC op))
let instr_not = instr (IOp Not)
let instr_setl local = instr (IMutator (SetL local))
let instr_unsetl local = instr (IMutator (UnsetL local))
let instr_issetl local = instr (IIsset (IssetL local))
let instr_cgetl local = instr (IGet (CGetL local))
let instr_cgetl2 local = instr (IGet (CGetL2 local))
let instr_cgetquietl local = instr (IGet (CGetQuietL local))
let instr_fpassl param local = instr (ICall (FPassL (param, local)))
let instr_popu = instr (IBasic PopU)
let instr_popr = instr (IBasic PopR)
let instr_popc = instr (IBasic PopC)
let instr_popa = instr (IBasic PopA)
let instr_pop flavor =
  match flavor with
  | Flavor.Ref -> instr_popr
  | Flavor.Cell -> instr_popc
  | Flavor.Classref -> instr_popa

let instr_pushl local = instr (IGet (PushL local))
let instr_throw = instr (IContFlow Throw)

let instr_add_elemc = instr (ILitConst (AddElemC))
let instr_add_new_elemc = instr (ILitConst (AddNewElemC))
let instr_col_add_new_elemc = instr (ILitConst (ColAddNewElemC))
let instr_switch labels = instr (IContFlow (Switch (Unbounded, 0, labels)))
let instr_fpushctord nargs id = instr (ICall (FPushCtorD (nargs, id)))
let instr_clone = instr (IOp Clone)
let instr_newstructarray keys = instr (ILitConst (NewStructArray keys))
let instr_newcol collection_type = instr (ILitConst (NewCol collection_type))
let instr_colfromarray collection_type =
  instr (ILitConst (ColFromArray collection_type))
let instr_unboxr = instr (IBasic UnboxR)
let instr_entrynop = instr (IBasic EntryNop)
let instr_dict x xs = instr (ILitConst (Dict(x, xs)))
let instr_staticlocinit local text = instr (IMisc (StaticLocInit(local, text)))
let instr_basel local mode = instr (IBase(BaseL(local, mode)))
let instr_baseh = instr (IBase BaseH)
let instr_fpushfuncd count text = instr (ICall(FPushFuncD(count, text)))
let instr_fcall count = instr (ICall(FCall count))
let instr_isuninit = instr (IMisc IsUninit)
let instr_cgetcunop = instr (IMisc CGetCUNop)
let instr_ugetcunop = instr (IMisc UGetCUNop)
let instr_memoget count local local_count =
  instr (IMisc (MemoSet(count, local, local_count)))
let instr_memoset count local local_count =
  instr (IMisc (MemoSet(count, local, local_count)))
let instr_getmemokeyl local = instr (IMisc (GetMemoKeyL local))
let instr_ismemotype = instr (IMisc IsMemoType)
let instr_maybememotype = instr (IMisc MaybeMemoType)
let instr_checkthis = instr (IMisc CheckThis)
let instr_dim op key = instr (IBase (Dim (op, key)))
let instr_dim_warn_pt key = instr_dim MemberOpMode.Warn (MemberKey.PT key)
let instr_dim_define_pt key = instr_dim MemberOpMode.Define (MemberKey.PT key)
let instr_fpushobjmethodd num_params method_ flavor =
  instr (ICall (FPushObjMethodD (num_params, method_, flavor)))
let instr_fpushobjmethodd_nullthrows num_params method_ =
  instr_fpushobjmethodd num_params method_ Ast.OG_nullthrows
let instr_querym num_params op key =
  instr (IFinal (QueryM (num_params, op, key)))
let instr_querym_cget_pt num_params key =
  instr_querym num_params QueryOp.CGet (MemberKey.PT key)
let instr_setm num_params key = instr (IFinal (SetM (num_params, key)))
let instr_setm_pt num_params key = instr_setm num_params (MemberKey.PT key)

(* Functions on instr_seq that correspond to existing Core.List functions *)
module InstrSeq = struct

  (* f takes an instruction and produces an instruction list to replace it. *)
  let rec flat_map instrseq ~f =
    let flat_map_list items ~f = Core.List.bind items f in
    match instrseq with
    | Instr_try_fault (try_body, fault_body) ->
      Instr_try_fault ((flat_map try_body ~f), (flat_map fault_body ~f))
    | Instr_list instrl ->
      Instr_list (flat_map_list instrl ~f)
    | Instr_concat instrseql ->
      Instr_concat (List.map instrseql (flat_map ~f))

  (* f takes an instruction and produces an instrseq to replace it. *)
  let rec flat_map_seq instrseq ~f =
    match instrseq with
    | Instr_try_fault (try_body, fault_body) ->
      Instr_try_fault ((flat_map_seq try_body ~f), (flat_map_seq fault_body ~f))
    | Instr_list instrl ->
      Instr_concat (List.map instrl ~f)
    | Instr_concat instrseql ->
      Instr_concat (List.map instrseql (flat_map_seq ~f))

  let rec fold_left instrseq ~f ~init =
    let fold_instrseq init instrseq =
      fold_left instrseq ~f ~init in
    match instrseq with
    | Instr_try_fault (try_body, fault_body) ->
      fold_left fault_body ~init:(fold_left try_body ~f ~init) ~f
    | Instr_list instrl ->
      List.fold_left instrl ~f ~init
    | Instr_concat instrseql ->
      List.fold_left instrseql ~f:fold_instrseq ~init

  let rec filter_map instrseq ~f =
    match instrseq with
    | Instr_try_fault (try_body, fault_body) ->
      Instr_try_fault ((filter_map try_body ~f), (filter_map fault_body ~f))
    | Instr_list instrl ->
      Instr_list (List.filter_map instrl ~f)
    | Instr_concat instrseql ->
      Instr_concat (List.map instrseql (filter_map ~f))

  let map instrseq ~f = filter_map instrseq ~f:(fun x -> Some (f x))

end

(* TODO: Can we eliminate this helper altogether? *)
(* Could write this in terms of InstrSeq.fold_left *)
let rec instr_seq_to_list_aux sl result =
  match sl with
  | [] -> List.rev result
  | s::sl ->
    match s with
    (* NOTE we discard fault blocks when linearizing an instruction sequence *)
    | Instr_try_fault (try_body, _fault_body) ->
      instr_seq_to_list_aux (try_body :: sl) result
    | Instr_list instrl ->
      instr_seq_to_list_aux sl (List.rev_append instrl result)
    | Instr_concat sl' -> instr_seq_to_list_aux (sl' @ sl) result

let instr_seq_to_list t = instr_seq_to_list_aux [t] []

let instr_try_fault_begin fault_label fault_body =
  let try_begin = instr (ITry (TryFaultBegin (fault_label))) in
  let fault_body = gather [
    instr_label fault_label;
    fault_body
  ] in
  Instr_try_fault (try_begin, fault_body)

let instr_try_fault_end = instr (ITry TryFaultEnd)
let instr_try_fault fault_label try_body fault_body =
  gather [
    instr_try_fault_begin fault_label fault_body;
    try_body;
    instr_try_fault_end;
  ]

let instr_try_catch_begin catch_label = instr (ITry (TryCatchBegin catch_label))
let instr_try_catch_end = instr (ITry TryCatchEnd)

let extract_decl_vars instrseq =
  let module ULS = Unique_list_string in
  (* TODO: Both reads and writes need to go into decl vars *)
  (* TODO: Default arguments should not be in declvars *)
  let folder uniq_list instruction =
    match instruction with
    | IMutator (SetL (Local.Named s)) -> ULS.add uniq_list s
    | _ -> uniq_list in
  let decl_vars = InstrSeq.fold_left instrseq ~init:ULS.empty ~f:folder in
  List.rev (ULS.items decl_vars)

(*  Note that at this time we do NOT want to recurse on the instruction
    sequence in the fault block. Why not?  Consider:
    try { x } finally { try { y } finally { z } }
    We make a copy of the code generated for "try { y } finally { z }" in
    both the "finally" code which follows try-fault F1 { x }, and in
    the fault block for the outer try. Which means that now there are two
    places in the code where there is a TryFaultBegin instruction for the
    *inner*  try. We don't want to detect it twice and generate fault blocks
    twice.

    This means that if we ever synthesize a fault-only try-fault, without
    a finally block copying its contents, and that fault block itself
    contains a try-fault or try-finally, then the fault block of the inner
    try-fault will never be detected here. Right now we never do that; we
    only generate synthetic try-faults for simple cleanup operations. If we
    ever do generate nested try-faults then we'll need a more sophisticated
    algorithm here to ensure that each fault block is emitted once.
 *)
let extract_fault_instructions instrseq =
  let rec aux instrseq acc =
    match instrseq with
    | Instr_try_fault (try_body, fault_body) ->
      aux try_body (fault_body :: acc)
    | Instr_list _ -> acc
    | Instr_concat ([]) -> acc
    | Instr_concat (h :: t) -> aux (Instr_concat t) (aux h acc) in
  gather (aux instrseq [])
