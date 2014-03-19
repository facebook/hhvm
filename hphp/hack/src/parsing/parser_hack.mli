(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type comments = (Pos.t * string) list

(*
 * Main parser entry point: parse a program out of a string.
 *
 * 'fail' indicates whether we should raise an exception if there were
 * parse errors.
 *)
val program : ?fail:bool -> string -> Ast.program

(* True if the last file parsed was in <?hh *)
val is_hh_file : bool ref

(* Parses a file *)
val from_file : string -> Ast.program

(* Same as from_file, but returns the list of comments as well as the tree *)
val from_file_with_comments : string -> comments * Ast.program
