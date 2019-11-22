(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Typing_defs
open Typing_env_types
module Env = Typing_env

(** Performs the union of two types.
The union is the least upper bound of the subtyping relation.

There is however one approximation: if type A is covariant,
then
  A<T1> | A<T2> = A<T1|T2>

This approximation is necessary to avoid type growing exponentially in size.
We have seen cases where it would otherwise generate unions involving all
the subsets of a set of types. *)
val union : env -> locl_ty -> locl_ty -> env * locl_ty

(** Computes the union of a list of types by union types two by two.
This is quadratic, so if this requires more than 20 two by two unions,
fall back to simply flatten the unions, bubble up the option and remove
duplicates. *)
val union_list : env -> Reason.t -> locl_ty list -> env * locl_ty

val simplify_unions :
  env ->
  ?on_tyvar:(env -> Reason.t -> Ident.t -> env * locl_ty) ->
  locl_ty ->
  env * locl_ty

val union_i :
  env -> Typing_reason.t -> internal_type -> locl_ty -> env * internal_type
