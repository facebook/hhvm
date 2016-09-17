(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module type S = sig
  type t
  type client

  exception Client_went_away

  val provider_from_file_descriptor : Unix.file_descr -> t
  val provider_for_test : unit -> t

  val sleep_and_check : t -> client option -> bool * bool
  val accept_client : t -> client
  val read_connection_type : client -> ServerCommandTypes.connection_type
  val send_response_to_client : client -> 'a -> unit
  val send_push_message_to_client : client -> ServerCommandTypes.push -> unit
  val read_client_msg: client -> 'a ServerCommandTypes.command
  val make_persistent : client -> client
  val is_persistent : client -> bool
  val shutdown_client: client -> unit

  (* TODO: temporary way to break the module abstraction. Remove after
   * converting all the callsites to use methods on this module instead of
   * directly using channels. *)
  val get_channels: client -> Timeout.in_channel * out_channel
end
