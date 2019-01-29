(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs

module Env = Typing_env

(** Performs the union of two types.
The union is the least upper bound of the subtyping relation.

There is however one approximation: if type A is covariant,
then
  A<T1> | A<T2> = A<T1|T2>

This approximation is necessary to avoid type growing exponentially in size.
We have seen cases where it would otherwise generate unions involving all
the subsets of a set of types. *)
val union: Env.env -> locl ty -> locl ty -> Env.env * locl ty
(** Since computing the union of a list is quadratic, this is a linear
approximation which simply removes duplicates from the list.
If the result is a singleton union, discard the union constructor. *)
val union_list_approx: locl ty list -> Typing_reason.t -> locl ty
