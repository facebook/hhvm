(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* cf: typing_env_types_sig.mli - These files should be the same *)

open Typing_defs
open Type_parameter_env
module TySet = Typing_set

let show_locl_ty _ = "<locl_ty>"
let pp_locl_ty _ _ = Printf.printf "%s\n" "<locl_ty>"
type locl_ty = locl ty

type fake_members = {
  last_call : Pos.t option;
  invalid   : SSet.t;
  valid     : SSet.t;
} [@@deriving show]
(* Along with a type, each local variable has a expression id associated with
 * it. This is used when generating expression dependent types for the 'this'
 * type. The idea is that if two local variables have the same expression_id
 * then they refer to the same late bound type, and thus have compatible
 * 'this' types.
 *)
type expression_id = Ident.t [@@deriving show]
type local = locl_ty * expression_id [@@deriving show]
type local_history = locl_ty list [@@deriving show]
type old_local = locl_ty list * locl_ty * expression_id [@@deriving show]

let show_local_id_map _ = "<local_id_map>"
let pp_local_id_map _ _ = Printf.printf "%s\n" "<local_id_map>"
type local_id_map = local Local_id.Map.t

let show_local_types _ = "<local_types>"
let pp_local_types _ _ = Printf.printf "%s\n" "<local_types>"
type local_types = local_id_map Typing_continuations.Map.t

let show_local_id_set_t _ = "<local_id_set_t>"
let pp_local_id_set_t _ _ = Printf.printf "%s\n" "<local_id_set_t>"
type local_id_set_t = Local_id.Set.t

let show_local_env _ = "<local_env>"
let pp_local_env _ _ = Printf.printf "%s\n" "<local_env>"

(* Local environment includes types of locals and bounds on type parameters. *)
type local_env = {
  fake_members       : fake_members;
  local_types        : local_types;
  local_mutability   : Typing_mutability_env.mutability_env;
  (* Whether current environment is reactive *)
  local_reactive : reactivity;
  local_type_history : local_history Local_id.Map.t;
  (* Local variables that were assigned in a `using` clause *)
  local_using_vars   : local_id_set_t;
  (* Type parameter environment
   * Lower and upper bounds on generic type parameters and abstract types
   * For constraints of the form Tu <: Tv where both Tu and Tv are type
   * parameters, we store an upper bound for Tu and a lower bound for Tv.
   * Contrasting with tenv and subst, bounds are *assumptions* for type
   * inference, not conclusions.
   *)
  tpenv              : tpenv;
}

let show_env _ = "<env>"
let pp_env _ _ = Printf.printf "%s\n" "<env>"

let show_genv _ = "<genv>"
let pp_genv _ _ = Printf.printf "%s\n" "<genv>"

let show_anon _ = "<anon>"
let pp_anon _ _ = Printf.printf "%s\n" "<anon>"

let show_tfun _ = "<tfun>"
let pp_tfun _ _ = Printf.printf "%s\n" "<tfun>"

type env = {
  (* position of the function/method being checked *)
  function_pos: Pos.t;
  pos     : Pos.t      ;
  (* Position and reason information on entry to a subtype or unification check *)
  outer_pos : Pos.t;
  outer_reason : Typing_reason.ureason;
  tenv    : locl_ty IMap.t ;
  subst   : int IMap.t ;
  lenv    : local_env  ;
  genv    : genv       ;
  decl_env: Decl_env.env;
  todo    : tfun list  ;
  in_loop : bool       ;
  inside_constructor: bool;
  (* A set of constraints that are global to a given method *)
  global_tpenv : tpenv ;
}
and genv = {
  tcopt   : TypecheckerOptions.t;
  return  : Typing_env_return_info.t;
  (* For each function parameter, its type and calling convention. *)
  params  : (locl_ty * param_mode) Local_id.Map.t;
  parent_id : string;
  parent  : decl ty;
  (* Identifier of the enclosing class *)
  self_id : string;
  (* Type of the enclosing class, instantiated at its generic parameters *)
  self    : locl_ty;
  static  : bool;
  fun_kind : Ast.fun_kind;
  fun_mutable : bool;
  anons   : anon IMap.t;
  file    : Relative_path.t;
}

(* A type-checker for an anonymous function
 * Parameters are
 * - the environment
 * - types of the parameters under which the body should be checked
 * - the arity of the function
 * - the expected return type of the body (optional)
 *)
and anon =
  reactivity *
  Nast.is_coroutine *
  int ref *
  Pos.t *
  (?el:Nast.expr list ->
  ?ret_ty: locl_ty ->
  env ->
  locl fun_params ->
  locl fun_arity ->
  env * Tast.expr * locl_ty)

(* A deferred check; return true if the check should now be removed from the list *)
and tfun = env -> env * bool
