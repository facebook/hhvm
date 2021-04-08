(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs

(* Local environment includes types of locals and bounds on type parameters. *)
type local_env = {
  per_cont_env: Typing_per_cont_env.t;
  (* Local variables that were assigned in a `using` clause *)
  local_using_vars: Local_id.Set.t;
}

type env = {
  (* position of the function/method being checked *)
  function_pos: Pos.t;
  fresh_typarams: SSet.t;
  lenv: local_env;
  genv: genv;
  decl_env: Decl_env.env;
  in_loop: bool;
  in_try: bool;
  in_case: bool;
  in_expr_tree: bool;
  inside_constructor: bool;
  (* Tracing_info is a way, when we record telemetry on costs, to also record which
  area of the typing code should be considered the originator of that work,
  so we can add up which area contributed most to overall costs. *)
  tracing_info: Decl_counters.tracing_info option;
  (* A set of constraints that are global to a given method *)
  global_tpenv: Type_parameter_env.t;
  log_levels: int SMap.t;
  inference_env: Typing_inference_env.t;
  allow_wildcards: bool;
  big_envs: (Pos.t * env) list ref;
  pessimize: bool;
  (* This is only filled in after type-checking the function in question *)
  fun_tast_info: Tast.fun_tast_info option;
}

and genv = {
  tcopt: TypecheckerOptions.t;
  return: Typing_env_return_info.t;
  (* For each function parameter, its type, position, calling convention. *)
  params: (locl_ty * Pos.t * param_mode) Local_id.Map.t;
  (* condition types associated with parameters.
     For every mayberx parameter that has condition type we create
     fresh type parameter (see: make_local_param_ty) and store mapping
     fresh type name -> condition type in env so it can be retrieved later *)
  condition_types: decl_ty SMap.t;
  (* Identifier and type of the parent class if it exists *)
  parent: (string * decl_ty) option;
  (* Identifier and type (instatiated at its generic parameters) of
     the enclosing class if there is one *)
  self: (string * locl_ty) option;
  static: bool;
  fun_kind: Ast_defs.fun_kind;
  val_kind: Typing_defs.val_kind;
  fun_is_ctor: bool;
  file: Relative_path.t;
}
