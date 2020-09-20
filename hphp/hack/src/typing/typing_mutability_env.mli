(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(* Tracks the different types of mutability of a given local variable
  Mutable: Objects marked as mutable must be affine, as in have 0-1 references.
  These are mutably owned, so they can be frozen to become immutable. They can
  have their properties set, but cannot be assigned to or passed to
  immutable contexts.

  Borrowed: Mutably borrowed. These can have their properties set, but cannot be
  frozen, and have all the same restrictions as mutable variables.
  A mutable parameter is always mutably borrowed.

  Const: Const types have all the restrictions of immutable vars and mutable
  vars. They cannot have their object properties be written toward, and cannot be
  reassigned. They are essentially read only.
*)
type mut_type =
  | Mutable
  | Borrowed
  | MaybeMutable
  | Immutable
[@@deriving eq]

type mutability = Pos.t * mut_type

(* Mapping from local variables to their mutability
  Local mutability is stored in the local environment.
  Once a variable is frozen, we remove it from the map,
  and it is treated like any immutable variable in its
  enclosing function.
*)
type mutability_env = mutability Local_id.Map.t

(* Given two mutability maps, intersect them. *)
val intersect_mutability :
  mutability_env -> mutability_env -> mutability_env -> mutability_env

val to_string : mutability -> string
