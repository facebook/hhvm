(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Provides IDE services in the client, without an instance of hh_server
running.

Basic approach: we load the naming table to give us just enough symbol
information to provide IDE services for just the files you're looking at. When
we need to look up declarations to service an IDE query, we parse and typecheck
the files containing those declarations on-demand, then answer your IDE query.
*)
type t

module Status : sig
  type t =
    | Initializing
        (** The IDE services are still initializing (e.g. loading saved-state or
        building indexes.) *)
    | Rpc of Telemetry.t list
        (** The IDE services will be available once they're done handling
        one or more existing requests. The telemetry items are information
        about each received request currently being performed. *)
    | Ready  (** The IDE services are available. *)
    | Stopped of ClientIdeMessage.rich_error
        (** The IDE services are not available. *)
end

module Stop_reason : sig
  type t =
    | Crashed  (** clientIdeService encountered an unexpected bug *)
    | Closed
        (** clientIdeService shut down normally, although we've failed to record why *)
    | Editor_exited  (** clientLsp decided to close in response to shutdown*)
    | Restarting
        (** clientLsp decided to close this clientIdeService and start a new one *)
    | Testing
        (** test harnesses can tell clientLsp to shut down clientIdeService *)

  val to_log_string : t -> string
end

(** Create an uninitialized IDE service. All queries made to this service will
fail immediately, unless otherwise requested in the initialization procedure. *)
val make : ClientIdeMessage.daemon_args -> t

(** Request that the IDE service initialize from the saved state. Queries made
to the service will fail until it is done initializing, unless
[wait_for_initialization] is [true], in which case queries made to the service
will block until the initializing is complete. *)
val initialize_from_saved_state :
  t ->
  root:Path.t ->
  naming_table_load_info:
    ClientIdeMessage.Initialize_from_saved_state.naming_table_load_info option ->
  config:(string * string) list ->
  ignore_hh_version:bool ->
  open_files:Path.t list ->
  (unit, ClientIdeMessage.rich_error) Lwt_result.t

(** Pump the message loop for the IDE service. Exits once the IDE service has
been [destroy]ed. *)
val serve : t -> unit Lwt.t

(** Clean up any resources held by the IDE service (such as the message loop
and background processes). Mark the service's status as "shut down" for the
given [reason]. *)
val stop :
  t ->
  tracking_id:string ->
  stop_reason:Stop_reason.t ->
  e:Exception.t option ->
  unit Lwt.t

(** This function does an rpc to the daemon: it pushes a message
onto the daemon's stdin queue, then awaits until [serve] has stuck
the stdout response from the daemon response onto our [response_emitter].
The daemon updates [ref_unblocked_time], the time at which the
daemon starting handling the rpc.

Our only caller of this function is [ClientLsp.ide_rpc], which aims
to uphold the invariant that it never makes two concurrent calls
to this function. We support that invariant here by logging an
invariant_violation_bug should it occur, but that's only to help
our caller; this function supports being called while an existing
rpc is outstanding, and guarantees that its results will be
delivered in order.

The progress callback will be invoked during this call to rpc, at times
when the result of [get_status t] might have changed. It's designed
so the caller of rpc can, in their callback, invoke get_status and display
some kind of progress message.

Note: it is not safe to Lwt.cancel this method, since we might end up
with no one reading the response to the message we just pushed, leading
to desync.

Note: If we're in Stopped state (due to someone calling [stop]) then
we'll refrain from sending the rpc. Stopped is the only state we enter
due to our own volition; all other states are just a reflection
of what state the daemon is in, and so it's fine for the daemon
to respond as it see fits while in the other states. *)
val rpc :
  t ->
  tracking_id:string ->
  ref_unblocked_time:float ref ->
  progress:(unit -> unit) ->
  'response ClientIdeMessage.t ->
  ('response, Lsp.Error.t) Lwt_result.t

(** Get a handle to the stream of notifications sent by the IDE service. These
notifications may be sent even during RPC requests, and so should be processed
asynchronously. *)
val get_notifications : t -> ClientIdeMessage.notification Lwt_message_queue.t

(** Get the status of the IDE services, based on the internal state and any
notifications that the IDE service process has sent to us. *)
val get_status : t -> Status.t
