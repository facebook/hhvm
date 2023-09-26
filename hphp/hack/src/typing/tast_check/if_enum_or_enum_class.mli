(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type kind =
  | Enum
  | EnumClass
  | EnumClassLabel

val apply :
  Tast_env.env ->
  default:'a ->
  f:(kind -> Tast_env.env -> string -> 'a) ->
  Decl_provider.type_key ->
  Typing_defs.locl_phase Typing_defs.ty list ->
  'a
