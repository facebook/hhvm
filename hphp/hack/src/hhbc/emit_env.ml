(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

type t = {
  env_pipe_var        : Local.t option;
  env_scope           : Ast_scope.Scope.t;
  env_namespace       : Namespace_env.env;
  env_needs_local_this: bool;
  env_jump_targets    : Jump_targets.t;
  env_in_try          : bool
}

type global_state =
{ global_explicit_use_set: SSet.t
; global_closure_namespaces: Namespace_env.env SMap.t
; global_closure_enclosing_classes: Ast.class_ SMap.t
; global_functions_with_finally: SSet.t
; global_function_to_labels_map: (bool SMap.t) SMap.t
}

let empty_global_state =
{ global_explicit_use_set = SSet.empty
; global_closure_namespaces = SMap.empty
; global_closure_enclosing_classes = SMap.empty
; global_functions_with_finally = SSet.empty
; global_function_to_labels_map = SMap.empty
}

let is_hh_file_ = ref false
let is_systemlib_ = ref false
let global_state_ = ref empty_global_state

let set_is_hh_file v = is_hh_file_ := v
let set_is_systemlib v = is_systemlib_ := v

let is_hh_syntax_enabled () =
  !is_hh_file_ || Hhbc_options.enable_hiphop_syntax !Hhbc_options.compiler_options
let is_systemlib () = !is_systemlib_

let get_explicit_use_set () = (!global_state_).global_explicit_use_set
let get_closure_namespaces () = (!global_state_).global_closure_namespaces
let get_closure_enclosing_classes () =
  (!global_state_).global_closure_enclosing_classes
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

let set_global_state s = global_state_ := s
let clear_global_state () = set_global_state empty_global_state

let empty = {
  env_pipe_var = None;
  env_scope = Ast_scope.Scope.toplevel;
  env_namespace = Namespace_env.empty_with_default_popt;
  env_needs_local_this = false;
  env_jump_targets = Jump_targets.empty;
  env_in_try = false;
}

let get_pipe_var env = env.env_pipe_var
let get_scope env = env.env_scope
let get_namespace env = env.env_namespace
let get_needs_local_this env = env.env_needs_local_this
let get_jump_targets env = env.env_jump_targets
let is_in_try env = env.env_in_try

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
let make_class_env ast_class =
  { env_pipe_var = None; env_scope = [Ast_scope.ScopeItem.Class ast_class];
    env_namespace = ast_class.Ast.c_namespace; env_needs_local_this = false;
    env_jump_targets = Jump_targets.empty; env_in_try = false; }
let with_try env = { env with env_in_try = true }

let do_in_loop_body break_label continue_label ?iter env s f =
  Jump_targets.with_loop (!is_hh_file_) break_label continue_label
    iter env.env_jump_targets s @@
    fun env_jump_targets s -> f { env with env_jump_targets } s

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
