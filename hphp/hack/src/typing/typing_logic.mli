(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs

type coercion_direction =
  | CoerceToDynamic
  | CoerceFromDynamic
[@@deriving show, eq]

(* Logical proposition about types *)
type subtype_prop =
  | IsSubtype of coercion_direction option * internal_type * internal_type
      (** IsSubtype(Some cd, ty1, ty2), if ty1 is a subtype of ty2 while potentially
          coercing to or from dynamic (depending on cd), written ty1 ~> ty2.
          IsSubtype(None, ty1, ty2) if ty1 is a subtype of ty2, written ty1 <: ty2
       *)
  | Conj of subtype_prop list  (** Conjunction. Conj [] means "true" *)
  | Disj of Typing_error.t option * subtype_prop list
      (** Disjunction. Disj f [] means "false".  The error message function f
        wraps the error that should be produced in this case. *)
[@@deriving show]

val equal_subtype_prop : subtype_prop -> subtype_prop -> bool

val size : subtype_prop -> int

(** Sum of the sizes of the disjunctions. *)
val n_disj : subtype_prop -> int

(** Sum of the sizes of the conjunctions. *)
val n_conj : subtype_prop -> int

val valid : subtype_prop

val invalid : fail:Typing_error.t option -> subtype_prop

val is_valid : subtype_prop -> bool

val is_unsat : subtype_prop -> bool

val get_error_if_unsat : subtype_prop -> Typing_error.t option

val conj : subtype_prop -> subtype_prop -> subtype_prop

val conj_list : subtype_prop list -> subtype_prop

val disj :
  fail:Typing_error.t option -> subtype_prop -> subtype_prop -> subtype_prop

val force_lazy_values : subtype_prop -> subtype_prop
