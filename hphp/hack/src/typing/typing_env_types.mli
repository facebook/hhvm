(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs

(** Local environment includes types of locals and bounds on type parameters. *)
type local_env = {
  per_cont_env: Typing_per_cont_env.t;
  local_using_vars: Local_id.Set.t;
      (** Local variables that were assigned in a `using` clause *)
}

(** Contains contextual information useful when type checking an
    expression tree. *)
type expr_tree_env = {
  dsl: Aast.hint;
      (** The DSL the expression tree is representing. For instance in:

          SomeDSL`1 + 1`

          This hint would reference `SomeDsl` *)
  outer_locals: Typing_local_types.t;
      (** The set of locals defined outside the expression tree. Ex:
        $x = 10;
        SomeDSL`1 + 1`;

        `$x` would be in this set *)
}

type env = {
  expression_id_provider: Expression_id.provider;
  tvar_id_provider: Tvid.provider;
  fresh_typarams: SSet.t;
  lenv: local_env;
  genv: genv;
  decl_env: Decl_env.env;
  in_loop: bool;
  in_try: bool;
  in_expr_tree: expr_tree_env option;
      (** If set to Some(_), then we are performing type checking within a
          expression tree. *)
  inside_constructor: bool;
  checked: Tast.check_status;
      (** Set to true when checking if a <<__SoundDynamicallyCallable>> method body
          is well-typed under dyn..dyn->dyn assumptions, that is if it can be safely called
          in a dynamic environment. *)
  tracing_info: Decl_counters.tracing_info option;
      (** Tracing_info is a way, when we record telemetry on costs, to also record which
          area of the typing code should be considered the originator of that work,
          so we can add up which area contributed most to overall costs. *)
  tpenv: Type_parameter_env.t;
      (** A set of constraints that are global to a given method *)
  log_levels: int SMap.t;
  inference_env: Typing_inference_env.t;
  allow_wildcards: bool;
  big_envs: (Pos.t * env) list ref;
  fun_tast_info: Tast.fun_tast_info option;
      (** This is only filled in after type-checking the function in question *)
  loaded_packages: SSet.t;
      (** The set of packages loaded via a "package <pkg>" expression *)
}

and genv = {
  tcopt: TypecheckerOptions.t;
  callable_pos: Pos.t;  (** position of the function/method being checked *)
  readonly: bool;
  (* Whether readonly analysis is needed on this function *)
  return: Typing_env_return_info.t;
  params: (locl_ty * Pos.t * locl_ty option) Local_id.Map.t;
      (** For each function/method parameter, its type, position, and inout "return" type. *)
  parent: (string * decl_ty) option;
      (** Identifier and type of the parent class if it exists *)
  self: (string * locl_ty) option;
      (** Identifier and type (instatiated at its generic parameters) of
          the enclosing class if there is one *)
  static: bool;
  fun_kind: Ast_defs.fun_kind;
  val_kind: Typing_defs.val_kind;
  fun_is_ctor: bool;  (** Is the method a constructor? *)
  file: Relative_path.t;
      (** The file containing the top-level definition that we are checking *)
  current_module: Ast_defs.id option;
      (** The module of the top-level definition that we are checking *)
  this_internal: bool;
      (** Is the definition that we are checking marked internal? *)
  this_support_dynamic_type: bool;
      (** Is the definition that we are checking marked <<__SupportDynamicType>>? *)
  no_auto_likes: bool;
      (** Is the definition that we are checking marked <<__NoAutoLikes>>? *)
}

val empty :
  ?origin:Decl_counters.origin ->
  ?mode:FileInfo.mode ->
  Provider_context.t ->
  Relative_path.t ->
  droot:Typing_deps.Dep.dependent Typing_deps.Dep.variant option ->
  env

val get_log_level : env -> string -> int

val next_cont_opt : env -> Typing_per_cont_env.per_cont_entry option

val get_tpenv : env -> Type_parameter_env.t

val get_pos_and_kind_of_generic :
  env -> string -> (Pos_or_decl.t * Typing_kinding_defs.kind) option

val get_lower_bounds :
  env -> string -> locl_ty list -> Type_parameter_env.tparam_bounds

val get_upper_bounds :
  env -> string -> locl_ty list -> Type_parameter_env.tparam_bounds

val get_equal_bounds :
  env -> string -> locl_ty list -> Type_parameter_env.tparam_bounds

val get_tparams_in_ty_and_acc : env -> SSet.t -> locl_ty -> SSet.t
