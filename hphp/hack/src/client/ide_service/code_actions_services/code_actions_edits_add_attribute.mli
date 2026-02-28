(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *)

(** Generates text edits for adding attributes to functions and methods.
    Used by quickfixes that need to insert attributes like __NeedsConcrete. *)

val create :
  Provider_context.t ->
  Provider_context.entry ->
  function_pos:Pos.t ->
  attribute_name:string ->
  Code_action_types.edit list
