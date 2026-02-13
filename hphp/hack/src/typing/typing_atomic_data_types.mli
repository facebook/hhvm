(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs
open Typing_env_types

(** Computes runtime data types for types that cannot be decomposed into smaller types *)

type t

(** Types we consider as atomic *)
type atomic_ty =
  | Primitive of Aast.tprim
  | Function
  | Nonnull
  | Tuple
  | Shape
  | Label
  | Class of string * locl_ty list

(** When safe_for_are_disjoint is true, do not specially handle interfaces'
    sealed attribute or required ancestors.
    Temporary flag to avoid an issue with `are_disjoint` T215053987 *)
val of_ty : safe_for_are_disjoint:bool -> env -> atomic_ty -> env * t

(** When safe_for_are_disjoint is true, do not specially handle interfaces'
    sealed attribute or required ancestors.
    Temporary flag to avoid an issue with `are_disjoint` T215053987 *)
val of_tag :
  safe_for_are_disjoint:bool -> env -> Typing_defs_core.type_tag -> env * t

(** The empty set of data types *)
val empty : t

(** Computes the complement for the set of values contained in [t] *)
val complement : t -> t

(** Computes the union of the two given sets **)
val union : t -> t -> t

(** Returns true if the given data types are known to have no values in
    common, otherwise returns false *)
val are_disjoint : env -> t -> t -> bool

(** Returns true iff the two given types are known to be disjoint based on
    comparison of their datatypes *)
val are_locl_tys_disjoint : env -> locl_ty -> locl_ty -> bool
