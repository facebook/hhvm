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
the subsets of a set of types.

If approx_cancel_neg is true, then some unions with negations are treated over-approximately:
C<t> | not C simplifies to mixed. *)
val union :
  env -> ?approx_cancel_neg:bool -> locl_ty -> locl_ty -> env * locl_ty

(** Computes the union of a list of types by unioning types two by two.
This has quadratic complexity, but in practice we benefit from aggressively
simplified unions. *)
val union_list :
  env -> ?approx_cancel_neg:bool -> Reason.t -> locl_ty list -> env * locl_ty

(** Simplify the unions within a type.
Return the result in a flattened, "normalized" form, with options and like types being
"pushed out" of the result.
E.g. transforms `(int | (int | null | X))` into `?(int | X)`

This has quadratic complexity, but in practice we benefit from aggressively
simplified unions. *)
val simplify_unions :
  env ->
  ?approx_cancel_neg:bool ->
  ?on_tyvar:(env -> Reason.t -> Tvid.t -> env * locl_ty) ->
  locl_ty ->
  env * locl_ty

val union_i :
  env ->
  ?approx_cancel_neg:bool ->
  Typing_reason.t ->
  internal_type ->
  locl_ty ->
  env * internal_type
