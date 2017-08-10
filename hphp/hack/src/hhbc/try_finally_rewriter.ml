(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Hhbc_ast
open Instruction_sequence
open Core

module JT = Jump_targets

(* Collect list of Ret* and non rewritten Break/Continue instructions inside
   try body. *)
let collect_jump_instructions instrseq env =
  let jump_targets = Emit_env.get_jump_targets env in
  let get_label_id ~is_break level =
    match JT.get_target_for_level is_break level jump_targets with
    | JT.ResolvedRegular (target_label, _)
    | JT.ResolvedTryFinally { JT.target_label = target_label; _ } ->
      JT.get_id_for_label target_label
    | _ -> failwith "impossible"
  in
  let folder map i =
    match i with
    | ISpecialFlow Break l ->
      IMap.add (get_label_id ~is_break:true l) i map
    | ISpecialFlow Continue l ->
      IMap.add (get_label_id ~is_break:false l) i map
    | IContFlow (RetC | RetV) ->
      IMap.add (JT.get_id_for_return ()) i map
    | _ -> map
  in
  InstrSeq.fold_left instrseq ~init:IMap.empty ~f:folder

(* Delete Ret*, Break/Continue instructions from the try body *)
let cleanup_try_body instrseq =
  let rewriter i =
    match i with
    | ISpecialFlow _ | IContFlow (RetC | RetV) -> None
    | _ -> Some i
  in
  InstrSeq.filter_map instrseq ~f:rewriter

let emit_jump_to_label l iters =
  match iters with
  | [] -> instr_jmp l
  | iters -> instr_iter_break l iters

let emit_save_label_id id =
  gather [
    instr_int id;
    instr_setl (Local.get_label_id_local ());
    instr_popc;
  ]

let emit_return ~need_ref ~verify_return ~in_finally_epilogue env =
  let ret_instr = if need_ref then instr_retv else instr_retc in
  (* check if there are try/finally region *)
  let jump_targets = Emit_env.get_jump_targets env in
  begin match JT.get_closest_enclosing_finally_label jump_targets with
  (* no finally blocks, but there might be some iterators that should be
      released before exit - do it *)
  | None ->
    let verify_return_instr =
      if verify_return
      then if need_ref then instr_verifyRetTypeV else instr_verifyRetTypeC
      else empty
    in
    let release_iterators_instr =
      let iterators_to_release = JT.collect_iterators jump_targets in
      gather @@ List.map iterators_to_release ~f:(fun (is_mutable, it) ->
        let iter_free = if is_mutable then MIterFree it else IterFree it in
        instr (IIterator iter_free)
      )
    in
    if in_finally_epilogue
    then
      let load_retval_instr =
        if need_ref then instr_vgetl (Local.get_retval_local ())
        else instr_cgetl (Local.get_retval_local ())
      in
      gather [
        release_iterators_instr;
        load_retval_instr;
        verify_return_instr;
        ret_instr;
      ]
    else gather [
      verify_return_instr;
      release_iterators_instr;
      ret_instr;
    ]
  (* ret is in finally block and there might be iterators to release -
    jump to finally block via Jmp/IterBreak *)
  | Some (target_label, iterators_to_release) ->
    let preamble =
      if in_finally_epilogue then empty
      else
      let save_state = emit_save_label_id (JT.get_id_for_return ()) in
      let save_retval =
        if need_ref then gather [
          instr_bindl (Local.get_retval_local ());
          instr_popv;
        ]
        else gather [
          instr_setl (Local.get_retval_local ());
          instr_popc;
        ]
      in
      gather [
        save_state;
        save_retval;
      ]
    in
    gather [
      preamble;
      emit_jump_to_label target_label iterators_to_release;
      (* emit ret instr as an indicator for try/finally rewriter to generate
        finally epilogue, try/finally rewriter will remove it. *)
      ret_instr;
    ]
  end

and emit_break_or_continue ~is_break ~in_finally_epilogue env pos level =
  let jump_targets = Emit_env.get_jump_targets env in
  match JT.get_target_for_level ~is_break level jump_targets with
  | JT.NotFound -> Emit_fatal.emit_fatal_for_break_continue pos level
  | JT.ResolvedRegular (target_label, iterators_to_release) ->
    let preamble =
      if in_finally_epilogue && level = 1 then instr_unsetl @@ Local.get_label_id_local ()
      else empty
    in
    gather [
      preamble;
      emit_jump_to_label target_label iterators_to_release;
    ]
  | JT.ResolvedTryFinally {
      JT.target_label;
      JT.finally_label;
      JT.iterators_to_release;
      JT.adjusted_level; } ->
    let preamble =
      if not in_finally_epilogue
      then emit_save_label_id (JT.get_id_for_label target_label)
      else empty
    in
    gather [
      preamble;
      emit_jump_to_label finally_label iterators_to_release;
      (* emit break/continue instr as an indicator for try/finally rewriter
         to generate finally epilogue - try/finally rewriter will remove it. *)
      if is_break then instr_break adjusted_level else instr_continue adjusted_level
    ]

let emit_finally_epilogue ~verify_return env pos jump_instructions finally_end =
  let emit_instr i =
    match i with
    | IContFlow RetC ->
      emit_return ~need_ref:false ~verify_return ~in_finally_epilogue:true env
    | IContFlow RetV ->
      emit_return ~need_ref:true ~verify_return ~in_finally_epilogue:true env
    | ISpecialFlow (Break l) ->
      emit_break_or_continue ~is_break:true ~in_finally_epilogue:true env pos l
    | ISpecialFlow (Continue l) ->
      emit_break_or_continue ~is_break:false ~in_finally_epilogue:true env pos l
    | _ -> failwith "unexpected instruction: only Ret* or Break/Continue are expected"
  in
  match IMap.elements jump_instructions with
  | [] -> empty
  | [_, h] ->
    gather [
      instr_issetl (Local.get_label_id_local ());
      instr_jmpz finally_end;
      emit_instr h; ]
  | (max_id, _) :: _ as lst ->
    (* mimic HHVM behavior:
      in some cases ids can be non-consequtive - this might happen i.e. return statement
       appear in the block and it was assigned a high id before.
       ((3, Return), (1, Break), (0, Continue))
       In thid case generate switch as
       switch  (id) {
          L0 -> handler for continue
          L1 -> handler for break
          FinallyEnd -> empty
          L3 -> handler for return
       }
       *)
    let rec aux n instructions labels bodies =
      match instructions with
      | [] -> (labels, bodies)
      | (id, instruction) :: t ->
        if id = n then
          let label = Label.next_regular () in
          let body = gather [
            instr_label label;
            emit_instr instruction;]
          in
          aux (n - 1) t (label :: labels) (body :: bodies)
        else
          aux (n - 1) instructions (finally_end :: labels) (empty :: bodies)
    in
    (* lst is already stored - IMap.elements took care of it *)
    (* TODO: add is_sorted assert to make sure this behavior is preserved *)
    let (labels, bodies) = aux max_id lst [] [] in
    let labels = labels in
    gather [
      instr_issetl (Local.get_label_id_local ());
      instr_jmpz finally_end;
      instr_cgetl (Local.get_label_id_local ());
      instr_switch labels;
      gather bodies ]
(*
TODO: This codegen is unnecessarily complex.  Basically we are generating

IsSetL temp
JmpZ   finally_end
CGetL  temp
Switch Unbounded 0 <L4 L5>
L5:
UnsetL temp
Jmp LContinue
L4:
UnsetL temp
Jmp LBreak

Two problems immediately come to mind. First, why is the unset in every case,
instead of after the CGetL?  Surely the unset doesn't modify the stack.
Second, now we have a jump-to-jump situation.

Would it not make more sense to instead say

IsSetL temp
JmpZ   finally_end
CGetL  temp
UnsetL temp
Switch Unbounded 0 <LBreak LContinue>

?
*)
