open Core
open Lsp_fmt

type client_message_kind =
 | Request
 | Notification

let kind_to_string = function
 | Request -> "Request"
 | Notification -> "Notification"

type client_message = {
  timestamp : float;
  kind : client_message_kind;
  method_ : string; (* mandatory for request+notification; empty otherwise *)
  id : Hh_json.json option; (* mandatory for request+response *)
  params : Hh_json.json option; (* optional for request+notification *)
}

type result =
  | Message of client_message
  | Error
  | Exit

type daemon_message =
  | Client_message of client_message
  | No_more_messages
  | Recoverable_exn of exn (* like parse errors *)
  | Fatal_exn of exn (* like stdin closing *)

type daemon_operation =
  | Read
  | Write

type t = {
  (* The file descriptor that the main process uses to read the messages sent by
     the daemon. *)
  daemon_in_fd : Unix.file_descr;

  messages : result Queue.t;
}

(********************************************)
(* Functions that run in the daemon process *)
(********************************************)

(* Try to read a message from the daemon's stdin, which is where all of the
   editor messages can be read from. May throw if the message is malformed. *)
let read_message (reader : Buffered_line_reader.t) : client_message =
  (* May throw an exception during parsing. *)
  let message = try
    reader
    |> Http_lite.read_message_utf8
    |> Hh_json.json_of_string
    with
    | Http_lite.Malformed message -> raise (Lsp.Error.Invalid_request message)
    | e -> raise (Lsp.Error.Invalid_request (Printexc.to_string e))
  in
  let json = Some message in

  let id = Jget.val_opt json "id" in
  let method_ = Jget.string_opt json "method" in
  let params = Jget.val_opt json "params" in
  let kind = match id, method_ with
    | Some _id, Some _method -> Request
    | None,     Some _method -> Notification
    | _,        _            -> raise (Lsp.Error.Invalid_request "Not JsonRPC")
  in
  {
    timestamp = Unix.gettimeofday ();
    id;
    method_ = Option.value method_ ~default:""; (* is easier to consume *)
    params;
    kind;
  }


(* Reads messages from the editor on stdin, parses them, and sends them to the
   main process.

   This runs in a different process because we also timestamp the messages, so
   we need to read them as soon as they come in. That is, we can't wait for any
   server computation to finish if we want to get an accurate timestamp. *)
let run_daemon' (oc : daemon_message Daemon.out_channel) : unit =
  let out_fd = Daemon.descr_of_out_channel oc in
  let reader = Buffered_line_reader.create Unix.stdin in
  let messages_to_send = Queue.create () in

  let rec loop () =
    let operation =
      if Buffered_line_reader.has_buffered_content reader
      then Read
      else begin
        let read_fds = [Unix.stdin] in
        let has_messages_to_send = not (Queue.is_empty messages_to_send) in
        let write_fds =
          if has_messages_to_send
          then [out_fd]
          else []
        in

        (* Note that if there are no queued messages, this will always block
           until we're ready to read, rather than returning `Write`, even if
           stdout is capable of being written to. Furthermore, we will never
           need to queue a message to be written until we have read
           something. *)
        let readable_fds, _, _ = Unix.select read_fds write_fds [] (-1.0) in
        let ready_for_read = not (List.is_empty readable_fds) in
        if ready_for_read
        then Read
        else Write
      end
    in

    let should_continue = match operation with
      | Read -> begin
        try
          let message = read_message reader in
          Queue.push message messages_to_send;
          true
        with
          | End_of_file ->
            Marshal_tools.to_fd_with_preamble out_fd No_more_messages;
            false
          (* Most likely a parse error or similar. *)
          | e ->
            Marshal_tools.to_fd_with_preamble out_fd (Recoverable_exn e);
            true
        end
      | Write ->
        assert (not (Queue.is_empty messages_to_send));
        let message = Queue.pop messages_to_send in
        (* We can assume that the entire write will succeed, since otherwise
           Marshal_tools.to_fd_with_preamble will throw an exception. *)
        Marshal_tools.to_fd_with_preamble out_fd (Client_message message);
        true
    in
    if should_continue then loop ()
  in
  loop ()

(*  Main function for the daemon process. *)
let run_daemon
    (_dummy_param : unit)
    (_ic, (oc : daemon_message Daemon.out_channel)) =
  try
    run_daemon' oc
  with e ->
    (* An exception that's gotten here is not simply a parse error, but
       something else, so we should terminate the daemon at this point. *)
    try
      let out_fd = Daemon.descr_of_out_channel oc in
      Marshal_tools.to_fd_with_preamble out_fd (Fatal_exn e)
    with _ ->
      (* There may be a broken pipe, for example. We should just give up on
         reporting the error. *)
      ()

let entry_point = Daemon.register_entry_point "ClientMessageQueue" run_daemon

(******************************************)
(* Functions that run in the main process *)
(******************************************)

let make () : t =
  let handle = Daemon.spawn
    ~channel_mode:`pipe
    (* We don't technically need to inherit stdout or stderr, but this might be
       useful in the event that we throw an unexpected exception in the daemon.
       It's also useful for print-statement debugging of the daemon. *)
    (Unix.stdin, Unix.stdout, Unix.stderr)
    entry_point
    ()
  in
  let (ic, _) = handle.Daemon.channels in
  {
    daemon_in_fd = Daemon.descr_of_in_channel ic;
    messages = Queue.create ();
  }

let get_read_fd (message_queue : t) : Unix.file_descr =
  message_queue.daemon_in_fd

(* Read a message into the queue, and return the just-read message. *)
let read_single_message_into_queue_blocking (message_queue : t) : result =
  let message =
    try Marshal_tools.from_fd_with_preamble message_queue.daemon_in_fd
    with End_of_file ->
      (* This is different from when the client hangs up. It handles the case
         that the daemon process exited: for example, if it was killed. *)
      No_more_messages
  in

  let message = match message with
    | Client_message message -> Message message
    | No_more_messages -> Exit
    | Recoverable_exn e ->
      Hh_logger.exc e ~prefix:"Client message queue recoverable exception: ";
      Error
    | Fatal_exn e ->
      Hh_logger.exc e ~prefix:"Client message queue fatal exception: ";
      Exit
  in
  Queue.push message message_queue.messages;
  message

let rec read_messages_into_queue_nonblocking (message_queue : t) : unit =
  let readable_fds, _, _ = Unix.select [message_queue.daemon_in_fd] [] [] 0.0 in
  if not (List.is_empty readable_fds) then begin
    (* We're expecting this not to block because we just checked `Unix.select`
       to make sure that there's something there. *)
    let message = read_single_message_into_queue_blocking message_queue in

    (* Now read any more messages that might be queued up. Only try to read more
       messages if the daemon is still available to read from. Otherwise, we may
       infinite loop as a result of `Unix.select` returning that a file
       descriptor is available to read on. *)
    if message <> Exit
    then read_messages_into_queue_nonblocking message_queue;
  end

let has_message (queue : t) : bool =
  read_messages_into_queue_nonblocking queue;
  not (Queue.is_empty queue.messages)

let get_message (queue : t) : result =
  (* Read one in a blocking manner to ensure that we have one. *)
  if Queue.is_empty queue.messages
  then ignore (read_single_message_into_queue_blocking queue);
  (* Then read any others that got queued up so that we can see the maximum
     number of messages at once for invalidation purposes. *)
  read_messages_into_queue_nonblocking queue;

  Queue.pop queue.messages
