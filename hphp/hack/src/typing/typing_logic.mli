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
  | PartialCoerceFromDynamic of collection_style * pos_id

(* Logical proposition about types *)
type subtype_prop =
  | Coerce of coercion_direction * locl_ty * locl_ty
      (** Coerce(cd, ty1, ty2), if ty1 is a subtype of ty2 while potentially
          coercing to or from dynamic (depending on cd), written ty1 ~> ty2. *)
  | IsSubtype of internal_type * internal_type
      (** IsSubtype(ty1,ty2) if ty1 is a subtype of ty2, written ty1 <: ty2 *)
  | Conj of subtype_prop list  (** Conjunction. Conj [] means "true" *)
  | Disj of (unit -> unit) * subtype_prop list
      (** Disjunction. Disj f [] means "false".  The error message function f
   * wraps the error that should be produced in this case. *)

val equal_subtype_prop : subtype_prop -> subtype_prop -> bool

val size : subtype_prop -> int

(** Sum of the sizes of the disjunctions. *)
val n_disj : subtype_prop -> int

(** Sum of the sizes of the conjunctions. *)
val n_conj : subtype_prop -> int

val valid : subtype_prop

val invalid : fail:(unit -> unit) -> subtype_prop

val is_valid : subtype_prop -> bool

val is_unsat : subtype_prop -> bool

val conj : subtype_prop -> subtype_prop -> subtype_prop

val conj_list : subtype_prop list -> subtype_prop

val disj : fail:(unit -> unit) -> subtype_prop -> subtype_prop -> subtype_prop
