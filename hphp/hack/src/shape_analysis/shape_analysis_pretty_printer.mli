(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Shape_analysis_types

type 'constraint_ show_constraint_ =
  Typing_env_types.env -> 'constraint_ -> string

val show_constraint : constraint_ show_constraint_

val show_inter_constraint : inter_constraint_ show_constraint_

val show_decorated_constraint_general :
  verbosity:int ->
  Typing_env_types.env ->
  show_constr:'constraint_ show_constraint_ ->
  'constraint_ decorated ->
  string

val show_decorated_constraint :
  verbosity:int -> Typing_env_types.env -> constraint_ decorated -> string

val show_decorated_inter_constraint :
  verbosity:int -> Typing_env_types.env -> inter_constraint_ decorated -> string

val show_shape_result : Typing_env_types.env -> shape_result -> string
