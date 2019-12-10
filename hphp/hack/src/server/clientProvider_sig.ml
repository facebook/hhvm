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

  type select_outcome =
    | Select_persistent
    | Select_new of client
    | Select_nothing

  exception Client_went_away

  val provider_from_file_descriptors :
    Unix.file_descr * Unix.file_descr * Unix.file_descr -> t

  val provider_for_test : unit -> t

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

  val has_persistent_connection_request : client -> bool

  val priority_fd : t -> Unix.file_descr option

  val get_client_fd : client -> Unix.file_descr option

  val read_connection_type : client -> ServerCommandTypes.connection_type

  val send_response_to_client : client -> 'a -> float -> unit

  val send_push_message_to_client : client -> ServerCommandTypes.push -> unit

  val client_has_message : client -> bool

  val read_client_msg : client -> 'a ServerCommandTypes.command

  val make_persistent : client -> client

  val is_persistent : client -> bool

  val priority_to_string : client -> string

  val shutdown_client : client -> unit

  val ping : client -> unit

  (* TODO: temporary way to break the module abstraction. Remove after
   * converting all the callsites to use methods on this module instead of
   * directly using channels. *)
  val get_channels : client -> Timeout.in_channel * out_channel
end
