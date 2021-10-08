(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* cf: typing_env_types_sig.mli - These files should be the same *)
open Hh_prelude
open Typing_defs

type locl_ty = Typing_defs.locl_ty

[@@@warning "-32"]

let show_local_id_set_t _ = "<local_id_set_t>"

let pp_local_id_set_t _ _ = Printf.printf "%s\n" "<local_id_set_t>"

type local_id_set_t = Local_id.Set.t

let show_local_env _ = "<local_env>"

let pp_local_env _ _ = Printf.printf "%s\n" "<local_env>"

type local_env = {
  per_cont_env: Typing_per_cont_env.t;
  local_using_vars: local_id_set_t;
}

let show_env _ = "<env>"

let pp_env _ _ = Printf.printf "%s\n" "<env>"

let show_genv _ = "<genv>"

let pp_genv _ _ = Printf.printf "%s\n" "<genv>"

let show_tfun _ = "<tfun>"

let pp_tfun _ _ = Printf.printf "%s\n" "<tfun>"

[@@@warning "+32"]

(** See the .mli file for the documentation of fields. *)
type env = {
  fresh_typarams: SSet.t;
  lenv: local_env;
  genv: genv;
  decl_env: Decl_env.env;
  in_loop: bool;
  in_try: bool;
  in_case: bool;
  in_expr_tree: bool;
  inside_constructor: bool;
  in_support_dynamic_type_method_check: bool;
  tracing_info: Decl_counters.tracing_info option;
  tpenv: Type_parameter_env.t;
  log_levels: int SMap.t;
  inference_env: Typing_inference_env.t;
  allow_wildcards: bool;
  big_envs: (Pos.t * env) list ref;
  pessimize: bool;
  fun_tast_info: Tast.fun_tast_info option;
}

(** See the .mli file for the documentation of fields. *)
and genv = {
  tcopt: TypecheckerOptions.t;
  callable_pos: Pos.t;
  readonly: bool;
  return: Typing_env_return_info.t;
  params: (locl_ty * Pos.t * param_mode) Local_id.Map.t;
  condition_types: decl_ty SMap.t;
  parent: (string * decl_ty) option;
  self: (string * locl_ty) option;
  static: bool;
  fun_kind: Ast_defs.fun_kind;
  val_kind: Typing_defs.val_kind;
  fun_is_ctor: bool;
  file: Relative_path.t;
  this_module: Typing_modules.t;
  this_internal: bool;
}
