(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
include Typing_env_types_sig.S

open Typing_defs
open Typing_heap
open Type_parameter_env

val show_env : env -> string
val pp_env : Format.formatter -> env -> unit
val get_tcopt : env -> TypecheckerOptions.t
val fresh : unit -> int
val fresh_type : env -> Pos.t -> env *  locl ty
val fresh_unresolved_type : env -> Pos.t -> env * locl ty
val fresh_invariant_type_var : env -> Pos.t -> env * locl ty
val open_tyvars : env -> env
val get_current_tyvars : env -> Ident.t list
val add_current_tyvar : env -> Pos.t -> Ident.t -> env
val close_tyvars : env -> env
val get_var : env -> int -> env * int
val rename : env -> int -> int -> env
val add : env -> int -> locl ty -> env
val get_type : env -> Reason.t -> int -> env * locl ty
val get_type_unsafe : env -> int -> env * locl ty
val expand_type : env -> locl ty -> env * locl ty
val tyvar_is_solved : env -> int -> bool
val make_ft : Pos.t -> reactivity -> bool -> decl fun_params -> decl ty -> decl fun_type
val get_shape_field_name : Nast.shape_field_name -> string
val empty_fake_members : fake_members
val empty_local_id_map : local_id_map
val add_to_local_id_map :
  Local_id.t -> local -> local_id_map -> local_id_map
val empty_local : tpenv -> reactivity -> local_env
val initial_local : tpenv -> reactivity -> local_env
val empty : TypecheckerOptions.t -> Relative_path.t ->
  droot: Typing_deps.Dep.variant option -> env
val is_typedef : Typedefs.key -> bool
val get_enum : env -> Classes.key -> Classes.t option
val is_enum : env -> Classes.key -> bool
val get_enum_constraint : env -> Classes.key -> decl ty option
val add_wclass : env -> string -> unit
val fresh_tenv : env -> (env -> 'a) -> 'a
val get_class : env -> Classes.key -> Classes.t option
val get_typedef : env -> Typedefs.key -> Typedefs.t option
val get_const : env -> Typing_classes_heap.t -> string -> class_const option
val get_typeconst : env -> Typing_classes_heap.t -> string -> typeconst_type option
val get_gconst : env -> GConsts.key -> GConsts.t option
val get_static_member : bool -> env -> Typing_classes_heap.t -> string -> class_elt option
val suggest_static_member :
  bool -> Typing_classes_heap.t -> string -> (Pos.t * string) option
val get_member : bool -> env -> Typing_classes_heap.t -> string -> class_elt option
val suggest_member : bool -> Typing_classes_heap.t -> string -> (Pos.t * string) option
val get_construct : env -> Typing_classes_heap.t -> class_elt option * bool
val check_todo : env -> env
val get_return : env -> Typing_env_return_info.t
val set_return : env -> Typing_env_return_info.t -> env
val get_params : env -> (locl ty * param_mode) Local_id.Map.t
val set_param : env -> Local_id.t -> locl ty * param_mode -> env
val set_log_level : env -> string -> int -> env
val get_log_level : env -> string -> int
val clear_params : env -> env
val with_env : env -> (env -> env * 'a) -> env * 'a
val is_static : env -> bool
val get_self : env -> locl ty
val get_self_id : env -> string
val is_outside_class : env -> bool
val get_parent_id : env -> string
val get_parent : env -> decl ty
val get_fn_kind : env -> Ast.fun_kind
val get_file : env -> Relative_path.t
val get_fun : env -> Funs.key -> Funs.t option
val set_fn_kind : env -> Ast.fun_kind -> env
val set_inside_ppl_class : env -> bool -> env
val add_todo : env -> tfun -> env
val check_now_or_add_todo : env -> tfun -> env
val add_anonymous : env -> anon -> env * int
val get_anonymous : env -> int -> anon option
val iter_anonymous : env -> (Pos.t -> locl ty list -> unit) -> unit
val set_self_id : env -> string -> env
val set_self : env -> locl ty -> env
val set_parent_id : env -> string -> env
val set_parent : env -> decl ty -> env
val set_static : env -> env
val set_mode : env -> FileInfo.mode -> env
val get_mode : env -> FileInfo.mode
val is_strict : env -> bool
val is_decl : env -> bool
val lost_info : string -> env -> locl ty -> env * locl ty
val forget_members : env -> Pos.t -> env
module FakeMembers :
  sig
    val make_id : Nast.expr -> string -> string
    val make_static_id : Nast.class_id_ -> string -> string
    val get : env -> Nast.expr -> string -> int option
    val is_invalid : env -> Nast.expr -> string -> bool
    val get_static : env -> Nast.class_id_ -> string -> int option
    val is_static_invalid : env -> Nast.class_id_ -> string -> bool
    val make : Pos.t -> env -> Nast.expr -> string -> env * Local_id.t
    val make_static : Pos.t -> env -> Nast.class_id_ -> string ->
      env * Local_id.t
  end
val unbind : env -> locl ty -> env * locl ty
val set_local : env -> Local_id.t -> locl ty -> env
val is_using_var : env -> Local_id.t -> bool
val set_using_var : env -> Local_id.t -> env
val unset_local : env -> Local_id.t -> env
val get_locals : env -> local Local_id.Map.t
val get_local : env -> Local_id.t -> locl ty
val is_local_defined : env -> Local_id.t -> bool
val get_local_check_defined : env -> Nast.lid -> locl ty
val set_local_expr_id : env -> Local_id.t -> expression_id -> env
val get_local_expr_id : env -> Local_id.t -> expression_id option
val get_tpenv_lower_bounds : tpenv -> string -> tparam_bounds
val get_tpenv_upper_bounds : tpenv -> string -> tparam_bounds
val get_tpenv_reified: tpenv -> string -> bool
val get_tpenv_enforceable: tpenv -> string -> bool
val get_tpenv_newable: tpenv -> string -> bool
val get_lower_bounds : env -> string -> tparam_bounds
val get_upper_bounds : env -> string -> tparam_bounds
val get_reified: env -> string -> bool
val get_enforceable: env -> string -> bool
val get_newable: env -> string -> bool
val add_upper_bound :
  ?intersect:(locl ty -> locl ty list -> locl ty list) ->
  env -> string -> locl ty -> env
val add_lower_bound :
  ?union:(locl ty -> locl ty list -> locl ty list) ->
  env -> string -> locl ty -> env
val get_equal_bounds : env -> string -> tparam_bounds
val get_tparams : env -> 'a ty -> SSet.t
val add_upper_bound_global : env -> string -> locl ty -> env
val env_with_tpenv : env -> tpenv -> env
val env_with_mut : env -> Typing_mutability_env.mutability_env -> env
val get_env_mutability : env -> Typing_mutability_env.mutability_env
(** Given a list of type parameter names, attempt to simplify away those
type parameters by looking for a type to which they are equal in the tpenv.
If such a type exists, remove the type parameter from the tpenv.
Returns a set of substitutions mapping each type parameter name to the type
to which it is equal if found, otherwise to itself. *)
val simplify_tpenv : env -> (string * Ast.variance) list -> Reason.t -> env * locl ty SMap.t
val env_with_global_tpenv : env -> tpenv -> env
val add_generic_parameters : env -> Nast.tparam list -> env
val get_generic_parameters : env -> string list
val is_generic_parameter: env -> string -> bool

(* Get or add to bounds on type variables *)
val get_tyvar_lower_bounds : env -> Ident.t -> tparam_bounds
val get_tyvar_upper_bounds : env -> Ident.t -> tparam_bounds
(* Optionally supply intersection or union operations to simplify the bounds *)
val add_tyvar_upper_bound :
  ?intersect:(locl ty -> locl ty list -> locl ty list) ->
  env -> Ident.t -> locl ty -> env
val add_tyvar_lower_bound :
  ?union:(locl ty -> locl ty list -> locl ty list) ->
  env -> Ident.t -> locl ty -> env
val remove_tyvar_upper_bound :
  env -> Ident.t -> Ident.t -> env
val remove_tyvar_lower_bound :
  env -> Ident.t -> Ident.t -> env
val set_tyvar_appears_covariantly :
  env -> Ident.t -> env
val set_tyvar_appears_contravariantly :
  env -> Ident.t -> env
val set_tyvar_eager_solve_fail :
  env -> Ident.t -> env
val get_tyvar_appears_covariantly :
  env -> Ident.t -> bool
val get_tyvar_appears_contravariantly :
  env -> Ident.t -> bool
val get_tyvar_info :
  env -> Ident.t -> tyvar_info
val get_tyvar_eager_solve_fail :
  env -> Ident.t -> bool
val get_tyvar_type_const :
  env -> int -> Nast.sid -> (Nast.sid * locl ty) option
val set_tyvar_type_const :
  env -> int -> Nast.sid -> locl ty -> env
val get_tyvar_type_consts :
  env -> int -> (Nast.sid * locl ty) SMap.t
val remove_tyvar :
  env -> Ident.t -> env
val remove_equivalent_tyvars :
  env -> Ident.t -> env
val error_if_reactive_context : env -> (unit -> unit) -> unit
val error_if_shallow_reactive_context : env -> (unit -> unit) -> unit
val add_fresh_generic_parameter : env -> string -> reified:bool -> enforceable:bool -> newable:bool -> env * string
val is_fresh_generic_parameter : string -> bool
val get_tpenv_size : env -> int
val get_tpenv_tparams : env -> SSet.t
val set_env_reactive : env -> reactivity -> env
val set_env_function_pos: env -> Pos.t -> env
val env_reactivity: env -> reactivity
val env_local_reactive : env -> bool
val add_mutable_var : env -> Local_id.t -> Typing_mutability_env.mutability -> env
val local_is_mutable : include_borrowed: bool -> env -> Local_id.t -> bool
val function_is_mutable : env -> param_mutability option
val set_fun_mutable : env -> param_mutability option -> env
val env_with_locals : env -> local_types -> env
val reinitialize_locals : env -> env
val anon : local_env -> env -> (env -> env * Tast.expr * locl ty) -> env * Tast.expr * locl ty
val in_loop : env -> (env -> env * 'a) -> env * 'a
val in_try : env -> (env -> env * 'a) -> env * 'a
val in_case : env -> (env -> env * 'a) -> env * 'a
val save : tpenv -> env -> Tast.saved_env
val set_condition_type: env -> SMap.key -> Typing_defs.decl Typing_defs.ty -> env
val get_condition_type: env -> SMap.key -> Typing_defs.decl Typing_defs.ty option
val add_subtype_prop: env -> Typing_logic.subtype_prop -> env
val set_tyvar_variance : env -> Typing_defs.locl Typing_defs.ty -> env
val update_variance_after_bind : env -> int -> Typing_defs.locl Typing_defs.ty -> env
