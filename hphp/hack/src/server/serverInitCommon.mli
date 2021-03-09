(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val hh_log_heap : unit -> unit

val indexing :
  ?hhi_filter:(string -> bool) ->
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
  profile_label:string ->
  profiling:CgroupProfiler.Profiling.t ->
  ServerEnv.env * float

val update_files :
  ServerEnv.genv ->
  Naming_table.t ->
  Provider_context.t ->
  float ->
  profile_label:string ->
  profiling:CgroupProfiler.Profiling.t ->
  float

val naming :
  ServerEnv.env ->
  float ->
  profile_label:string ->
  profiling:CgroupProfiler.Profiling.t ->
  ServerEnv.env * float

val type_check :
  ServerEnv.genv ->
  ServerEnv.env ->
  Relative_path.t list ->
  Telemetry.t ->
  float ->
  profile_label:string ->
  profiling:CgroupProfiler.Profiling.t ->
  ServerEnv.env * float
