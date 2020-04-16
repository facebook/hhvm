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
    | Not_started
    | Initializing
    | Processing_files of ClientIdeMessage.Processing_files.t
    | Ready
    | Stopped of ClientIdeMessage.error_data

  let to_string (t : t) : string =
    match t with
    | Not_started -> "Not_started"
    | Initializing -> "Initializing"
    | Processing_files p ->
      Printf.sprintf
        "Processing_files(%s)"
        (ClientIdeMessage.Processing_files.to_string p)
    | Ready -> "Ready"
    | Stopped { ClientIdeMessage.short_user_message; _ } ->
      Printf.sprintf "Stopped(%s)" short_user_message
end

module Stop_reason = struct
  type t =
    | Crashed
    | Editor_exited
    | Restarting
    | Testing

  let to_string (t : t) : string =
    match t with
    | Crashed -> "crashed"
    | Editor_exited -> "editor exited"
    | Restarting -> "restarting"
    | Testing -> "testing-only, you should not see this"
end

type state =
  | Uninitialized of { wait_for_initialization: bool }
      (** The ide_service is created. We may or may not have yet sent an initialize
      message to the daemon, but we certainly haven't heard back yet. *)
  | Failed_to_initialize of ClientIdeMessage.error_data
      (** The response to our initialize message was a failure. This is
      a terminal state. *)
  | Initialized of { status: Status.t }
      (** We have received an initialize response from the daemon and all
      is well. The only thing that can take us out of this state is if
      someone invokes [stop], or if the daemon connection gets EOF. *)
  | Stopped of Stop_reason.t * ClientIdeMessage.error_data
      (** Someone called [stop] or the daemon connection got EOF.
      This is a terminal state. *)

let state_to_string (state : state) : string =
  match state with
  | Uninitialized { wait_for_initialization = true } ->
    "Uninitialized(will_wait_for_init)"
  | Uninitialized _ -> "Uninitialized"
  | Failed_to_initialize { ClientIdeMessage.short_user_message; _ } ->
    Printf.sprintf "Failed_to_initialize(%s)" short_user_message
  | Initialized env ->
    Printf.sprintf "Initialized(%s)" (Status.to_string env.status)
  | Stopped (reason, _) ->
    Printf.sprintf "Stopped(%s)" (Stop_reason.to_string reason)

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

type t = {
  mutable state: state;
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
    (state_to_string t.state)
    (state_to_string new_state);
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
    state = Uninitialized { wait_for_initialization = false };
    state_changed_cv = Lwt_condition.create ();
    daemon_handle;
    in_fd;
    out_fd;
    messages_to_send = Lwt_message_queue.create ();
    response_emitter = Lwt_message_queue.create ();
    notification_emitter = Lwt_message_queue.create ();
  }

let rec wait_for_initialization (t : t) : unit Lwt.t =
  match t.state with
  | Uninitialized _
  | Failed_to_initialize _ ->
    let%lwt () = Lwt_condition.wait t.state_changed_cv in
    wait_for_initialization t
  | Initialized _ -> Lwt.return_unit
  | Stopped _ ->
    failwith
      ( "Should not be waiting for the initialization of a stopped IDE service, "
      ^ "as this is a terminal state" )

(** rpc_internal pushes a message onto the daemon's queue, and awaits until
it can pop the response back. It updates ref_unnblocked_time, the time
at which the daemon started handling the message.
Note: it's not safe to cancel this, since we might end up
with no one reading the answer to the message we just pushed,
leading to desync. *)
let rpc_internal
    (t : t)
    ~(tracked_message : 'a ClientIdeMessage.tracked_t)
    ~(ref_unblocked_time : float ref) : ('a, Lsp.Error.t) Lwt_result.t =
  try%lwt
    let success =
      Lwt_message_queue.push
        t.messages_to_send
        (Message_wrapper tracked_message)
    in
    if not success then failwith "Could not send message (queue was closed)";

    (* Lwt_message_queue.pop is built upon Lwt_condition.wait, which has
    the guarantee that pops will be fulfilled in the order in which they
    were called. *)
    let%lwt (response : response_wrapper option) =
      Lwt_message_queue.pop t.response_emitter
    in
    match response with
    | None -> failwith "Could not read response: queue was closed"
    | Some (Response_wrapper { ClientIdeMessage.response; unblocked_time }) ->
      (* We don't carry around the tag at runtime that tells us what type of
      message the response was for. We're relying here on the invariant that
      responses are provided in the order that requests are sent, and that we're
      not sending any requests in parallel. This looks unsafe, but it's not
      adding additional un-safety. The `Response` message here came from
      `Marshal_tools_lwt.from_fd_with_preamble`, which is inherently unsafe
      (returns `'a`), and we've just happened to pass around that `'a` rather
      than coercing its type immediately. *)
      ref_unblocked_time := unblocked_time;
      let response = Result.map ~f:Obj.magic response in
      (match response with
      | Ok r -> Lwt.return_ok r
      | Error { ClientIdeMessage.medium_user_message; debug_details; _ } ->
        Lwt.return_error
          {
            Lsp.Error.code = Lsp.Error.UnknownErrorCode;
            message = medium_user_message;
            data = Lsp_fmt.error_data_of_string ~key:"log_string" debug_details;
          })
  with e ->
    let e = Exception.wrap e in
    Lwt.return_error
      {
        Lsp.Error.code = Lsp.Error.UnknownErrorCode;
        message =
          "Internal error during RPC call to IDE services: "
          ^ Exception.get_ctor_string e;
        data = Lsp_fmt.error_data_of_stack (Exception.to_string e);
      }

(** This function will always fail if we're in a terminal state.
If invoked with [needs_init:false] then we'll happily pass
the message on to the daemon regardless of its state.
But if invoked with [needs_init:true] and we haven't yet sent
an initialize request to the daemon, or haven't yet heard a response,
then we'll fail. This last behavior can be tweaked by the wait_for_initialize
flag passed to initialize_from_saved_state, used for tests; it means that instead
of failing in this case we'll await until the daemon's initialize
response.
Note: it's not safe to cancel this method: we might get an item on
the outgoing queue and no one to consume it, leading to desync.
Note: it's safe to call this even while another rpc is outstanding.
The responses will come back in the right order. That's because the
queue is built on top of [Lwt_condition.wait] and [Lwt_condition.signal]:
the Lwt_condition is an Lwt_sequence.t, and 'wait' sticks an item onto
the end of the sequence, and 'signal' waks up the first item. *)
let rpc
    (t : t)
    ~(tracking_id : string)
    ~(ref_unblocked_time : float ref)
    ~(needs_init : bool)
    (message : 'a ClientIdeMessage.t) : ('a, Lsp.Error.t) Lwt_result.t =
  let needs_init =
    if needs_init then
      `Needs_init
    else
      `Fine_without_init
  in
  let tracked_message = { ClientIdeMessage.tracking_id; message } in
  let open ClientIdeMessage in
  match (t.state, needs_init) with
  | (Uninitialized { wait_for_initialization = false }, `Needs_init) ->
    Lwt.return_error
      {
        Lsp.Error.code = Lsp.Error.RequestCancelled;
        message = "IDE service has not yet been initialized";
        data = None;
      }
  | (Failed_to_initialize info, _) ->
    Lwt.return_error
      {
        Lsp.Error.code = Lsp.Error.RequestCancelled;
        message = "IDE service failed to initialize: " ^ info.short_user_message;
        data =
          Lsp_fmt.error_data_of_string
            ~key:"not_running_reason"
            info.debug_details;
      }
  | (Stopped (reason, info), _) ->
    Lwt.return_error
      {
        Lsp.Error.code = Lsp.Error.RequestCancelled;
        message =
          Printf.sprintf
            "IDE service is stopped: %s. %s"
            (Stop_reason.to_string reason)
            info.short_user_message;
        data =
          Lsp_fmt.error_data_of_string
            ~key:"not_running_reason"
            info.debug_details;
      }
  | (Uninitialized { wait_for_initialization = true }, `Needs_init) ->
    let%lwt () = wait_for_initialization t in
    let%lwt result = rpc_internal t ~tracked_message ~ref_unblocked_time in
    Lwt.return result
  | (Initialized _, (`Needs_init | `Fine_without_init))
  | (Uninitialized _, `Fine_without_init) ->
    let%lwt result = rpc_internal t ~tracked_message ~ref_unblocked_time in
    Lwt.return result

let initialize_from_saved_state
    (t : t)
    ~(root : Path.t)
    ~(naming_table_saved_state_path : Path.t option)
    ~(wait_for_initialization : bool)
    ~(use_ranked_autocomplete : bool)
    ~(config : (string * string) list)
    ~(open_files : Path.t list) :
    (int, ClientIdeMessage.error_data) Lwt_result.t =
  set_state t (Uninitialized { wait_for_initialization });

  let param =
    {
      ClientIdeMessage.Initialize_from_saved_state.root;
      naming_table_saved_state_path;
      use_ranked_autocomplete;
      config;
      open_files;
    }
  in
  (* Do not use `do_rpc` here, as that depends on a running event loop in
  `serve`. But `serve` should only be called once the IDE service is
  initialized, after this function has completed. *)
  let message = ClientIdeMessage.Initialize_from_saved_state param in
  let tracked_message = { ClientIdeMessage.tracking_id = "init"; message } in
  log_debug
    "-> %s [initialize_from_saved_state]"
    (ClientIdeMessage.tracked_t_to_string tracked_message);
  let%lwt (_ : int) =
    Marshal_tools_lwt.to_fd_with_preamble t.out_fd tracked_message
  in
  let%lwt (response : ClientIdeMessage.message_from_daemon) =
    Marshal_tools_lwt.from_fd_with_preamble t.in_fd
  in
  log_debug
    "<- %s [initialize_from_saved_state]"
    (ClientIdeMessage.message_from_daemon_to_string response);
  match response with
  | ClientIdeMessage.Response { ClientIdeMessage.response = Ok r; _ } ->
    (* See comment on other use of Obj.magic in this file for why it's valid *)
    let (r : ClientIdeMessage.Initialize_from_saved_state.result) =
      Obj.magic r
    in
    let total =
      r
        .ClientIdeMessage.Initialize_from_saved_state
         .num_changed_files_to_process
    in
    log
      "Initialized IDE service process (log file at %s) (%d changed files)"
      (ServerFiles.client_ide_log root)
      total;
    let status =
      if total = 0 then
        Status.Ready
      else
        Status.Processing_files
          { ClientIdeMessage.Processing_files.processed = 0; total }
    in
    set_state t (Initialized { status });
    Lwt.return_ok total
  | ClientIdeMessage.Notification _ ->
    let stack = Exception.get_current_callstack_string 100 in
    let debug_details =
      "Failed to initialize IDE service process "
      ^ "because we received a notification before the initialization response"
    in
    log "%s\n%s" debug_details stack;
    let error_data = ClientIdeMessage.make_error_data debug_details ~stack in
    set_state t (Failed_to_initialize error_data);
    Lwt.return_error error_data
  | ClientIdeMessage.Response
      { ClientIdeMessage.response = Error error_data; _ } ->
    log "Failed to initialize IDE service process";
    set_state t (Failed_to_initialize error_data);
    Lwt.return_error error_data

let process_status_notification
    (t : t) (notification : ClientIdeMessage.notification) : unit =
  match t.state with
  | Uninitialized _
  | Failed_to_initialize _
  | Stopped _ ->
    ()
  | Initialized _state ->
    (match notification with
    | ClientIdeMessage.Initializing ->
      set_state t (Initialized { status = Status.Initializing })
    | ClientIdeMessage.Processing_files processed_files ->
      set_state
        t
        (Initialized { status = Status.Processing_files processed_files })
    | ClientIdeMessage.Done_processing ->
      set_state t (Initialized { status = Status.Ready }))

let destroy (t : t) ~(tracking_id : string) : unit Lwt.t =
  let%lwt () =
    match t.state with
    | Uninitialized _
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
                ~needs_init:true
                (ClientIdeMessage.Shutdown ());
              (let%lwt () = Lwt_unix.sleep 5.0 in
               Lwt.return_error
                 { code = InternalError; message = "timeout"; data = None });
            ]
        with e ->
          let e = Exception.wrap e in
          Lwt.return_error
            {
              code = InternalError;
              message = Exception.get_ctor_string e;
              data =
                Lsp_fmt.error_data_of_stack (Exception.get_backtrace_string e);
            }
      in
      let () =
        match result with
        | Ok () -> HackEventLogger.serverless_ide_destroy_ok start_time
        | Error { message; data; _ } ->
          HackEventLogger.serverless_ide_destroy_error start_time message data;
          log "ClientIdeService.destroy %s" message
      in
      Daemon.kill t.daemon_handle;
      Lwt.return_unit
  in
  Lwt_message_queue.close t.messages_to_send;
  Lwt_message_queue.close t.notification_emitter;
  Lwt_message_queue.close t.response_emitter;
  Lwt.return_unit

let stop (t : t) ~(tracking_id : string) ~(reason : Stop_reason.t) : unit Lwt.t
    =
  let stack = Exception.get_current_callstack_string 100 in
  let error_data = ClientIdeMessage.make_error_data "stopped" ~stack in
  let%lwt () = destroy t ~tracking_id in
  (* Correctness here is very subtle... During the course of that call to
  'destroy', we do let%lwt on an rpc call to shutdown the daemon.
  Either that will return in 5s, or it won't; either way, we will
  synchronously kill the daemon handle and close the message queus.
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
  set_state t (Stopped (reason, error_data));
  Lwt.return_unit

let cleanup_upon_shutdown_or_exn (t : t) ~(e : Exception.t option) : unit Lwt.t
    =
  (* We are invoked with e=None when one of the message-queues has said that
  it's closed. This indicates an orderly shutdown has been performed by 'stop'.
  We are invoked with e=Some when we had an exception in our main serve loop. *)
  let (message, stack) =
    match e with
    | None -> ("message-queue closed", "")
    | Some e -> (Exception.get_ctor_string e, Exception.get_backtrace_string e)
  in
  HackEventLogger.serverless_ide_crash ~message ~stack;
  log "Shutdown triggered by %s\n%s" message stack;
  (* We might as well call 'stop' in both cases; there'll be no harm. *)
  match t.state with
  | Stopped _ -> Lwt.return_unit
  | _ ->
    log "Shutting down...";
    let%lwt () = stop t ~tracking_id:"exception" ~reason:Stop_reason.Crashed in
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
  with e ->
    let e = Exception.wrap e in
    (* cleanup function below will log the exception *)
    let%lwt () = cleanup_upon_shutdown_or_exn t ~e:(Some e) in
    Lwt.return_unit

let get_notifications (t : t) : notification_emitter = t.notification_emitter

let get_status (t : t) : Status.t =
  match t.state with
  | Uninitialized _ -> Status.Initializing
  | Failed_to_initialize error_data -> Status.Stopped error_data
  | Stopped (reason, error_data) ->
    let debug_details =
      Stop_reason.to_string reason
      ^ "\n"
      ^ error_data.ClientIdeMessage.debug_details
    in
    let error_data = { error_data with ClientIdeMessage.debug_details } in
    Status.Stopped error_data
  | Initialized { status } -> status
