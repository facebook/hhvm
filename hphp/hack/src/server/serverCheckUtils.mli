(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val get_check_info :
  check_reason:string ->
  log_errors:bool ->
  ServerEnv.genv ->
  ServerEnv.env ->
  Typing_service_types.check_info

val user_filter_type_check_files :
  to_recheck:Relative_path.Set.t ->
  reparsed:Relative_path.Set.t ->
  Relative_path.Set.t

val get_naming_table_fallback_path : ServerEnv.genv -> string option

(** [extend_defs_per_file genv defs_per_file naming_table additional_files]
  extends [defs_per_file] file-to-defs table with definitions in [additional_files]
  by querying [naming_table] for their definitions. *)
val extend_defs_per_file :
  ServerEnv.genv ->
  FileInfo.names Relative_path.Map.t ->
  Naming_table.t ->
  Relative_path.Set.t ->
  FileInfo.names Relative_path.Map.t
