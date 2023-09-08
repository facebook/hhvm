(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module type S = sig
  type t

  type client

  type handoff = {
    client: client;
    m2s_sequence_number: int;
        (** A unique number incremented for each client socket handoff from monitor to server.
            Useful to correlate monitor and server logs. *)
  }

  (** Outcome of the POSIX [select] system call. *)
  type select_outcome =
    | Select_persistent
    | Select_new of handoff
    | Select_nothing
    | Select_exception of Exception.t
    | Not_selecting_hg_updating

  exception Client_went_away

  val provider_from_file_descriptors :
    Unix.file_descr * Unix.file_descr * Unix.file_descr -> t

  val provider_for_test : unit -> t

  (** Wait up to 0.1 seconds and checks for new connection attempts.
      Select what client to serve next and retrieve channels to
      client from monitor process (connection hand-off). *)
  val sleep_and_check :
    t ->
    (* Optional persistent client. *)
    client option ->
    ide_idle:
      (* Whether the most recent message received from persistent client
       * was IDE_IDLE *)
      bool ->
    idle_gc_slice:int ->
    [ `Any | `Priority | `Force_dormant_start_only ] ->
    select_outcome

  val priority_fd : t -> Unix.file_descr option

  (** See explanation in Connection_tracker.track *)
  val track :
    key:Connection_tracker.key ->
    ?time:float ->
    ?log:bool ->
    ?msg:string ->
    ?long_delay_okay:bool ->
    client ->
    unit

  val read_connection_type : client -> ServerCommandTypes.connection_type

  val send_response_to_client : client -> 'a -> unit

  val read_client_msg : client -> 'a ServerCommandTypes.command

  val priority_to_string : client -> string

  (** Shutdown socket connection to client *)
  val shutdown_client : client -> unit

  val ping : client -> unit

  (* TODO: temporary way to break the module abstraction. Remove after
   * converting all the callsites to use methods on this module instead of
   * directly using channels. *)
  val get_channels : client -> Timeout.in_channel * out_channel
end
