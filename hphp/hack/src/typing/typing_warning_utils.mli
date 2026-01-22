(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
val codes : ('x, 'a) Typing_warning.kind -> Error_codes.Warning.t list

val code_is_enabled : GlobalOptions.t -> Error_codes.Warning.t -> bool

val add_ : TypecheckerOptions.t -> ('x, 'a) Typing_warning.t -> unit

val add : Typing_env_types.env -> ('x, 'a) Typing_warning.t -> unit

val add_for_migration :
  GlobalOptions.t ->
  as_lint:Tast.check_status option option ->
  ('x, Typing_warning.migrated) Typing_warning.t ->
  unit
