(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Env = Typing_env
open Typing_defs
open Typing_env_types

(** Given a list of type parameter names, attempt to simplify away those
type parameters by looking for a type to which they are equal in the tpenv.
If such a type exists, remove the type parameter from the tpenv.
Returns a set of substitutions mapping each type parameter name to the type
to which it is equal if found, otherwise to itself. *)
val simplify_tpenv :
  env ->
  (('a tparam * string) option * locl_ty) list ->
  Typing_reason.t ->
  env * locl_ty SMap.t

(** Merge two type parameter environments. Given tpenv1 and tpenv2 we want
    to compute a "merged" environment tpenv such that
        tpenv1 |- tpenv
    and tpenv2 |- tpenv

    If a type parameter is defined only on one input, we do not include it in tpenv.
    If it appears in both, supposing we have
        l1 <: T <: u1 in tpenv1
    and l2 <: T <: u2 in tpenv2
    with multiple lower bounds reduced to a union, and multiple upper bounds
    reduced to an intersection, then the resulting tpenv will have
        l1&l2 <: T <: u1|u2
    *)
val join :
  env ->
  Type_parameter_env.t ->
  Type_parameter_env.t ->
  env * Type_parameter_env.t
