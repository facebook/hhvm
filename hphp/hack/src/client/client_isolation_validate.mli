(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Print the verdict, the references that contradict it, and the candidate
    paths nothing was checked for. *)
val output :
  Server_command_types.Isolation_validation.result -> output_json:bool -> unit

(** Exit status for the verdict, so a script need not parse stdout. *)
val status : Server_command_types.Isolation_validation.result -> Exit_status.t
