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
  profile_log: bool;
  silent: bool;
  exit_on_failure: bool;
  ai_mode: string option;
  debug_port: Unix.file_descr option;
  ignore_hh_version: bool;
  saved_state_ignore_hhconfig: bool;
  dynamic_view: bool;
  prechecked: bool option;
  config: (string * string) list;
  allow_non_opt_build: bool;
}

val main : env -> Exit_status.t Lwt.t

val start_server : env -> unit
