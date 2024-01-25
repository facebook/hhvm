(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val server_exists : string -> bool

val connect_once :
  tracker:Connection_tracker.t ->
  timeout:int ->
  terminate_monitor_on_version_mismatch:bool ->
  Path.t ->
  MonitorRpc.handoff_options ->
  ( Timeout.in_channel * out_channel * ServerCommandTypes.server_specific_files,
    MonitorUtils.connection_error )
  result

val connect_and_shut_down :
  tracker:Connection_tracker.t ->
  Path.t ->
  (MonitorUtils.shutdown_result, MonitorUtils.connection_error) result
