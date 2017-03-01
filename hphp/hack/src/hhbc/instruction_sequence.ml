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
let instr_label label = instr (ILabel label)
let instr_label_catch label = instr (IExceptionLabel (label, CatchL))
let instr_continue level = instr (IContFlow (Continue level))
let instr_break level = instr (IContFlow (Break level))
let instr_unwind = instr (IContFlow Unwind)
let instr_false = instr (ILitConst False)
let instr_true = instr (ILitConst True)
let instr_eq = instr (IOp Eq)
let instr_retc = instr (IContFlow RetC)
let instr_null = instr (ILitConst Null)
let instr_catch = instr (IMisc Catch)
let instr_dup = instr (IBasic Dup)
let instr_instanceofd s = instr (IOp (InstanceOfD s))

let instr_setl_named local =
  instr (IMutator (SetL (Local_named local)))
let instr_setl_unnamed local =
  instr (IMutator (SetL (Local_unnamed local)))
let instr_unsetl_unnamed local =
  instr (IMutator (UnsetL (Local_unnamed local)))
let instr_cgetl2_pipe = instr (IGet (CGetL2 (Local_pipe)))
let instr_popc = instr (IBasic PopC)
let instr_throw = instr (IContFlow Throw)

let instr_add_elemc = instr (ILitConst (AddElemC))
let instr_add_new_elemc = instr (ILitConst (AddNewElemC))

(* Functions on instr_seq that correspond to existing Core.List functions *)
module InstrSeq = struct
  let rec flat_map instrseq ~f =
    let flat_map_list items ~f = Core.List.bind items f in
    match instrseq with
    | Instr_list instrl ->
      let mapper instr =
        match instr with
        | ITryFault(l, instrl1, instrl2) ->
          [ ITryFault(l, flat_map_list instrl1 f,
            flat_map_list instrl2 f) ]
        | ITryCatch(x, instrl) ->
          [ ITryCatch(x, flat_map_list instrl f) ]
        | _ ->
          f instr in
      Instr_list (flat_map_list instrl ~f:mapper)
    | Instr_concat instrseql ->
      Instr_concat (List.map instrseql (flat_map ~f))

  let rec fold_left instrseq ~f ~init =
    let rec fold_instruction init instruction =
      match instruction with
      | ITryFault(_, try_instrs, fault_instrs) ->
        let try_ = List.fold_left try_instrs ~f:fold_instruction ~init in
        List.fold_left fault_instrs ~f:fold_instruction ~init:try_
      | ITryCatch(_, try_instrs) ->
        List.fold_left try_instrs ~f:fold_instruction ~init
      | _ -> f init instruction in
    let fold_instrseq init instrseq =
      fold_left instrseq ~f ~init in
    match instrseq with
    | Instr_list instrl ->
      List.fold_left instrl ~f:fold_instruction ~init
    | Instr_concat instrseql ->
      List.fold_left instrseql ~f:fold_instrseq ~init

  let rec filter_map instrseq ~f =
    match instrseq with
    | Instr_list instrl ->
      Instr_list (List.filter_map instrl ~f:(fun instr ->
        match instr with
        | ITryFault(l, instrl1, instrl2) ->
          Some (ITryFault(l, List.filter_map instrl1 f,
            List.filter_map instrl2 f))
        | ITryCatch(x, instrl) ->
          Some (ITryCatch(x, List.filter_map instrl f))
        | _ ->
          f instr
        ))
    | Instr_concat instrseql ->
      Instr_concat (List.map instrseql (filter_map ~f))

  let map instrseq ~f = filter_map instrseq ~f:(fun x -> Some (f x))

end

(* Could write this in terms of InstrSeq.fold_left *)
let rec instr_seq_to_list_aux sl result =
  match sl with
  | [] -> List.rev result
  | s::sl ->
    match s with
    | Instr_list instrl ->
      instr_seq_to_list_aux sl (List.rev_append instrl result)
    | Instr_concat sl' -> instr_seq_to_list_aux (sl' @ sl) result

let instr_seq_to_list t = instr_seq_to_list_aux [t] []

let instr_try_fault_no_catch fault_label try_body fault_body =
  let try_body = instr_seq_to_list try_body in
  let fault_body = instr_seq_to_list fault_body in
  let fl = IExceptionLabel (fault_label, FaultL) in
  instr (ITryFault (fault_label, try_body, fl :: fault_body))

let instr_try_catch catch_label try_body =
  let try_body = instr_seq_to_list try_body in
  instr (ITryCatch (catch_label, try_body))
