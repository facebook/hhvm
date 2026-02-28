(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Typecheck only definitions whose identifier matches the regular expression. *)
val if_matches_regexp :
  default:'b ->
  TypecheckerOptions.t ->
  Typing_env_types.env option ->
  'a * string ->
  (unit -> 'b) ->
  'b
