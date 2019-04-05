(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Core_kernel

module T = Tast

type t = {
  env_pipe_var             : Local.t option;
  env_scope                : Ast_scope.Scope.t;
  env_namespace            : Namespace_env.env;
  env_needs_local_this     : bool;
  env_jump_targets         : Jump_targets.t;
  env_in_try               : bool;
  env_allows_array_append  : bool;
  env_in_rx_body           : bool;
}

type t_tast = {
  env_pipe_var_tast             : Local.t option;
  env_scope_tast                : Ast_scope.Scope.t_tast;
  env_namespace_tast            : Namespace_env.env;
  env_needs_local_this_tast     : bool;
  env_jump_targets_tast         : Jump_targets.t;
  env_in_try_tast               : bool;
  env_allows_array_append_tast  : bool;
  env_in_rx_body_tast           : bool;
}

type global_state =
{ global_explicit_use_set: SSet.t
; global_closure_namespaces: Namespace_env.env SMap.t
; global_closure_enclosing_classes: Ast.class_ SMap.t
; global_functions_with_finally: SSet.t
; global_function_to_labels_map: (bool SMap.t) SMap.t
; global_lambda_rx_of_scope: Rx.t SMap.t
}

type global_state_tast =
{ global_explicit_use_set_tast : SSet.t
; global_closure_namespaces_tast : Namespace_env.env SMap.t
; global_closure_enclosing_classes_tast : T.class_ SMap.t
; global_functions_with_finally_tast : SSet.t
; global_function_to_labels_map_tast : (bool SMap.t) SMap.t
; global_functions_with_hhas_blocks_tast : (string list) SMap.t
; global_lambda_rx_of_scope_tast : Rx.t SMap.t
}

let empty_global_state =
{ global_explicit_use_set = SSet.empty
; global_closure_namespaces = SMap.empty
; global_closure_enclosing_classes = SMap.empty
; global_functions_with_finally = SSet.empty
; global_function_to_labels_map = SMap.empty
; global_lambda_rx_of_scope = SMap.empty
}

let empty_global_state_tast =
{ global_explicit_use_set_tast = SSet.empty
; global_closure_namespaces_tast = SMap.empty
; global_closure_enclosing_classes_tast = SMap.empty
; global_functions_with_finally_tast = SSet.empty
; global_function_to_labels_map_tast = SMap.empty
; global_functions_with_hhas_blocks_tast = SMap.empty
; global_lambda_rx_of_scope_tast = SMap.empty
}

let is_hh_file_ = ref false
let is_js_file_ = ref false
let is_systemlib_ = ref false
let global_state_ = ref empty_global_state

let global_state__tast = ref empty_global_state_tast

let set_is_hh_file v = is_hh_file_ := v
let set_is_js_file v = is_js_file_ := v
let set_is_systemlib v = is_systemlib_ := v

let is_hh_syntax_enabled () =
  !is_hh_file_ || Hhbc_options.enable_hiphop_syntax !Hhbc_options.compiler_options
let is_systemlib () = !is_systemlib_
let is_js () = !is_js_file_


let get_explicit_use_set () = (!global_state_).global_explicit_use_set
let get_closure_namespaces () = (!global_state_).global_closure_namespaces
let get_closure_enclosing_classes () =
  (!global_state_).global_closure_enclosing_classes

let get_closure_enclosing_classes_tast () =
  (!global_state__tast).global_closure_enclosing_classes_tast

let get_functions_with_finally () =
  (!global_state_).global_functions_with_finally
let get_function_to_labels_map () =
  (!global_state_).global_function_to_labels_map

let get_unique_id_for_main () =
  "|"
let get_unique_id_for_function { Ast.f_name = (_, n); _ } =
  "|" ^ n
let get_unique_id_for_method { Ast.c_name = (_, cls); _ } { Ast.m_name = (_, m); _} =
  cls ^ "|" ^ m

let get_unique_id_for_function_tast fn =
  "|" ^ (snd fn.T.f_name)

let get_unique_id_for_method_tast cls mthd =
  (snd cls.T.c_name) ^ "|" ^ (snd mthd.T.m_name)

let get_lambda_rx_of_scope cd md =
  let key = get_unique_id_for_method cd md in
  SMap.get key (!global_state_).global_lambda_rx_of_scope
  |> Option.value ~default:Rx.NonRx

let get_lambda_rx_of_scope_tast cd md =
  let key = get_unique_id_for_method_tast cd md in
  SMap.get key (!global_state__tast).global_lambda_rx_of_scope_tast
  |> Option.value ~default:Rx.NonRx

let set_global_state_tast s = global_state__tast := s
let set_global_state s = global_state_ := s
let clear_global_state () = set_global_state empty_global_state

let empty = {
  env_pipe_var = None;
  env_scope = Ast_scope.Scope.toplevel;
  env_namespace = Namespace_env.empty_with_default_popt;
  env_needs_local_this = false;
  env_jump_targets = Jump_targets.empty;
  env_in_try = false;
  env_allows_array_append = false;
  env_in_rx_body = false;
}

let empty_tast = {
  env_pipe_var_tast = None;
  env_scope_tast = Ast_scope.Scope.toplevel_tast;
  env_namespace_tast = Namespace_env.empty_with_default_popt;
  env_needs_local_this_tast = false;
  env_jump_targets_tast = Jump_targets.empty;
  env_in_try_tast = false;
  env_allows_array_append_tast = false;
  env_in_rx_body_tast = false;
}

let make_class_env ast_class = {
  env_pipe_var = None;
  env_scope = [Ast_scope.ScopeItem.Class ast_class];
  env_namespace = ast_class.Ast.c_namespace;
  env_needs_local_this = false;
  env_jump_targets = Jump_targets.empty;
  env_in_try = false;
  env_allows_array_append = false;
  env_in_rx_body = false;
}

let make_class_env_tast ast_class = {
  env_pipe_var_tast = None;
  env_scope_tast = [Ast_scope.ScopeItem.Class_tast ast_class];
  env_namespace_tast = ast_class.T.c_namespace;
  env_needs_local_this_tast = false;
  env_jump_targets_tast = Jump_targets.empty;
  env_in_try_tast = false;
  env_allows_array_append_tast = false;
  env_in_rx_body_tast = false;
}

let get_pipe_var env = env.env_pipe_var

let get_pipe_var_tast env = env.env_pipe_var_tast

let get_scope env = env.env_scope

let get_scope_tast env = env.env_scope_tast

let get_namespace env = env.env_namespace

let get_namespace_tast env = env.env_namespace_tast

let get_needs_local_this env = env.env_needs_local_this

let get_needs_local_this_tast env = env.env_needs_local_this_tast

let get_jump_targets env = env.env_jump_targets

let get_jump_targets_tast env = env.env_jump_targets_tast

let is_in_try env = env.env_in_try

let is_in_try_tast env = env.env_in_try_tast

let does_env_allow_array_append env = env.env_allows_array_append

let does_env_allow_array_append_tast env = env.env_allows_array_append_tast

let is_in_rx_body env = env.env_in_rx_body

(* Environment is second parameter so we can chain these e.g.
 *   empty |> with_scope scope |> with_namespace ns
 *)
let with_scope scope env =
  { env with env_scope = scope }
let with_namespace namespace env =
  { env with env_namespace = namespace }
let with_needs_local_this needs_local_this env =
  { env with env_needs_local_this = needs_local_this }
let with_pipe_var v env =
  { env with env_pipe_var = Some v }
let with_try env =
  { env with env_in_try = true }
let with_rx_body rx_body env =
  { env with env_in_rx_body = rx_body }

let with_scope_tast scope env =
  { env with env_scope_tast = scope }

let with_namespace_tast namespace env =
  { env with env_namespace_tast = namespace }

let with_needs_local_this_tast needs_local_this env =
  { env with env_needs_local_this_tast = needs_local_this }

let with_pipe_var_tast v env =
  { env with env_pipe_var_tast = Some v }

let with_try_tast env =
  { env with env_in_try_tast = true }

let with_rx_body_tast rx_body env =
  { env with env_in_rx_body_tast = rx_body }

let do_in_loop_body break_label continue_label ?iter env s f =
  Jump_targets.with_loop (!is_hh_file_) break_label continue_label
    iter env.env_jump_targets s @@
    fun env_jump_targets s -> f { env with env_jump_targets } s

let do_in_loop_body_tast break_label continue_label ?iter env s f =
  Jump_targets.with_loop_tast (!is_hh_file_) break_label continue_label
    iter env.env_jump_targets_tast s @@
    fun env_jump_targets_tast s -> f { env with env_jump_targets_tast } s

let do_in_switch_body end_label env s f =
  Jump_targets.with_switch (!is_hh_file_) end_label
    env.env_jump_targets s @@
    fun env_jump_targets s -> f { env with env_jump_targets } s

let do_in_try_body finally_label env s f =
  Jump_targets.with_try (!is_hh_file_) finally_label
    env.env_jump_targets s @@
    fun env_jump_targets s -> f { env with env_jump_targets } s

let do_in_finally_body env s f =
  Jump_targets.with_finally (!is_hh_file_)
    env.env_jump_targets s @@
    fun env_jump_targets s -> f { env with env_jump_targets } s

let do_in_using_body finally_label env s f =
  Jump_targets.with_using (!is_hh_file_) finally_label
    env.env_jump_targets s @@
    fun env_jump_targets s -> f { env with env_jump_targets } s

let do_function env s f =
  Jump_targets.with_function (!is_hh_file_)
    env.env_jump_targets s @@
    fun env_jump_targets s -> f { env with env_jump_targets } s

let do_function_tast (env : t_tast) (s : Tast.program) f =
  Jump_targets.with_function_tast (!is_hh_file_)
    env.env_jump_targets_tast s @@
    fun env_jump_targets_tast s -> f { env with env_jump_targets_tast } s
