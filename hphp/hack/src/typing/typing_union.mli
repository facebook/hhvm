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
(** Computes the union of a list of types by union types two by two.
This is quadratic, so if this requires more than 20 two by two unions,
fall back to simply flatten the unions, bubble up the option and remove
duplicates. *)
val union_list: Env.env -> Reason.t -> locl ty list -> Env.env * locl ty
(** Simplify unions in a type. *)
val simplify_unions:
  Env.env ->
  ?on_tyvar:(Env.env -> Reason.t -> Ident.t -> Env.env * locl ty) ->
  locl ty ->
  Env.env * locl ty
(** A cheap type difference. If ty1 is a union, remove ty2 from this union.
Assumes ty1 is a flattened union or an option of a flattened union. *)
val diff: locl ty -> locl ty -> locl ty
