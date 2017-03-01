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

let rewrite_continue_break instrs cont_label break_label =
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
    | IContFlow (Continue level) when level > 1 ->
      IContFlow (Continue (level - 1))
    | IContFlow (Continue _) ->
      IContFlow (Jmp cont_label)
    | IContFlow (Break level) when level > 1 ->
      IContFlow (Break (level - 1))
    | IContFlow (Break _) ->
      IContFlow (Jmp break_label)
    | _ -> i in
  InstrSeq.map instrs ~f:rewriter
