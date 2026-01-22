(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val print_error_list :
  Out_channel.t ->
  stale_msg:string option ->
  output_json:bool ->
  error_format:Diagnostics.format option ->
  error_list:Diagnostics.finalized_diagnostic list ->
  recheck_stats:Telemetry.t option ->
  unit
