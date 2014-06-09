(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type parser_return =
  bool *                       (* True if we are dealing with a hack file *)
  (Pos.t * string) list *      (* Comments *)
  Ast.program

val program : string -> parser_return

(* Parses a file *)
val from_file : string -> parser_return

