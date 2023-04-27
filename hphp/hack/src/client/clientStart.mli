(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type env = {
  root: Path.t;
  from: string;
  no_load: bool;
  watchman_debug_logging: bool;
  log_inference_constraints: bool;
  silent: bool;
  exit_on_failure: bool;
  ignore_hh_version: bool;
  save_64bit: string option;
  save_human_readable_64bit_dep_map: string option;
  saved_state_ignore_hhconfig: bool;
  prechecked: bool option;
  mini_state: string option;
  config: (string * string) list;
  custom_hhi_path: string option;
  custom_telemetry_data: (string * string) list;
  allow_non_opt_build: bool;
}

val main : env -> Exit_status.t Lwt.t

val start_server : env -> unit
