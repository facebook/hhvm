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
  ServerEnv.genv ->
  ServerEnv.env ->
  get_next:Relative_path.t list Bucket.next ->
  ?count:int ->
  float ->
  trace:bool ->
  cache_decls:bool ->
  telemetry_label:string ->
  cgroup_steps:CgroupProfiler.step_group ->
  worker_call:MultiWorker.call_wrapper ->
  ServerEnv.env * float

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
  ServerEnv.Init_telemetry.t ->
  float ->
  telemetry_label:string ->
  cgroup_steps:CgroupProfiler.step_group ->
  ServerEnv.env * float
