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

let rewrite_in_loop instrseq cont_label break_label =
  (* PHP supports multi-level continue and break; the level must be a compile-
  time constant.

  TODO: Right now we only supports one-level continue and break, but this code
  is designed to handle multi-level continue and break.  Write tests for
  multi-level continue and break.

  TODO: This rewriting will be more complex once we support continue and break
  inside try-finally blocks.

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

  let rewriter i =
    match i with
    | ISpecialFlow (Continue (level, original)) when level > 1 ->
      ISpecialFlow (Continue ((level - 1), original))
    | ISpecialFlow (Continue _) ->
      IContFlow (Jmp cont_label)
    | ISpecialFlow (Break (level, original)) when level > 1 ->
      ISpecialFlow (Break ((level - 1), original))
    | ISpecialFlow (Break _) ->
      IContFlow (Jmp break_label)
    | _ -> i in
  InstrSeq.map instrseq ~f:rewriter

let rewrite_in_switch instrseq end_label =
  rewrite_in_loop instrseq end_label end_label

let rewrite_in_finally instrseq =
  (* TODO: continue and break that would branch out of a finally is
  illegal, but we allow it at parse time. Consider disallowing it
  at parse time rather than producing a run-time error. *)
  let plural n =
    if n = 1 then "" else "s" in
  let rewriter instruction =
    match instruction with
    | ISpecialFlow (Continue (_, original))
    | ISpecialFlow (Break (_, original))  ->
      let message = Printf.sprintf "Cannot break/continue %d level%s"
        original (plural original) in
      [
        (ILitConst (String message));
        (IOp Fatal)
      ]
    | _ -> [ instruction ] in
  InstrSeq.flat_map instrseq ~f:rewriter

(* Obtain a list of unique not-rewritten continues and breaks. *)
let get_continues_and_breaks instrseq =
  let module ULCB = Unique_list_continues_breaks in
  let folder uniq_list instruction =
    match instruction with
    | ISpecialFlow x ->  ULCB.add uniq_list x
    | _ -> uniq_list in
  let results = InstrSeq.fold_left instrseq ~init:ULCB.empty ~f:folder in
  List.rev (ULCB.items results)

(* TODO: We must have this already somewhere. *)
let index_of items item =
  let rec aux items index =
  match items with
  | [] -> failwith "index_of asked to find an item that isn't there"
  | h :: t -> if item = h then index else aux t (index + 1) in
  aux items 0

let rewrite_in_try_finally instrseq cont_breaks temp_local finally_label =
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
  let rewriter instruction =
    match instruction with
    | ISpecialFlow x ->
      gather [
        instr_int (index_of cont_breaks x);
        instr_setl_unnamed temp_local;
        instr_popc;
        instr_jmp finally_label
      ]
    | _ -> instr instruction in
  if cont_breaks = [] then
    instrseq
  else
    InstrSeq.flat_map_seq instrseq ~f:rewriter
