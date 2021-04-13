(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs

exception InconsistentTypeVarState of string

(** contains the inference env, containing all the information necessary to
perform type inference, including global type variables. *)
type t

(** contains only the information related to global type variables. This is returned
after typechecking individual functions and all the global inference environments
then get merged into a single one before being solved. *)
type t_global

type t_global_with_pos = Pos.t * t_global

module Log : sig
  val inference_env_as_value : t -> Typing_log_value.value

  val global_inference_env_as_value : t_global -> Typing_log_value.value

  (** Convert a type variable from a global inference environment into json *)
  val tyvar_to_json_g :
    (locl_ty -> string) ->
    (internal_type -> string) ->
    t_global ->
    Ident.t ->
    Hh_json.json

  (** Convert a type variable from an environment into json *)
  val tyvar_to_json :
    (locl_ty -> string) ->
    (internal_type -> string) ->
    t ->
    Ident.t ->
    Hh_json.json
end

val pp : Format.formatter -> t -> unit

val empty_inference_env : t

val get_allow_solve_globals : t -> bool

val set_allow_solve_globals : t -> bool -> t

val fresh_type : ?variance:Ast_defs.variance -> t -> Pos.t -> t * locl_ty

(** Same as fresh_type but takes a specific reason as parameter. *)
val fresh_type_reason :
  ?variance:Ast_defs.variance -> t -> Pos.t -> Reason.t -> t * locl_ty

val fresh_invariant_type_var : t -> Pos.t -> t * locl_ty

val open_tyvars : t -> Pos.t -> t

val get_current_tyvars : t -> Ident.t list

val close_tyvars : t -> t

val tyvar_is_solved : t -> Ident.t -> bool

val tyvar_is_solved_or_skip_global : t -> Ident.t -> bool

val tyvar_occurs_in_tyvar : t -> Ident.t -> in_:Ident.t -> bool

val get_tyvar_occurrences : t -> Ident.t -> ISet.t

val get_tyvars_in_tyvar : t -> Ident.t -> ISet.t

val contains_unsolved_tyvars : t -> Ident.t -> bool

val make_tyvar_no_more_occur_in_tyvar : t -> int -> no_more_in:int -> t

val bind : t -> ?tyvar_pos:Pos.t -> Ident.t -> locl_ty -> t

val add : t -> ?tyvar_pos:Pos.t -> Ident.t -> locl_ty -> t

val get_direct_binding : t -> Ident.t -> locl_ty option

val get_type : t -> Reason.t -> Ident.t -> t * locl_ty

val expand_var : t -> Reason.t -> Ident.t -> t * locl_ty

val expand_type : t -> locl_ty -> t * locl_ty

val expand_internal_type : t -> internal_type -> t * internal_type

val get_tyvar_pos : t -> Ident.t -> Pos.t

(* Get or add to bounds on type variables *)
val get_tyvar_lower_bounds : t -> Ident.t -> Internal_type_set.t

val get_tyvar_upper_bounds : t -> Ident.t -> Internal_type_set.t

val set_tyvar_lower_bounds : t -> Ident.t -> Internal_type_set.t -> t

val set_tyvar_upper_bounds : t -> Ident.t -> Internal_type_set.t -> t

(* Optionally supply intersection or union operations to simplify the bounds *)
val add_tyvar_upper_bound :
  ?intersect:(internal_type -> internal_type list -> internal_type list) ->
  t ->
  Ident.t ->
  internal_type ->
  t

val add_tyvar_lower_bound :
  ?union:(internal_type -> internal_type list -> internal_type list) ->
  t ->
  Ident.t ->
  internal_type ->
  t

val remove_tyvar_upper_bound : t -> Ident.t -> Ident.t -> t

val remove_tyvar_lower_bound : t -> Ident.t -> Ident.t -> t

val set_tyvar_appears_covariantly : t -> Ident.t -> t

val set_tyvar_appears_contravariantly : t -> Ident.t -> t

val set_tyvar_eager_solve_fail : t -> Ident.t -> t

val get_tyvar_appears_covariantly : t -> Ident.t -> bool

val get_tyvar_appears_contravariantly : t -> Ident.t -> bool

val get_tyvar_appears_invariantly : t -> Ident.t -> bool

val is_global_tyvar : t -> Ident.t -> bool

val get_global_tyvar_reason : t -> Ident.t -> Reason.t option

val new_global_tyvar : t -> ?i:int -> Typing_reason.t -> t * locl_ty

val wrap_ty_in_var : t -> Typing_reason.t -> locl_ty -> t * locl_ty

val get_tyvar_eager_solve_fail : t -> Ident.t -> bool

val get_tyvar_type_const : t -> Ident.t -> pos_id -> (pos_id * locl_ty) option

val set_tyvar_type_const : t -> Ident.t -> pos_id -> locl_ty -> t

val get_tyvar_type_consts : t -> Ident.t -> (pos_id * locl_ty) SMap.t

val add_subtype_prop : t -> Typing_logic.subtype_prop -> t

val get_current_pos_from_tyvar_stack : t -> Pos.t option

(** At the end of typechecking a function body, extract the remaining
inference env, which should only contain global type variables. *)
val extract_global_inference_env : t -> t * t_global

(** Get the list of all type variables in the environment *)
val get_vars : t -> Ident.t list

val is_empty_g : t_global -> bool

val get_vars_g : t_global -> Ident.t list

(** Get the list of all unsolved type variables in the environment *)
val get_unsolved_vars : t -> Ident.t list

val initialize_tyvar_as_in : as_in:t_global -> t -> int -> t

val get_tyvar_reason_exn_g : t_global -> Ident.t -> Reason.t

val get_tyvar_pos_exn_g : t_global -> Ident.t -> Pos_or_decl.t

(** Move a type variable from a global env to an env. If the variable
    already exists in the env, union the constraints of both variables.
    Doesn't perform any transitive closure. *)
val move_tyvar_from_genv_to_env : Ident.t -> to_:t -> from:t_global -> t

(** Move a type variable from a global env to an env. If the variable
    already exists in the env, create a new identifier for it and return it. *)
val copy_tyvar_from_genv_to_env :
  Ident.t -> to_:t -> from:t_global -> t * Ident.t

module Size : sig
  val ty_size : t -> locl_ty -> int

  val inference_env_size : t -> int

  val global_inference_env_size : t_global -> int
end

(** Only merge the tvenv parts. Resolve conflicts stupidly by taking the first mapping of the two. *)
val simple_merge : t -> t -> t

(** Merge type variables by transferring all constraints from the first to the second
    and binding the first to the second.
    Does not perform anything clever, especially does not perform any transitive closure.
    Will throw an exception of the type variables are already solved. *)
val merge_tyvars : t -> Ident.t -> Ident.t -> t

(** Gets the part of the subtype proposition that should not be added to the
constraint graph (e.g. because it contains disjunctions). *)
val get_nongraph_subtype_prop : t -> Typing_logic.subtype_prop

val is_alias_for_another_var : t -> Ident.t -> bool

(** Remove variables containing no information. *)
val compress : t -> t

(** Remove variables containing no information. *)
val compress_g : t_global -> t_global

(** Split multiple global environments into (weakly) connected components. *)
val connected_components_g :
  t_global_with_pos list ->
  additional_edges:ISet.t list ->
  (ISet.t * t_global_with_pos list) list

(** Remove solved variable from environment by replacing it by its binding. *)
val remove_var :
  t ->
  Ident.t ->
  search_in_upper_bounds_of:ISet.t ->
  search_in_lower_bounds_of:ISet.t ->
  t

(** Remove any occurence of the tyvar from the global environment *)
val forget_tyvar_g : t_global -> Ident.t -> t_global

(** Visit all types in the global inference environment.
  *
  * This functions takes a type mapper, instead of a type visitor, as it
  * will also visit constraint types.
  *
  * Localized type constants and PU-related types are ignored.
  *
  * Types returned by the type mapper are ignored.
  *)
val visit_types_g :
  t_global -> 'a Type_mapper_generic.internal_type_mapper_type -> 'a -> 'a

val unsolve : t -> Ident.t -> t
