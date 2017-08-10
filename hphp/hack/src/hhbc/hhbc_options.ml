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

module J = Hh_json

(* Compiler configuration options, as set by -v key=value on the command line *)
type t = {
  option_ints_overflow_to_ints : bool option;
  option_enable_hiphop_syntax : bool;
  option_constant_folding : bool;
  option_optimize_null_check : bool;
  option_optimize_cuf : bool;
  option_max_array_elem_size_on_the_stack : int;
  option_aliased_namespaces : (string * string) list;
  option_source_mapping : bool;
}

let default = {
  option_ints_overflow_to_ints = None;
  option_enable_hiphop_syntax = false;
  option_constant_folding = false;
  option_optimize_null_check = false;
  option_optimize_cuf = false;
  option_max_array_elem_size_on_the_stack = 12;
  option_aliased_namespaces = [];
  option_source_mapping = false;
}

let enable_hiphop_syntax o = o.option_enable_hiphop_syntax
let constant_folding o = o.option_constant_folding
let optimize_null_check o = o.option_optimize_null_check
let optimize_cuf o = o.option_optimize_cuf
let max_array_elem_size_on_the_stack o =
  o.option_max_array_elem_size_on_the_stack
let aliased_namespaces o = o.option_aliased_namespaces
let source_mapping o = o.option_source_mapping

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
  | "hack.compiler.sourcemapping" ->
    { options with option_source_mapping = as_bool value }
  | _ -> options

let get_value_from_config_ config key =
  let f k1 (k2, _) = k1 = k2 in
  match List.find config ~f:(f key) with
  | None -> None
  | Some (_, json) ->
    begin match List.find (J.get_object_exn json) ~f:(f "global_value") with
    | None -> None
    | Some (_, json) -> Some json
    end

let get_value_from_config_int config key =
  let json_opt = get_value_from_config_ config key in
  Option.map json_opt ~f:(fun json ->
  match J.get_string_exn json with
  | "" -> 0
  | x -> int_of_string x)

let get_value_from_config_str config key =
  let json_opt = get_value_from_config_ config key in
  Option.map json_opt ~f:(fun json -> J.get_string_exn json)

let get_value_from_config_kv_list config key =
  let json_opt = get_value_from_config_ config key in
  Option.map json_opt ~f:(fun json ->
    match json with
    | J.JSON_Array [] -> []
    | json ->
    let keys_with_json = J.get_object_exn json in
    List.map ~f:(fun (k, v) -> k, J.get_string_exn v) keys_with_json)

let set_value name get set config opts =
  let value = get config name in
  Option.value_map value ~default:opts ~f:(set opts)

let value_setters = [
  (set_value "hhvm.aliased_namespaces" get_value_from_config_kv_list @@
    fun opts v -> { opts with option_aliased_namespaces = v });
  (set_value "hhvm.force_hh" get_value_from_config_int @@
    fun opts v -> { opts with option_enable_hiphop_syntax = (v = 1) });
  (set_value "hhvm.hack.lang.ints_overflow_to_ints" get_value_from_config_int @@
    fun opts v -> { opts with option_ints_overflow_to_ints = Some (v = 1) });
  (set_value "hack.compiler.constant_folding" get_value_from_config_int @@
    fun opts v -> { opts with option_constant_folding = (v = 1) });
  (set_value "hack.compiler.optimize_null_checks" get_value_from_config_int @@
    fun opts v -> { opts with option_optimize_null_check = (v = 1) });
  (set_value "hack.compiler.optimize_cuf" get_value_from_config_int @@
    fun opts v -> { opts with option_optimize_cuf = (v = 1) })
]

let extract_config_options_from_json ~init config_json =
  match config_json with None -> init | Some config_json ->
  let config = J.get_object_exn config_json in
  List.fold_left value_setters ~init ~f:(fun opts setter -> setter config opts)

(* Construct an instance of Hhbc_options.t from the options passed in as well as
 * as specified in `-v str` on the command line.
 *)
let get_options_from_config config_json config_list =
  let init = extract_config_options_from_json ~init:default config_json in
  List.fold_left config_list ~init ~f:begin
    fun options str ->
    match Str.split_delim (Str.regexp "=") str with
    | [name; value] -> set_option options name value
    | _ -> options
  end

let compiler_options = ref default
let set_compiler_options o = compiler_options := o
