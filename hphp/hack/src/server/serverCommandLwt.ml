open ServerCommandTypes

let rec wait_for_rpc_response_lwt fd state callback =
  let error state e =
    let stack = Printexc.get_callstack 100 |> Printexc.raw_backtrace_to_string in
    Lwt.return (Error (state, Utils.Callstack stack, e))
  in
  try%lwt
    let%lwt message = Marshal_tools_lwt.from_fd_with_preamble fd in
    begin match message with
    | Response (r, t) ->
      Lwt.return (Ok (state, r, t))
    | Push (ServerCommandTypes.FATAL_EXCEPTION remote_e_data) ->
      error state (ServerCommand.Remote_fatal_exception remote_e_data)
    | Push (ServerCommandTypes.NONFATAL_EXCEPTION remote_e_data) ->
      error state (ServerCommand.Remote_nonfatal_exception remote_e_data)
    | Push m ->
      let state = callback state m in
      let%lwt response = wait_for_rpc_response_lwt fd state callback in
      Lwt.return response
    | Hello ->
      error state (Failure "unexpected hello after connection already established")
    | Ping ->
      error state (Failure "unexpected ping on persistent connection")
  end with e ->
    let stack = Printexc.get_backtrace () in
    Lwt.return (Error (state, Utils.Callstack stack, e))

(** Same as [ServerCommand.rpc_persistent], but returns an Lwt promise instead
of blocking. Note: although this function returns a promise, it is not safe to
call this function multiple times in parallel, since they are writing to the
same output channel. *)
let rpc_persistent_lwt :
  type a s.
  Timeout.in_channel * out_channel -> s -> (s -> push -> s) -> a t
  -> (s * a * float, s * Utils.callstack * exn) result Lwt.t
  = fun (_, oc) state callback cmd ->
  try%lwt
    let fd = Unix.descr_of_out_channel oc in
    let oc = Lwt_io.of_unix_fd fd ~mode:Lwt_io.Output in
    let buffer = Marshal.to_string (Rpc cmd) [] in
    let%lwt () = Lwt_io.write oc buffer in
    let%lwt () = Lwt_io.flush oc in
    let%lwt response = wait_for_rpc_response_lwt
      (Lwt_unix.of_unix_file_descr fd)
      state
      callback
    in
    Lwt.return response
  with e ->
    let stack = Printexc.get_backtrace () in
    Lwt.return (Error (state, Utils.Callstack stack, e))
