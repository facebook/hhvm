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

let rewrite_in_loop instrseq cont_label break_label opt_iterator =
  (* PHP supports multi-level continue and break; the level must be a compile-
  time constant.

  TODO: Right now Hack only supports one-level continue and break, but this code
  is designed to handle multi-level continue and break.  Write tests for
  multi-level continue and break.

  Suppose we are generating code for:

  do {
    do {
      if (a)
        break 2;
      if (b)
        break;
    } while (c);
    if (d) break;
  } while (e);

When we codegen the nested do-loop we will first generate the inner do loop:

L1:  a
     JmpZ L2
     Break 2
L2:  b
     JmpZ L3
     Break 1
L3:  c
     JmpNZ L1
L4:

Then we run a pass over it which changes all the Break 1 to Jmp L4, and
all the Break 2 to Break 1:

L1:  a
     JmpZ L2
     Break 1
L2:  b
     JmpZ L3
     Jmp L4
L3:  c
     JmpNZ L1
L4:

Then we keep on generating code for the outer do-loop:

L1:  a
     JmpZ L2
     Break 1
L2:  b
     JmpZ L3
     Jmp L4
L3:  c
     JmpNZ L1
L4:  d
     JmpZ L5
     Break 1
L5:  e
     JmpNZ L1
L6:

And then we run another rewriting pass over the whole thing which turns
the Break 1s into Jmp L6:

L1:  a
     JmpZ L2
     Jmp L6
L2:  b
     JmpZ L3
     Jmp L4
L3:  c
     JmpNZ L1
L4:  d
     JmpZ L5
     Jmp L6
L5:  e
     JmpNZ L1
L6:

And we're done. We've rewritten all the multi-level break and continues into
unconditional jumps to the right place. *)

  let add_iterator lst =
    match opt_iterator with
      | Some x -> x :: lst
      | None -> lst in
  let rewrite_return ~is_retc itrs =
    match opt_iterator with
    | Some it ->
      (* foreach might be enclosed in try/finally block.
         Rewrite Ret* to SF_Ret* to make sure that iterator is released
         after jumping to finally prologue *)
      let itrs = it :: itrs in
      let iter_free_and_ret =
        if is_retc then SF_RetC itrs else SF_RetV itrs
      in
      instr (ISpecialFlow iter_free_and_ret)
    | _ -> if is_retc then instr_retc else instr_retv
  in
  let rewriter i =
    match i with
      | ISpecialFlow (Continue (level, original)) when level > 1 ->
        instr (ISpecialFlow (Continue ((level - 1), original)))
      | ISpecialFlow (Continue _) ->
        instr (IContFlow (Jmp cont_label))
      | ISpecialFlow (Break (level, original, itrs)) when level > 1 ->
        instr (ISpecialFlow (Break ((level - 1), original, add_iterator itrs)))
      | ISpecialFlow (Break (_, _, itrs)) ->
        (match add_iterator itrs with
        | [] -> instr (IContFlow (Jmp break_label))
        | itrs -> instr (IIterator (IterBreak (break_label, itrs))))
      | ISpecialFlow (SF_RetC itrs) ->
        rewrite_return ~is_retc:true itrs
      | ISpecialFlow (SF_RetV itrs) ->
        rewrite_return ~is_retc:false itrs
      | IContFlow RetC ->
        rewrite_return ~is_retc:true []
      | IContFlow RetV ->
        rewrite_return ~is_retc:false []
      | _ -> instr i in
  InstrSeq.flat_map_seq instrseq ~f:rewriter

let rewrite_in_switch instrseq end_label =
  rewrite_in_loop instrseq end_label end_label None

(* Replace SF_Ret instructions with their normal counterparts and release
   iterators if necessary *)
let rewrite_special_flow_returns instrseq =
  let emit_ret ~emit_retc itrs = gather [
    gather @@ List.rev_map ~f:(fun (is_mutable, it) ->
      let iter_free = if is_mutable then MIterFree it else IterFree it in
      instr (IIterator iter_free)
    ) itrs;
    if emit_retc then instr_retc else instr_retv
  ]
  in
  let rewriter i =
    match i with
    | ISpecialFlow (SF_RetC itrs) -> emit_ret ~emit_retc:true itrs
    | ISpecialFlow (SF_RetV itrs) -> emit_ret ~emit_retc:false itrs
    | _ -> instr i
  in
  InstrSeq.flat_map_seq instrseq ~f:rewriter

let rewrite_in_finally instrseq =
  (* TODO: continue and break that would branch out of a finally is
  illegal, but we allow it at parse time. Consider disallowing it
  at parse time rather than producing a run-time error. *)
  let plural n =
    if n = 1 then "" else "s" in
  let rewriter instruction =
    match instruction with
    | ISpecialFlow (Continue (_, original))
    | ISpecialFlow (Break (_, original, _))  ->
      let message = Printf.sprintf "Cannot break/continue %d level%s"
        original (plural original) in
      [
        (ILitConst (String message));
        (IOp (Fatal FatalOp.Runtime))
      ]
    | _ -> [ instruction ] in
  InstrSeq.flat_map instrseq ~f:rewriter

(* Obtain a list of unique not-rewritten continues, breaks and SF_Ret* . *)
let collect_special_flow_instructions instrseq =
  let module ULSFI = Unique_list_special_flow_instr in
  let is_return = function SF_RetC _ | SF_RetV _ -> true | _ -> false in
  let folder (uniq_list, has_return as acc) instruction =
    match instruction with
    | ISpecialFlow x ->  ULSFI.add uniq_list x, has_return || is_return x
    (* body of try_block can contain regular Ret* instructions -
       collect them as well to make sure they are also rewritten to jumps to
       the finally prologue *)
    | IContFlow RetC -> ULSFI.add uniq_list (SF_RetC []), true
    | IContFlow RetV -> ULSFI.add uniq_list (SF_RetV []), true
    | _ -> acc in
  let results, has_return = InstrSeq.fold_left instrseq
    ~init:(ULSFI.empty, false) ~f:folder in
  (* These can be in any old order; we're building a lookup table after all.
  However, the order should be (1) consistent from run to run, and (2) ideally
  should be in source-code order, so that if there is a continue followed by
  a break, then the continue is zero and the break is one.
  TODO: We might consider sorting the list using the comparator in ULCB.
  That way we know the list order will be stable even if the order in which
  we visit the instructions changes for some reason. It won't match HHVM then
  though. For now, stick with the source-code order. *)
  ULSFI.items results, has_return

(* TODO: We must have this already somewhere. *)
let index_of items item =
  let module ULSFI = Unique_list_special_flow_instr in
  let rec aux items index =
  match items with
  | [] -> failwith "index_of asked to find an item that isn't there"
  | h :: t ->
    if ULSFI.SpecialFlowInstr.compare h item = 0 then index
    else aux t (index + 1)
  in
  aux items 0

let rewrite_in_try_finally instrseq cont_breaks temp_local ret_local finally_label =
(*
A continue which appears in the try of a try-finally that branches to a loop
label inside the try has already been rewritten as a jump. Therefore if there
are continues and breaks left un-rewritten they must be finally-blocked.

We make a list of all the distinct continues and breaks -- remember, some
may be multi-level. We then use the index into that list of distinct
possibilities as a key for what to do. The continue or break is replaced by
a set of a temporary local to the index, and a jump to the beginning of the
finally. An epilogue on the finally will deal with ensuring the right
instruction is generated.
*)
  let emit_jmp_to_finally_label_for_ret_instr itrs =
    if List.is_empty itrs then instr_jmp finally_label
    else instr_iter_break finally_label itrs
  in
  let rewrite_special_flow_instr x =
    let index = index_of cont_breaks x in
    let final_instr =
      match x with
      | SF_RetC itrs ->
        gather [
          (* store return value in local *)
          instr_setl ret_local;
          instr_popc;
          emit_jmp_to_finally_label_for_ret_instr itrs;
        ]
      | SF_RetV itrs ->
        gather [
          (* store return value in local *)
          instr_bindl ret_local;
          instr_popv;
          emit_jmp_to_finally_label_for_ret_instr itrs
        ]
      | _ -> instr_jmp finally_label
    in
    gather [
      instr_int index;
      instr_setl temp_local;
      instr_popc;
      final_instr
    ]
  in

  let rewriter instruction =
    match instruction with
    | ISpecialFlow x -> rewrite_special_flow_instr x
    (* handle regular Ret* instructions *)
    | IContFlow RetC -> rewrite_special_flow_instr (SF_RetC [])
    | IContFlow RetV -> rewrite_special_flow_instr (SF_RetV [])
    | _ -> instr instruction in
  if cont_breaks = [] then
    instrseq
  else
    InstrSeq.flat_map_seq instrseq ~f:rewriter

let emit_finally_epilogue cont_and_break temp_local ret_local finally_end =
  let emit_instr i =
    match i with
    | SF_RetC _ -> gather [
        instr_cgetl ret_local;
        instr (IContFlow RetC);
      ]
    | SF_RetV _ -> gather [
        instr_vgetl ret_local;
        instr (IContFlow RetV);
      ]
    | i -> gather [
        instr_unsetl temp_local;
        instr (ISpecialFlow i);
      ]
  in
  match cont_and_break with
  | [] -> empty
  | [h] ->
    gather [
      instr_issetl temp_local;
      instr_jmpz finally_end;
      emit_instr h; ]
  | _ ->
    let rec aux instructions labels bodies =
      match instructions with
      | [] -> (labels, bodies)
      | instruction :: t ->
        let label = Label.next_regular () in
        let body = gather [
          instr_label label;
          emit_instr instruction;]
        in
        aux t (label :: labels) (body :: bodies) in
    let (labels, bodies) = aux cont_and_break [] [] in
    let labels = List.rev labels in
    gather [
      instr_issetl temp_local;
      instr_jmpz finally_end;
      instr_cgetl temp_local;
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
