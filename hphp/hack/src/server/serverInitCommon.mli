(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val indexing :
  ?hhi_filter:(string -> bool) ->
  telemetry_label:string ->
  ServerEnv.genv ->
  Relative_path.t list Bucket.next * float

val parsing :
  lazy_parse:bool ->
  ServerEnv.genv ->
  ServerEnv.env ->
  get_next:Relative_path.t list Bucket.next ->
  ?count:int ->
  float ->
  trace:bool ->
  cache_decls:bool ->
  telemetry_label:string ->
  cgroup_steps:CgroupProfiler.step_group ->
  ServerEnv.env * float

val update_files :
  ?warn_on_naming_costly_iter:bool ->
  ServerEnv.genv ->
  Naming_table.t ->
  Provider_context.t ->
  float ->
  telemetry_label:string ->
  cgroup_steps:CgroupProfiler.step_group ->
  float

val naming :
  ServerEnv.env ->
  float ->
  telemetry_label:string ->
  cgroup_steps:CgroupProfiler.step_group ->
  ServerEnv.env * float

val type_check :
  ServerEnv.genv ->
  ServerEnv.env ->
  Relative_path.t list ->
  Telemetry.t ->
  float ->
  telemetry_label:string ->
  cgroup_steps:CgroupProfiler.step_group ->
  ServerEnv.env * float
