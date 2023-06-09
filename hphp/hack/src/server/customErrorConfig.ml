(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Sys_utils

let config_path_relative_to_repo_root = "CUSTOM_ERRORS.json"

let repo_config_path =
  Relative_path.from_root ~suffix:config_path_relative_to_repo_root

let log_debug (msg : ('a, unit, string, string, string, unit) format6) : 'a =
  Hh_logger.debug ~category:"custom errors" ?exn:None msg

let load_and_parse ?(path = repo_config_path) () =
  let config_abs_path = Relative_path.to_absolute path in
  Option.value ~default:Custom_error_config.empty
  @@
  if not @@ Sys.file_exists config_abs_path then
    None
  else
    let ch = open_in_no_fail config_abs_path in
    let cfg_opt =
      match Custom_error_config.initialize ch with
      | Ok (tco_custom_error_config, bad_rules) ->
        if Core.List.is_empty bad_rules then
          log_debug "Parsed %s" config_abs_path
        else
          log_debug "Parsed %s; found bad rules: %s" config_abs_path
          @@ Core.String.concat ~sep:"\n" bad_rules;
        Some tco_custom_error_config
      | Error json_error ->
        log_debug "Error whilst parsing %s: %s" config_abs_path json_error;
        None
    in
    close_in_no_fail "CustomErrorConfig load_and_parse" ch;
    cfg_opt
