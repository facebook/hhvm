(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Typing_env_types
open Decl_provider
open Typing_defs
module TPEnv = Type_parameter_env

val show_env : env -> string

val pp_env : Format.formatter -> env -> unit

val get_tcopt : env -> TypecheckerOptions.t

val fresh_type : env -> Pos.t -> env * locl_ty

val fresh_type_reason :
  ?variance:Ast_defs.variance -> env -> Reason.t -> env * locl_ty

val fresh_invariant_type_var : env -> Pos.t -> env * locl_ty

val open_tyvars : env -> Pos.t -> env

val get_current_tyvars : env -> Ident.t list

val add_current_tyvar :
  ?variance:Ast_defs.variance -> env -> Pos.t -> Ident.t -> env

val close_tyvars : env -> env

val get_var : env -> int -> env * int

val rename : env -> int -> int -> env

val add : env -> int -> locl_ty -> env

val get_type : env -> Reason.t -> int -> env * locl_ty

val expand_var : env -> Reason.t -> Ident.t -> env * locl_ty

val expand_type : env -> locl_ty -> env * locl_ty

val get_shape_field_name : Ast_defs.shape_field_name -> string

val get_shape_field_name_pos : Ast_defs.shape_field_name -> Pos.t

val empty :
  ?mode:FileInfo.mode ->
  TypecheckerOptions.t ->
  Relative_path.t ->
  droot:Typing_deps.Dep.variant option ->
  env

val is_typedef : typedef_key -> bool

val get_enum : env -> class_key -> class_decl option

val is_enum : env -> class_key -> bool

val get_enum_constraint : env -> class_key -> decl_ty option

val add_wclass : env -> string -> unit

val get_class : env -> class_key -> class_decl option

val get_class_dep : env -> class_key -> class_decl option

val get_typedef : env -> typedef_key -> typedef_decl option

val get_const : env -> class_decl -> string -> class_const option

val get_typeconst : env -> class_decl -> string -> typeconst_type option

val get_pu_enum : env -> class_decl -> string -> pu_enum_type option

val get_gconst : env -> gconst_key -> gconst_decl option

val get_static_member : bool -> env -> class_decl -> string -> class_elt option

val suggest_static_member :
  bool -> class_decl -> string -> (Pos.t * string) option

val get_member : bool -> env -> class_decl -> string -> class_elt option

val suggest_member : bool -> class_decl -> string -> (Pos.t * string) option

val get_construct : env -> class_decl -> class_elt option * consistent_kind

val get_return : env -> Typing_env_return_info.t

val set_return : env -> Typing_env_return_info.t -> env

val get_params : env -> (locl_ty * param_mode) Local_id.Map.t

val set_param : env -> Local_id.t -> locl_ty * param_mode -> env

val set_log_level : env -> string -> int -> env

val get_log_level : env -> string -> int

val set_env_log_function : (Pos.t -> string -> env -> env -> unit) -> unit

val log_env_change : string -> ?level:int -> env -> env -> env

val clear_params : env -> env

val with_env : env -> (env -> env * 'a) -> env * 'a

val is_static : env -> bool

val get_val_kind : env -> Typing_defs.val_kind

val get_self : env -> locl_ty

val get_self_id : env -> string

val is_outside_class : env -> bool

val get_parent_id : env -> string

val get_parent : env -> decl_ty

val get_fn_kind : env -> Ast_defs.fun_kind

val get_file : env -> Relative_path.t

val set_fn_kind : env -> Ast_defs.fun_kind -> env

val set_inside_ppl_class : env -> bool -> env

val add_anonymous : env -> anon -> env * int

val get_anonymous : env -> int -> anon option

val iter_anonymous : env -> (Pos.t -> locl_ty list -> unit) -> unit

val set_self_id : env -> string -> env

val set_self : env -> locl_ty -> env

val set_parent_id : env -> string -> env

val set_parent : env -> decl_ty -> env

val set_static : env -> env

val set_val_kind : env -> Typing_defs.val_kind -> env

val set_mode : env -> FileInfo.mode -> env

val get_mode : env -> FileInfo.mode

val is_strict : env -> bool

val is_decl : env -> bool

val forget_members : env -> Typing_fake_members.blame -> env

val get_fake_members : env -> Typing_fake_members.t

module FakeMembers : sig
  val update_fake_members : env -> Typing_fake_members.t -> env

  val is_valid : env -> Nast.expr -> string -> bool

  val is_valid_static : env -> Nast.class_id_ -> string -> bool

  val check_static_invalid :
    env -> Nast.class_id_ -> string -> locl_ty -> env * locl_ty

  val check_instance_invalid :
    env -> Nast.expr -> string -> locl_ty -> env * locl_ty

  val make : env -> Nast.expr -> string -> env * Local_id.t

  val make_static : env -> Nast.class_id_ -> string -> env * Local_id.t
end

val tany : env -> locl_phase ty_

val decl_tany : env -> decl_phase ty_

val next_cont_opt : env -> Typing_per_cont_env.per_cont_entry option

val all_continuations : env -> Typing_continuations.t list

val set_local : env -> Local_id.t -> locl_ty -> env

val is_using_var : env -> Local_id.t -> bool

val set_using_var : env -> Local_id.t -> env

val unset_local : env -> Local_id.t -> env

val get_local : env -> Local_id.t -> locl_ty

val get_locals : env -> Aast.lid list -> Typing_local_types.t

val set_locals : env -> Typing_local_types.t -> env

val set_fake_members : env -> Typing_fake_members.t -> env

val is_local_defined : env -> Local_id.t -> bool

val get_local_check_defined : env -> Aast.lid -> locl_ty

val set_local_expr_id :
  env -> Local_id.t -> Typing_local_types.expression_id -> env

val get_local_expr_id :
  env -> Local_id.t -> Typing_local_types.expression_id option

val get_tpenv : env -> TPEnv.t

val get_lower_bounds : env -> string -> TPEnv.tparam_bounds

val get_upper_bounds : env -> string -> TPEnv.tparam_bounds

val get_reified : env -> string -> Aast.reify_kind

val get_enforceable : env -> string -> bool

val get_newable : env -> string -> bool

val add_upper_bound :
  ?intersect:(locl_ty -> locl_ty list -> locl_ty list) ->
  env ->
  string ->
  locl_ty ->
  env

val add_lower_bound :
  ?union:(locl_ty -> locl_ty list -> locl_ty list) ->
  env ->
  string ->
  locl_ty ->
  env

val get_equal_bounds : env -> string -> TPEnv.tparam_bounds

val get_tparams : env -> locl_ty -> SSet.t

val add_upper_bound_global : env -> string -> locl_ty -> env

val env_with_tpenv : env -> TPEnv.t -> env

val env_with_mut : env -> Typing_mutability_env.mutability_env -> env

val get_env_mutability : env -> Typing_mutability_env.mutability_env

val env_with_global_tpenv : env -> TPEnv.t -> env

val add_generic_parameters : env -> decl_tparam list -> env

val get_generic_parameters : env -> string list

val is_generic_parameter : env -> string -> bool

(* Get or add to bounds on type variables *)
val get_tyvar_lower_bounds : env -> Ident.t -> TPEnv.tparam_bounds

val get_tyvar_upper_bounds : env -> Ident.t -> TPEnv.tparam_bounds

val set_tyvar_lower_bounds : env -> Ident.t -> TPEnv.tparam_bounds -> env

val set_tyvar_upper_bounds : env -> Ident.t -> TPEnv.tparam_bounds -> env

(* Optionally supply intersection or union operations to simplify the bounds *)
val add_tyvar_upper_bound :
  ?intersect:(locl_ty -> locl_ty list -> locl_ty list) ->
  env ->
  Ident.t ->
  locl_ty ->
  env

val add_tyvar_lower_bound :
  ?union:(locl_ty -> locl_ty list -> locl_ty list) ->
  env ->
  Ident.t ->
  locl_ty ->
  env

val remove_tyvar_upper_bound : env -> Ident.t -> Ident.t -> env

val remove_tyvar_lower_bound : env -> Ident.t -> Ident.t -> env

val set_tyvar_appears_covariantly : env -> Ident.t -> env

val set_tyvar_appears_contravariantly : env -> Ident.t -> env

val set_tyvar_eager_solve_fail : env -> Ident.t -> env

val get_tyvar_appears_covariantly : env -> Ident.t -> bool

val get_tyvar_appears_contravariantly : env -> Ident.t -> bool

val get_tyvar_appears_invariantly : env -> Ident.t -> bool

val get_tyvar_info : env -> Ident.t -> tyvar_info_

val create_global_tyvar :
  ?variance:Ast_defs.variance -> env -> Ident.t -> Pos.t -> env

val get_tyvar_eager_solve_fail : env -> Ident.t -> bool

val create_tyvar_info : ?variance:Ast_defs.variance -> Pos.t -> tyvar_info_

val empty_tyvar_info : tyvar_info_

val get_tyvar_type_const :
  env -> int -> Aast.sid -> (Aast.sid * locl_ty) option

val set_tyvar_type_const : env -> int -> Aast.sid -> locl_ty -> env

val get_tyvar_type_consts : env -> int -> (Aast.sid * locl_ty) SMap.t

val remove_tyvar : env -> Ident.t -> env

val error_if_reactive_context : env -> (unit -> unit) -> unit

val error_if_shallow_reactive_context : env -> (unit -> unit) -> unit

val add_fresh_generic_parameter :
  env ->
  string ->
  reified:Aast.reify_kind ->
  enforceable:bool ->
  newable:bool ->
  env * string

val is_fresh_generic_parameter : string -> bool

val get_tpenv_size : env -> int

val get_tpenv_tparams : env -> SSet.t

val set_env_reactive : env -> reactivity -> env

val set_env_function_pos : env -> Pos.t -> env

val set_env_pessimize : env -> env

val env_local_reactive : env -> bool

val add_mutable_var :
  env -> Local_id.t -> Typing_mutability_env.mutability -> env

val local_is_mutable : include_borrowed:bool -> env -> Local_id.t -> bool

val function_is_mutable : env -> param_mutability option

val set_fun_mutable : env -> param_mutability option -> env

val env_with_locals : env -> Typing_per_cont_env.t -> env

val reinitialize_locals : env -> env

val anon :
  local_env ->
  env ->
  (env -> env * Tast.expr * locl_ty) ->
  env * Tast.expr * locl_ty

val in_loop : env -> (env -> env * 'a) -> env * 'a

val in_try : env -> (env -> env * 'a) -> env * 'a

val in_case : env -> (env -> env * 'a) -> env * 'a

val save : TPEnv.t -> env -> Tast.saved_env

val set_condition_type : env -> SMap.key -> Typing_defs.decl_ty -> env

val get_condition_type : env -> SMap.key -> Typing_defs.decl_ty option

val add_subtype_prop : env -> Typing_logic.subtype_prop -> env

val set_tyvar_variance : env -> ?flip:bool -> Typing_defs.locl_ty -> env

val update_variance_after_bind : env -> int -> Typing_defs.locl_ty -> env

val is_consistent : env -> bool

val mark_inconsistent : env -> env
