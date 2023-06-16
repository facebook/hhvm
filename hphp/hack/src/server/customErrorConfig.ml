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
  | Ok (cfg, bad_rules) ->
    List.iter (log_debug "CustomErrorConfig bad rule: %s") bad_rules;
    cfg
  | Error msg ->
    log_debug "CustomErrorConfig: %s" msg;
    Custom_error_config.empty
