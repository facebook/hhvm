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

(** `step_group "foo" ~log:true callback` indicates that group "foo"
will span all the steps created within callback. If [log] is true
then each step will be logged to Hh_logger and HackEventLogger. *)
val step_group : string -> log:bool -> (step_group -> 'a) -> 'a

(** This records cgroup stats at a point in time *)
val step : step_group -> ?telemetry_ref:Telemetry.t option ref -> string -> unit

(** `step_start_end phase "bar" callback` records cgroup stats
immediately, then executes the callback, then records cgroup a second time. *)
val step_start_end :
  step_group ->
  ?telemetry_ref:Telemetry.t option ref ->
  string ->
  (unit -> 'a) ->
  'a

(** This captures the cgroup name, and what initial values it had upon process startup.
It's so that subsequent measurements of cgroup memory can be recorded relative to
this value. *)
type initial_reading

(** Upon module load, this module takes an [initial_reading] and stores it in mutable state.
This function lets us retrieve it. Except: if someone has subsequently called [use_initial_reading],
then the captured value will be irretrievable, and we'll return the [use_initial_reading] instead. *)
val get_initial_reading : unit -> initial_reading

(** This module doesn't actually respect the [initial_reading] it took upon program startup;
it only uses a reading that was passed to it by this function. Why? imagine if after
some heavy allocation, serverMain used Daemon.spawn to spawn a child process;
we'd usually prefer that child process to record numbers relative to the intial_reading
that was captured by the initial process upon its initial startup, not the one captured
by the child upon child process startup. *)
val use_initial_reading : initial_reading -> unit
