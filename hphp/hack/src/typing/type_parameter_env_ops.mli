(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Env = Typing_env
open Typing_defs

(** Given a list of type parameter names, attempt to simplify away those
type parameters by looking for a type to which they are equal in the tpenv.
If such a type exists, remove the type parameter from the tpenv.
Returns a set of substitutions mapping each type parameter name to the type
to which it is equal if found, otherwise to itself. *)
val simplify_tpenv :
  Env.env ->
  (('a tparam * string) option * locl ty) list ->
  Typing_reason.t ->
  Env.env * locl ty SMap.t

val join :
  Env.env ->
  Type_parameter_env.t ->
  Type_parameter_env.t ->
  Env.env * Type_parameter_env.t
