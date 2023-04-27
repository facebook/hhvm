(****************************************************************************)
(* Called by the client *)
(****************************************************************************)

exception Remote_fatal_exception of Marshal_tools.remote_exception_data

exception Remote_nonfatal_exception of Marshal_tools.remote_exception_data

val rpc_persistent :
  Timeout.in_channel * Core.Out_channel.t ->
  's ->
  ('s -> ServerCommandTypes.push -> 's) ->
  desc:string ->
  'a ServerCommandTypes.t ->
  ('s * 'a * Connection_tracker.t, 's * Utils.callstack * exn) result Lwt.t

val send_connection_type : out_channel -> 'a -> unit
