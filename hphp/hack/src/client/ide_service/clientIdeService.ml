(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

module Status = struct
  type t =
    | Initializing
    | Rpc of Telemetry.t list
    | Ready
    | Stopped of ClientIdeMessage.rich_error

  let is_ready (t : t) : bool =
    match t with
    | Ready -> true
    | _ -> false

  let to_log_string (t : t) : string =
    match t with
    | Initializing -> "Initializing"
    | Rpc requests ->
      Printf.sprintf
        "Rpc [%s]"
        (requests |> List.map ~f:Telemetry.to_string |> String.concat ~sep:",")
    | Ready -> "Ready"
    | Stopped { ClientIdeMessage.short_user_message; _ } ->
      Printf.sprintf "Stopped(%s)" short_user_message
end

module Stop_reason = struct
  type t =
    | Crashed
    | Closed
    | Editor_exited
    | Restarting
    | Testing

  let to_log_string (t : t) : string =
    match t with
    | Crashed -> "crashed"
    | Closed -> "closed"
    | Editor_exited -> "editor exited"
    | Restarting -> "restarting"
    | Testing -> "testing-only, you should not see this"
end

type state =
  | Uninitialized
      (** The ide_service is created. We may or may not have yet sent an
      initialize message to the daemon. *)
  | Failed_to_initialize of ClientIdeMessage.rich_error
      (** The response to our initialize message was a failure. This is
      a terminal state. *)
  | Initialized of { status: Status.t }
      (** We have received an initialize response from the daemon and all
      is well. The only thing that can take us out of this state is if
      someone invokes [stop], or if the daemon connection gets EOF. *)
  | Stopped of ClientIdeMessage.rich_error
      (** Someone called [stop] or the daemon connection got EOF.
      This is a terminal state. This is the only state that arose
      from actions on our side; all the other states arose from
      responses from clientIdeDaemon. *)

let state_to_log_string (state : state) : string =
  match state with
  | Uninitialized -> Printf.sprintf "Uninitialized"
  | Failed_to_initialize { ClientIdeMessage.category; _ } ->
    Printf.sprintf "Failed_to_initialize(%s)" category
  | Initialized env ->
    Printf.sprintf "Initialized(%s)" (Status.to_log_string env.status)
  | Stopped { ClientIdeMessage.category; _ } ->
    Printf.sprintf "Stopped(%s)" category

type message_wrapper =
  | Message_wrapper : 'a ClientIdeMessage.tracked_t -> message_wrapper
      (** Existential type wrapper for `ClientIdeMessage.t`s, so that we can put
      them in a queue without the typechecker trying to infer a concrete type for
      `'a` based on its first use. *)

type message_queue = message_wrapper Lwt_message_queue.t

type response_wrapper =
  | Response_wrapper : 'a ClientIdeMessage.timed_response -> response_wrapper
      (** Similar to [Message_wrapper] above. *)

type response_emitter = response_wrapper Lwt_message_queue.t

type notification_emitter = ClientIdeMessage.notification Lwt_message_queue.t

module Active_rpc_requests = struct
  type t = {
    requests: Telemetry.t IMap.t;
        (** These are requests which have been sent to the daemon but we haven't yet had
          a response, e.g. if we sent requests #3 and #4 and #5 and a response #5 has come
          back, then active would be #3 and #4. *)
    counter: int;
        (** Monotonically increasing counter, used as indices in [requests]. E.g. we might
          at one moment have requests #3 and #4 active, while the counter is at #6. *)
  }

  let new_ () : t = { requests = IMap.empty; counter = 0 }

  let add (telemetry : Telemetry.t) (t : t) : t * int =
    let id = t.counter in
    ({ requests = IMap.add id telemetry t.requests; counter = id + 1 }, id)

  let remove (id : int) (t : t) : t =
    { t with requests = IMap.remove id t.requests }

  let is_empty (t : t) : bool = IMap.is_empty t.requests

  let values (t : t) : Telemetry.t list = IMap.values t.requests
end

type t = {
  mutable state: state;
  mutable active_rpc_requests: Active_rpc_requests.t;
  state_changed_cv: unit Lwt_condition.t;
      (** Used to notify tasks when the state changes, so that they can wait for the
      IDE service to be initialized. *)
  daemon_handle: (unit, unit) Daemon.handle;
      (** The handle to the daemon process backing the IDE service.

      Note that `(unit, unit)` here refers to the input and output types of the
      IDE service. However, we don't use the Daemon API's method of
      producing/consuming these messages and instead do it with Lwt, so these
      type parameters are not used. *)
  in_fd: Lwt_unix.file_descr;
  out_fd: Lwt_unix.file_descr;
  messages_to_send: message_queue;
      (** The queue of messages that we have yet to send to the daemon. *)
  response_emitter: response_emitter;
      (** The queue of responses that we received from RPC calls to the daemon. We
      assume that we receive the responses in the same order that we sent their
      requests. *)
  notification_emitter: notification_emitter;
      (** The queue of notifications that the daemon emitted. Notifications can be
      emitted at any time, not just in response to an RPC call. *)
}

let log s = Hh_logger.log ("[ide-service] " ^^ s)

let log_debug s = Hh_logger.debug ("[ide-service] " ^^ s)

let set_state (t : t) (new_state : state) : unit =
  log
    "ClientIdeService.set_state %s -> %s"
    (state_to_log_string t.state)
    (state_to_log_string new_state);
  t.state <- new_state;
  Lwt_condition.broadcast t.state_changed_cv ()

let make (args : ClientIdeMessage.daemon_args) : t =
  let daemon_handle =
    Daemon.spawn
      ~channel_mode:`pipe
      (Unix.stdin, Unix.stdout, Unix.stderr)
      ClientIdeDaemon.daemon_entry_point
      args
  in
  let (ic, oc) = daemon_handle.Daemon.channels in
  let in_fd = Lwt_unix.of_unix_file_descr (Daemon.descr_of_in_channel ic) in
  let out_fd = Lwt_unix.of_unix_file_descr (Daemon.descr_of_out_channel oc) in
  {
    state = Uninitialized;
    active_rpc_requests = Active_rpc_requests.new_ ();
    state_changed_cv = Lwt_condition.create ();
    daemon_handle;
    in_fd;
    out_fd;
    messages_to_send = Lwt_message_queue.create ();
    response_emitter = Lwt_message_queue.create ();
    notification_emitter = Lwt_message_queue.create ();
  }

(** This function does an rpc to the daemon: it pushes a message
onto the daemon's stdin queue, then awaits until [serve] has stuck
the stdout response from the daemon response onto our [response_emitter].
The daemon updates [ref_unblocked_time], the time at which the
daemon starting handling the rpc.

The progress callback will be invoked during this call to rpc, at times
when the result of [get_status t] might have changed. It's designed
so the caller of rpc can, in their callback, invoke get_status and display
some kind of progress message.

Note: it is safe to call this method even while an existing rpc
is outstanding - we guarantee that results will be delivered in order.

Note: it is not safe to cancel this method, since we might end up
with no one reading the response to the message we just pushed, leading
to desync.

Note: If we're in Stopped state (due to someone calling [stop]) then
we'll refrain from sending the rpc. Stopped is the only state we enter
due to our own volition; all other states are just a reflection
of what state the daemon is in, and so it's fine for the daemon
to respond as it see fits while in the other states. *)
let rpc
    (t : t)
    ~(tracking_id : string)
    ~(ref_unblocked_time : float ref)
    ~(progress : unit -> unit)
    (message : 'a ClientIdeMessage.t) : ('a, Lsp.Error.t) Lwt_result.t =
  let tracked_message = { ClientIdeMessage.tracking_id; message } in
  try%lwt
    match t.state with
    | Stopped reason -> Lwt.return_error (ClientIdeUtils.to_lsp_error reason)
    | Uninitialized
    | Initialized _
    | Failed_to_initialize _ ->
      let success =
        Lwt_message_queue.push
          t.messages_to_send
          (Message_wrapper tracked_message)
      in
      if not success then
        (* queue closure is normal part of shutdown *)
        failwith "Could not send message (queue was closed)";

      (* If any rpc takes too long, we'll ask clientLsp to refresh status
         a short time in, and then again when it's done. *)
      let telemetry =
        Telemetry.create ()
        |> Telemetry.string_ ~key:"tracking_id" ~value:tracking_id
        |> Telemetry.string_
             ~key:"message"
             ~value:(ClientIdeMessage.t_to_string message)
      in
      let (active, id) =
        Active_rpc_requests.add telemetry t.active_rpc_requests
      in
      t.active_rpc_requests <- active;
      let pingPromise = Lwt_unix.sleep 0.2 |> Lwt.map progress in
      let%lwt (response : response_wrapper option) =
        Lwt_message_queue.pop t.response_emitter
      in
      t.active_rpc_requests <-
        Active_rpc_requests.remove id t.active_rpc_requests;
      Lwt.cancel pingPromise;
      progress ();

      (* when might t.active_rpc_count <> 0? well, if the caller did
         Lwt.pick [rpc t message1, rpc t message2], then active_rpc_count will
         reach a peak of 2, then when the first rpc has finished it will go dowwn
         to 1, then when the second rpc has finished it will go down to 0. *)

      (* Discussion about why the following is safe, even if multiple people call rpc:
         Imagine `let%lwt x = rpc(X) and y = rpc(Y)`.
         We will therefore push X onto the [messages_to_send] queue, then
         await [Lwt_message_queue.pop] on [response_emitter] for a response to X,
         then we'll push Y onto the queue, then await pop for a response to Y.
         The [messages_to_send] queue will necessarily send them in order X,Y.
         The daemon will receive X,Y in order.
         The daemon will handle X first (the daemon only handles a single message
         at a time) and send it response, then handle Y next and send its response.
         Our [serve] will read the responses to X,Y in order and put them onto
         [response_emitter].
         Why will "pop x" wake up and pick an element before "pop y" even though
         they're both waiting? The crucial fact is that "pop x" was called
         before "pop y". And [Lwt_message_queue.pop] is built upon
         [Lwt_condition.wait], which in turn is built upon [Lwt_sequence.t]:
         It maintains an ordered queue of callers who are awaiting on pop,
         and when a new item arrives then it wakes up the first one. *)
      (match response with
      | Some (Response_wrapper timed_response)
        when String.equal
               tracked_message.ClientIdeMessage.tracking_id
               timed_response.ClientIdeMessage.tracking_id ->
        ref_unblocked_time := timed_response.ClientIdeMessage.unblocked_time;
        let (response : ('a, Lsp.Error.t) result) =
          Result.map ~f:Obj.magic timed_response.ClientIdeMessage.response
        in
        (* Obj.magic cast is safe because if we pushed an 'a request
           then the daemon guarantees to return an 'a. *)
        Lwt.return response
      | Some _ ->
        (* as discussed above, this case will never be hit. *)
        failwith "ClientIdeService desync"
      | None ->
        (* queue closure is part of normal shutdown *)
        failwith "Could not read response: queue was closed")
  with
  | exn ->
    let e = Exception.wrap exn in
    Lwt.return_error
      (ClientIdeUtils.make_rich_error "rpc" ~e |> ClientIdeUtils.to_lsp_error)

let initialize_from_saved_state
    (t : t)
    ~(root : Path.t)
    ~(naming_table_load_info :
       ClientIdeMessage.Initialize_from_saved_state.naming_table_load_info
       option)
    ~(config : (string * string) list)
    ~(ignore_hh_version : bool)
    ~(open_files : Path.t list) :
    (unit, ClientIdeMessage.rich_error) Lwt_result.t =
  let open ClientIdeMessage in
  set_state t Uninitialized;

  try%lwt
    let message =
      {
        tracking_id = "init";
        message =
          Initialize_from_saved_state
            {
              Initialize_from_saved_state.root;
              naming_table_load_info;
              config;
              ignore_hh_version;
              open_files;
            };
      }
    in

    (* Can't use [rpc] here, since that depends on the [serve] event loop,
       which is called only after we return. We rely on the invariant
       that daemon will send us no messages until after it has responded
       to this first message. *)
    log_debug "-> %s [init]" (tracked_t_to_string message);
    let%lwt (_ : int) =
      Marshal_tools_lwt.to_fd_with_preamble t.out_fd message
    in
    let%lwt (response : message_from_daemon) =
      Marshal_tools_lwt.from_fd_with_preamble t.in_fd
    in
    log_debug "<- %s [init]" (message_from_daemon_to_string response);
    match response with
    | Response { response = Ok _; tracking_id = "init"; _ } -> Lwt.return_ok ()
    | Response { response = Error e; tracking_id = "init"; _ } ->
      (* that error has structure, which we wish to preserve in our error_data. *)
      Lwt.return_error
        (ClientIdeUtils.make_rich_error
           e.Lsp.Error.message
           ~data:e.Lsp.Error.data)
    | _ ->
      (* What we got back wasn't the 'init' response we expected:
         the clientIdeDaemon has violated its contract. *)
      failwith ("desync: " ^ message_from_daemon_to_string response)
  with
  | exn ->
    let e = Exception.wrap exn in
    let reason = ClientIdeUtils.make_rich_error "init_failed" ~e in
    set_state t (Failed_to_initialize reason);
    Lwt.return_error reason

let process_status_notification
    (t : t) (notification : ClientIdeMessage.notification) : unit =
  let open ClientIdeMessage in
  match (t.state, notification) with
  | (Failed_to_initialize _, _)
  | (Stopped _, _) ->
    (* terminal states, which don't change with notifications *)
    ()
  | (Uninitialized, Done_init (Ok ())) ->
    set_state t (Initialized { status = Status.Ready })
  | (Uninitialized, Done_init (Error edata)) ->
    set_state t (Failed_to_initialize edata)
  | (_, _) ->
    let message =
      Printf.sprintf
        "Unexpected notification '%s' in state '%s'"
        (ClientIdeMessage.notification_to_string notification)
        (state_to_log_string t.state)
    in
    ClientIdeUtils.log_bug message ~telemetry:true;
    ()

let destroy (t : t) ~(tracking_id : string) : unit Lwt.t =
  let%lwt () =
    match t.state with
    | Uninitialized
    | Failed_to_initialize _
    | Stopped _ ->
      Lwt.return_unit
    | Initialized _ ->
      let open Lsp.Error in
      let start_time = Unix.gettimeofday () in
      let ref_unblocked_time = ref 0. in
      let%lwt result =
        try%lwt
          Lwt.pick
            [
              rpc
                t
                ~tracking_id
                ~ref_unblocked_time
                ~progress:(fun () -> ())
                (ClientIdeMessage.Shutdown ());
              (let%lwt () = Lwt_unix.sleep 5.0 in
               Lwt.return_error
                 { code = InternalError; message = "timeout"; data = None });
            ]
        with
        | exn ->
          let e = Exception.wrap exn in
          Lwt.return_error
            (ClientIdeUtils.make_rich_error "destroy" ~e
            |> ClientIdeUtils.to_lsp_error)
      in
      let () =
        match result with
        | Ok () -> HackEventLogger.serverless_ide_destroy_ok start_time
        | Error { message; data; _ } ->
          HackEventLogger.serverless_ide_destroy_error start_time message data;
          log "ClientIdeService.destroy %s" message
      in
      Daemon.force_quit t.daemon_handle;
      Lwt.return_unit
  in
  Lwt_message_queue.close t.messages_to_send;
  Lwt_message_queue.close t.notification_emitter;
  Lwt_message_queue.close t.response_emitter;
  Lwt.return_unit

let stop
    (t : t)
    ~(tracking_id : string)
    ~(stop_reason : Stop_reason.t)
    ~(e : Exception.t option) : unit Lwt.t =
  (* we store both a user-facing reason here, and a programmatic error
     for use in subsequent telemetry *)
  let reason = ClientIdeUtils.make_rich_error "stop" ?e in
  (* We'll stick the stop_reason into that programmatic error, so that subsequent
     telemetry can pick it up. (It never affects the user-facing message.) *)
  let items =
    match reason.ClientIdeMessage.data with
    | None -> []
    | Some (Hh_json.JSON_Object items) -> items
    | Some json -> [("data", json)]
  in
  let items =
    ("stop_reason", stop_reason |> Stop_reason.to_log_string |> Hh_json.string_)
    :: items
  in
  let reason =
    { reason with ClientIdeMessage.data = Some (Hh_json.JSON_Object items) }
  in

  let%lwt () = destroy t ~tracking_id in
  (* Correctness here is very subtle... During the course of that call to
     'destroy', we do let%lwt on an rpc call to shutdown the daemon.
     Either that will return in 5s, or it won't; either way, we will
     synchronously force quit the daemon handle and close the message queue.
     The interleaving we have to worry about is: what will other code
     do while the state is still "Initialized", after we've sent the shutdown
     message to the daemon, and we're let%lwt awaiting for a responnse?
     Will anything go wrong?
     Well, the daemon has responded to 'shutdown' by deleting its hhi dir
     but leaving itself in its "Initialized" state.
     Meantime, some of our code uses the state "Stopped" as a signal to not
     do work, and some uses a closed message-queue as a signal to not do work
     and neither is met. We might indeed receive further requests from
     clientLsp and dutifully pass them on to the daemon and have it fail
     weirdly because of the lack of hhi.
     Luckily we're saved from that because clientLsp never makes rpc requests
     to us after it has called 'stop'. *)
  set_state t (Stopped reason);
  Lwt.return_unit

let cleanup_upon_shutdown_or_exn (t : t) ~(e : Exception.t option) : unit Lwt.t
    =
  (* We are invoked with e=None when one of the message-queues has said that
     it's closed. This indicates an orderly shutdown has been performed by 'stop'.
     We are invoked with e=Some when we had an exception in our main serve loop. *)
  let stop_reason =
    match e with
    | None ->
      log "Normal shutdown due to message-queue closure";
      Stop_reason.Closed
    | Some e ->
      ClientIdeUtils.log_bug "shutdown" ~e ~telemetry:true;
      Stop_reason.Crashed
  in
  (* We might as well call 'stop' in both cases; there'll be no harm. *)
  match t.state with
  | Stopped _ -> Lwt.return_unit
  | _ ->
    log "Shutting down...";
    let%lwt () = stop t ~tracking_id:"cleanup_or_shutdown" ~stop_reason ~e in
    Lwt.return_unit

let rec serve (t : t) : unit Lwt.t =
  (* Behavior of 'serve' is (1) take items from `messages_to_send` and send
     then over the wire to the daemon, (2) take items from the wire from the
     daemon and put them onto `notification_emitter` or `response_emitter` queues,
     (3) keep doing this until we discover that a queue has been closed, which
     is the "cancellation" signal for us to stop our loop.
     The code looks a bit funny because the only way to tell if a queue is closed
     is when we attemept to awaitingly-read or synchronously-write to it. *)
  try%lwt
    (* We mutate the data in `t` which is why we don't return a new `t` here. *)
    let%lwt next_action =
      (* Care! we only put things in a Pick which are safe to cancel. *)
      Lwt.pick
        [
          (let%lwt outgoing_opt = Lwt_message_queue.pop t.messages_to_send in
           match outgoing_opt with
           | None -> Lwt.return `Close
           | Some outgoing -> Lwt.return (`Outgoing outgoing));
          (let%lwt () = Lwt_unix.wait_read t.in_fd in
           Lwt.return `Incoming);
        ]
    in
    match next_action with
    | `Close ->
      let%lwt () = cleanup_upon_shutdown_or_exn t ~e:None in
      Lwt.return_unit
    | `Outgoing (Message_wrapper next_message) ->
      log_debug "-> %s" (ClientIdeMessage.tracked_t_to_string next_message);
      let%lwt (_ : int) =
        Marshal_tools_lwt.to_fd_with_preamble t.out_fd next_message
      in
      serve t
    | `Incoming ->
      let%lwt (message : ClientIdeMessage.message_from_daemon) =
        Marshal_tools_lwt.from_fd_with_preamble t.in_fd
      in
      log_debug "<- %s" (ClientIdeMessage.message_from_daemon_to_string message);
      let queue_is_open =
        match message with
        | ClientIdeMessage.Notification notification ->
          process_status_notification t notification;
          Lwt_message_queue.push t.notification_emitter notification
        | ClientIdeMessage.Response response ->
          Lwt_message_queue.push t.response_emitter (Response_wrapper response)
      in
      if queue_is_open then
        serve t
      else
        let%lwt () = cleanup_upon_shutdown_or_exn t ~e:None in
        Lwt.return_unit
  with
  | exn ->
    let e = Exception.wrap exn in
    (* cleanup function below will log the exception *)
    let%lwt () = cleanup_upon_shutdown_or_exn t ~e:(Some e) in
    Lwt.return_unit

let get_notifications (t : t) : notification_emitter = t.notification_emitter

let get_status (t : t) : Status.t =
  match t.state with
  | Uninitialized -> Status.Initializing
  | Failed_to_initialize error_data -> Status.Stopped error_data
  | Stopped reason -> Status.Stopped reason
  | Initialized { status } ->
    if
      Status.is_ready status
      && not (Active_rpc_requests.is_empty t.active_rpc_requests)
    then
      Status.Rpc (Active_rpc_requests.values t.active_rpc_requests)
    else
      status
