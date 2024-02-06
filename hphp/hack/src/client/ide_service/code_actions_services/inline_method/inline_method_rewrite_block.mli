(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(**
 * Apply libhackfmt to a string corresponding to a block (one or multiple statements).
 * `start_indent_amount` is the amount the beginning of the text is *already* indented
 * *)
val format_block :
  Relative_path.t -> string -> start_indent_amount:int -> string

(** rename variables and replace `return` with a variable assignment *)
val rewrite_block :
  Inline_method_rename.t ->
  Relative_path.t ->
  string ->
  return_var_raw_name:string ->
  Inline_method_rename.t * string
