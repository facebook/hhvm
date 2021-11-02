(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* The CgroupProfiler module is to help us keep track of how cgroup
memory usage evolves during the various stages of work done by hh_server.

What is a "stage"? An example stage is "update naming table".
For each stage we record telemetry about cgroup usage at the start
of that stage and at the end.

It helps to group stages. We use the term "event" for a sequence
of stages. An example event is "init". It is made up of stages
"update naming table" then "update depgraph". *)

(** Represents an event that's underway; created by [event] *)
type event

(** Represents a stage that's underway; created by [stage] *)
type stage

(** `event ~event:"foo" ~log:true callback` indicates that event "foo"
will span all the stages created within callback. If [log] is true
then a summary of cgroup usage at each stage will be printed
at the end of the event. *)
val event : event:string -> log:bool -> (event -> 'a) -> 'a

(** `stage event_token ~stage:"bar" callback` indicates that stage "bar"
will span the execution of callback. It will gather cgroup stats
before callback, and after, and log telemetry about them. *)
val stage : event -> stage:string -> (stage -> 'a) -> 'a

(** Sometimes for long-running stages it's not enough merely to log cgroup
stats at at the start and end; we might also want to record the "high water mark"
cgroup total ever reached within that stage. This method is to record a
moment's cgroup total, in case it bumps up the high water mark. *)
val update_cgroup_total : float -> stage -> unit
