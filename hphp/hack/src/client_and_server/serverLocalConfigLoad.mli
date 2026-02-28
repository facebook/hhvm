(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Load configuration flags from the following sources:
  - /etc/hh.conf
  - overridden by Knobs
  - overridden by ExperimentsConfig
  - overridden by [overrides]

  If not [silent], then prints what it's doing to stderr. *)
val load :
  silent:bool ->
  current_version:Config_file_version.version ->
  current_rolled_out_flag_idx:int ->
  deactivate_saved_state_rollout:bool ->
  from:string ->
  overrides:Config_file_common.t ->
  ServerLocalConfig.t

(** Load ServerLocalConfig from already-parsed config contents.
    This is intended for testing, bypassing file reads and overrides. *)
val load_from_config :
  silent:bool ->
  current_version:Config_file_version.version ->
  current_rolled_out_flag_idx:int ->
  deactivate_saved_state_rollout:bool ->
  Config_file_common.t ->
  ServerLocalConfig.t

val to_rollout_flags : ServerLocalConfig.t -> HackEventLogger.rollout_flags

val system_config_path : string

val default : ServerLocalConfig.t
