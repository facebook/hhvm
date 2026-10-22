(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** How long a move waits for the server to see the files at their new place. *)
val default_wait_seconds : float

(** [move_into ~wait_seconds ~options ~parent ~output_json ~isolation_query
    ~status] moves the directory [options.seed_dir], repo-relative, into
    [parent] under its own name, if nothing outside references into it and the
    move introduces no errors, and otherwise leaves it where it was. It prints
    what came of it, as JSON when [output_json] is set, and gives the exit
    status: zero only when the directory moved.

    [isolation_query options] is the server's answer to the isolation query for
    [options.seed_dir]: how many Hack files it holds, and whether any file
    outside references into it. [status ()] is the server's current errors.

    The server gets up to [wait_seconds] after each move to know the files.
    Source control is not told. Raises if an undo cannot be completed. *)
val move_into :
  wait_seconds:float ->
  options:Server_isolation_types.options ->
  parent:string ->
  output_json:bool ->
  isolation_query:
    (Server_isolation_types.options -> Server_isolation_types.result Lwt.t) ->
  status:(unit -> Server_command_types.Server_status.t Lwt.t) ->
  Exit_status.t Lwt.t
