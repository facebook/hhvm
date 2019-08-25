(****************************************************************************)
(* Called by the client *)
(****************************************************************************)

exception Remote_fatal_exception of Marshal_tools.remote_exception_data

exception Remote_nonfatal_exception of Marshal_tools.remote_exception_data

val rpc_persistent :
  Timeout.in_channel * Core_kernel.Out_channel.t ->
  's ->
  ('s -> ServerCommandTypes.push -> 's) ->
  'a ServerCommandTypes.t ->
  ('s * 'a * float, 's * Utils.callstack * exn) result Lwt.t

val connect_debug : out_channel -> unit

val send_connection_type : out_channel -> 'a -> unit
