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
  env_jump_targets    : Jump_targets.t
}

let is_hh_file_ = ref false

let set_is_hh_file v = is_hh_file_ := v
let is_hh_file () = !is_hh_file_

let empty = {
  env_pipe_var = None;
  env_scope = Ast_scope.Scope.toplevel;
  env_namespace = Namespace_env.empty_with_default_popt;
  env_needs_local_this = false;
  env_jump_targets = Jump_targets.empty;
}

let get_pipe_var env = env.env_pipe_var
let get_scope env = env.env_scope
let get_namespace env = env.env_namespace
let get_needs_local_this env = env.env_needs_local_this
let get_jump_targets env = env.env_jump_targets

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
    env_jump_targets = Jump_targets.empty; }

let do_in_loop_body break_label continue_label ?iter env f =
  Jump_targets.with_loop break_label continue_label iter env.env_jump_targets @@
    fun env_jump_targets -> f { env with env_jump_targets }

let do_in_switch_body end_label env f =
  Jump_targets.with_switch end_label env.env_jump_targets @@
    fun env_jump_targets -> f { env with env_jump_targets }

let do_in_try_body finally_label env f =
  Jump_targets.with_try finally_label env.env_jump_targets @@
    fun env_jump_targets -> f { env with env_jump_targets }

let do_in_finally_body env f =
  Jump_targets.with_finally env.env_jump_targets @@
    fun env_jump_targets -> f { env with env_jump_targets }
