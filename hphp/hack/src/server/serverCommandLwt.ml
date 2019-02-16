open Core_kernel
open ServerCommandTypes

exception Remote_fatal_exception of Marshal_tools.remote_exception_data
exception Remote_nonfatal_exception of Marshal_tools.remote_exception_data

let rec wait_for_rpc_response fd state callback =
  let error state e =
    let stack = Caml.Printexc.get_callstack 100
      |> Caml.Printexc.raw_backtrace_to_string in
    Lwt.return (Error (state, Utils.Callstack stack, e))
  in
  try%lwt
    let%lwt message = Marshal_tools_lwt.from_fd_with_preamble fd in
    begin match message with
    | Response (r, t) ->
      Lwt.return (Ok (state, r, t))
    | Push (ServerCommandTypes.FATAL_EXCEPTION remote_e_data) ->
      error state (Remote_fatal_exception remote_e_data)
    | Push (ServerCommandTypes.NONFATAL_EXCEPTION remote_e_data) ->
      error state (Remote_nonfatal_exception remote_e_data)
    | Push m ->
      let state = callback state m in
      let%lwt response = wait_for_rpc_response fd state callback in
      Lwt.return response
    | Hello ->
      error state (Failure "unexpected hello after connection already established")
    | Ping ->
      error state (Failure "unexpected ping on persistent connection")
  end with e ->
    let stack = Printexc.get_backtrace () in
    Lwt.return (Error (state, Utils.Callstack stack, e))

(** Sends a message over the given `out_channel`, then listens for incoming
messages - either an exception which it raises, or a push which it dispatches
via the supplied callback, or a response which it returns.

Note: although this function returns a promise, it is not safe to call this
function multiple times in parallel, since they are writing to the same output
channel, and the server is not equipped to serve parallel requests anyways.
*)
let rpc_persistent :
  type a s.
  Timeout.in_channel * Out_channel.t -> s -> (s -> push -> s) -> a t
  -> (s * a * float, s * Utils.callstack * exn) result Lwt.t
  = fun (_, oc) state callback cmd ->
  try%lwt
    let fd = Unix.descr_of_out_channel oc in
    let oc = Lwt_io.of_unix_fd fd ~mode:Lwt_io.Output in
    let buffer = Marshal.to_string (Rpc cmd) [] in
    let%lwt () = Lwt_io.write oc buffer in
    let%lwt () = Lwt_io.flush oc in
    let%lwt response = wait_for_rpc_response
      (Lwt_unix.of_unix_file_descr fd)
      state
      callback
    in
    Lwt.return response
  with e ->
    let stack = Printexc.get_backtrace () in
    Lwt.return (Error (state, Utils.Callstack stack, e))

let connect_debug oc =
  Marshal.to_channel oc Debug [];
  Out_channel.flush oc

let send_connection_type oc t =
  Marshal.to_channel oc t [];
  Out_channel.flush oc
