(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Refactor_sd_types
module T = Tast

exception Refactor_sd_exn = Refactor_sd_exn

let do_
    (_function_name : string)
    (options : options)
    (ctx : Provider_context.t)
    (_tast : T.program) =
  let _empty_typing_env =
    Tast_env.tast_env_as_typing_env (Tast_env.empty ctx)
  in
  match options.mode with
  | _ -> ()

let contains_upcast = function
  | Exists_Upcast -> true
  | _ -> false
