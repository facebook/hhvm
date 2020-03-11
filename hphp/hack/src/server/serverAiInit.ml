(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open ServerEnv

let ai_check
    (genv : ServerEnv.genv)
    (files_info : Naming_table.t)
    (env : ServerEnv.env)
    (t : float) : ServerEnv.env * float =
  match ServerArgs.ai_mode genv.options with
  | Some ai_opt ->
    let failures =
      List.map
        ~f:(fun k -> (k, Errors.get_failed_files env.errorl k))
        [Errors.Parsing; Errors.Decl; Errors.Naming; Errors.Typing]
    in
    let all_passed =
      List.for_all failures ~f:(fun (k, m) ->
          if Relative_path.Set.is_empty m then
            true
          else (
            Hh_logger.log
              "Cannot run AI because of errors in source in phase %s"
              (Errors.phase_to_string k);
            false
          ))
    in
    if not all_passed then
      (env, t)
    else
      let check_mode = ServerArgs.check_mode genv.options in
      let errorl =
        Ai.go
          genv.workers
          files_info
          (Provider_utils.ctx_from_server_env env)
          ai_opt
          check_mode
      in
      let env = { env with errorl (* Just Zonk errors. *) } in
      (env, Hh_logger.log_duration "Ai" t)
  | None -> (env, t)
