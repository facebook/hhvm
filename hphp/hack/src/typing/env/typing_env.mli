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
module Cls = Decl_provider.Class
module TPEnv = Type_parameter_env

type 'a class_or_typedef_result =
      'a Decl_enforceability.class_or_typedef_result =
  | ClassResult of 'a
  | TypedefResult of Typing_defs.typedef_type

val simplify_unions_ref : (env -> locl_ty -> env * locl_ty) ref

val show_env : env -> string

val pp_env : Format.formatter -> env -> unit

val get_tcopt : env -> TypecheckerOptions.t

val map_tcopt : env -> f:(TypecheckerOptions.t -> TypecheckerOptions.t) -> env

val get_deps_mode : env -> Typing_deps_mode.t

val get_ctx : env -> Provider_context.t

val get_tracing_info : env -> Decl_counters.tracing_info option

(** Functions related to type variable scopes *)

(**
   Open a new "type variable" scope and record this in the environment.
   Within this scope, you can
   - generate fresh type variables, using [fresh_type_X] functions
   - query the currently fresh type variables, using [get_current_tyvars]
   - close the scope

   The usual usage pattern within type inference is:

   1. Open a type variable scope using [open_tyvars] before checking an AST node.

   2. Check the AST node; generate fresh type variables as necessary; make calls
     to subtyping functions such as [Typing_ops.sub_type] that will record constraints
     on those type variables.

   3. Call [set_tyvar_variance] on the type of the expression to correctly set the
     variance of the type variables.

   4. Call [Typing_solver.close_tyvars_and_solve] to solve type variables that can
     be solved immediately, and close the type variable scope.
*)
val open_tyvars : env -> Pos.t -> env

(** Generate a fresh type variable type with variance not yet recorded *)
val fresh_type : env -> Pos.t -> env * locl_ty

(** Generate a fresh type variable type with optional variance and
    the given reason information *)
val fresh_type_reason :
  ?variance:Ast_defs.variance -> env -> Pos.t -> Reason.t -> env * locl_ty

(** Generate a fresh type variable type that is assumed to be invariant, so
    it won't be solved automatically at the end of the scope *)
val fresh_type_invariant : env -> Pos.t -> env * locl_ty

(** Generate a fresh type variable to stand for an unknown type in the
    case of type errors. *)
val fresh_type_error : env -> Pos.t -> env * locl_ty

val fresh_type_error_contravariant : env -> Pos.t -> env * locl_ty

(** What type variables are fresh in the current scope? *)
val get_current_tyvars : env -> Ident.t list

(** Close the current type variable scope.
    You might want to call [Typing_solver.close_tyvars_and_solve] instead. *)
val close_tyvars : env -> env

val add : env -> ?tyvar_pos:Pos.t -> int -> locl_ty -> env

val make_tyvar_no_more_occur_in_tyvar : env -> int -> no_more_in:int -> env

val tyvar_is_solved : env -> int -> bool

val wrap_ty_in_var :
  env -> Reason.t -> locl_ty -> Typing_env_types.env * locl_ty

val get_type : env -> Reason.t -> int -> env * locl_ty

val expand_var : env -> Reason.t -> Ident.t -> env * locl_ty

val expand_type : env -> locl_ty -> env * locl_ty

val expand_internal_type : env -> internal_type -> env * internal_type

val is_typedef : env -> type_key -> bool

val is_typedef_visible :
  env -> ?expand_visible_newtype:bool -> name:string -> typedef_type -> bool

val get_enum : env -> type_key -> class_decl option

val is_enum : env -> type_key -> bool

val is_enum_class : env -> type_key -> bool

val get_enum_constraint : env -> type_key -> decl_ty option

val env_with_method_droot_member : env -> string -> static:bool -> env

val env_with_constructor_droot_member : env -> env

(** Get class declaration from the appropriate backend and add dependency. *)
val get_class : env -> type_key -> class_decl option

val add_parent_dep : env -> skip_constructor_dep:bool -> string -> unit

(** Get function declaration from the appropriate backend and add dependency. *)
val get_fun : env -> Decl_provider.fun_key -> Decl_provider.fun_decl option

(** Get type alias declaration from the appropriate backend and add dependency. *)
val get_typedef : env -> type_key -> typedef_decl option

val get_class_or_typedef :
  env -> type_key -> Typing_classes_heap.Api.t class_or_typedef_result option

(** Get class constant declaration from the appropriate backend and add dependency. *)
val get_const : env -> class_decl -> string -> class_const option

(** Get class constants declaration from the appropriate backend and add dependency. *)
val consts : env -> class_decl -> (string * class_const) list

(** Get type constant declaration from the appropriate backend and add dependency. *)
val get_typeconst : env -> class_decl -> string -> typeconst_type option

(** Get global constant declaration from the appropriate backend and add dependency. *)
val get_gconst : env -> gconst_key -> gconst_decl option

(** Get static member declaration of a class from the appropriate backend and add dependency. *)
val get_static_member : bool -> env -> class_decl -> string -> class_elt option

val most_similar : string -> 'a list -> ('a -> string) -> 'a option

val suggest_static_member :
  bool -> class_decl -> string -> (Pos_or_decl.t * string) option

(** Get class member declaration from the appropriate backend and add dependency. *)
val get_member : bool -> env -> class_decl -> string -> class_elt option

val suggest_member :
  bool -> class_decl -> string -> (Pos_or_decl.t * string) option

(** Get class constructor declaration from the appropriate backend and add dependency. *)
val get_construct : env -> class_decl -> class_elt option * consistent_kind

val get_return : env -> Typing_env_return_info.t

val set_return : env -> Typing_env_return_info.t -> env

val get_readonly : env -> bool

val set_readonly : env -> bool -> env

val get_params : env -> (locl_ty * Pos.t * locl_ty option) Local_id.Map.t

val set_param : env -> Local_id.t -> locl_ty * Pos.t * locl_ty option -> env

val set_log_level : env -> string -> int -> env

val get_log_level : env -> string -> int

val log_env_change_ : string -> ?level:int -> env -> env * 'res -> env * 'res

val log_env_change : string -> ?level:int -> env -> env -> env

val clear_params : env -> env

val with_env : env -> (env -> env * 'a) -> env * 'a

val with_origin : env -> Decl_counters.origin -> (env -> env * 'a) -> env * 'a

val with_origin2 :
  env -> Decl_counters.origin -> (env -> env * 'a * 'b) -> env * 'a * 'b

val with_inside_expr_tree :
  env -> Aast_defs.hint -> (env -> env * 'a * 'b) -> env * 'a * 'b

val with_outside_expr_tree :
  env -> (env -> Aast.class_name option -> env * 'a * 'b) -> env * 'a * 'b

val inside_expr_tree : env -> Aast_defs.hint -> env

val outside_expr_tree : env -> env

val is_in_expr_tree : env -> bool

val is_static : env -> bool

val get_val_kind : env -> Typing_defs.val_kind

val get_self_ty : env -> locl_ty option

val get_self_class_type : env -> (pos_id * exact * locl_ty list) option

val get_self_id : env -> string option

val get_self_class : env -> class_decl option

val get_parent_id : env -> string option

val get_parent_ty : env -> decl_ty option

val get_parent_class : env -> class_decl option

val get_fn_kind : env -> Ast_defs.fun_kind

val get_file : env -> Relative_path.t

val get_current_decl_and_file : env -> Pos_or_decl.ctx

(** Check that the position is in the current decl and if it is, resolve
    it with the current file. *)
val fill_in_pos_filename_if_in_current_decl :
  env -> Pos_or_decl.t -> Pos.t option

(** This will check that the first position of the given reasons is in the
    current decl and if yes use it as primary error position. If no,
    it will error at a default position in the current file and log the failed
    assertion.
    This also sets the error code to the code for unification error
    if none is provided. *)
val unify_error_assert_primary_pos_in_current_decl :
  env -> Typing_error.Reasons_callback.t

(** This will check that the first position of the given reasons is in the
    current decl and if yes use it as primary error position. If no,
    it will error at a default position in the current file and log the failed
    assertion.
    This also sets the error code to the code for invalid type hint error
    if none is provided. *)
val invalid_type_hint_assert_primary_pos_in_current_decl :
  env -> Typing_error.Reasons_callback.t

val set_fn_kind : env -> Ast_defs.fun_kind -> env

val set_current_module : env -> Ast_defs.id option -> env

val set_internal : env -> bool -> env

val set_support_dynamic_type : env -> bool -> env

val set_everything_sdt : env -> bool -> env

val set_no_auto_likes : env -> bool -> env

val get_module : env -> module_key -> module_decl option

val get_current_module : env -> string option

(** Register the current top-level structure as being dependent on the current
    module *)
val make_depend_on_current_module : Typing_env_types.env -> unit

val mark_members_declared_in_depgraph :
  Typing_env_types.env -> Nast.class_ -> unit

val get_internal : env -> bool

val get_support_dynamic_type : env -> bool

val get_no_auto_likes : env -> bool

val set_self : env -> string -> locl_ty -> env

(** Run a given function with self unset in the environment.
    Restore self after the function finishes executing. This is used when
    checking attributes applied to a class, because the self keyword is not
    allowed as part of an argument to an attribute *)
val run_with_no_self : env -> (env -> env * 'a) -> env * 'a

val set_parent : env -> string -> decl_ty -> env

val set_static : env -> env

val set_val_kind : env -> Typing_defs.val_kind -> env

val set_mode : env -> FileInfo.mode -> env

val get_mode : env -> FileInfo.mode

val is_strict : env -> bool

val is_hhi : env -> bool

val forget_members : env -> Reason.blame -> env

val forget_prefixed_members : env -> Local_id.t -> Reason.blame -> env

val forget_suffixed_members : env -> string -> Reason.blame -> env

val get_fake_members : env -> Typing_fake_members.t

module FakeMembers : sig
  val update_fake_members : env -> Typing_fake_members.t -> env

  val is_valid : env -> Nast.expr -> string -> bool

  val is_valid_static : env -> Nast.class_id_ -> string -> bool

  val check_static_invalid :
    env -> Nast.class_id_ -> string -> locl_ty -> env * locl_ty

  val check_instance_invalid :
    env -> Nast.expr -> string -> locl_ty -> env * locl_ty

  val make : env -> Nast.expr -> string -> Pos.t -> env * Local_id.t

  val make_static : env -> Nast.class_id_ -> string -> Pos.t -> env * Local_id.t
end

val tany : env -> locl_phase ty_

val next_cont_opt : env -> Typing_per_cont_env.per_cont_entry option

val all_continuations : env -> Typing_continuations.t list

val set_local :
  ?immutable:bool ->
  is_defined:bool ->
  bound_ty:locl_ty option ->
  env ->
  Local_id.t ->
  locl_ty ->
  Pos.t ->
  env

val is_using_var : env -> Local_id.t -> bool

val set_using_var : env -> Local_id.t -> env

val unset_local : env -> Local_id.t -> env

val get_local : env -> Local_id.t -> Typing_local_types.local

val get_locals :
  ?quiet:bool -> env -> 'a Aast.capture_lid list -> Typing_local_types.t

val set_locals : env -> Typing_local_types.t -> env

val set_fake_members : env -> Typing_fake_members.t -> env

val is_local_present : env -> Local_id.t -> bool

val get_local_check_defined : env -> Aast.lid -> Typing_local_types.local

val set_local_expr_id :
  env ->
  Local_id.t ->
  Typing_local_types.expression_id ->
  (env, env * Typing_error.t) result

val get_local_expr_id :
  env -> Local_id.t -> Typing_local_types.expression_id option

val get_tpenv : env -> TPEnv.t

val get_global_tpenv : env -> TPEnv.t

val get_pos_and_kind_of_generic :
  env -> string -> (Pos_or_decl.t * Typing_kinding_defs.kind) option

val get_lower_bounds : env -> string -> locl_ty list -> TPEnv.tparam_bounds

val get_upper_bounds : env -> string -> locl_ty list -> TPEnv.tparam_bounds

val get_equal_bounds : env -> string -> locl_ty list -> TPEnv.tparam_bounds

val get_reified : env -> string -> Aast.reify_kind

val get_enforceable : env -> string -> bool

val get_newable : env -> string -> bool

val get_require_dynamic : env -> string -> bool

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

val get_tparams : env -> locl_ty -> SSet.t

val add_lower_bound_global : env -> string -> locl_ty -> env

val add_upper_bound_global : env -> string -> locl_ty -> env

val env_with_tpenv : env -> TPEnv.t -> env

val env_with_global_tpenv : env -> TPEnv.t -> env

val add_generic_parameters : env -> decl_tparam list -> env

val get_generic_parameters : env -> string list

val is_generic_parameter : env -> string -> bool

val get_tyvar_pos : env -> Ident.t -> Pos.t

(* Get or add to bounds on type variables *)
val get_tyvar_lower_bounds : env -> Ident.t -> Internal_type_set.t

val get_tyvar_upper_bounds : env -> Ident.t -> Internal_type_set.t

val set_tyvar_lower_bounds : env -> Ident.t -> Internal_type_set.t -> env

val set_tyvar_upper_bounds : env -> Ident.t -> Internal_type_set.t -> env

(* Optionally supply intersection or union operations to simplify the bounds *)
val add_tyvar_upper_bound :
  ?intersect:(internal_type -> internal_type list -> internal_type list) ->
  env ->
  Ident.t ->
  internal_type ->
  env

val add_tyvar_upper_bound_and_update_variances :
  ?intersect:(internal_type -> internal_type list -> internal_type list) ->
  env ->
  Ident.t ->
  internal_type ->
  env

val add_tyvar_lower_bound :
  ?union:(internal_type -> internal_type list -> internal_type list) ->
  env ->
  Ident.t ->
  internal_type ->
  env

val add_tyvar_lower_bound_and_update_variances :
  ?union:(internal_type -> internal_type list -> internal_type list) ->
  env ->
  Ident.t ->
  internal_type ->
  env

val remove_tyvar_upper_bound : env -> Ident.t -> Ident.t -> env

val remove_tyvar_lower_bound : env -> Ident.t -> Ident.t -> env

val set_tyvar_appears_covariantly : env -> Ident.t -> env

val set_tyvar_appears_contravariantly : env -> Ident.t -> env

val set_tyvar_eager_solve_fail : env -> Ident.t -> env

val get_tyvar_appears_covariantly : env -> Ident.t -> bool

val get_tyvar_appears_contravariantly : env -> Ident.t -> bool

val get_tyvar_appears_invariantly : env -> Ident.t -> bool

val get_tyvar_eager_solve_fail : env -> Ident.t -> bool

val get_tyvar_type_const : env -> int -> pos_id -> (pos_id * locl_ty) option

val set_tyvar_type_const : env -> int -> pos_id -> locl_ty -> env

val get_tyvar_type_consts : env -> int -> (pos_id * locl_ty) SMap.t

val get_all_tyvars : env -> Ident.t list

val fresh_param_name : env -> string -> env * string

val add_fresh_generic_parameter_by_kind :
  env -> Pos_or_decl.t -> string -> Typing_kinding_defs.kind -> env * string

val add_fresh_generic_parameter :
  env ->
  Pos_or_decl.t ->
  string ->
  reified:Aast.reify_kind ->
  enforceable:bool ->
  newable:bool ->
  env * string

val is_fresh_generic_parameter : string -> bool

val get_tpenv_size : env -> int

val get_tpenv_tparams : env -> SSet.t

val set_env_callable_pos : env -> Pos.t -> env

val fun_is_constructor : env -> bool

val set_fun_is_constructor : env -> bool -> env

val set_fun_tast_info : env -> Tast.fun_tast_info -> env

val env_with_locals : env -> Typing_per_cont_env.t -> env

val reinitialize_locals : env -> env

val closure : env -> (env -> env * 'a) -> env * 'a

val in_try : env -> (env -> env * 'a) -> env * 'a

val save : TPEnv.t -> env -> Tast.saved_env

val set_condition_type : env -> SMap.key -> Typing_defs.decl_ty -> env

val get_condition_type : env -> SMap.key -> Typing_defs.decl_ty option

val add_subtype_prop : env -> Typing_logic.subtype_prop -> env

val set_tyvar_variance_i :
  env -> ?flip:bool -> ?for_all_vars:bool -> Typing_defs.internal_type -> env

val set_tyvar_variance :
  env -> ?flip:bool -> ?for_all_vars:bool -> Typing_defs.locl_ty -> env

val update_variance_after_bind : env -> int -> Typing_defs.locl_ty -> env

val is_consistent : env -> bool

val mark_inconsistent : env -> env

val get_package_for_module : env -> string -> Package.t option

val get_package_by_name : env -> string -> Package.t option

val load_packages : env -> SSet.t -> env

val load_cross_packages_from_attr : env -> ('a, 'b) Aast.user_attributes -> env

val with_packages : env -> SSet.t -> (env -> env * 'a) -> env * 'a

val is_package_loaded : env -> string -> bool

(** Remove solved variable from environment by replacing it by its binding. *)
val remove_var :
  env ->
  Ident.t ->
  search_in_upper_bounds_of:ISet.t ->
  search_in_lower_bounds_of:ISet.t ->
  env

module Log : sig
  (** Convert a type variable from an environment into json *)
  val tyvar_to_json :
    (locl_ty -> string) ->
    (internal_type -> string) ->
    env ->
    Ident.t ->
    Hh_json.json
end

val make_ident : env -> Ident_provider.Ident.t
