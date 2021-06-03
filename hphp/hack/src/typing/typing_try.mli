(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module CMap = Typing_continuations.Map

val finally_merge :
  (Typing_env_types.env ->
  CMap.key ->
  Typing_per_cont_env.per_cont_entry option ->
  Typing_per_cont_env.per_cont_entry option ->
  Typing_env_types.env * Typing_per_cont_env.per_cont_entry option) ->
  Typing_env_types.env ->
  Typing_per_cont_env.t CMap.t ->
  CMap.key list ->
  Typing_env_types.env * Typing_per_cont_env.t
