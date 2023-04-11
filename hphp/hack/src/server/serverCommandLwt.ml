open Hh_prelude
open ServerCommandTypes

exception Remote_fatal_exception of Marshal_tools.remote_exception_data

exception Remote_nonfatal_exception of Marshal_tools.remote_exception_data

let rec wait_for_rpc_response stack fd state callback =
  try%lwt
    let%lwt message = Marshal_tools_lwt.from_fd_with_preamble fd in
    match message with
    | Response (r, tracker) -> Lwt.return (Ok (state, r, tracker))
    | Push (ServerCommandTypes.FATAL_EXCEPTION remote_e_data) ->
      Lwt.return (Error (state, stack, Remote_fatal_exception remote_e_data))
    | Push (ServerCommandTypes.NONFATAL_EXCEPTION remote_e_data) ->
      Lwt.return (Error (state, stack, Remote_nonfatal_exception remote_e_data))
    | Push m ->
      let state = callback state m in
      let%lwt response = wait_for_rpc_response stack fd state callback in
      Lwt.return response
    | Hello ->
      Lwt.return (Error (state, stack, Failure "unexpected second hello"))
    | Ping ->
      Lwt.return
        (Error (state, stack, Failure "unexpected ping on persistent connection"))
    | Monitor_failed_to_handoff ->
      Lwt.return
        (Error
           ( state,
             stack,
             Failure
               "unexpected monitor_failed_to_handoff on persistent connection"
           ))
  with
  | e -> Lwt.return (Error (state, stack, e))

(** Sends a message over the given `out_channel`, then listens for incoming
    messages - either an exception which it raises, or a push which it dispatches
    via the supplied callback, or a response which it returns.

    Note: although this function returns a promise, it is not safe to call this
    function multiple times in parallel, since they are writing to the same output
    channel, and the server is not equipped to serve parallel requests anyways. *)
let rpc_persistent :
    type a s.
    Timeout.in_channel * Out_channel.t ->
    s ->
    (s -> push -> s) ->
    desc:string ->
    a t ->
    (s * a * Connection_tracker.t, s * Utils.callstack * exn) result Lwt.t =
 fun (_, oc) state callback ~desc cmd ->
  let stack =
    Caml.Printexc.get_callstack 100 |> Caml.Printexc.raw_backtrace_to_string
  in
  let stack = Utils.Callstack stack in
  try%lwt
    let fd = Unix.descr_of_out_channel oc in
    let oc = Lwt_io.of_unix_fd fd ~mode:Lwt_io.Output in
    let metadata = { ServerCommandTypes.from = "hh_client"; desc } in
    let buffer = Marshal.to_string (Rpc (metadata, cmd)) [] in
    let%lwt () = Lwt_io.write oc buffer in
    let%lwt () = Lwt_io.flush oc in
    let%lwt response =
      wait_for_rpc_response
        stack
        (Lwt_unix.of_unix_file_descr fd)
        state
        callback
    in
    Lwt.return response
  with
  | e -> Lwt.return (Error (state, stack, e))

let send_connection_type oc t =
  Marshal.to_channel oc t [];
  Out_channel.flush oc
