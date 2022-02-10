(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Measure the time it takes to evaluate the last argument and write the
    result to console along with the given identifier. *)
val measure_elapsed_time_and_report :
  TypecheckerOptions.t ->
  Typing_env_types.env option ->
  'a * string ->
  (unit -> 'b) ->
  'b
