(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Hh_core

module J = Hh_json

(* Compiler configuration options, as set by -v key=value on the command line *)
type t = {
  option_ints_overflow_to_ints            : bool option;
  option_enable_hiphop_syntax             : bool;
  option_php7_scalar_types                : bool;
  option_enable_xhp                       : bool;
  option_constant_folding                 : bool;
  option_optimize_null_check              : bool;
  option_max_array_elem_size_on_the_stack : int;
  option_aliased_namespaces               : (string * string) list option;
  option_source_mapping                   : bool;
  option_relabel                          : bool;
  option_php7_uvs                         : bool;
  option_php7_ltr_assign                  : bool;
  option_create_inout_wrapper_functions   : bool;
  option_reffiness_invariance             : bool;
  option_throw_on_call_by_ref_annotation_mismatch : bool;
  option_hack_arr_compat_notices          : bool;
  option_hack_arr_dv_arrs                 : bool;
  option_dynamic_invoke_functions         : SSet.t;
  option_repo_authoritative               : bool;
  option_jit_enable_rename_function       : bool;
  option_can_inline_gen_functions         : bool;
  option_php7_int_semantics               : bool;
  option_autoprime_generators             : bool;
  option_enable_is_expr_primitive_migration : bool;
  option_enable_coroutines                : bool;
  option_hacksperimental                  : bool;
  option_doc_root                         : string;
  option_include_search_paths             : string list;
  option_include_roots                    : string SMap.t;
  option_enable_perf_logging              : bool;
  option_disable_return_by_reference      : bool;
}

let default = {
  option_ints_overflow_to_ints = None;
  option_enable_hiphop_syntax = false;
  option_php7_scalar_types = false;
  option_enable_xhp = false;
  option_constant_folding = false;
  option_optimize_null_check = false;
  option_max_array_elem_size_on_the_stack = 64;
  option_aliased_namespaces = None;
  option_source_mapping = false;
  option_php7_uvs = false;
  option_php7_ltr_assign = false;
  (* If true, then renumber labels after generating code for a method
   * body. Semantic diff doesn't care about labels, but for visual diff against
   * HHVM it's helpful to renumber in order that the labels match more closely *)
  option_relabel = true;
  option_create_inout_wrapper_functions = true;
  option_reffiness_invariance = false;
  option_throw_on_call_by_ref_annotation_mismatch = false;
  option_hack_arr_compat_notices = false;
  option_hack_arr_dv_arrs = false;
  option_dynamic_invoke_functions = SSet.empty;
  option_repo_authoritative = false;
  option_jit_enable_rename_function = false;
  option_can_inline_gen_functions = true;
  option_php7_int_semantics = false;
  option_autoprime_generators = true;
  option_enable_is_expr_primitive_migration = true;
  option_enable_coroutines = true;
  option_hacksperimental = false;
  option_doc_root = "";
  option_include_search_paths = [];
  option_include_roots = SMap.empty;
  option_enable_perf_logging = false;
  option_disable_return_by_reference = false;

}

let enable_hiphop_syntax o = o.option_enable_hiphop_syntax
let php7_scalar_types o = o.option_php7_scalar_types
let enable_xhp o = o.option_enable_xhp
let constant_folding o = o.option_constant_folding
let optimize_null_check o = o.option_optimize_null_check
let max_array_elem_size_on_the_stack o =
  o.option_max_array_elem_size_on_the_stack
let aliased_namespaces o = o.option_aliased_namespaces
let source_mapping o = o.option_source_mapping
let relabel o = o.option_relabel
let enable_uniform_variable_syntax o = o.option_php7_uvs
let php7_ltr_assign o = o.option_php7_ltr_assign
let create_inout_wrapper_functions o = o.option_create_inout_wrapper_functions
let reffiness_invariance o = o.option_reffiness_invariance
let throw_on_call_by_ref_annotation_mismatch o =
  o.option_throw_on_call_by_ref_annotation_mismatch
let hack_arr_compat_notices o = o.option_hack_arr_compat_notices
let hack_arr_dv_arrs o = o.option_hack_arr_dv_arrs
let dynamic_invoke_functions o = o.option_dynamic_invoke_functions
let repo_authoritative o = o.option_repo_authoritative
let jit_enable_rename_function o = o.option_jit_enable_rename_function
let can_inline_gen_functions o = o.option_can_inline_gen_functions
let php7_int_semantics o = o.option_php7_int_semantics
let autoprime_generators o = o.option_autoprime_generators
let enable_is_expr_primitive_migration o = o.option_enable_is_expr_primitive_migration
let enable_coroutines o = o.option_enable_coroutines
let hacksperimental o = o.option_hacksperimental
let doc_root o = o.option_doc_root
let include_search_paths o = o.option_include_search_paths
let include_roots o = o.option_include_roots
let enable_perf_logging o = o.option_enable_perf_logging
let disable_return_by_reference o = o.option_disable_return_by_reference
let to_string o =
  let dynamic_invokes =
    String.concat ", " (SSet.elements (dynamic_invoke_functions o)) in
  let search_paths = String.concat ", " (include_search_paths o) in
  let inc_roots = SMap.bindings (include_roots o)
    |> List.map ~f:(fun (root, path) -> root ^ ": " ^ path)
    |> String.concat ", " in
  String.concat "\n"
    [ Printf.sprintf "enable_hiphop_syntax: %B" @@ enable_hiphop_syntax o
    ; Printf.sprintf "php7_scalar_types: %B" @@ php7_scalar_types o
    ; Printf.sprintf "enable_xhp: %B" @@ enable_xhp o
    ; Printf.sprintf "constant_folding: %B" @@ constant_folding o
    ; Printf.sprintf "optimize_null_check: %B" @@ optimize_null_check o
    ; Printf.sprintf "max_array_elem_size_on_the_stack: %d"
      @@ max_array_elem_size_on_the_stack o
    ; Printf.sprintf "source_mapping: %B" @@ source_mapping o
    ; Printf.sprintf "relabel: %B" @@ relabel o
    ; Printf.sprintf "enable_uniform_variable_syntax: %B"
      @@ enable_uniform_variable_syntax o
    ; Printf.sprintf "php7_ltr_assign: %B" @@ php7_ltr_assign o
    ; Printf.sprintf "create_inout_wrapper_functions: %B"
      @@ create_inout_wrapper_functions o
    ; Printf.sprintf "reffiness_invariance: %B" @@ reffiness_invariance o
    ; Printf.sprintf "throw_on_call_by_ref_annotation_mismatch: %B"
      @@ throw_on_call_by_ref_annotation_mismatch o
    ; Printf.sprintf "hack_arr_compat_notices: %B" @@ hack_arr_compat_notices o
    ; Printf.sprintf "hack_arr_dv_arrs: %B" @@ hack_arr_dv_arrs o
    ; Printf.sprintf "dynamic_invoke_functions: [%s]" dynamic_invokes
    ; Printf.sprintf "repo_authoritative: %B" @@ repo_authoritative o
    ; Printf.sprintf "jit_enable_rename_function: %B"
      @@ jit_enable_rename_function o
    ; Printf.sprintf "can_inline_gen_functions: %B" @@ can_inline_gen_functions o
    ; Printf.sprintf "php7_int_semantics: %B" @@ php7_int_semantics o
    ; Printf.sprintf "autoprime_generators: %B" @@ autoprime_generators o
    ; Printf.sprintf "enable_is_expr_primitive_migration: %B"
      @@ enable_is_expr_primitive_migration o
    ; Printf.sprintf "enable_coroutines: %B" @@ enable_coroutines o
    ; Printf.sprintf "hacksperimental: %B" @@ hacksperimental o
    ; Printf.sprintf "doc_root: %s" @@ doc_root o
    ; Printf.sprintf "include_search_paths: [%s]" search_paths
    ; Printf.sprintf "include_roots: {%s}" inc_roots
    ; Printf.sprintf "enable_perf_logging: %B" @@ enable_perf_logging o
    ; Printf.sprintf "disable_return_by_reference: %B" @@ disable_return_by_reference o
    ]

(* The Hack.Lang.IntsOverflowToInts setting overrides the
 * Eval.EnableHipHopSyntax setting *)
let ints_overflow_to_ints options =
  match options.option_ints_overflow_to_ints with
  | Some v -> v
  | None -> options.option_enable_hiphop_syntax

let as_bool s =
  match String.lowercase_ascii s with
  | "0" | "false" -> false
  | "1" | "true"  -> true
  | _             -> raise (Arg.Bad (s ^ " can't be cast to bool"))

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
  (* Keep both for backwards compatibility until next release *)
  | "hack.compiler.sourcemapping" | "eval.disassemblersourcemapping" ->
    { options with option_source_mapping = as_bool value }
  | "hhvm.php7.scalar_types" ->
    { options with option_php7_scalar_types = as_bool value }
  | "hhvm.php7.ltr_assign" ->
    { options with option_php7_ltr_assign = as_bool value }
  | "hhvm.enable_xhp" ->
    { options with option_enable_xhp = as_bool value }
  | "hhvm.php7.uvs" ->
    { options with option_php7_uvs = as_bool value }
  | "hack.compiler.relabel" ->
    { options with option_relabel = as_bool value }
  | "eval.createinoutwrapperfunctions" ->
    { options with option_create_inout_wrapper_functions = as_bool value }
  | "eval.reffinessinvariance" ->
    { options with option_reffiness_invariance = as_bool value }
  | "eval.throwoncallbyrefannotationmismatch" ->
    { options with option_throw_on_call_by_ref_annotation_mismatch = as_bool value }
  | "eval.hackarrcompatnotices" ->
    { options with option_hack_arr_compat_notices = as_bool value }
  | "eval.hackarrdvarrs" ->
    { options with option_hack_arr_dv_arrs = as_bool value }
  | "hhvm.repo_authoritative" ->
    { options with option_repo_authoritative = as_bool value }
  | "eval.jitenablerenamefunction" ->
    { options with option_jit_enable_rename_function = as_bool value }
  | "eval.disablehphpcopts" ->
    let v = not (as_bool value) in
    { options with option_constant_folding = v;
                   option_can_inline_gen_functions = v}
  | "hhvm.php7.int_semantics" ->
    { options with option_php7_int_semantics = as_bool value }
  | "hack.lang.autoprimegenerators" ->
    { options with option_autoprime_generators = as_bool value }
  | "hack.lang.enableisexprprimitivemigration" ->
    { options with option_enable_is_expr_primitive_migration = as_bool value }
  | "hack.lang.enablecoroutines" ->
    { options with option_enable_coroutines = as_bool value }
  | "hack.lang.hacksperimental" ->
    { options with option_hacksperimental = as_bool value }
  | "eval.logexterncompilerperf" ->
    { options with option_enable_perf_logging = as_bool value }
  | "hhvm.disable_return_by_reference" ->
    { options with option_disable_return_by_reference = as_bool value}
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

let get_value_from_config_string config key =
  get_value_from_config_ config key |> Option.map ~f:J.get_string_exn

let get_value_from_config_kv_list config key =
  let json_opt = get_value_from_config_ config key in
  Option.map json_opt ~f:(fun json ->
    match json with
    | J.JSON_Array [] -> []
    | json ->
    let keys_with_json = J.get_object_exn json in
    List.map ~f:(fun (k, v) -> k, J.get_string_exn v) keys_with_json)

let get_value_from_config_string_array config key =
  let json_opt = get_value_from_config_ config key in
  match json_opt with
    | None                   -> None
    | Some (J.JSON_Array fs) -> Some (List.map fs J.get_string_exn)
    | _                      -> raise (Arg.Bad ("Expected list of strings at " ^ key))

let get_value_from_config_string_to_string_map config key =
  let json_opt = get_value_from_config_ config key in
  match json_opt with
    | None -> None
    | Some (J.JSON_Object fs) -> Some (
      List.fold_left fs ~init:SMap.empty ~f:(fun acc json ->
        match json with
          | (root, J.JSON_String path) -> SMap.add root path acc
          | _ -> raise (Arg.Bad ("Expected {root:path} pair"))))
    | _ -> raise (Arg.Bad ("Expected string-to-string map strings at " ^ key))

let set_value name get set config opts =
  try
    let value = get config name in
    Option.value_map value ~default:opts ~f:(set opts)
  with
    | Arg.Bad why      -> raise (Arg.Bad ("Option " ^ name ^ ": " ^ why))
    | Assert_failure _ -> raise (Arg.Bad ("Option " ^ name ^ ": error parsing JSON"))

let value_setters = [
  (set_value "hhvm.aliased_namespaces" get_value_from_config_kv_list @@
    fun opts v -> { opts with option_aliased_namespaces = Some v });
  (set_value "hhvm.force_hh" get_value_from_config_int @@
    fun opts v -> { opts with option_enable_hiphop_syntax = (v = 1) });
  (set_value "hhvm.enable_xhp" get_value_from_config_int @@
    fun opts v -> { opts with option_enable_xhp = (v = 1) });
  (set_value "hhvm.php7.scalar_types" get_value_from_config_int @@
    fun opts v -> { opts with option_php7_scalar_types = (v = 1) });
  (set_value "hhvm.hack.lang.ints_overflow_to_ints" get_value_from_config_int @@
    fun opts v -> { opts with option_ints_overflow_to_ints = Some (v = 1) });
  (set_value "hack.compiler.constant_folding" get_value_from_config_int @@
    fun opts v -> { opts with option_constant_folding = (v = 1) });
  (set_value "hack.compiler.optimize_null_checks" get_value_from_config_int @@
    fun opts v -> { opts with option_optimize_null_check = (v = 1) });
  (set_value "hhvm.disable_hphpc_opts" get_value_from_config_int @@
    fun opts v -> { opts with option_constant_folding = (v = 0);
                              option_can_inline_gen_functions = (v = 0) });
  (set_value "eval.disassembler_source_mapping" get_value_from_config_int @@
    fun opts v -> { opts with option_source_mapping = (v = 1) });
  (set_value "hhvm.php7.uvs" get_value_from_config_int @@
    fun opts v -> { opts with option_php7_uvs = (v = 1) });
  (set_value "hhvm.php7.ltr_assign" get_value_from_config_int @@
    fun opts v -> { opts with option_php7_ltr_assign = (v = 1) });
  (set_value "hhvm.create_in_out_wrapper_functions" get_value_from_config_int @@
    fun opts v -> { opts with option_create_inout_wrapper_functions = (v = 1)});
  (set_value "hhvm.reffiness_invariance" get_value_from_config_int @@
    fun opts v -> { opts with option_reffiness_invariance = (v = 1) });
  (set_value "hhvm.throw_on_call_by_ref_annotation_mismatch" get_value_from_config_int @@
    fun opts v -> { opts with option_throw_on_call_by_ref_annotation_mismatch = (v = 1) });
  (set_value "hhvm.hack_arr_compat_notices" get_value_from_config_int @@
    fun opts v -> { opts with option_hack_arr_compat_notices = (v = 1) });
  (set_value "hhvm.hack_arr_dv_arrs" get_value_from_config_int @@
    fun opts v -> { opts with option_hack_arr_dv_arrs = (v = 1) });
  (set_value "hhvm.dynamic_invoke_functions" get_value_from_config_string_array @@
    fun opts v -> {opts with option_dynamic_invoke_functions =
        SSet.of_list (List.map v String.lowercase_ascii)});
  (set_value "hhvm.repo.authoritative" get_value_from_config_int @@
    fun opts v -> { opts with option_repo_authoritative = (v = 1) });
  (set_value "hhvm.jit_enable_rename_function" get_value_from_config_int @@
    fun opts v -> { opts with option_jit_enable_rename_function = (v = 1) });
  (set_value "hhvm.php7.int_semantics" get_value_from_config_int @@
    fun opts v -> { opts with option_php7_int_semantics = (v = 1) });
  (set_value "hhvm.hack.lang.autoprime_generators" get_value_from_config_int @@
    fun opts v -> { opts with option_autoprime_generators = (v = 1) });
  (set_value "hhvm.hack.lang.enable_is_expr_primitive_migration" get_value_from_config_int @@
    fun opts v -> { opts with option_enable_is_expr_primitive_migration = (v = 1) });
  (set_value "hhvm.hack.lang.enable_coroutines" get_value_from_config_int @@
    fun opts v -> { opts with option_enable_coroutines = (v = 1) });
  (set_value "hhvm.hack.lang.hacksperimental" get_value_from_config_int @@
    fun opts v -> { opts with option_hacksperimental = (v = 1) });
  (set_value "doc_root" get_value_from_config_string @@
    fun opts v -> { opts with option_doc_root = v });
  (set_value "hhvm.server.include_search_paths" get_value_from_config_string_array @@
     fun opts v -> { opts with option_include_search_paths = v });
  (set_value "hhvm.include_roots" get_value_from_config_string_to_string_map @@
     fun opts v -> { opts with option_include_roots = v });
  (set_value "hhvm.log_extern_compiler_perf" get_value_from_config_int @@
     fun opts v -> { opts with option_enable_perf_logging = (v = 1) });
  (set_value "hhvm.disable_return_by_reference" get_value_from_config_int @@
     fun opts v -> { opts with option_disable_return_by_reference = (v = 1)});
]

let extract_config_options_from_json ~init config_json =
  match config_json with None -> init | Some config_json ->
  let config = J.get_object_exn config_json in
  List.fold_left value_setters ~init ~f:(fun opts setter -> setter config opts)

(* Construct an instance of Hhbc_options.t from the options passed in as well as
 * as specified in `-v str` on the command line.
 *)
let get_options_from_config ?(init=default) ?(config_list=[]) config_json =
  let init = extract_config_options_from_json ~init config_json in
  List.fold_left config_list ~init ~f:begin
    fun options str ->
    match Str.split_delim (Str.regexp "=") str with
    | [name; value] -> set_option options name value
    | _ -> options
  end

let compiler_options = ref default
let set_compiler_options o = compiler_options := o
