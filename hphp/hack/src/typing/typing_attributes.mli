(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val check_def :
  Typing_env_types.env ->
  (expected:'a option ->
  check_parent:bool ->
  check_not_abstract:bool ->
  is_using_clause:bool ->
  Pos.t ->
  Typing_env_types.env ->
  Nast.class_id_ ->
  'b list ->
  Nast.expr list ->
  'c option ->
  Typing_env_types.env * 'l * 'm * 'n * 'o * 'p * 'q) ->
  string ->
  Nast.user_attribute list ->
  Typing_env_types.env
