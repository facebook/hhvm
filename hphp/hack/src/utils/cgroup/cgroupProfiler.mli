(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* The CgroupProfiler module is to help us keep track of how cgroup
memory usage evolves during the various steps of work done by hh_server.

It helps to group steps into groups. An example step-group is "init",
which is made up of steps "update naming table" then "update depgraph". *)

(** Represents a step-group that's underway; created by [step_group] *)
type step_group

(** Represents a step that's underway; created by [step] *)
type step

(** `step_group "foo" ~log:true callback` indicates that group "foo"
will span all the steps created within callback. If [log] is true
then each step will be logged to the server log. *)
val step_group : string -> log:bool -> (step_group -> 'a) -> 'a

(** This records cgroup stats at a point in time *)
val step : step_group -> ?telemetry_ref:Telemetry.t option ref -> string -> unit

(** `step_start_end phase "bar" callback` records cgroup stats
immediately, then executes the callback, then records cgroup a second time. *)
val step_start_end :
  step_group ->
  ?telemetry_ref:Telemetry.t option ref ->
  string ->
  (step -> 'a) ->
  'a

(** Sometimes for long-running steps it's not enough merely to log cgroup
stats at at the start and end; we might also want to record the "high water mark"
cgroup total ever reached within that step. This method is to record a
moment's cgroup total, in case it bumps up the high water mark. *)
val update_cgroup_total : int -> step -> unit
