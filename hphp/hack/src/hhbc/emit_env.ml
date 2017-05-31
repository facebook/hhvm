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
}

let empty = {
  env_pipe_var = None;
  env_scope = Ast_scope.Scope.toplevel;
  env_namespace = Namespace_env.empty_with_default_popt;
  env_needs_local_this = false;
}

let get_pipe_var env = env.env_pipe_var
let get_scope env = env.env_scope
let get_namespace env = env.env_namespace
let get_needs_local_this env = env.env_needs_local_this

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
    env_namespace = ast_class.Ast.c_namespace; env_needs_local_this = false }
