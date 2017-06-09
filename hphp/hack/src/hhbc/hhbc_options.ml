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

(* Compiler configuration options, as set by -v key=value on the command line *)
type t = {
  option_ints_overflow_to_ints : bool option;
  option_enable_hiphop_syntax : bool;
  option_constant_folding : bool;
  option_optimize_null_check : bool;
  option_optimize_cuf : bool;
  option_max_array_elem_size_on_the_stack : int;
}

(* Default, as with HHVM, is for Eval.EnableHipHopSyntax=0 *)
let default = {
  option_ints_overflow_to_ints = None;
  option_enable_hiphop_syntax = false;
  option_constant_folding = true;
  option_optimize_null_check = true;
  option_optimize_cuf = true;
  option_max_array_elem_size_on_the_stack = 12;
}

let enable_hiphop_syntax options = options.option_enable_hiphop_syntax
let constant_folding options = options.option_constant_folding
let optimize_null_check options = options.option_optimize_null_check
let optimize_cuf options = options.option_optimize_cuf
let max_array_elem_size_on_the_stack options =
  options.option_max_array_elem_size_on_the_stack

(* The Hack.Lang.IntsOverflowToInts setting overrides the
 * Eval.EnableHipHopSyntax setting *)
let ints_overflow_to_ints options =
  match options.option_ints_overflow_to_ints with
  | Some v -> v
  | None -> options.option_enable_hiphop_syntax

let as_bool s =
  match String.lowercase_ascii s with
  | "0" | "false" -> false
  | _ -> true

let set_option options name value =
  match String.lowercase_ascii name with
  | "eval.enablehiphopsyntax" ->
    { options with option_enable_hiphop_syntax = as_bool value }
  | "hack.lang.intsoverflowtoints" ->
    { options with option_ints_overflow_to_ints = Some (as_bool value) }
  | "hack.compiler.constantfolding" ->
    { options with option_constant_folding = as_bool value }
  | "hack.compiler.optimizenullcheck" ->
    { options with option_optimize_null_check = as_bool value }
  | "hack.compiler.optimizecuf" ->
    { options with option_optimize_cuf = as_bool value }
  | _ -> options

(* Construct an instance of Hhbc_options.t from the options as specified
 * in `-v str` on the command line.
 *)
let get_options_from_config config_list =
  List.fold_left config_list ~init:default ~f:begin
    fun options str ->
    match Str.split_delim (Str.regexp "=") str with
    | [name; value] -> set_option options name value
    | _ -> options
  end

let compiler_options = ref default
let set_compiler_options o = compiler_options := o
