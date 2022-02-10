(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Env = Typing_env

let measure_elapsed_time_and_report tcopt env_opt id f =
  if TypecheckerOptions.profile_toplevel_definitions tcopt then (
    let start_time = Unix.gettimeofday () in
    let result = f () in
    let end_time = Unix.gettimeofday () in
    let elapsed_time_ms = (end_time -. start_time) *. 1000. in
    let id =
      match Option.(env_opt >>= Env.get_self_id) with
      | Some class_name -> class_name ^ "::" ^ snd id
      | None -> snd id
    in
    Printf.printf "%s: %0.2fms\n" id elapsed_time_ms;
    result
  ) else
    f ()
