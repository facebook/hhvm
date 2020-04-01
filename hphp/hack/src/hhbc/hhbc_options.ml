(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
module J = Hh_json

(* Compiler configuration options, as set by -v key=value on the command line *)
type t = {
  option_constant_folding: bool;
  option_optimize_null_checks: bool;
  option_max_array_elem_size_on_the_stack: int;
  option_aliased_namespaces: (string * string) list;
  option_relabel: bool;
  option_php7_uvs: bool;
  option_php7_ltr_assign: bool;
  option_hack_arr_compat_notices: bool;
  option_hack_arr_dv_arrs: bool;
  option_dynamic_invoke_functions: SSet.t;
  option_repo_authoritative: bool;
  option_jit_enable_rename_function: bool;
  option_enable_coroutines: bool;
  option_doc_root: string;
  option_include_search_paths: string list;
  option_include_roots: string SMap.t;
  option_log_extern_compiler_perf: bool;
  option_enable_intrinsics_extension: bool;
  option_phpism_disallow_execution_operator: bool;
  option_phpism_disable_nontoplevel_declarations: bool;
  option_phpism_disable_static_closures: bool;
  option_emit_func_pointers: bool;
  option_emit_cls_meth_pointers: bool;
  option_emit_inst_meth_pointers: bool;
  option_emit_meth_caller_func_pointers: bool;
  option_rx_is_enabled: bool;
  option_disable_lval_as_an_expression: bool;
  option_enable_pocket_universes: bool;
  option_array_provenance: bool;
  option_enable_class_level_where_clauses: bool;
  option_disable_legacy_soft_typehints: bool;
  option_allow_new_attribute_syntax: bool;
  option_disable_legacy_attribute_syntax: bool;
  option_const_default_func_args: bool;
  option_const_static_props: bool;
  option_abstract_static_props: bool;
  option_disable_unset_class_const: bool;
  option_disallow_func_ptrs_in_constants: bool;
  option_emit_generics_ub: bool;
  option_check_int_overflow: bool;
  option_disable_xhp_element_mangling: bool;
  option_disable_xhp_children_declarations: bool;
  option_enable_xhp_class_modifier: bool;
  option_rust_top_level_elaborator: bool;
  option_enable_first_class_function_pointers: bool;
  option_widen_is_array: bool;
  option_disable_partial: bool;
  option_disable_array: bool;
}

let default =
  {
    option_constant_folding = true;
    option_optimize_null_checks = false;
    option_max_array_elem_size_on_the_stack = 64;
    option_aliased_namespaces = [];
    option_php7_uvs = false;
    option_php7_ltr_assign = false;
    (* If true, then renumber labels after generating code for a method
     * body. Semantic diff doesn't care about labels, but for visual diff against
     * HHVM it's helpful to renumber in order that the labels match more closely *)
    option_relabel = true;
    option_hack_arr_compat_notices = false;
    option_hack_arr_dv_arrs = false;
    option_dynamic_invoke_functions = SSet.empty;
    option_repo_authoritative = false;
    option_jit_enable_rename_function = false;
    option_enable_coroutines = true;
    option_doc_root = "";
    option_include_search_paths = [];
    option_include_roots = SMap.empty;
    option_log_extern_compiler_perf = false;
    option_enable_intrinsics_extension = false;
    option_phpism_disallow_execution_operator = false;
    option_phpism_disable_nontoplevel_declarations = false;
    option_phpism_disable_static_closures = false;
    option_emit_func_pointers = true;
    option_emit_cls_meth_pointers = true;
    option_emit_inst_meth_pointers = true;
    option_emit_meth_caller_func_pointers = true;
    option_rx_is_enabled = false;
    option_disable_lval_as_an_expression = false;
    option_enable_pocket_universes = false;
    option_array_provenance = false;
    option_enable_class_level_where_clauses = false;
    option_disable_legacy_soft_typehints = false;
    option_allow_new_attribute_syntax = false;
    option_disable_legacy_attribute_syntax = false;
    option_const_default_func_args = false;
    option_const_static_props = false;
    option_abstract_static_props = false;
    option_disable_unset_class_const = false;
    option_disallow_func_ptrs_in_constants = false;
    option_emit_generics_ub = false;
    option_check_int_overflow = false;
    option_disable_xhp_element_mangling = false;
    option_disable_xhp_children_declarations = false;
    option_enable_xhp_class_modifier = false;
    option_rust_top_level_elaborator = true;
    option_enable_first_class_function_pointers = false;
    option_widen_is_array = false;
    option_disable_partial = false;
    option_disable_array = false;
  }

let constant_folding o = o.option_constant_folding

let optimize_null_checks o = o.option_optimize_null_checks

let max_array_elem_size_on_the_stack o =
  o.option_max_array_elem_size_on_the_stack

let aliased_namespaces o = o.option_aliased_namespaces

let relabel o = o.option_relabel

let enable_uniform_variable_syntax o = o.option_php7_uvs

let php7_ltr_assign o = o.option_php7_ltr_assign

let hack_arr_compat_notices o = o.option_hack_arr_compat_notices

let hack_arr_dv_arrs o = o.option_hack_arr_dv_arrs

let dynamic_invoke_functions o = o.option_dynamic_invoke_functions

let repo_authoritative o = o.option_repo_authoritative

let jit_enable_rename_function o = o.option_jit_enable_rename_function

let enable_coroutines o = o.option_enable_coroutines

let doc_root o = o.option_doc_root

let include_search_paths o = o.option_include_search_paths

let include_roots o = o.option_include_roots

let log_extern_compiler_perf o = o.option_log_extern_compiler_perf

let enable_intrinsics_extension o = o.option_enable_intrinsics_extension

let phpism_disallow_execution_operator o =
  o.option_phpism_disallow_execution_operator

let phpism_disable_nontoplevel_declarations o =
  o.option_phpism_disable_nontoplevel_declarations

let phpism_disable_static_closures o = o.option_phpism_disable_static_closures

let emit_func_pointers o = o.option_emit_func_pointers

let emit_cls_meth_pointers o = o.option_emit_cls_meth_pointers

let emit_inst_meth_pointers o = o.option_emit_inst_meth_pointers

let emit_meth_caller_func_pointers o = o.option_emit_meth_caller_func_pointers

let rx_is_enabled o = o.option_rx_is_enabled

let disable_lval_as_an_expression o = o.option_disable_lval_as_an_expression

let enable_pocket_universes o = o.option_enable_pocket_universes

let array_provenance o = o.option_array_provenance

let enable_class_level_where_clauses o =
  o.option_enable_class_level_where_clauses

let disable_legacy_soft_typehints o = o.option_disable_legacy_soft_typehints

let allow_new_attribute_syntax o = o.option_allow_new_attribute_syntax

let disable_legacy_attribute_syntax o = o.option_disable_legacy_attribute_syntax

let const_default_func_args o = o.option_const_default_func_args

let const_static_props o = o.option_const_static_props

let abstract_static_props o = o.option_abstract_static_props

let disable_unset_class_const o = o.option_disable_unset_class_const

let disallow_func_ptrs_in_constants o = o.option_disallow_func_ptrs_in_constants

let disable_xhp_element_mangling o = o.option_disable_xhp_element_mangling

let disable_xhp_children_declarations o =
  o.option_disable_xhp_children_declarations

let enable_xhp_class_modifier o = o.option_enable_xhp_class_modifier

let emit_generics_ub o = o.option_emit_generics_ub

let check_int_overflow o = o.option_check_int_overflow

let rust_top_level_elaborator o = o.option_rust_top_level_elaborator

let enable_first_class_function_pointers o =
  o.option_enable_first_class_function_pointers

let disable_modes o = o.option_disable_partial

let disable_array o = o.option_disable_array

(**
 * Widen the default behavior of `is_array` from "is exactly a PHP array to"
 * "is any kind of CoW container" (i.e. `HH\is_any_array`)
 *)
let widen_is_array o = o.option_widen_is_array

let to_string o =
  let aliased_namespaces_str =
    aliased_namespaces o
    |> List.map ~f:(fun (fst, snd) -> Printf.sprintf "(%s, %s)" fst snd)
    |> String.concat ~sep:", "
  in
  let dynamic_invokes =
    String.concat ~sep:", " (SSet.elements (dynamic_invoke_functions o))
  in
  let search_paths = String.concat ~sep:", " (include_search_paths o) in
  let inc_roots =
    SMap.bindings (include_roots o)
    |> List.map ~f:(fun (root, path) -> root ^ ": " ^ path)
    |> String.concat ~sep:", "
  in
  String.concat
    ~sep:"\n"
    [
      Printf.sprintf "constant_folding: %B" @@ constant_folding o;
      Printf.sprintf "optimize_null_checks: %B" @@ optimize_null_checks o;
      Printf.sprintf "max_array_elem_size_on_the_stack: %d"
      @@ max_array_elem_size_on_the_stack o;
      Printf.sprintf "aliased_namespaces: %s" aliased_namespaces_str;
      Printf.sprintf "relabel: %B" @@ relabel o;
      Printf.sprintf "enable_uniform_variable_syntax: %B"
      @@ enable_uniform_variable_syntax o;
      Printf.sprintf "php7_ltr_assign: %B" @@ php7_ltr_assign o;
      Printf.sprintf "hack_arr_compat_notices: %B" @@ hack_arr_compat_notices o;
      Printf.sprintf "hack_arr_dv_arrs: %B" @@ hack_arr_dv_arrs o;
      Printf.sprintf "dynamic_invoke_functions: [%s]" dynamic_invokes;
      Printf.sprintf "repo_authoritative: %B" @@ repo_authoritative o;
      Printf.sprintf "jit_enable_rename_function: %B"
      @@ jit_enable_rename_function o;
      Printf.sprintf "enable_coroutines: %B" @@ enable_coroutines o;
      Printf.sprintf "doc_root: %s" @@ doc_root o;
      Printf.sprintf "include_search_paths: [%s]" search_paths;
      Printf.sprintf "include_roots: {%s}" inc_roots;
      Printf.sprintf "log_extern_compiler_perf: %B"
      @@ log_extern_compiler_perf o;
      Printf.sprintf "enable_intrinsics_extension: %B"
      @@ enable_intrinsics_extension o;
      Printf.sprintf "phpism_disallow_execution_operator: %B"
      @@ phpism_disallow_execution_operator o;
      Printf.sprintf "phpism_disable_nontoplevel_declarations: %B"
      @@ phpism_disable_nontoplevel_declarations o;
      Printf.sprintf "phpism_disable_static_closures: %B"
      @@ phpism_disable_static_closures o;
      Printf.sprintf "emit_func_pointers: %B" @@ emit_func_pointers o;
      Printf.sprintf "emit_cls_meth_pointers: %B" @@ emit_cls_meth_pointers o;
      Printf.sprintf "emit_inst_meth_pointers: %B" @@ emit_inst_meth_pointers o;
      Printf.sprintf "emit_meth_caller_func_pointers: %B"
      @@ emit_meth_caller_func_pointers o;
      Printf.sprintf "rx_is_enabled: %B" @@ rx_is_enabled o;
      Printf.sprintf "disable_lval_as_an_expression: %B"
      @@ disable_lval_as_an_expression o;
      Printf.sprintf "enable_pocket_universes: %B" @@ enable_pocket_universes o;
      Printf.sprintf "array_provenance: %B" @@ array_provenance o;
      Printf.sprintf "enable_class_level_where_clauses: %B"
      @@ enable_class_level_where_clauses o;
      Printf.sprintf "disable_legacy_soft_typehints: %B"
      @@ disable_legacy_soft_typehints o;
      Printf.sprintf "allow_new_attribute_syntax: %B"
      @@ allow_new_attribute_syntax o;
      Printf.sprintf "disable_legacy_attribute_syntax: %B"
      @@ disable_legacy_attribute_syntax o;
      Printf.sprintf "const_default_func_args: %B" @@ const_default_func_args o;
      Printf.sprintf "const_static_props: %B" @@ const_static_props o;
      Printf.sprintf "abstract_static_props: %B" @@ abstract_static_props o;
      Printf.sprintf "disable_unset_class_const: %B"
      @@ disable_unset_class_const o;
      Printf.sprintf "disallow_func_ptrs_in_constants: %B"
      @@ disallow_func_ptrs_in_constants o;
      Printf.sprintf "emit_generics_ub: %B" @@ emit_generics_ub o;
      Printf.sprintf "check_int_overflow: %B" @@ check_int_overflow o;
      Printf.sprintf "enable_xhp_class_modifier: %B"
      @@ enable_xhp_class_modifier o;
      Printf.sprintf "disable_xhp_element_mangling: %B"
      @@ disable_xhp_element_mangling o;
      Printf.sprintf "rust_top_level_elaborator: %B"
      @@ rust_top_level_elaborator o;
      Printf.sprintf "disable_array: %B" @@ disable_array o;
    ]

let as_bool s =
  match String.lowercase s with
  | "0"
  | "false" ->
    false
  | "1"
  | "true" ->
    true
  | _ -> raise (Arg.Bad (s ^ " can't be cast to bool"))

let set_of_csv_string s =
  SSet.of_list @@ String.split ~on:(Char.of_string ",") s

let set_option options name value =
  match String.lowercase name with
  | "hack.compiler.constantfolding" ->
    { options with option_constant_folding = as_bool value }
  | "hack.compiler.optimizenullcheck" ->
    { options with option_optimize_null_checks = as_bool value }
  | "hhvm.php7.ltr_assign" ->
    { options with option_php7_ltr_assign = as_bool value }
  | "hhvm.php7.uvs" -> { options with option_php7_uvs = as_bool value }
  | "hack.compiler.relabel" -> { options with option_relabel = as_bool value }
  | "eval.hackarrcompatnotices" ->
    { options with option_hack_arr_compat_notices = as_bool value }
  | "eval.hackarrdvarrs" ->
    { options with option_hack_arr_dv_arrs = as_bool value }
  | "hhvm.repo_authoritative" ->
    { options with option_repo_authoritative = as_bool value }
  | "eval.jitenablerenamefunction" ->
    { options with option_jit_enable_rename_function = as_bool value }
  | "hack.lang.enablecoroutines" ->
    { options with option_enable_coroutines = as_bool value }
  | "eval.logexterncompilerperf" ->
    { options with option_log_extern_compiler_perf = as_bool value }
  | "eval.enableintrinsicsextension" ->
    { options with option_enable_intrinsics_extension = as_bool value }
  | "hack.lang.phpism.disallowexecutionoperator" ->
    { options with option_phpism_disallow_execution_operator = as_bool value }
  | "hack.lang.phpism.disablenontopleveldeclarations" ->
    {
      options with
      option_phpism_disable_nontoplevel_declarations = as_bool value;
    }
  | "hack.lang.phpism.disablestaticclosures" ->
    { options with option_phpism_disable_static_closures = as_bool value }
  | "hhvm.emit_func_pointers" ->
    { options with option_emit_func_pointers = int_of_string value > 0 }
  | "hhvm.emit_cls_meth_pointers" ->
    { options with option_emit_cls_meth_pointers = int_of_string value > 0 }
  | "hhvm.emit_inst_meth_pointers" ->
    { options with option_emit_inst_meth_pointers = int_of_string value > 0 }
  | "hhvm.emit_meth_caller_func_pointers" ->
    {
      options with
      option_emit_meth_caller_func_pointers = int_of_string value > 0;
    }
  | "hhvm.rx_is_enabled" ->
    { options with option_rx_is_enabled = int_of_string value > 0 }
  | "hack.lang.disable_lval_as_an_expression" ->
    { options with option_disable_lval_as_an_expression = as_bool value }
  | "hack.lang.enablepocketuniverses" ->
    { options with option_enable_pocket_universes = as_bool value }
  | "hhvm.array_provenance" ->
    { options with option_array_provenance = as_bool value }
  | "hhvm.lang.enable_class_level_where_clauses" ->
    { options with option_enable_class_level_where_clauses = as_bool value }
  | "hhvm.lang.disable_legacy_soft_typehints" ->
    { options with option_disable_legacy_soft_typehints = as_bool value }
  | "hhvm.lang.allow_new_attribute_syntax" ->
    { options with option_allow_new_attribute_syntax = as_bool value }
  | "hhvm.lang.disable_legacy_attribute_syntax" ->
    { options with option_disable_legacy_attribute_syntax = as_bool value }
  | "hhvm.lang.constdefaultfuncargs" ->
    { options with option_const_default_func_args = as_bool value }
  | "hhvm.lang.conststaticprops" ->
    { options with option_const_static_props = as_bool value }
  | "hhvm.lang.abstractstaticprops" ->
    { options with option_abstract_static_props = as_bool value }
  | "hhvm.lang.disableunsetclassconst" ->
    { options with option_disable_unset_class_const = as_bool value }
  | "hhvm.lang.disallow_func_ptrs_in_constants" ->
    { options with option_disallow_func_ptrs_in_constants = as_bool value }
  | "hhvm.hack.lang.enable_xhp_class_modifier" ->
    { options with option_enable_xhp_class_modifier = as_bool value }
  | "hhvm.emit_generics_ub" ->
    { options with option_emit_generics_ub = as_bool value }
  | "hhvm.hack.lang.disable_xhp_element_mangling" ->
    { options with option_disable_xhp_element_mangling = as_bool value }
  | "hhvm.hack.lang.check_int_overflow" ->
    { options with option_check_int_overflow = int_of_string value > 0 }
  | "hhvm.hack.lang.rust_top_level_elaborator" ->
    { options with option_rust_top_level_elaborator = as_bool value }
  | "hhvm.hack.lang.enable_first_class_function_pointers" ->
    {
      options with
      option_enable_first_class_function_pointers = int_of_string value > 0;
    }
  | "hhvm.widen_is_array" ->
    { options with option_widen_is_array = as_bool value }
  | "hhvm.hack.lang.disable_array" ->
    { options with option_disable_array = as_bool value }
  | _ -> options

let get_value_from_config_ config key =
  let f k1 (k2, _) = k1 = k2 in
  match List.find config ~f:(f key) with
  | None -> None
  | Some (_, json) ->
    begin
      match List.find (J.get_object_exn json) ~f:(f "global_value") with
      | None -> None
      | Some (_, json) -> Some json
    end

let get_value_from_config_int config key =
  let json_opt = get_value_from_config_ config key in
  Option.map json_opt ~f:(function
      | J.JSON_String s
      | J.JSON_Number s ->
        (match s with
        | "" -> 0
        | x -> int_of_string x)
      | J.JSON_Bool s ->
        (* Boolean options are currently parsed as ints; for migration to Rust,
         * it is important they can be parsed from JSON bools as well *)
        if s then
          1
        else
          0
      | _ -> raise (Arg.Bad ("option can't be cast to integer: " ^ key)))

let get_value_from_config_string config key =
  get_value_from_config_ config key |> Option.map ~f:J.get_string_exn

let get_value_from_config_kv_list config key =
  let json_opt = get_value_from_config_ config key in
  Option.map json_opt ~f:(fun json ->
      match json with
      | J.JSON_Array [] -> []
      | json ->
        let keys_with_json = J.get_object_exn json in
        List.map ~f:(fun (k, v) -> (k, J.get_string_exn v)) keys_with_json)

let get_value_from_config_string_array config key =
  let json_opt = get_value_from_config_ config key in
  match json_opt with
  | None -> None
  | Some (J.JSON_Array fs) -> Some (List.map fs J.get_string_exn)
  | _ -> raise (Arg.Bad ("Expected list of strings at " ^ key))

let get_value_from_config_string_to_string_map config key =
  let json_opt = get_value_from_config_ config key in
  match json_opt with
  | None -> None
  | Some (J.JSON_Object fs) ->
    Some
      (List.fold_left fs ~init:SMap.empty ~f:(fun acc json ->
           match json with
           | (root, J.JSON_String path) -> SMap.add root path acc
           | _ -> raise (Arg.Bad "Expected {root:path} pair")))
  | _ -> raise (Arg.Bad ("Expected string-to-string map strings at " ^ key))

let set_value name get set config opts =
  try
    let value = get config name in
    Option.value_map value ~default:opts ~f:(set opts)
  with
  | Arg.Bad why -> raise (Arg.Bad ("Option " ^ name ^ ": " ^ why))
  | Assert_failure _ ->
    raise (Arg.Bad ("Option " ^ name ^ ": error parsing JSON"))

let value_setters =
  [
    ( set_value "hhvm.aliased_namespaces" get_value_from_config_kv_list
    @@ fun opts v -> { opts with option_aliased_namespaces = v } );
    ( set_value "hack.compiler.constant_folding" get_value_from_config_int
    @@ fun opts v -> { opts with option_constant_folding = v = 1 } );
    ( set_value "hack.compiler.optimize_null_checks" get_value_from_config_int
    @@ fun opts v -> { opts with option_optimize_null_checks = v = 1 } );
    ( set_value "hhvm.php7.uvs" get_value_from_config_int @@ fun opts v ->
      { opts with option_php7_uvs = v = 1 } );
    ( set_value "hhvm.php7.ltr_assign" get_value_from_config_int @@ fun opts v ->
      { opts with option_php7_ltr_assign = v = 1 } );
    ( set_value "hhvm.hack_arr_compat_notices" get_value_from_config_int
    @@ fun opts v -> { opts with option_hack_arr_compat_notices = v = 1 } );
    ( set_value "hhvm.hack_arr_dv_arrs" get_value_from_config_int
    @@ fun opts v -> { opts with option_hack_arr_dv_arrs = v = 1 } );
    ( set_value
        "hhvm.dynamic_invoke_functions"
        get_value_from_config_string_array
    @@ fun opts v ->
      {
        opts with
        option_dynamic_invoke_functions =
          SSet.of_list (List.map v String.lowercase);
      } );
    ( set_value "hhvm.repo.authoritative" get_value_from_config_int
    @@ fun opts v -> { opts with option_repo_authoritative = v = 1 } );
    ( set_value "hhvm.jit_enable_rename_function" get_value_from_config_int
    @@ fun opts v -> { opts with option_jit_enable_rename_function = v = 1 } );
    ( set_value "hhvm.hack.lang.enable_coroutines" get_value_from_config_int
    @@ fun opts v -> { opts with option_enable_coroutines = v = 1 } );
    ( set_value
        "hhvm.hack.lang.disable_lval_as_an_expression"
        get_value_from_config_int
    @@ fun opts v -> { opts with option_disable_lval_as_an_expression = v = 1 }
    );
    ( set_value
        "hhvm.hack.lang.enable_pocket_universes"
        get_value_from_config_int
    @@ fun opts v -> { opts with option_enable_pocket_universes = v = 1 } );
    ( set_value
        "hhvm.hack.lang.disable_xhp_element_mangling"
        get_value_from_config_int
    @@ fun opts v -> { opts with option_disable_xhp_element_mangling = v = 1 }
    );
    ( set_value
        "hhvm.hack.lang.enable_xhp_class_modifier"
        get_value_from_config_int
    @@ fun opts v -> { opts with option_enable_xhp_class_modifier = v = 1 } );
    ( set_value "doc_root" get_value_from_config_string @@ fun opts v ->
      { opts with option_doc_root = v } );
    ( set_value
        "hhvm.server.include_search_paths"
        get_value_from_config_string_array
    @@ fun opts v -> { opts with option_include_search_paths = v } );
    ( set_value "hhvm.include_roots" get_value_from_config_string_to_string_map
    @@ fun opts v -> { opts with option_include_roots = v } );
    ( set_value "hhvm.log_extern_compiler_perf" get_value_from_config_int
    @@ fun opts v -> { opts with option_log_extern_compiler_perf = v = 1 } );
    ( set_value "hhvm.enable_intrinsics_extension" get_value_from_config_int
    @@ fun opts v -> { opts with option_enable_intrinsics_extension = v = 1 } );
    ( set_value
        "hhvm.hack.lang.phpism.disallow_execution_operator"
        get_value_from_config_int
    @@ fun opts v ->
      { opts with option_phpism_disallow_execution_operator = v = 1 } );
    ( set_value
        "hhvm.hack.lang.phpism.disable_nontoplevel_declarations"
        get_value_from_config_int
    @@ fun opts v ->
      { opts with option_phpism_disable_nontoplevel_declarations = v = 1 } );
    ( set_value
        "hhvm.hack.lang.phpism.disable_static_closures"
        get_value_from_config_int
    @@ fun opts v -> { opts with option_phpism_disable_static_closures = v = 1 }
    );
    ( set_value "hhvm.emit_func_pointers" get_value_from_config_int
    @@ fun opts v -> { opts with option_emit_func_pointers = v > 0 } );
    ( set_value "hhvm.emit_cls_meth_pointers" get_value_from_config_int
    @@ fun opts v -> { opts with option_emit_cls_meth_pointers = v > 0 } );
    ( set_value "hhvm.emit_meth_caller_func_pointers" get_value_from_config_int
    @@ fun opts v -> { opts with option_emit_meth_caller_func_pointers = v > 0 }
    );
    ( set_value "hhvm.emit_inst_meth_pointers" get_value_from_config_int
    @@ fun opts v -> { opts with option_emit_inst_meth_pointers = v > 0 } );
    ( set_value "hhvm.rx_is_enabled" get_value_from_config_int @@ fun opts v ->
      { opts with option_rx_is_enabled = v > 0 } );
    ( set_value "hhvm.array_provenance" get_value_from_config_int
    @@ fun opts v -> { opts with option_array_provenance = v > 0 } );
    ( set_value
        "hhvm.hack.lang.enable_class_level_where_clauses"
        get_value_from_config_int
    @@ fun opts v ->
      { opts with option_enable_class_level_where_clauses = v = 1 } );
    ( set_value
        "hhvm.hack.lang.disable_legacy_soft_typehints"
        get_value_from_config_int
    @@ fun opts v -> { opts with option_disable_legacy_soft_typehints = v = 1 }
    );
    ( set_value
        "hhvm.hack.lang.allow_new_attribute_syntax"
        get_value_from_config_int
    @@ fun opts v -> { opts with option_allow_new_attribute_syntax = v = 1 } );
    ( set_value
        "hhvm.hack.lang.disable_legacy_attribute_syntax"
        get_value_from_config_int
    @@ fun opts v ->
      { opts with option_disable_legacy_attribute_syntax = v = 1 } );
    ( set_value
        "hhvm.hack.lang.const_default_func_args"
        get_value_from_config_int
    @@ fun opts v -> { opts with option_const_default_func_args = v = 1 } );
    ( set_value "hhvm.hack.lang.const_static_props" get_value_from_config_int
    @@ fun opts v -> { opts with option_const_static_props = v = 1 } );
    ( set_value "hhvm.hack.lang.abstract_static_props" get_value_from_config_int
    @@ fun opts v -> { opts with option_abstract_static_props = v = 1 } );
    ( set_value
        "hhvm.hack.lang.disable_unset_class_const"
        get_value_from_config_int
    @@ fun opts v -> { opts with option_disable_unset_class_const = v = 1 } );
    ( set_value
        "hhvm.hack.lang.disallow_func_ptrs_in_constants"
        get_value_from_config_int
    @@ fun opts v ->
      { opts with option_disallow_func_ptrs_in_constants = v = 1 } );
    ( set_value "hhvm.emit_generics_ub" get_value_from_config_int
    @@ fun opts v -> { opts with option_emit_generics_ub = v > 0 } );
    ( set_value "hhvm.hack.lang.check_int_overflow" get_value_from_config_int
    @@ fun opts v -> { opts with option_check_int_overflow = v > 0 } );
    ( set_value
        "hhvm.hack.lang.rust_top_level_elaborator"
        get_value_from_config_int
    @@ fun opts v -> { opts with option_rust_top_level_elaborator = v = 1 } );
    ( set_value
        "hhvm.hack.lang.enable_first_class_function_pointers"
        get_value_from_config_int
    @@ fun opts v ->
      { opts with option_enable_first_class_function_pointers = v = 1 } );
    ( set_value "hhvm.widen_is_array" get_value_from_config_int @@ fun opts v ->
      { opts with option_widen_is_array = v = 1 } );
    ( set_value "hhvm.hack.lang.disable_array" get_value_from_config_int
    @@ fun opts v -> { opts with option_disable_array = v = 1 } );
  ]

let extract_config_options_from_json ~init config_json =
  match config_json with
  | None -> init
  | Some config_json ->
    let config = J.get_object_exn config_json in
    List.fold_left value_setters ~init ~f:(fun opts setter ->
        setter config opts)

(* Apply overrides by parsing CLI arguments in format `-vNAME=VALUE` *)
let override_from_cli config_list init =
  List.fold_left config_list ~init ~f:(fun options str ->
      match Str.split_delim (Str.regexp "=") str with
      | [name; value] -> set_option options name value
      | _ -> options)

(* Construct an instance of Hhbc_options.t from the options passed in as well as
 * as specified in `-v str` on the command line.
 *)
let apply_config_overrides_statelessly config_list config_jsons =
  List.fold_right
    ~init:default
    ~f:(fun config_json init ->
      extract_config_options_from_json
        ~init
        (Some (J.json_of_string config_json)))
    config_jsons
  |> override_from_cli config_list

let compiler_options = ref default

let set_compiler_options o = compiler_options := o
