(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type finale_data = {
  exit_status: Exit_status.t;
      (** exit_status is shown to the user in CLI and HackIDE,
      just so they have an error code they can quote back at
      developers. It's also used by hh_client to decide, on the
      basis of that hh_server exit_status, whether to auto-restart
      hh_server or not. And it appears in logs and telemetry. *)
  msg: string option;
      (** msg is a human-readable message for the end-user to explain why
      hh_server stopped. It appears in the CLI, and in HackIDE in a
      hover tooltip. It also is copied into the logs. *)
  stack: Utils.callstack;
}

(** human-readable debug output for hack developers, not for general public *)
val show_finale_data : finale_data -> string

val exit : ?msg:string -> ?stack:string -> Exit_status.t -> 'a

val add_hook_upon_clean_exit : (finale_data -> unit) -> unit
