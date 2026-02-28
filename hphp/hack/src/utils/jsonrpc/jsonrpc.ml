(* Wrapper for handling JSON-RPC *)
(* Spec: http://www.jsonrpc.org/specification *)
(* Practical readbable guide: https://github.com/Microsoft/language-server-protocol/blob/master/protocol.md#base-protocol-json-structures *)

open Hh_prelude

(**************************************************************
  HOW THIS ALL WORKS.
  GOAL 1: is to always be ready to read on stdin so we can timestamp
  accurately the moment that a request was delivered to us. If our
  stdin pipe gets so full that a client can't write requests to us,
  or if we're busy doing something else before we read on stdin,
  both things would result in incorrect timestamps.
  GOAL 2: when we process a message, then all subsequent messages that
  have already been presented to our stdin should already be in our own
  queue data-structure (rather than an OS pipe buffer) in case any
  of the messages involved cancellation.
  GOAL 3: we're in ocaml, so our threading possibilties are limited,
  and unfortunately our caller has cpu-blocking chunks of code.

  We kick off single background process called "daemon", running a loop
  in [internal_run_daemon]. It will take in messages from stdin,
  queue them up in its queue `let messages_to_send = Queue.create ()`,
  and write them over a pipe to the main (calling) process.
  See type [daemon_next_action] how it choses what to do.

  Callers of this library will invoke an Lwt API [get_message].
  This maintains its own queue [queue.messages] of items that it has
  so far received over the pipe from the daemon. When a caller invokes
  [get_message] then we block if necessary until at least one message
  has come over the pipe into the queue, but we also slurp up any further
  messages that have come over the pipe as well. This way, if the client
  calls [find_already_queued_message] then they have a better chance of
  success.

  CARE!!! Jsonrpc is vulnerable to incomplete requests and malformed
  Content-length headers...
  The way it works around lack of threading in ocaml is with the
  assumption that if any data is available on stdin then a complete
  jsonrpc request can be read from stdin. If this is violated e.g.
  if the Content-length header is one byte short, then Jsonrpc will
  read a json string that lacks the final }, and will report this as
  a recoverable error (malformed json). Next, Jsonrpc will see that
  there is more data available on stdin, namely that final }, and so
  will block until a header+body has been read on stdin -- but nothing
  further will come beyond that }, so it blocks indefinitely.
  The only solution is to take care that Content-length is exact!
  ***************************************************************)

type writer = Hh_json.json -> unit

type timestamped_json = {
  json: Hh_json.json;
  timestamp: float;
}

(** These messages are the ones stored in the daemon's internal queue,
that are marshalled between daemon and main process, that are stored
in the main process queue, and that are handed to callers. *)
type queue_message =
  | Timestamped_json of timestamped_json
  | Fatal_exception of Marshal_tools.remote_exception_data
  | Recoverable_exception of Marshal_tools.remote_exception_data

(** This is the abstraction that callers use to get messages. It resides
in the caller's process. The fd is where we read from the pipe with the daemon,
and the queue holds all messages that we've read from the daemon's pipe so far. *)
type t = {
  daemon_in_fd: Unix.file_descr;
  (* fd used by main process to read messages from queue *)
  messages: queue_message Queue.t;
}

(** The daemon uses a 'select' syscall. It has to deal with stdin pipe
of messages from client that it has to read whenever available (but which might
be blocked if the client hasn't yet provided any further messages); it has to
deal with its queue of messages which it wants to write to the main process
(but not if such a write would be blocking). This type says which option
it will chose based on (1) what the 'select' syscall says is available,
(2) its own further logic. *)
type daemon_next_action =
  | Daemon_end_due_to_stdin_eof_and_empty_queue
      (** We received an EOF on stdin, and our queue is empty, so the daemon has nothing left to do. *)
  | Daemon_write_to_main_process_pipe
      (** There is no data to be read from stdin,
      and there are items in the daemon queue,
      and the pipe to the main process is open enough for us to write them without blocking. *)
  | Daemon_read_from_stdin
      (** EITHER there is data to be read from stdin so we prioritize that above all else,
      OR there are no items in the daemon queue so we might as well block on stdin until
      something arrives. *)

(* Try to read a message from the daemon's stdin, which is where all of the
   editor messages can be read from. May throw if the message is malformed. *)
let internal_read_message (reader : Buffered_line_reader.t) : timestamped_json =
  let message = reader |> Http_lite.read_message_utf8 in
  let json = Hh_json.json_of_string message in
  let timestamp = Unix.gettimeofday () in
  { json; timestamp }

(* Reads messages from the editor on stdin, parses them, and sends them to the
   main process.
   This runs in a different process because we also timestamp the messages, so
   we need to read them as soon as they come in. That is, we can't wait for any
   server computation to finish if we want to get an accurate timestamp. *)
let internal_run_daemon' (oc : queue_message Daemon.out_channel) : unit =
  let out_fd = Daemon.descr_of_out_channel oc in
  let reader = Buffered_line_reader.create Unix.stdin in
  let messages_to_send = Queue.create () in
  let rec loop ~allowed_to_read : unit =
    let daemon_next_action =
      if Buffered_line_reader.has_buffered_content reader then
        Daemon_read_from_stdin
      else
        let read_fds =
          if allowed_to_read then
            [Unix.stdin]
          else
            []
        in
        let write_fds =
          if not (Queue.is_empty messages_to_send) then
            [out_fd]
          else
            []
        in
        if List.is_empty read_fds && List.is_empty write_fds then
          Daemon_end_due_to_stdin_eof_and_empty_queue
        else
          (* An indefinite wait until we're able to either read or write.
             Reading will always take priority. *)
          let (readable_fds, _, _) = Unix.select read_fds write_fds [] (-1.0) in
          let ready_for_read = not (List.is_empty readable_fds) in
          if ready_for_read then
            Daemon_read_from_stdin
          else
            Daemon_write_to_main_process_pipe
    in
    let (should_continue, allowed_to_read) =
      match daemon_next_action with
      | Daemon_read_from_stdin ->
        (try
           let timestamped_json = internal_read_message reader in
           Queue.enqueue messages_to_send (Timestamped_json timestamped_json);
           (true, allowed_to_read)
         with
        | exn ->
          let e = Exception.wrap exn in
          let edata = Marshal_tools.of_exception e in
          let (allowed_to_read, message) =
            match exn with
            | Hh_json.Syntax_error _ -> (true, Recoverable_exception edata)
            | End_of_file
            | _ ->
              (false, Fatal_exception edata)
          in
          Queue.enqueue messages_to_send message;
          (true, allowed_to_read))
      | Daemon_write_to_main_process_pipe ->
        assert (not (Queue.is_empty messages_to_send));
        let message = Queue.dequeue_exn messages_to_send in
        (* We can assume that the entire write will succeed, since otherwise
            Marshal_tools.to_fd_with_preamble will throw an exception. *)
        Marshal_tools.to_fd_with_preamble out_fd message |> ignore;
        (true, allowed_to_read)
      | Daemon_end_due_to_stdin_eof_and_empty_queue -> (false, false)
    in
    if should_continue then
      loop ~allowed_to_read
    else
      ()
  in
  loop ~allowed_to_read:true;
  ()

(*  Main function for the daemon process. *)
let internal_run_daemon
    (_dummy_param : unit) (_ic, (oc : queue_message Daemon.out_channel)) =
  Printexc.record_backtrace true;
  try internal_run_daemon' oc with
  | exn ->
    let e = Exception.wrap exn in
    (* An exception that's gotten here is not simply a parse error, but
       something else, so we should terminate the daemon at this point. *)
    (try
       let out_fd = Daemon.descr_of_out_channel oc in
       Marshal_tools.to_fd_with_preamble
         out_fd
         (Fatal_exception (Marshal_tools.of_exception e))
       |> ignore
     with
    | _ ->
      (* There may be a broken pipe, for example. We should just give up on
         reporting the error. *)
      ())

let internal_entry_point : (unit, unit, queue_message) Daemon.entry =
  Daemon.register_entry_point "Jsonrpc" internal_run_daemon

(************************************************)
(* Queue functions that run in the main process *)
(************************************************)

let make_t () : t =
  let handle =
    Daemon.spawn
      ~channel_mode:`pipe
      (* We don't technically need to inherit stdout or stderr, but this might be
         useful in the event that we throw an unexpected exception in the daemon.
         It's also useful for print-statement debugging of the daemon. *)
      (Unix.stdin, Unix.stdout, Unix.stderr)
      internal_entry_point
      ()
  in
  let (ic, _) = handle.Daemon.channels in
  { daemon_in_fd = Daemon.descr_of_in_channel ic; messages = Queue.create () }

(* Read a message into the queue, and return the just-read message. *)
let read_single_message_into_queue_wait (t : t) : queue_message Lwt.t =
  let%lwt message =
    try%lwt
      let%lwt message =
        Marshal_tools_lwt.from_fd_with_preamble
          (Lwt_unix.of_unix_file_descr t.daemon_in_fd)
      in
      Lwt.return message
    with
    | (End_of_file | Unix.Unix_error (Unix.EBADF, _, _)) as exn ->
      let e = Exception.wrap exn in
      (* This is different from when the client hangs up. It handles the case
         that the daemon process exited: for example, if it was killed. *)
      Lwt.return (Fatal_exception (Marshal_tools.of_exception e))
  in
  Queue.enqueue t.messages message;
  Lwt.return message

let rec read_messages_into_queue_no_wait (t : t) : unit Lwt.t =
  let is_readable =
    Lwt_unix.readable (Lwt_unix.of_unix_file_descr t.daemon_in_fd)
  in
  let%lwt () =
    if is_readable then
      (* We're expecting this not to block because we just checked
         to make sure that there's something there. *)
      let%lwt message = read_single_message_into_queue_wait t in
      (* Now read any more messages that might be queued up. Only try to read more
         messages if the daemon is still available to read from. Otherwise, we may
         infinite loop as a result of `Unix.select` returning that a file
         descriptor is available to read on. *)
      match message with
      | Fatal_exception _ -> Lwt.return_unit
      | _ ->
        let%lwt () = read_messages_into_queue_no_wait t in
        Lwt.return_unit
    else
      Lwt.return_unit
  in
  Lwt.return_unit

let has_message (t : t) : bool =
  let is_readable =
    Lwt_unix.readable (Lwt_unix.of_unix_file_descr t.daemon_in_fd)
  in
  is_readable || not (Queue.is_empty t.messages)

let await_until_message (t : t) =
  (* The next message will come either from the queue or (if it's empty) then
     from some data coming in from [daemon_in_fd]. *)
  if Queue.is_empty t.messages then
    `Wait_for_data_here t.daemon_in_fd
  else
    `Already_has_message

let find_already_queued_message ~(f : timestamped_json -> bool) (t : t) :
    timestamped_json option =
  Queue.fold
    ~f:(fun found message ->
      match (found, message) with
      | (Some found, _) -> Some found
      | (None, Timestamped_json message) when f message -> Some message
      | _ -> None)
    ~init:None
    t.messages

let await_until_found
    (t : t)
    ~(predicate : timestamped_json -> bool)
    ~(cancellation_token : unit Lwt.t) : timestamped_json option Lwt.t =
  match find_already_queued_message ~f:predicate t with
  | Some message -> Lwt.return_some message
  | None ->
    let rec loop () : timestamped_json option Lwt.t =
      let%lwt () =
        Lwt.pick
          [
            Lwt_unix.wait_read (Lwt_unix.of_unix_file_descr t.daemon_in_fd);
            cancellation_token;
          ]
      in
      let was_cancelled = not (Lwt.is_sleeping cancellation_token) in
      if was_cancelled then
        Lwt.return_none
      else
        match%lwt read_single_message_into_queue_wait t with
        | Timestamped_json message when predicate message ->
          Lwt.return_some message
        | Timestamped_json _ -> loop ()
        | Fatal_exception _
        | Recoverable_exception _ ->
          Lwt.return_none
    in
    loop ()

let get_message (t : t) =
  (* Read one in a blocking manner to ensure that we have one. *)
  let%lwt () =
    if Queue.is_empty t.messages then
      let%lwt (_message : queue_message) =
        read_single_message_into_queue_wait t
      in
      Lwt.return_unit
    else
      Lwt.return_unit
  in
  (* Then read any others that got queued up so that we can see the maximum
     number of messages at once for invalidation purposes. *)
  let%lwt () = read_messages_into_queue_no_wait t in
  let item = Queue.dequeue_exn t.messages in
  match item with
  | Timestamped_json timestamped_json -> Lwt.return (`Message timestamped_json)
  | Fatal_exception data -> Lwt.return (`Fatal_exception data)
  | Recoverable_exception data -> Lwt.return (`Recoverable_exception data)

(************************************************)
(* Output functions for request                 *)
(************************************************)

let requests_counter : IMap.key ref = ref 0

let get_next_request_id () : int =
  incr requests_counter;
  !requests_counter
