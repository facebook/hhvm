(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t
(** Provides IDE services in the client, without an instance of hh_server
running.

Basic approach: we load the naming table to give us just enough symbol
information to provide IDE services for just the files you're looking at. When
we need to look up declarations to service an IDE query, we parse and typecheck
the files containing those declarations on-demand, then answer your IDE query.
*)

val make : unit -> t
(** Create an uninitialized IDE service. All queries made to this service will
fail immediately, unless otherwise requested in the initialization procedure. *)

val initialize_from_saved_state :
  t ->
  root:Path.t ->
  naming_table_saved_state_path:Path.t option ->
  wait_for_initialization:bool ->
  (unit, string) Lwt_result.t
(** Request that the IDE service initialize from the saved state. Queries made
to the service will fail until it is done initializing, unless
[wait_for_initialization] is [true], in which case queries made to the service
will block until the initializing is complete. *)

val serve : t -> unit Lwt.t
(** Pump the message loop for the IDE service. Exits once the IDE service has
been [destroy]ed. *)

val destroy : t -> unit Lwt.t
(** Clean up any resources held by the IDE service (such as the message loop and
background processes). *)

val notify_file_changed : t -> Path.t -> unit
(** The caller is expected to call this function to notify the IDE service
whenever a Hack file changes on disk, so that it can update its indexes
appropriately. *)

val rpc : t -> 'response ClientIdeMessage.t -> ('response, string) Lwt_result.t
(** Make an RPC call to the IDE service. *)

val get_notifications : t -> ClientIdeMessage.notification Lwt_message_queue.t
(** Get a handle to the stream of notifications sent by the IDE service. These
notifications may be sent even during RPC requests, and so should be processed
asynchronously. *)
