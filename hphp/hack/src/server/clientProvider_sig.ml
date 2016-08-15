module type S = sig
  type t
  type client

  val provider_from_file_descriptor : Unix.file_descr -> t
  val provider_for_test : unit -> t
  val client_from_channel_pair:  Timeout.in_channel * out_channel -> client

  val sleep_and_check : t -> Unix.file_descr option -> bool * bool
  val accept_client : t -> client
  val read_connection_type : client -> ServerCommandTypes.connection_type
  val send_response_to_client : client -> 'a -> unit
  val read_client_msg: client -> 'a ServerCommandTypes.command
  val is_persistent: client -> ServerEnv.env -> bool
  val shutdown_client: client -> unit

  (* TODO: temporary way to break the module abstraction. Remove after
   * converting all the callsites to use methods on this module instead of
   * directly using channels. *)
  val get_channels: client -> Timeout.in_channel * out_channel
end
