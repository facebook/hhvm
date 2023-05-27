(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Walks the directory tree to find all .php files that match [Find_utils.file_filter].
It actually returns a [Bucket.next], i.e. a lazy list, rather than doing it eagerly. *)
val directory_walk :
  ?hhi_filter:(string -> bool) ->
  telemetry_label:string ->
  ServerEnv.genv ->
  Relative_path.t list Bucket.next * float

(** This parses all the lazy list of files provided by [get_next] to get [FileInfo.t]
information for all of them, then updates the forward naming table [env.naming_table]. *)
val parse_files_and_update_forward_naming_table :
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

(** This walks [env.naming_table], the forward-naming-table, and uses it to
update the (global mutable) reverse naming table. It also adds
duplicate-name-errors into [env.errors], and also puts them also
into [env.failed_naming] since that's how we currently accomplish
incremental updates that fix duplicate names. *)
val update_reverse_naming_table_from_env_and_get_duplicate_name_errors :
  ServerEnv.env ->
  float ->
  telemetry_label:string ->
  cgroup_steps:CgroupProfiler.step_group ->
  ServerEnv.env * float

(** Just a quick validation that there are no errors *)
val validate_no_errors : Errors.t -> unit

(** This function has two different behaviors:
- For the normal case, it adds the provided list of files into [env.needs_recheck].
  It also sets [env.full_check_status=Full_check_started] which is important for
  correctness of streaming errors (it guarantees that ServerTypeCheck will do
  an iteration, even if [env.needs_recheck] is empty).
  It also sets [env.init_env] which is important for telemetry, so the first
  ServerTypeCheck loop will record information about the Init_telemetry (i.e.
  saved-state-loading) which determined the typecheck.
- For Zoncolan, and "hh_server check" (i.e. full init that doesn't support incremental updates),
  and "hh_server --save-state" (i.e. full-init which saves a state and then quits),
  it typechecks all the files provided in the list. *)
val defer_or_do_type_check :
  ServerEnv.genv ->
  ServerEnv.env ->
  Relative_path.t list ->
  ServerEnv.Init_telemetry.t ->
  float ->
  telemetry_label:string ->
  cgroup_steps:CgroupProfiler.step_group ->
  ServerEnv.env * float
