(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** The caller should invoke [start_heartbeat_telemetry ()] once, as close as possible
to process start. From this point until process termination, it will log once
a second to HackEventLogger what the latest report is.
(It's not called automatically because sometimes the startup path might determine
that it doesn't want heartbeats. *)
val start_heartbeat_telemetry : unit -> unit

(** [report ~to_stderr ~angery_reaccs_only message_opt] is used to report progress:
* Heartbeat: all subsequent heartbeats will report this [message_opt] to telemetry,
  at least until the next time [report] is invoked
* Hh_logger: if [message_opt] is [Some], and differs from what was previously reported,
  then it will be written to Hh_logger i.e. to `$(hh --client-logname)`.
* Stderr: if [to_stderr] is true and [message_opt] is Some then it will be displayed
  on stderr with an animated spinner (which continues to animate automatically, even if
  you don't call [report] again).
* Use [message_opt=None] to erase the spinner from stderr. This won't be written to the log.
  It will mean that future heartbeats report [None]. *)
val report : to_stderr:bool -> angery_reaccs_only:bool -> string option -> unit
