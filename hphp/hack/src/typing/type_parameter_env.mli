(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type tparam_name = string

type tparam_bounds = Typing_set.t

type tparam_info = Typing_kinding_defs.kind

type t [@@deriving hash]

val empty : t

val mem : tparam_name -> t -> bool

val get : tparam_name -> t -> tparam_info option

val get_with_pos : tparam_name -> t -> (Pos_or_decl.t * tparam_info) option

val add : def_pos:Pos_or_decl.t -> tparam_name -> tparam_info -> t -> t

val union : t -> t -> t

val size : t -> int

val fold : (tparam_name -> tparam_info -> 'a -> 'a) -> t -> 'a -> 'a

val merge_env :
  'env ->
  t ->
  t ->
  combine:
    ('env ->
    tparam_name ->
    (Pos_or_decl.t * tparam_info) option ->
    (Pos_or_decl.t * tparam_info) option ->
    'env * (Pos_or_decl.t * tparam_info) option) ->
  'env * t

val get_lower_bounds :
  t -> tparam_name -> Typing_defs.locl_ty list -> tparam_bounds

val get_upper_bounds :
  t -> tparam_name -> Typing_defs.locl_ty list -> tparam_bounds

(** value > 0, indicates higher-kinded type parameter *)
val get_arity : t -> tparam_name -> int

val get_reified : t -> tparam_name -> Aast.reify_kind

val get_enforceable : t -> tparam_name -> bool

val get_newable : t -> tparam_name -> bool

val get_require_dynamic : t -> tparam_name -> bool

val get_tparam_names : t -> tparam_name list

val get_tparams : t -> (Pos_or_decl.t * tparam_info) SMap.t

val is_consistent : t -> bool

(** When we detect that the set of constraints on the type parameters cannot be
    satisfied, we can mark the env as inconsistent using this. *)
val mark_inconsistent : t -> t

(** Add a single new upper bound to a generic parameter.
    If the optional [intersect] operation is supplied, then use this to avoid
    adding redundant bounds by merging the type with existing bounds. This makes
    sense because a conjunction of upper bounds
      (T <: t1) /\ ... /\ (T <: tn)
    is equivalent to a single upper bound
      T <: (t1 & ... & tn)
    *)
val add_upper_bound :
  ?intersect:
    (Typing_defs.locl_ty ->
    Typing_defs.locl_ty list ->
    Typing_defs.locl_ty list) ->
  t ->
  tparam_name ->
  Typing_defs.locl_ty ->
  t

(** Add a single new upper lower to a generic parameter.
    If the optional [union] operation is supplied, then use this to avoid
    adding redundant bounds by merging the type with existing bounds. This makes
    sense because a conjunction of lower bounds
      (t1 <: T) /\ ... /\ (tn <: T)
    is equivalent to a single lower bound
      (t1 | ... | tn) <: T
    *)
val add_lower_bound :
  ?union:
    (Typing_defs.locl_ty ->
    Typing_defs.locl_ty list ->
    Typing_defs.locl_ty list) ->
  t ->
  tparam_name ->
  Typing_defs.locl_ty ->
  t

(** Add type parameters to environment, initially with no bounds.
    Existing type parameters with the same name will be overridden. *)
val add_generic_parameters : t -> Typing_defs.decl_tparam list -> t

val remove : t -> tparam_name -> t

val get_parameter_names : tparam_info -> tparam_name list

val pp : Format.formatter -> t -> unit

val force_lazy_values : t -> t
