(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Env = Typing_env

let if_matches_regexp ~default tcopt env_opt id f =
  match TypecheckerOptions.typecheck_if_name_matches_regexp tcopt with
  | None -> f ()
  | Some regexp ->
    let id =
      match Option.(env_opt >>= Env.get_self_id) with
      | Some class_name -> class_name ^ "::" ^ snd id
      | None -> snd id
    in
    if Str.string_match (Str.regexp regexp) id 0 then
      f ()
    else
      default
