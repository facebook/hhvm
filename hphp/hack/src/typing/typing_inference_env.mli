(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs

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
end

val pp : Format.formatter -> t -> unit

val empty_inference_env : t

val fresh_type : ?variance:Ast_defs.variance -> t -> Pos.t -> t * locl_ty

(** Same as fresh_type but takes a specific reason as parameter. *)
val fresh_type_reason :
  ?variance:Ast_defs.variance -> t -> Reason.t -> t * locl_ty

val fresh_invariant_type_var : t -> Pos.t -> t * locl_ty

val open_tyvars : t -> Pos.t -> t

val get_current_tyvars : t -> Ident.t list

val close_tyvars : t -> t

val tyvar_is_solved : t -> Ident.t -> bool

val tyvar_occurs_in_tyvar : t -> Ident.t -> in_:Ident.t -> bool

val get_tyvar_occurrences : t -> Ident.t -> ISet.t

val get_tyvars_in_tyvar : t -> Ident.t -> ISet.t

val contains_unsolved_tyvars : t -> Ident.t -> bool

val make_tyvar_no_more_occur_in_tyvar : t -> int -> no_more_in:int -> t

val add : t -> ?tyvar_pos:Pos.t -> Ident.t -> locl_ty -> t

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

val new_global_tyvar : t -> Ident.t -> ?variance:Ast_defs.variance -> Pos.t -> t

val get_tyvar_eager_solve_fail : t -> Ident.t -> bool

val get_tyvar_type_const :
  t -> Ident.t -> Aast.sid -> (Aast.sid * locl_ty) option

val get_tyvar_pu_access :
  t -> Ident.t -> Aast.sid -> (locl_ty * Aast.sid * locl_ty * Aast.sid) option

val set_tyvar_type_const : t -> Ident.t -> Aast.sid -> locl_ty -> t

val set_tyvar_pu_access :
  t -> Ident.t -> locl_ty -> Aast.sid -> locl_ty -> Aast.sid -> t

val get_tyvar_type_consts : t -> Ident.t -> (Aast.sid * locl_ty) SMap.t

val add_subtype_prop : t -> Typing_logic.subtype_prop -> t

val get_current_pos_from_tyvar_stack : t -> Pos.t option

(** At the end of typechecking a function body, extract the remaining
inference env, which should only contain global type variables. *)
val extract_global_inference_env : t -> t * t_global

val get_vars : t -> Ident.t list

val get_vars_g : t_global -> Ident.t list

val copy_tyvar_from_genv_to_env :
  Ident.t -> to_:t -> from:t_global -> t * Ident.t

module Size : sig
  val ty_size : t -> locl_ty -> int

  val inference_env_size : t -> int
end

(** Only merge the tvenv parts. Resolve conflicts stupidly by taking the first mapping of the two. *)
val simple_merge : t -> t -> t

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
  t_global_with_pos list -> t_global_with_pos list list
