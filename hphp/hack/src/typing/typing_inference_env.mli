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
perform type inference. *)
type t

module Log : sig
  val inference_env_as_value : t -> Typing_log_value.value

  (** Convert a type variable from an environment into json *)
  val tyvar_to_json :
    (locl_ty -> string) ->
    (internal_type -> string) ->
    t ->
    Tvid.t ->
    Hh_json.json
end

val empty_inference_env : t

val fresh_type :
  ?variance:Ast_defs.variance -> t -> Tvid.provider -> Pos.t -> t * locl_ty

(** Same as fresh_type but takes a specific reason as parameter. *)
val fresh_type_reason :
  ?variance:Ast_defs.variance ->
  t ->
  Tvid.provider ->
  Pos.t ->
  Reason.t ->
  t * locl_ty

val fresh_type_invariant : t -> Tvid.provider -> Pos.t -> t * locl_ty

val open_tyvars : t -> Pos.t -> t

val get_current_tyvars : t -> Tvid.t list

val close_tyvars : t -> t

val tyvar_is_solved : t -> Tvid.t -> bool

val tyvar_occurs_in_tyvar : t -> Tvid.t -> in_:Tvid.t -> bool

val get_tyvar_occurrences : t -> Tvid.t -> Tvid.Set.t

val get_tyvars_in_tyvar : t -> Tvid.t -> Tvid.Set.t

val contains_unsolved_tyvars : t -> Tvid.t -> bool

val make_tyvar_no_more_occur_in_tyvar : t -> Tvid.t -> no_more_in:Tvid.t -> t

val bind : t -> ?tyvar_pos:Pos.t -> Tvid.t -> locl_ty -> t

val add : t -> ?tyvar_pos:Pos.t -> Tvid.t -> locl_ty -> t

val get_direct_binding : t -> Tvid.t -> locl_ty option

val get_type : t -> Reason.t -> Tvid.t -> t * locl_ty

val expand_var : t -> Reason.t -> Tvid.t -> t * locl_ty

val expand_type : t -> locl_ty -> t * locl_ty

val expand_internal_type : t -> internal_type -> t * internal_type

val fully_expand_type : t -> locl_ty -> t * locl_ty

val get_tyvar_pos : t -> Tvid.t -> Pos.t

(* Get or add to bounds on type variables *)
val get_tyvar_lower_bounds : t -> Tvid.t -> Internal_type_set.t

val get_tyvar_upper_bounds : t -> Tvid.t -> Internal_type_set.t

val set_tyvar_lower_bounds : t -> Tvid.t -> Internal_type_set.t -> t

val set_tyvar_upper_bounds : t -> Tvid.t -> Internal_type_set.t -> t

(* Optionally supply intersection or union operations to simplify the bounds *)
val add_tyvar_upper_bound :
  ?intersect:(internal_type -> internal_type list -> internal_type list) ->
  t ->
  Tvid.t ->
  internal_type ->
  t

val add_tyvar_lower_bound :
  ?union:(internal_type -> internal_type list -> internal_type list) ->
  t ->
  Tvid.t ->
  internal_type ->
  t

val remove_tyvar_upper_bound : t -> Tvid.t -> Tvid.t -> t

val remove_tyvar_lower_bound : t -> Tvid.t -> Tvid.t -> t

val set_tyvar_appears_covariantly : t -> Tvid.t -> t

val set_tyvar_appears_contravariantly : t -> Tvid.t -> t

val set_tyvar_eager_solve_fail : t -> Tvid.t -> t

val get_tyvar_appears_covariantly : t -> Tvid.t -> bool

val get_tyvar_appears_contravariantly : t -> Tvid.t -> bool

val get_tyvar_appears_invariantly : t -> Tvid.t -> bool

val wrap_ty_in_var :
  t -> Tvid.provider -> Typing_reason.t -> locl_ty -> t * locl_ty

val get_tyvar_eager_solve_fail : t -> Tvid.t -> bool

val get_tyvar_type_const : t -> Tvid.t -> pos_id -> (pos_id * locl_ty) option

val set_tyvar_type_const : t -> Tvid.t -> pos_id -> locl_ty -> t

val get_tyvar_type_consts : t -> Tvid.t -> (pos_id * locl_ty) SMap.t

val add_subtype_prop : t -> Typing_logic.subtype_prop -> t

val get_current_pos_from_tyvar_stack : t -> Pos.t option

(** Get the list of all type variables in the environment *)
val get_vars : t -> Tvid.t list

(** Get the list of all unsolved type variables in the environment *)
val get_unsolved_vars : t -> Tvid.t list

module Size : sig
  val ty_size : t -> locl_ty -> int

  val inference_env_size : t -> int
end

(** Only merge the tvenv parts. Resolve conflicts stupidly by taking the first mapping of the two. *)
val simple_merge : t -> t -> t

(** Merge type variables by transferring all constraints from the first to the second
    and binding the first to the second.
    Does not perform anything clever, especially does not perform any transitive closure.
    Will throw an exception of the type variables are already solved. *)
val merge_tyvars : t -> Tvid.t -> Tvid.t -> t

(** Gets the part of the subtype proposition that should not be added to the
constraint graph (e.g. because it contains disjunctions). *)
val get_nongraph_subtype_prop : t -> Typing_logic.subtype_prop

val is_alias_for_another_var : t -> Tvid.t -> bool

(** Remove variables containing no information. *)
val compress : t -> t

(** Remove solved variable from environment by replacing it by its binding. *)
val remove_var :
  t ->
  Tvid.t ->
  search_in_upper_bounds_of:Tvid.Set.t ->
  search_in_lower_bounds_of:Tvid.Set.t ->
  t

val force_lazy_values : t -> t
