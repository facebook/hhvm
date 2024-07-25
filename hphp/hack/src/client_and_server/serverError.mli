(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val get_save_state_result_props_json :
  SaveStateServiceTypes.save_state_result -> (string * Hh_json.json) list

val print_error_list :
  Out_channel.t ->
  stale_msg:string option ->
  output_json:bool ->
  error_format:Errors.format option ->
  error_list:Errors.finalized_error list ->
  save_state_result:SaveStateServiceTypes.save_state_result option ->
  recheck_stats:Telemetry.t option ->
  unit
