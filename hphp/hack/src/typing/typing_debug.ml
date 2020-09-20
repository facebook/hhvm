(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_env_types
open Typing_inference_env.Size
module Env = Typing_env
module Log = Typing_log
module ITySet = Internal_type_set

let local_env_size env =
  match Env.next_cont_opt env with
  | None -> 0
  | Some Typing_per_cont_env.{ local_types; _ } ->
    Local_id.Map.fold
      (fun _ (ty, _, _) size -> size + ty_size env.inference_env ty)
      local_types
      0

let env_size env = local_env_size env + inference_env_size env.inference_env

let log_env_if_too_big pos env =
  if
    (Env.get_tcopt env).GlobalOptions.tco_timeout > 0
    && List.length !(env.big_envs) < 1
    && env_size env >= 1000
  then
    env.big_envs := (pos, env) :: !(env.big_envs)
