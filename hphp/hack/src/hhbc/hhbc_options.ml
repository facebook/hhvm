(**
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
  option_hack_arr_compat_notices          : bool;
  option_hack_arr_dv_arrs                 : bool;
  option_dynamic_invoke_functions         : SSet.t;
  option_repo_authoritative               : bool;
  option_jit_enable_rename_function       : bool;
  option_can_inline_gen_functions         : bool;
  option_php7_int_semantics               : bool;
  option_enable_is_expr_primitive_migration : bool;
  option_enable_coroutines                : bool;
  option_hacksperimental                  : bool;
  option_doc_root                         : string;
  option_include_search_paths             : string list;
  option_include_roots                    : string SMap.t;
  option_enable_perf_logging              : bool;
  option_enable_reified_generics          : bool;
  option_enable_intrinsics_extension      : bool;
  option_enable_hhjs                      : bool;
  option_dump_hhjs                        : bool;
  option_hhjs_additional_transform        : string;
  option_hhjs_unique_filenames            : bool;
  option_hhjs_babel_transform             : string;
  option_hhjs_node_modules                : SSet.t;
  option_enable_concurrent                : bool;
  option_enable_await_as_an_expression    : bool;
  option_phpism_disallow_execution_operator: bool;
  option_phpism_disable_nontoplevel_declarations : bool;
  option_phpism_disable_static_closures : bool;
  option_phpism_disable_instanceof        : bool;
  option_emit_func_pointers               : bool;
  option_emit_cls_meth_pointers           : bool;
  option_emit_inst_meth_pointers          : bool;
  option_emit_meth_caller_func_pointers   : bool;
  option_rx_is_enabled                    : bool;
  option_enable_stronger_await_binding    : bool;
  option_disable_lval_as_an_expression    : bool;
  option_enable_pocket_universes          : bool;
  option_notice_on_byref_argument_typehint_violation : bool;
}

let default = {
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
  option_hack_arr_compat_notices = false;
  option_hack_arr_dv_arrs = false;
  option_dynamic_invoke_functions = SSet.empty;
  option_repo_authoritative = false;
  option_jit_enable_rename_function = false;
  option_can_inline_gen_functions = true;
  option_php7_int_semantics = false;
  option_enable_is_expr_primitive_migration = true;
  option_enable_coroutines = true;
  option_hacksperimental = false;
  option_doc_root = "";
  option_include_search_paths = [];
  option_include_roots = SMap.empty;
  option_enable_perf_logging = false;
  option_enable_reified_generics = false;
  option_enable_intrinsics_extension = false;
  option_enable_hhjs = false;
  option_dump_hhjs = false;
  option_hhjs_additional_transform = "";
  option_hhjs_unique_filenames = true;
  option_hhjs_babel_transform = "";
  option_hhjs_node_modules = SSet.empty;
  option_enable_concurrent = false;
  option_enable_await_as_an_expression = false;
  option_phpism_disallow_execution_operator = false;
  option_phpism_disable_nontoplevel_declarations = false;
  option_phpism_disable_static_closures = false;
  option_phpism_disable_instanceof = false;
  option_emit_func_pointers = true;
  option_emit_cls_meth_pointers = true;
  option_emit_inst_meth_pointers = true;
  option_emit_meth_caller_func_pointers = true;
  option_rx_is_enabled = false;
  option_enable_stronger_await_binding = false;
  option_disable_lval_as_an_expression = false;
  option_enable_pocket_universes = false;
  option_notice_on_byref_argument_typehint_violation = false;
}

let enable_hiphop_syntax o = o.option_enable_hiphop_syntax
let php7_scalar_types o = o.option_php7_scalar_types
let enable_xhp o = o.option_enable_xhp
let constant_folding o = o.option_constant_folding
let optimize_null_check o = o.option_optimize_null_check
let max_array_elem_size_on_the_stack o =
  o.option_max_array_elem_size_on_the_stack
let aliased_namespaces o = Option.value o.option_aliased_namespaces ~default:[]
let source_mapping o = o.option_source_mapping
let relabel o = o.option_relabel
let enable_uniform_variable_syntax o = o.option_php7_uvs
let php7_ltr_assign o = o.option_php7_ltr_assign
let create_inout_wrapper_functions o = o.option_create_inout_wrapper_functions
let reffiness_invariance o = o.option_reffiness_invariance
let hack_arr_compat_notices o = o.option_hack_arr_compat_notices
let hack_arr_dv_arrs o = o.option_hack_arr_dv_arrs
let dynamic_invoke_functions o = o.option_dynamic_invoke_functions
let repo_authoritative o = o.option_repo_authoritative
let jit_enable_rename_function o = o.option_jit_enable_rename_function
let can_inline_gen_functions o = o.option_can_inline_gen_functions
let php7_int_semantics o = o.option_php7_int_semantics
let enable_is_expr_primitive_migration o = o.option_enable_is_expr_primitive_migration
let enable_coroutines o = o.option_enable_coroutines
let hacksperimental o = o.option_hacksperimental
let doc_root o = o.option_doc_root
let include_search_paths o = o.option_include_search_paths
let include_roots o = o.option_include_roots
let enable_perf_logging o = o.option_enable_perf_logging
let enable_reified_generics o = o.option_enable_reified_generics
let enable_intrinsics_extension o = o.option_enable_intrinsics_extension
let enable_hhjs o = o.option_enable_hhjs
let dump_hhjs o = o.option_dump_hhjs
let hhjs_additional_transform o = o.option_hhjs_additional_transform
let hhjs_unique_filenames o = o.option_hhjs_unique_filenames
let hhjs_babel_transform o = o.option_hhjs_babel_transform
let hhjs_node_modules o = o.option_hhjs_node_modules
let enable_concurrent o = o.option_enable_concurrent
let enable_await_as_an_expression o = o.option_enable_await_as_an_expression
let phpism_disallow_execution_operator o = o.option_phpism_disallow_execution_operator
let phpism_disable_nontoplevel_declarations o = o.option_phpism_disable_nontoplevel_declarations
let phpism_disable_static_closures o = o.option_phpism_disable_static_closures
let phpism_disable_instanceof o = o.option_phpism_disable_instanceof
let emit_func_pointers o = o.option_emit_func_pointers
let emit_cls_meth_pointers o = o.option_emit_cls_meth_pointers
let emit_inst_meth_pointers o = o.option_emit_inst_meth_pointers
let emit_meth_caller_func_pointers o = o.option_emit_meth_caller_func_pointers
let rx_is_enabled o = o.option_rx_is_enabled
let enable_stronger_await_binding o = o.option_enable_stronger_await_binding
let disable_lval_as_an_expression o = o.option_disable_lval_as_an_expression
let enable_pocket_universes o = o.option_enable_pocket_universes
let notice_on_byref_argument_typehint_violation o = o.option_notice_on_byref_argument_typehint_violation
let to_string o =
  let dynamic_invokes =
    String.concat ~sep:", " (SSet.elements (dynamic_invoke_functions o)) in
  let search_paths = String.concat ~sep:", " (include_search_paths o) in
  let inc_roots = SMap.bindings (include_roots o)
    |> List.map ~f:(fun (root, path) -> root ^ ": " ^ path)
    |> String.concat ~sep:", " in
  String.concat ~sep:"\n"
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
    ; Printf.sprintf "hack_arr_compat_notices: %B" @@ hack_arr_compat_notices o
    ; Printf.sprintf "hack_arr_dv_arrs: %B" @@ hack_arr_dv_arrs o
    ; Printf.sprintf "dynamic_invoke_functions: [%s]" dynamic_invokes
    ; Printf.sprintf "repo_authoritative: %B" @@ repo_authoritative o
    ; Printf.sprintf "jit_enable_rename_function: %B"
      @@ jit_enable_rename_function o
    ; Printf.sprintf "can_inline_gen_functions: %B" @@ can_inline_gen_functions o
    ; Printf.sprintf "php7_int_semantics: %B" @@ php7_int_semantics o
    ; Printf.sprintf "enable_is_expr_primitive_migration: %B"
      @@ enable_is_expr_primitive_migration o
    ; Printf.sprintf "enable_coroutines: %B" @@ enable_coroutines o
    ; Printf.sprintf "hacksperimental: %B" @@ hacksperimental o
    ; Printf.sprintf "doc_root: %s" @@ doc_root o
    ; Printf.sprintf "include_search_paths: [%s]" search_paths
    ; Printf.sprintf "include_roots: {%s}" inc_roots
    ; Printf.sprintf "enable_perf_logging: %B" @@ enable_perf_logging o
    ; Printf.sprintf "enable_intrinsics_extension: %B" @@ enable_intrinsics_extension o
    ; Printf.sprintf "enable_hhjs: %B" @@ enable_hhjs o
    ; Printf.sprintf "enable_concurrent: %B" @@ enable_concurrent o
    ; Printf.sprintf "enable_await_as_an_expression: %B" @@ enable_await_as_an_expression o
    ; Printf.sprintf "phpism_disallow_execution_operator %B" @@ phpism_disallow_execution_operator o
    ; Printf.sprintf "phpism_disable_nontoplevel_declarations %B"
      @@ phpism_disable_nontoplevel_declarations o
    ; Printf.sprintf "phpism_disable_static_closures %B"
      @@ phpism_disable_static_closures o
    ; Printf.sprintf "phpism_disable_instanceof: %B" @@ phpism_disable_instanceof o
    ; Printf.sprintf "emit_func_pointers: %B" @@ emit_func_pointers o
    ; Printf.sprintf "emit_cls_meth_pointers: %B" @@ emit_cls_meth_pointers o
    ; Printf.sprintf "emit_inst_meth_pointers: %B" @@ emit_inst_meth_pointers o
    ; Printf.sprintf "rx_is_enabled: %B" @@ rx_is_enabled o
    ; Printf.sprintf "enable_stronger_await_binding: %B" @@ enable_stronger_await_binding o
    ; Printf.sprintf "disable_lval_as_an_expression: %B" @@ disable_lval_as_an_expression o
    ; Printf.sprintf "enable_pocket_universes: %B" @@ enable_pocket_universes o
    ; Printf.sprintf "notice_on_byref_argument_typehint_violation: %B" @@ notice_on_byref_argument_typehint_violation o
    ]

let as_bool s =
  match String.lowercase s with
  | "0" | "false" -> false
  | "1" | "true"  -> true
  | _             -> raise (Arg.Bad (s ^ " can't be cast to bool"))

let set_of_csv_string s = SSet.of_list @@ String.split ~on:(Char.of_string ",") s

let get_hhjs_node_modules_from_string = function
  | Some "" -> Some SSet.empty
  | Some v ->
    let v_set = set_of_csv_string v in
    let all_have_trailing_slash = SSet.for_all (fun s ->
      String_utils.string_ends_with s Filename.dir_sep
    ) v_set in
    if not all_have_trailing_slash then
      raise (Arg.Bad ("Expected all paths of HHJSNodeModules to end in " ^
        Filename.dir_sep ^ ", however, at least one path did not."))
    else
      Some v_set
  | None -> None

let set_option options name value =
  match String.lowercase name with
  | "eval.enablehiphopsyntax" ->
    { options with option_enable_hiphop_syntax = as_bool value }
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
  | "hack.lang.enableisexprprimitivemigration" ->
    { options with option_enable_is_expr_primitive_migration = as_bool value }
  | "hack.lang.enablecoroutines" ->
    { options with option_enable_coroutines = as_bool value }
  | "hack.lang.enablereifiedgenerics" ->
    { options with option_enable_reified_generics = as_bool value }
  | "hack.lang.hacksperimental" ->
    { options with option_hacksperimental = as_bool value }
  | "eval.logexterncompilerperf" ->
    { options with option_enable_perf_logging = as_bool value }
  | "eval.enableintrinsicsextension" ->
    { options with option_enable_intrinsics_extension = as_bool value }
  | "eval.enablehhjs" ->
    { options with option_enable_hhjs = as_bool value }
  | "eval.dumphhjs" ->
    { options with option_dump_hhjs = as_bool value }
  | "eval.hhjsuniquefilenames" ->
    { options with option_hhjs_unique_filenames = as_bool value }
  | "eval.hhjsadditionaltransform" ->
    { options with option_hhjs_additional_transform = value }
  | "eval.hhjsbabeltransform" ->
    { options with option_hhjs_babel_transform = value }
  | "eval.hhjsnodemodules" ->
    let hhjs_node_modules = match get_hhjs_node_modules_from_string @@ Some value with
    | Some s -> s
    | None -> SSet.empty
    in
    { options with option_hhjs_node_modules = hhjs_node_modules }
  | "hack.lang.phpism.disallowexecutionoperator" ->
    { options with option_phpism_disallow_execution_operator = as_bool value }
  | "hack.lang.phpism.disablenontopleveldeclarations" ->
    { options with option_phpism_disable_nontoplevel_declarations = as_bool value }
  | "hack.lang.phpism.disablestaticclosures" ->
    { options with option_phpism_disable_static_closures = as_bool value }
  | "hack.lang.phpism.disableinstanceof" ->
    { options with option_phpism_disable_instanceof = as_bool value }
  | "hack.lang.enableconcurrent" ->
    { options with option_enable_concurrent = as_bool value }
  | "hhvm.emit_func_pointers" ->
    { options with option_emit_func_pointers = int_of_string value > 0 }
  | "hhvm.emit_cls_meth_pointers" ->
    { options with option_emit_cls_meth_pointers = int_of_string value > 0 }
  | "hhvm.emit_inst_meth_pointers" ->
    { options with option_emit_inst_meth_pointers = int_of_string value > 0 }
  | "hhvm.emit_meth_caller_func_pointers" ->
    { options with option_emit_meth_caller_func_pointers = int_of_string value > 0 }
  | "hhvm.rx_is_enabled" ->
    { options with option_rx_is_enabled = int_of_string value > 0 }
  | "hack.lang.enableawaitasanexpression" ->
    { options with option_enable_await_as_an_expression = as_bool value }
  | "hack.lang.enable_stronger_await_binding" ->
    { options with option_enable_stronger_await_binding = as_bool value }
  | "hack.lang.disable_lval_as_an_expression" ->
    { options with option_disable_lval_as_an_expression = as_bool value }
  | "hack.lang.enablepocketuniverses" ->
    { options with option_enable_pocket_universes = as_bool value }
  | "hhvm.notice_on_by_ref_argument_typehint_violation" ->
    { options with option_notice_on_byref_argument_typehint_violation = as_bool value }
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

let get_hhjs_node_modules_from_config_string config key =
  get_hhjs_node_modules_from_string @@ get_value_from_config_string config key

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
  (set_value "hhvm.hack_arr_compat_notices" get_value_from_config_int @@
    fun opts v -> { opts with option_hack_arr_compat_notices = (v = 1) });
  (set_value "hhvm.hack_arr_dv_arrs" get_value_from_config_int @@
    fun opts v -> { opts with option_hack_arr_dv_arrs = (v = 1) });
  (set_value "hhvm.dynamic_invoke_functions" get_value_from_config_string_array @@
    fun opts v -> {opts with option_dynamic_invoke_functions =
        SSet.of_list (List.map v String.lowercase)});
  (set_value "hhvm.repo.authoritative" get_value_from_config_int @@
    fun opts v -> { opts with option_repo_authoritative = (v = 1) });
  (set_value "hhvm.jit_enable_rename_function" get_value_from_config_int @@
    fun opts v -> { opts with option_jit_enable_rename_function = (v = 1) });
  (set_value "hhvm.php7.int_semantics" get_value_from_config_int @@
    fun opts v -> { opts with option_php7_int_semantics = (v = 1) });
  (set_value "hhvm.hack.lang.enable_is_expr_primitive_migration" get_value_from_config_int @@
    fun opts v -> { opts with option_enable_is_expr_primitive_migration = (v = 1) });
  (set_value "hhvm.hack.lang.enable_coroutines" get_value_from_config_int @@
    fun opts v -> { opts with option_enable_coroutines = (v = 1) });
  (set_value "hhvm.hack.lang.hacksperimental" get_value_from_config_int @@
    fun opts v -> { opts with option_hacksperimental = (v = 1) });
  (set_value "hhvm.hack.lang.enable_reified_generics" get_value_from_config_int @@
    fun opts v -> { opts with option_enable_reified_generics = (v = 1) });
  (set_value "hhvm.hack.lang.enable_concurrent" get_value_from_config_int @@
    fun opts v -> { opts with option_enable_concurrent = (v = 1) });
  (set_value "hhvm.hack.lang.enable_await_as_an_expression" get_value_from_config_int @@
    fun opts v -> { opts with option_enable_await_as_an_expression = (v = 1) });
  (set_value "hhvm.hack.lang.enable_stronger_await_binding" get_value_from_config_int @@
    fun opts v -> { opts with option_enable_stronger_await_binding = (v = 1) });
  (set_value "hhvm.hack.lang.disable_lval_as_an_expression" get_value_from_config_int @@
    fun opts v -> { opts with option_disable_lval_as_an_expression = (v = 1) });
  (set_value "hhvm.hack.lang.enable_pocket_universes" get_value_from_config_int @@
    fun opts v -> { opts with option_enable_pocket_universes = (v = 1) });
  (set_value "hhvm.notice_on_by_ref_argument_typehint_violation" get_value_from_config_int @@
    fun opts v -> { opts with option_notice_on_byref_argument_typehint_violation = (v = 1) });
  (set_value "doc_root" get_value_from_config_string @@
    fun opts v -> { opts with option_doc_root = v });
  (set_value "hhvm.server.include_search_paths" get_value_from_config_string_array @@
     fun opts v -> { opts with option_include_search_paths = v });
  (set_value "hhvm.include_roots" get_value_from_config_string_to_string_map @@
     fun opts v -> { opts with option_include_roots = v });
  (set_value "hhvm.log_extern_compiler_perf" get_value_from_config_int @@
     fun opts v -> { opts with option_enable_perf_logging = (v = 1) });
  (set_value "hhvm.enable_intrinsics_extension" get_value_from_config_int @@
     fun opts v -> { opts with option_enable_intrinsics_extension = (v = 1) });
  (set_value "hhvm.enable_hhjs" get_value_from_config_int @@
     fun opts v -> { opts with option_enable_hhjs = (v = 1) });
  (set_value "hhvm.dump_hhjs" get_value_from_config_int @@
     fun opts v -> { opts with option_dump_hhjs = (v = 1) });
  (set_value "hhvm.hhjs_additional_transform" get_value_from_config_string @@
     fun opts v -> { opts with option_hhjs_additional_transform = v });
  (set_value "hhvm.hhjs_unique_filenames" get_value_from_config_int @@
     fun opts v -> { opts with option_hhjs_unique_filenames = (v = 1) });
  (set_value "hhvm.hhjs_babel_transform" get_value_from_config_string @@
     fun opts v -> { opts with option_hhjs_babel_transform = v });
  (set_value "hhvm.hhjs_node_modules" get_hhjs_node_modules_from_config_string @@
    fun opts v -> { opts with option_hhjs_node_modules = v });
  (set_value "hhvm.hack.lang.phpism.disallow_execution_operator" get_value_from_config_int @@
     fun opts v -> { opts with option_phpism_disallow_execution_operator = (v = 1) });
  (set_value "hhvm.hack.lang.phpism.disable_nontoplevel_declarations" get_value_from_config_int @@
     fun opts v -> { opts with option_phpism_disable_nontoplevel_declarations = (v = 1) });
  (set_value "hhvm.hack.lang.phpism.disable_static_closures" get_value_from_config_int @@
     fun opts v -> { opts with option_phpism_disable_static_closures = (v = 1) });
  (set_value "hhvm.hack.lang.phpism.disable_instanceof" get_value_from_config_int @@
     fun opts v -> { opts with option_phpism_disable_instanceof = (v = 1) });
  (set_value "hhvm.emit_func_pointers" get_value_from_config_int @@
     fun opts v -> { opts with option_emit_func_pointers = (v > 0) });
  (set_value "hhvm.emit_cls_meth_pointers" get_value_from_config_int @@
     fun opts v -> { opts with option_emit_cls_meth_pointers = (v > 0) });
  (set_value "hhvm.emit_meth_caller_func_pointers" get_value_from_config_int @@
     fun opts v -> { opts with option_emit_meth_caller_func_pointers = (v > 0) });
  (set_value "hhvm.emit_inst_meth_pointers" get_value_from_config_int @@
     fun opts v -> { opts with option_emit_inst_meth_pointers = (v > 0) });
  (set_value "hhvm.rx_is_enabled" get_value_from_config_int @@
     fun opts v -> { opts with option_rx_is_enabled = (v > 0) });
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
