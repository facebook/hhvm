(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let config_path_relative_to_repo_root = "CUSTOM_ERRORS.json"

let repo_config_path =
  Relative_path.from_root ~suffix:config_path_relative_to_repo_root

let log_debug (msg : ('a, unit, string, string, string, unit) format6) : 'a =
  Hh_logger.debug ~category:"custom errors" ?exn:None msg

let load_and_parse ?(path = repo_config_path) () =
  match Custom_error_config.initialize (`Relative path) with
  | Ok cfg ->
    List.iter
      (fun rule ->
        log_debug "CustomErrorConfig bad rule: %s" rule.Custom_error.name)
      cfg.Custom_error_config.invalid;
    cfg
  | Error msg ->
    log_debug "CustomErrorConfig: %s" msg;
    Hh_logger.log
      ~lvl:Hh_logger.Level.Fatal
      "Custom errors config malformed: %s"
      msg;
    Exit.exit Exit_status.Config_error
