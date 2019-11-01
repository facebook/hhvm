(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val to_segments :
  ?path:Relative_path.t ->
  ?dump_symbol_refs:bool ->
  Hhas_program.t ->
  string list

(**
 * Materializing the hhbc as a single string may introduce additional runtime
 * memory usage. Prefer to_hhbc_accumulator.
 *)
val to_string :
  ?path:Relative_path.t -> ?dump_symbol_refs:bool -> Hhas_program.t -> string

val string_of_instruction : Hhbc_ast.instruct -> string
