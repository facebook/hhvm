(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val go_streaming :
  ClientEnv.client_check_env ->
  ServerLocalConfig.t ->
  Filter_diagnostics.Filter.t ->
  partial_telemetry_ref:Telemetry.t option ref ->
  connect_then_close:(unit -> unit Lwt.t) ->
  (Exit_status.t * Telemetry.t) Lwt.t

val go :
  ServerCommandTypes.Server_status.t ->
  Diagnostics.format option ->
  is_interactive:bool ->
  output_json:bool ->
  max_errors:int option ->
  Exit_status.t
