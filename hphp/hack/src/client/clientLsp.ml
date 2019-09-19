(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Lsp
open Lsp_fmt
open Hh_json_helpers

(* All hack-specific code relating to LSP goes in here. *)

(* The environment for hh_client with LSP *)
type env = {
  from: string;
  (* The source where the client was spawned from, i.e. nuclide, vim, emacs, etc. *)
  use_ffp_autocomplete: bool;
  (* Flag to turn on the (experimental) FFP based autocomplete *)
  use_serverless_ide: bool; (* Flag to provide IDE services from `hh_client` *)
}

(* We cache the state of the typecoverageToggle button, so that when Hack restarts,
  dynamic view stays in sync with the button in Nuclide *)
let cached_toggle_state = ref false

(************************************************************************)
(* Protocol orchestration & helpers                                     *)
(************************************************************************)

(** We have an idea of server state based on what we hear from the server:
   When we attempt a connection, we hear hopefully hear back that it's
   INITIALIZING, and when we eventually receive "hello" that means it's
   HANDLING_OR_READY, i.e. either handling a message, or ready to accept one.
   But at connection attempt, we might see that it's STOPPED, or hear from it
   that it's DENYING_CONNECTION (typically due to rebase).
   When the server's running normally, we sometimes here push notifications to
   tell us that it's TYPECHECKING, or has been STOLEN by another editor.
   At any point of communication we might hear from the server that it
   encountered a fatal exception, i.e. shutting down the pipe, so presumably
   it has been STOPPED. When we reattempt to connect once a second, maybe we'll
   get a better idea. *)
type hh_server_state =
  | Hh_server_stopped
  | Hh_server_initializing
  | Hh_server_handling_or_ready
  | Hh_server_denying_connection
  | Hh_server_unknown
  | Hh_server_typechecking_local
  | Hh_server_typechecking_global_blocking
  | Hh_server_typechecking_global_interruptible
  | Hh_server_typechecking_global_remote_blocking
  | Hh_server_stolen
  | Hh_server_forgot

type server_message = {
  push: ServerCommandTypes.push;
  has_updated_server_state: bool;
}
(** A push message from the server might come while we're waiting for a server-rpc
   response, or while we're free. The current architecture allows us to have
   arbitrary responses to push messages while we're free, but only a limited set
   of responses while we're waiting for a server-rpc - e.g. we can update our
   notion of the server_state, or send a message to the client, but we can't
   update our own state monad. The has_* fields are ad-hoc push-specific indicators
   of whether we've done some part of the response during the rpc. *)

type server_conn = {
  ic: Timeout.in_channel;
  oc: Out_channel.t;
  server_finale_file: string;
  pending_messages: server_message Queue.t;
      (* ones that arrived during current rpc *)
}

module Main_env = struct
  type t = {
    conn: server_conn;
    needs_idle: bool;
    editor_open_files: Lsp.TextDocumentItem.t SMap.t;
    uris_with_diagnostics: SSet.t;
    uris_with_unsaved_changes: SSet.t;
    (* see comment in get_uris_with_unsaved_changes *)
    status: status_params;
  }

  and status_params = {
    message: string;
    shortMessage: string option;
    type_: MessageType.t;
  }
end

module In_init_env = struct
  type t = {
    conn: server_conn;
    first_start_time: float;
    (* our first attempt to connect *)
    most_recent_start_time: float;
    (* for subsequent retries *)
    editor_open_files: Lsp.TextDocumentItem.t SMap.t;
    file_edits: Hh_json.json ImmQueue.t;
    uris_with_unsaved_changes: SSet.t;
        (* see comment in get_uris_with_unsaved_changes *)
  }
end

module Lost_env = struct
  type t = {
    p: params;
    editor_open_files: Lsp.TextDocumentItem.t SMap.t;
    uris_with_unsaved_changes: SSet.t;
    (* see comment in get_uris_with_unsaved_changes *)
    lock_file: string;
  }

  and params = {
    explanation: string;
    new_hh_server_state: hh_server_state;
    start_on_click: bool;
    (* if user clicks Restart, do we ClientStart before reconnecting? *)
    trigger_on_lsp: bool;
    (* reconnect if we receive any LSP request/notification *)
    trigger_on_lock_file: bool; (* reconnect if lockfile is created *)
  }
end

type state =
  (* Pre_init: we haven't yet received the initialize request.           *)
  | Pre_init
  (* In_init: we did respond to the initialize request, and now we're    *)
  (* waiting for a "Hello" from the server. When that comes we'll        *)
  (* request a permanent connection from the server, and process the     *)
  (* file_changes backlog, and switch to Main_loop.                      *)
  | In_init of In_init_env.t
  (* Main_loop: we have a working connection to both server and client.  *)
  | Main_loop of Main_env.t
  (* Lost_server: someone stole the persistent connection from us.       *)
  (* We might choose to grab it back if prompted...                      *)
  | Lost_server of Lost_env.t
  (* Post_shutdown: we received a shutdown request from the client, and  *)
  (* therefore shut down our connection to the server. We can't handle   *)
  (* any more requests from the client and will close as soon as it      *)
  (* notifies us that we can exit.                                       *)
  | Post_shutdown

type on_result = result:Hh_json.json option -> state -> state Lwt.t

type on_error =
  code:int ->
  message:string ->
  data:Hh_json.json option ->
  state ->
  state Lwt.t

(* Note: we assume that we won't ever initialize more than once, since this
promise can only be resolved once. *)
let ( (initialize_params_promise : Lsp.Initialize.params Lwt.t),
      (initialize_params_resolver : Lsp.Initialize.params Lwt.u) ) =
  Lwt.task ()

let hhconfig_version : string ref = ref "[NotYetInitialized]"

let can_autostart_after_mismatch : bool ref = ref true

let callbacks_outstanding : (on_result * on_error) IdMap.t ref =
  ref IdMap.empty

(* head is newest *)
let hh_server_state : (float * hh_server_state) list ref = ref []

let ref_from : string ref = ref ""

let showStatus_outstanding : string ref = ref ""

let log s = Hh_logger.log ("[client-lsp] " ^^ s)

let initialize_params_exc () : Lsp.Initialize.params =
  match Lwt.poll initialize_params_promise with
  | None -> failwith "initialize_params not yet received"
  | Some initialize_params -> initialize_params

let to_stdout (json : Hh_json.json) : unit =
  let s = Hh_json.json_to_string json ^ "\r\n\r\n" in
  Http_lite.write_message stdout s

let get_editor_open_files (state : state) :
    Lsp.TextDocumentItem.t SMap.t option =
  match state with
  | Main_loop menv -> Main_env.(Some menv.editor_open_files)
  | In_init ienv -> In_init_env.(Some ienv.editor_open_files)
  | Lost_server lenv -> Lost_env.(Some lenv.editor_open_files)
  | _ -> None

type event =
  | Server_hello
  | Server_message of server_message
  | Client_message of Jsonrpc.message
  | Client_ide_notification of ClientIdeMessage.notification
  (* once per second, on idle *)
  | Tick

(* Here are some exit points. *)
let exit_ok () = exit 0

let exit_fail () = exit 1

(* The following connection exceptions inform the main LSP event loop how to  *)
(* respond to an exception: was the exception a connection-related exception  *)
(* (one of these) or did it arise during other logic (not one of these)? Can  *)
(* we report the exception to the LSP client? Can we continue handling        *)
(* further LSP messages or must we quit? If we quit, can we do so immediately *)
(* or must we delay?  --  Separately, they also help us marshal callstacks    *)
(* across daemon- and process-boundaries.                                     *)
exception
  Client_fatal_connection_exception of Marshal_tools.remote_exception_data

exception
  Client_recoverable_connection_exception of
    Marshal_tools.remote_exception_data

exception
  Server_fatal_connection_exception of Marshal_tools.remote_exception_data

exception Server_nonfatal_exception of Marshal_tools.remote_exception_data

let state_to_string (state : state) : string =
  match state with
  | Pre_init -> "Pre_init"
  | In_init _ienv -> "In_init"
  | Main_loop _menv -> "Main_loop"
  | Lost_server _lenv -> "Lost_server"
  | Post_shutdown -> "Post_shutdown"

let hh_server_state_to_string (hh_server_state : hh_server_state) : string =
  match hh_server_state with
  | Hh_server_denying_connection -> "hh_server denying connection"
  | Hh_server_initializing -> "hh_server initializing"
  | Hh_server_stopped -> "hh_server stopped"
  | Hh_server_stolen -> "hh_server stolen"
  | Hh_server_typechecking_local -> "hh_server typechecking (local)"
  | Hh_server_typechecking_global_blocking ->
    "hh_server typechecking (global, blocking)"
  | Hh_server_typechecking_global_interruptible ->
    "hh_server typechecking (global, interruptible)"
  | Hh_server_typechecking_global_remote_blocking ->
    "hh_server typechecking (global remote, blocking)"
  | Hh_server_handling_or_ready -> "hh_server ready"
  | Hh_server_unknown -> "hh_server unknown state"
  | Hh_server_forgot -> "hh_server forgotten state"

(* This conversion is imprecise.  Comments indicate potential gaps *)
let completion_kind_to_si_kind
    (completion_kind : Completion.completionItemKind option) :
    SearchUtils.si_kind =
  Lsp.(
    SearchUtils.(
      match completion_kind with
      | Some Completion.Class -> SI_Class
      | Some Completion.Method -> SI_ClassMethod
      | Some Completion.Function -> SI_Function
      | Some Completion.Variable ->
        SI_LocalVariable (* or SI_Mixed, but that's never used *)
      | Some Completion.Property -> SI_Property
      | Some Completion.Constant -> SI_GlobalConstant (* or SI_ClassConstant *)
      | Some Completion.Interface -> SI_Interface (* or SI_Trait *)
      | Some Completion.Enum -> SI_Enum
      | Some Completion.Module -> SI_Namespace
      | Some Completion.Constructor -> SI_Constructor
      | Some Completion.Keyword -> SI_Keyword
      | Some Completion.Value -> SI_Literal
      | Some Completion.TypeParameter -> SI_Typedef
      (* The completion enum includes things we don't really support *)
      | _ -> SI_Unknown))

let si_kind_to_completion_kind (kind : SearchUtils.si_kind) :
    Completion.completionItemKind option =
  match kind with
  | SearchUtils.SI_XHP
  | SearchUtils.SI_Class ->
    Some Completion.Class
  | SearchUtils.SI_ClassMethod -> Some Completion.Method
  | SearchUtils.SI_Function -> Some Completion.Function
  | SearchUtils.SI_Mixed
  | SearchUtils.SI_LocalVariable ->
    Some Completion.Variable
  | SearchUtils.SI_Property -> Some Completion.Property
  | SearchUtils.SI_ClassConstant -> Some Completion.Constant
  | SearchUtils.SI_Interface
  | SearchUtils.SI_Trait ->
    Some Completion.Interface
  | SearchUtils.SI_Enum -> Some Completion.Enum
  | SearchUtils.SI_Namespace -> Some Completion.Module
  | SearchUtils.SI_Constructor -> Some Completion.Constructor
  | SearchUtils.SI_Keyword -> Some Completion.Keyword
  | SearchUtils.SI_Literal -> Some Completion.Value
  | SearchUtils.SI_GlobalConstant -> Some Completion.Constant
  | SearchUtils.SI_Typedef -> Some Completion.TypeParameter
  | SearchUtils.SI_Unknown -> None

(** We keep a log of server state over the past 2mins. When adding a new server
   state: if this state is the same as the current one, then ignore it. Also,
   retain only states younger than 2min plus the first one older than 2min.
   Newest state is at head of list. *)
let set_hh_server_state (new_hh_server_state : hh_server_state) : unit =
  let new_time = Unix.gettimeofday () in
  let rec retain rest =
    match rest with
    | [] -> []
    | (time, state) :: rest when time >= new_time -. 120.0 ->
      (time, state) :: retain rest
    | (time, state) :: _rest -> [(time, state)]
    (* retain only the first that's older *)
  in
  hh_server_state :=
    match !hh_server_state with
    | (prev_time, prev_hh_server_state) :: rest
      when prev_hh_server_state = new_hh_server_state ->
      (prev_time, prev_hh_server_state) :: retain rest
    | rest -> (new_time, new_hh_server_state) :: retain rest

let get_current_hh_server_state () : hh_server_state =
  (* current state is at head of list. *)
  match List.hd !hh_server_state with
  | None -> Hh_server_unknown
  | Some (_, hh_server_state) -> hh_server_state

let get_older_hh_server_state (requested_time : float) : hh_server_state =
  (* find the first item which is older than the specified time. *)
  match
    List.find !hh_server_state ~f:(fun (time, _) -> time <= requested_time)
  with
  | None -> Hh_server_forgot
  | Some (_, hh_server_state) -> hh_server_state

let get_root_opt () : Path.t option =
  match Lwt.poll initialize_params_promise with
  | None -> None (* haven't yet received initialize so we don't know *)
  | Some initialize_params ->
    let path = Some (Lsp_helpers.get_root initialize_params) in
    Some (Wwwroot.get path)

let get_root_wait () : Path.t Lwt.t =
  let%lwt initialize_params = initialize_params_promise in
  let path = Lsp_helpers.get_root initialize_params in
  Lwt.return (Wwwroot.get (Some path))

let read_hhconfig_version () : string Lwt.t =
  match get_root_opt () with
  | None -> Lwt.return "[NoRoot]"
  | Some root ->
    let file = Filename.concat (Path.to_string root) ".hhconfig" in
    let%lwt config = Config_file_lwt.parse_hhconfig file in
    (match config with
    | Ok (_hash, config) ->
      let version = SMap.get "version" config in
      Lwt.return (Option.value version ~default:"[NoVersion]")
    | Error message -> Lwt.return (Printf.sprintf "[NoHhconfig:%s]" message))

(* get_uris_with_unsaved_changes is the set of files for which we've          *)
(* received didChange but haven't yet received didSave/didOpen. It is purely  *)
(* a description of what we've heard of the editor, and is independent of     *)
(* whether or not they've yet been synced with hh_server.                     *)
(* As it happens: in Main_loop state all these files will already have been   *)
(* sent to hh_server; in In_init state all these files will have been queued  *)
(* up inside file_edits ready to be sent when we receive the hello; in        *)
(* Lost_server state they're not even queued up, and if ever we see hh_server *)
(* ready then we'll terminate the LSP server and trust the client to relaunch *)
(* us and resend a load of didOpen/didChange events.                          *)
let get_uris_with_unsaved_changes (state : state) : SSet.t =
  match state with
  | Main_loop menv -> menv.Main_env.uris_with_unsaved_changes
  | In_init ienv -> ienv.In_init_env.uris_with_unsaved_changes
  | Lost_server lenv -> lenv.Lost_env.uris_with_unsaved_changes
  | _ -> SSet.empty

let update_hh_server_state_if_necessary (event : event) : unit =
  ServerCommandTypes.(
    let helper push =
      match push with
      | BUSY_STATUS Needs_local_typecheck
      | BUSY_STATUS Done_local_typecheck
      | BUSY_STATUS (Done_global_typecheck _) ->
        set_hh_server_state Hh_server_handling_or_ready
      | BUSY_STATUS Doing_local_typecheck ->
        set_hh_server_state Hh_server_typechecking_local
      | BUSY_STATUS (Doing_global_typecheck global_typecheck_kind) ->
        set_hh_server_state
          (match global_typecheck_kind with
          | Blocking -> Hh_server_typechecking_global_blocking
          | Interruptible -> Hh_server_typechecking_global_interruptible
          | Remote_blocking _ -> Hh_server_typechecking_global_remote_blocking)
      | NEW_CLIENT_CONNECTED -> set_hh_server_state Hh_server_stolen
      | DIAGNOSTIC _
      | FATAL_EXCEPTION _
      | NONFATAL_EXCEPTION _ ->
        ()
    in
    match event with
    | Server_message { push; has_updated_server_state = false } -> helper push
    | _ -> ())

let rpc_lock = Lwt_mutex.create ()

let rpc
    (server_conn : server_conn)
    (ref_unblocked_time : float ref)
    (command : 'a ServerCommandTypes.t) : 'a Lwt.t =
  let%lwt result =
    Lwt_mutex.with_lock rpc_lock (fun () ->
        let callback () push =
          update_hh_server_state_if_necessary
            (Server_message { push; has_updated_server_state = false });
          Queue.enqueue
            server_conn.pending_messages
            { push; has_updated_server_state = true }
        in
        let%lwt result =
          ServerCommandLwt.rpc_persistent
            (server_conn.ic, server_conn.oc)
            ()
            callback
            command
        in
        match result with
        | Ok ((), res, start_server_handle_time) ->
          ref_unblocked_time := start_server_handle_time;
          Lwt.return res
        | Error
            ( (),
              Utils.Callstack _,
              ServerCommandLwt.Remote_fatal_exception remote_e_data ) ->
          raise (Server_fatal_connection_exception remote_e_data)
        | Error
            ( (),
              Utils.Callstack _,
              ServerCommandLwt.Remote_nonfatal_exception remote_e_data ) ->
          raise (Server_nonfatal_exception remote_e_data)
        | Error ((), Utils.Callstack stack, e) ->
          let message = Exn.to_string e in
          raise
            (Server_fatal_connection_exception { Marshal_tools.message; stack }))
  in
  Lwt.return result

let rpc_with_retry server_conn ref_unblocked_time command =
  ServerCommandTypes.Done_or_retry.call ~f:(fun () ->
      rpc server_conn ref_unblocked_time command)

(* Determine whether to read a message from the client (the editor) or the
   server (hh_server), or whether neither is ready within 1s. *)
let get_message_source (server : server_conn) (client : Jsonrpc.queue) :
    [ `From_server | `From_client | `From_ide_service of event | `No_source ]
    Lwt.t =
  (* Take action on server messages in preference to client messages, because
     server messages are very easy and quick to service (just send a message to
     the client), while client messages require us to launch a potentially
     long-running RPC command. *)
  let has_server_messages = not (Queue.is_empty server.pending_messages) in
  if has_server_messages then
    Lwt.return `From_server
  else if Jsonrpc.has_message client then
    Lwt.return `From_client
  else
    (* If no immediate messages are available, then wait up to 1 second. *)
    let server_read_fd =
      Unix.descr_of_out_channel server.oc |> Lwt_unix.of_unix_file_descr
    in
    let client_read_fd =
      Jsonrpc.get_read_fd client |> Lwt_unix.of_unix_file_descr
    in
    let%lwt message_source =
      Lwt.pick
        [
          (let%lwt () = Lwt_unix.sleep 1.0 in
           Lwt.return `No_source);
          (* Note that `wait_read` waits for the file descriptor to be readable, but
    does not actually read anything from it (so we won't end up with a race
    condition where we've read data from both file descriptors but only process
    the data from either the client or the server). *)
            (let%lwt () = Lwt_unix.wait_read server_read_fd in
             Lwt.return `From_server);
          (let%lwt () = Lwt_unix.wait_read client_read_fd in
           Lwt.return `From_client);
        ]
    in
    Lwt.return message_source

(* A simplified version of get_message_source which only looks at client *)
let get_client_message_source
    (client : Jsonrpc.queue) (ide_service : ClientIdeService.t) :
    [ `From_client | `From_ide_service of event | `No_source ] Lwt.t =
  if Jsonrpc.has_message client then
    Lwt.return `From_client
  else
    let client_read_fd =
      Jsonrpc.get_read_fd client |> Lwt_unix.of_unix_file_descr
    in
    let%lwt message_source =
      Lwt.pick
        [
          (let%lwt () = Lwt_unix.sleep 1.0 in
           Lwt.return `No_source);
          (let%lwt () = Lwt_unix.wait_read client_read_fd in
           Lwt.return `From_client);
          (let queue = ClientIdeService.get_notifications ide_service in
           let%lwt (notification : ClientIdeMessage.notification option) =
             Lwt_message_queue.pop queue
           in
           match notification with
           | None ->
             let%lwt () = Lwt_unix.sleep 1.1 in
             failwith
               "this `sleep` should have deferred to the `No_source case above"
           | Some message ->
             Lwt.return (`From_ide_service (Client_ide_notification message)));
        ]
    in
    Lwt.return message_source

(*  Read a message unmarshaled from the server's out_channel. *)
let read_message_from_server (server : server_conn) : event Lwt.t =
  ServerCommandTypes.(
    try%lwt
      let fd =
        Unix.descr_of_out_channel server.oc |> Lwt_unix.of_unix_file_descr
      in
      let%lwt (message : 'a ServerCommandTypes.message_type) =
        Marshal_tools_lwt.from_fd_with_preamble fd
      in
      match message with
      | Response _ -> failwith "unexpected response without request"
      | Push push ->
        Lwt.return (Server_message { push; has_updated_server_state = false })
      | Hello -> Lwt.return Server_hello
      | Ping -> failwith "unexpected ping on persistent connection"
    with e ->
      let message = Exn.to_string e in
      let stack = Printexc.get_backtrace () in
      raise
        (Server_fatal_connection_exception { Marshal_tools.message; stack }))

(* get_next_event: picks up the next available message from either client or
   server. The way it's implemented, at the first character of a message
   from either client or server, we block until that message is completely
   received. Note: if server is None (meaning we haven't yet established
   connection with server) then we'll just block waiting for client. *)
let get_next_event
    (state : state) (client : Jsonrpc.queue) (ide_service : ClientIdeService.t)
    : event Lwt.t =
  let from_server (server : server_conn) : event Lwt.t =
    if Queue.is_empty server.pending_messages then
      read_message_from_server server
    else
      Lwt.return (Server_message (Queue.dequeue_exn server.pending_messages))
  in
  let from_client (client : Jsonrpc.queue) : event Lwt.t =
    let%lwt message = Jsonrpc.get_message client in
    match message with
    | `Message message -> Lwt.return (Client_message message)
    | `Fatal_exception edata -> raise (Client_fatal_connection_exception edata)
    | `Recoverable_exception edata ->
      raise (Client_recoverable_connection_exception edata)
  in
  match state with
  | Main_loop { Main_env.conn; _ }
  | In_init { In_init_env.conn; _ } ->
    let%lwt message_source = get_message_source conn client in
    (match message_source with
    | `From_client ->
      let%lwt message = from_client client in
      Lwt.return message
    | `From_server ->
      let%lwt message = from_server conn in
      Lwt.return message
    | `From_ide_service message -> Lwt.return message
    | `No_source -> Lwt.return Tick)
  | _ ->
    let%lwt message_source = get_client_message_source client ide_service in
    (match message_source with
    | `From_client ->
      let%lwt message = from_client client in
      Lwt.return message
    | `From_ide_service message -> Lwt.return message
    | `No_source -> Lwt.return Tick)

type powered_by =
  | Hh_server
  | Language_server
  | Serverless_ide

let respond_jsonrpc
    ~(powered_by : powered_by)
    (message : Jsonrpc.message)
    (json : Hh_json.json) : unit =
  let powered_by =
    match powered_by with
    | Serverless_ide -> Some "serverless_ide"
    | Hh_server
    | Language_server ->
      None
  in
  Jsonrpc.respond to_stdout ?powered_by message json

let notify_jsonrpc
    ~(powered_by : powered_by) (method_ : string) (json : Hh_json.json) : unit
    =
  let powered_by =
    match powered_by with
    | Serverless_ide -> Some "serverless_ide"
    | Hh_server
    | Language_server ->
      None
  in
  Jsonrpc.notify to_stdout ?powered_by method_ json

(* respond_to_error: if we threw an exception during the handling of a request,
   report the exception to the client as the response to their request. *)
let respond_to_error (event : event option) (e : exn) (stack : string) : unit =
  Error.(
    let e = error_of_exn e in
    match event with
    | Some (Client_message c) when c.Jsonrpc.kind = Jsonrpc.Request ->
      print_error e stack |> respond_jsonrpc ~powered_by:Language_server c
    | _ ->
      Lsp_helpers.telemetry_error
        to_stdout
        (Printf.sprintf "%s [%i]\n%s" e.message e.code stack))

let status_tick () : string =
  (* OCaml has pretty poor Unicode support.
   # @lint-ignore TXT5 The # sign is needed for the ignore to be respected. *)
  let statusFrames = [|"□"; "■"|] in
  let time = Unix.time () in
  statusFrames.(int_of_float time mod 2)

(* request_showStatus: pops up a dialog *)
let request_showStatus
    ?(on_result : on_result = (fun ~result:_ state -> Lwt.return state))
    ?(on_error : on_error =
      (fun ~code:_ ~message:_ ~data:_ state -> Lwt.return state))
    (params : ShowStatus.params) : unit =
  let initialize_params = initialize_params_exc () in
  if not (Lsp_helpers.supports_status initialize_params) then
    ()
  else
    (* We try not to send duplicate statuses. *)
    (* That means: if you call request_showStatus but your message is the same as *)
    (* what's already up, then you won't be shown, and your callbacks won't be shown. *)
    let msg = params.ShowStatus.request.ShowMessageRequest.message in
    if msg = !showStatus_outstanding then
      ()
    else (
      showStatus_outstanding := msg;
      let id = NumberId (Jsonrpc.get_next_request_id ()) in
      let json =
        Lsp_fmt.print_lsp (RequestMessage (id, ShowStatusRequest params))
      in
      to_stdout json;

      (* save the callback-handlers *)
      let on_result2 ~result state =
        if msg = !showStatus_outstanding then showStatus_outstanding := "";
        on_result result state
      in
      let on_error2 ~code ~message ~data state =
        if msg = !showStatus_outstanding then showStatus_outstanding := "";
        on_error code message data state
      in
      callbacks_outstanding :=
        IdMap.add id (on_result2, on_error2) !callbacks_outstanding
    )

(* request_showMessage: pops up a dialog *)
let request_showMessage
    (on_result : on_result)
    (on_error : on_error)
    (type_ : MessageType.t)
    (message : string)
    (titles : string list) : ShowMessageRequest.t =
  (* send the request *)
  let id = NumberId (Jsonrpc.get_next_request_id ()) in
  let actions =
    List.map titles ~f:(fun title -> { ShowMessageRequest.title })
  in
  let request =
    ShowMessageRequestRequest { ShowMessageRequest.type_; message; actions }
  in
  let json = Lsp_fmt.print_lsp (RequestMessage (id, request)) in
  to_stdout json;

  (* save the callback-handlers *)
  callbacks_outstanding :=
    IdMap.add id (on_result, on_error) !callbacks_outstanding;

  (* return a token *)
  ShowMessageRequest.Present { id }

(* dismiss_showMessageRequest: sends a cancellation-request for the dialog *)
let dismiss_showMessageRequest (dialog : ShowMessageRequest.t) :
    ShowMessageRequest.t =
  begin
    match dialog with
    | ShowMessageRequest.Absent -> ()
    | ShowMessageRequest.Present { id; _ } ->
      let notification = CancelRequestNotification { CancelRequest.id } in
      let json = Lsp_fmt.print_lsp (NotificationMessage notification) in
      to_stdout json
  end;
  ShowMessageRequest.Absent

(* dismiss_ui: dismisses all dialogs, progress- and action-required           *)
(* indicators and diagnostics in a state.                                     *)
let dismiss_ui (state : state) : state =
  match state with
  | In_init ienv -> In_init ienv
  | Main_loop menv ->
    Main_env.(
      Main_loop
        {
          menv with
          uris_with_diagnostics =
            Lsp_helpers.dismiss_diagnostics
              to_stdout
              menv.uris_with_diagnostics;
        })
  | Lost_server lenv -> Lost_server lenv
  | Pre_init -> Pre_init
  | Post_shutdown -> Post_shutdown

(************************************************************************)
(* Conversions - ad-hoc ones written as needed them, not systematic     *)
(************************************************************************)

let lsp_uri_to_path = Lsp_helpers.lsp_uri_to_path

let path_to_lsp_uri = Lsp_helpers.path_to_lsp_uri

let lsp_position_to_ide (position : Lsp.position) : Ide_api_types.position =
  { Ide_api_types.line = position.line + 1; column = position.character + 1 }

let lsp_file_position_to_hack (params : Lsp.TextDocumentPositionParams.t) :
    string * int * int =
  Lsp.TextDocumentPositionParams.(
    let { Ide_api_types.line; column } = lsp_position_to_ide params.position in
    let filename =
      Lsp_helpers.lsp_textDocumentIdentifier_to_filename params.textDocument
    in
    (filename, line, column))

let rename_params_to_document_position (params : Lsp.Rename.params) :
    Lsp.TextDocumentPositionParams.t =
  Rename.
    {
      TextDocumentPositionParams.textDocument = params.textDocument;
      position = params.position;
    }

let hack_pos_to_lsp_range (pos : 'a Pos.pos) : Lsp.range =
  (* .hhconfig errors are Positions with a filename, but dummy start/end
   * positions. Handle that case - and Pos.none - specially, as the LSP
   * specification requires line and character >= 0, and VSCode silently
   * drops diagnostics that violate the spec in this way *)
  if pos = Pos.make_from (Pos.filename pos) then
    { start = { line = 0; character = 0 }; end_ = { line = 0; character = 0 } }
  else
    let (line1, col1, line2, col2) = Pos.destruct_range pos in
    {
      start = { line = line1 - 1; character = col1 - 1 };
      end_ = { line = line2 - 1; character = col2 - 1 };
    }

let hack_pos_to_lsp_location (pos : string Pos.pos) ~(default_path : string) :
    Lsp.Location.t =
  Lsp.Location.
    {
      uri = path_to_lsp_uri (Pos.filename pos) ~default_path;
      range = hack_pos_to_lsp_range pos;
    }

let ide_range_to_lsp (range : Ide_api_types.range) : Lsp.range =
  {
    Lsp.start =
      {
        Lsp.line = range.Ide_api_types.st.Ide_api_types.line - 1;
        character = range.Ide_api_types.st.Ide_api_types.column - 1;
      };
    end_ =
      {
        Lsp.line = range.Ide_api_types.ed.Ide_api_types.line - 1;
        character = range.Ide_api_types.ed.Ide_api_types.column - 1;
      };
  }

let lsp_range_to_ide (range : Lsp.range) : Ide_api_types.range =
  Ide_api_types.
    {
      st = lsp_position_to_ide range.start;
      ed = lsp_position_to_ide range.end_;
    }

let hack_symbol_definition_to_lsp_construct_location
    (symbol : string SymbolDefinition.t) ~(default_path : string) :
    Lsp.Location.t =
  SymbolDefinition.(hack_pos_to_lsp_location symbol.span ~default_path)

let hack_pos_definition_to_lsp_identifier_location
    (sid : Pos.absolute * string) ~(default_path : string) :
    Lsp.DefinitionLocation.t =
  let (pos, title) = sid in
  let location = hack_pos_to_lsp_location pos ~default_path in
  Lsp.DefinitionLocation.{ location; title = Some title }

let hack_symbol_definition_to_lsp_identifier_location
    (symbol : string SymbolDefinition.t) ~(default_path : string) :
    Lsp.DefinitionLocation.t =
  SymbolDefinition.(
    let location = hack_pos_to_lsp_location symbol.pos ~default_path in
    Lsp.DefinitionLocation.
      {
        location;
        title = Some (Utils.strip_ns symbol.SymbolDefinition.full_name);
      })

let hack_errors_to_lsp_diagnostic
    (filename : string) (errors : Pos.absolute Errors.error_ list) :
    PublishDiagnostics.params =
  Lsp.Location.(
    let location_message (error : Pos.absolute * string) :
        Lsp.Location.t * string =
      let (pos, message) = error in
      let { uri; range } =
        hack_pos_to_lsp_location pos ~default_path:filename
      in
      ({ Location.uri; range }, message)
    in
    let hack_error_to_lsp_diagnostic (error : Pos.absolute Errors.error_) =
      let all_messages =
        Errors.to_list error |> List.map ~f:location_message
      in
      let (first_message, additional_messages) =
        match all_messages with
        | hd :: tl -> (hd, tl)
        | [] -> failwith "Expected at least one error in the error list"
      in
      let ({ range; _ }, message) = first_message in
      let relatedInformation =
        additional_messages
        |> List.map ~f:(fun (location, message) ->
               {
                 PublishDiagnostics.relatedLocation = location;
                 relatedMessage = message;
               })
      in
      let severity =
        match Errors.get_severity error with
        | Errors.Error -> Some PublishDiagnostics.Error
        | Errors.Warning -> Some PublishDiagnostics.Warning
      in
      {
        Lsp.PublishDiagnostics.range;
        severity;
        code = PublishDiagnostics.IntCode (Errors.get_code error);
        source = Some "Hack";
        message;
        relatedInformation;
        relatedLocations = relatedInformation (* legacy FB extension *);
      }
    in
    (* The caller is required to give us a non-empty filename. If it is empty,  *)
    (* the following path_to_lsp_uri will fall back to the default path - which *)
    (* is also empty - and throw, logging appropriate telemetry.                *)
    {
      Lsp.PublishDiagnostics.uri = path_to_lsp_uri filename ~default_path:"";
      diagnostics = List.map errors ~f:hack_error_to_lsp_diagnostic;
    })

(************************************************************************)
(* Protocol                                                             *)
(************************************************************************)
let get_document_contents
    (editor_open_files : Lsp.TextDocumentItem.t SMap.t) (uri : string) :
    string option =
  match SMap.get uri editor_open_files with
  | Some document -> Some document.TextDocumentItem.text
  | None ->
    let rawpath = String_utils.lstrip uri "file://" in
    (try
       let contents = Disk.cat rawpath in
       Some contents
     with _ -> None)

let get_document_location
    (editor_open_files : Lsp.TextDocumentItem.t SMap.t)
    (params : Lsp.TextDocumentPositionParams.t) :
    ClientIdeMessage.document_location =
  let (file_path, line, column) = lsp_file_position_to_hack params in
  let uri =
    params.TextDocumentPositionParams.textDocument.TextDocumentIdentifier.uri
  in
  let file_path = Path.make file_path in
  let file_contents = get_document_contents editor_open_files uri in
  { ClientIdeMessage.file_path; file_contents; line; column }

let do_shutdown
    (state : state)
    (ide_service : ClientIdeService.t)
    (ref_unblocked_time : float ref) : state Lwt.t =
  log "Received shutdown request";
  let state = dismiss_ui state in
  let%lwt () =
    match state with
    | Main_loop menv ->
      (* In Main_loop state, we're expected to unsubscribe diagnostics and tell *)
      (* server to disconnect so it can revert the state of its unsaved files.  *)
      Main_env.(
        Hh_logger.log
          "Diag_subscribe: clientLsp do_shutdown unsubscribing diagnostic 0 ";
        let%lwt () =
          rpc
            menv.conn
            ref_unblocked_time
            (ServerCommandTypes.UNSUBSCRIBE_DIAGNOSTIC 0)
        in
        let%lwt () = rpc menv.conn (ref 0.0) ServerCommandTypes.DISCONNECT in
        Lwt.return_unit)
    | In_init _ienv ->
      (* In In_init state, even though we have a 'conn', it's still waiting for *)
      (* the server to become responsive, so there's no use sending any rpc     *)
      (* messages to the server over it.                                        *)
      Lwt.return_unit
    | _ ->
      (* No other states have a 'conn' to send any disconnect messages over.    *)
      Lwt.return_unit
  and () = ClientIdeService.destroy ide_service in
  Lwt.return Post_shutdown

let state_to_rage (state : state) : string =
  let details =
    match state with
    | Pre_init -> []
    | Post_shutdown -> []
    | Main_loop menv ->
      Main_env.
        [
          "needs_idle";
          menv.needs_idle |> string_of_bool;
          "editor_open_files";
          menv.editor_open_files |> SMap.keys |> List.length |> string_of_int;
          "uris_with_diagnostics";
          menv.uris_with_diagnostics |> SSet.cardinal |> string_of_int;
          "uris_with_unsaved_changes";
          menv.uris_with_unsaved_changes |> SSet.cardinal |> string_of_int;
          "status.message";
          menv.status.message;
          "status.shortMessage";
          Option.value menv.status.shortMessage ~default:"";
        ]
    | In_init ienv ->
      In_init_env.
        [
          "first_start_time";
          ienv.first_start_time |> string_of_float;
          "most_recent_start_time";
          ienv.most_recent_start_time |> string_of_float;
          "editor_open_files";
          ienv.editor_open_files |> SMap.keys |> List.length |> string_of_int;
          "file_edits";
          ienv.file_edits |> ImmQueue.length |> string_of_int;
          "uris_with_unsaved_changes";
          ienv.uris_with_unsaved_changes |> SSet.cardinal |> string_of_int;
        ]
    | Lost_server lenv ->
      Lost_env.
        [
          "editor_open_files";
          lenv.editor_open_files |> SMap.keys |> List.length |> string_of_int;
          "uris_with_unsaved_changes";
          lenv.uris_with_unsaved_changes |> SSet.cardinal |> string_of_int;
          "lock_file";
          lenv.lock_file;
          "explanation";
          lenv.p.explanation;
          "new_hh_server_state";
          lenv.p.new_hh_server_state |> hh_server_state_to_string;
          "start_on_click";
          lenv.p.start_on_click |> string_of_bool;
          "trigger_on_lsp";
          lenv.p.trigger_on_lsp |> string_of_bool;
          "trigger_on_lock_file";
          lenv.p.trigger_on_lock_file |> string_of_bool;
        ]
  in
  state_to_string state ^ "\n" ^ String.concat ~sep:"\n" details ^ "\n"

let do_rage (state : state) (ref_unblocked_time : float ref) :
    Rage.result Lwt.t =
  Rage.(
    let items : rageItem list ref = ref [] in
    let add item = items := item :: !items in
    let add_data data = add { title = None; data } in
    let add_fn fn =
      if Sys.file_exists fn then
        add { title = Some fn; data = Sys_utils.cat fn }
    in
    let get_stack (pid, reason) : string Lwt.t =
      let pid = string_of_int pid in
      let format_data msg : string Lwt.t =
        Lwt.return (Printf.sprintf "PSTACK %s (%s) - %s\n\n" pid reason msg)
      in
      Hh_logger.log "Getting pstack for %s" pid;
      match%lwt Lwt_utils.exec_checked "pstack" [|pid|] with
      | Ok result ->
        let stack = result.Lwt_utils.Process_success.stdout in
        format_data stack
      | Error _ ->
        (* pstack is just an alias for gstack, but it's not present on all systems. *)
        Hh_logger.log
          "Failed to execute pstack for %s. Executing gstack instead"
          pid;
        (match%lwt Lwt_utils.exec_checked "gstack" [|pid|] with
        | Ok result ->
          let stack = result.Lwt_utils.Process_success.stdout in
          format_data stack
        | Error e ->
          let err =
            "unable to get pstack - " ^ e.Lwt_utils.Process_failure.stderr
          in
          format_data err)
    in
    (* logfiles. Start them, but don't wait yet because we want this to run concurrently with fetching
     * the server logs. *)
    let get_log_files =
      match get_root_opt () with
      | Some root ->
        add_fn (ServerFiles.log_link root);
        add_fn (ServerFiles.log_link root ^ ".old");
        add_fn (ServerFiles.monitor_log_link root);
        add_fn (ServerFiles.monitor_log_link root ^ ".old");
        add_fn (ServerFiles.client_lsp_log root);
        add_fn (ServerFiles.client_lsp_log root ^ ".old");
        add_fn (ServerFiles.client_ide_log root);
        add_fn (ServerFiles.client_ide_log root ^ ".old");
        (try%lwt
           let pids = PidLog.get_pids (ServerFiles.pids_file root) in
           let is_interesting (_, reason) =
             not (String_utils.string_starts_with reason "slave")
           in
           let%lwt stacks =
             Lwt.pick
               [
                 (let%lwt () = Lwt_unix.sleep 4.50 in
                  Lwt.return ["Timed out while getting pstacks"]);
                 pids
                 |> List.filter ~f:is_interesting
                 |> Lwt_list.map_p get_stack;
               ]
           in
           List.iter stacks ~f:add_data;
           Lwt.return_unit
         with e ->
           let message = Exn.to_string e in
           let stack = Printexc.get_backtrace () in
           Lwt.return
             (add_data
                (Printf.sprintf "Failed to get PIDs: %s - %s" message stack)))
      | None -> Lwt.return_unit
    in
    (* client *)
    add_data ("LSP adapter state: " ^ state_to_rage state ^ "\n");

    (* client: version *)
    let current_version = read_hhconfig_version () in
    (* client's log of server state *)
    let tnow = Unix.gettimeofday () in
    let server_state_to_string (tstate, state) =
      Unix.(
        let tdiff = tnow -. tstate in
        let state = hh_server_state_to_string state in
        let tm = Unix.localtime tstate in
        let ms = int_of_float (tstate *. 1000.) mod 1000 in
        Printf.sprintf
          "[%02d:%02d:%02d.%03d] [%03.3fs ago] %s\n"
          tm.tm_hour
          tm.tm_min
          tm.tm_sec
          ms
          tdiff
          state)
    in
    let server_state_strings =
      List.map ~f:server_state_to_string !hh_server_state
    in
    add_data
      (String.concat
         ~sep:""
         ("LSP belief of hh_server_state:\n" :: server_state_strings));

    (* server *)
    let server_promise =
      match state with
      | Main_loop menv ->
        Main_env.(
          let%lwt items =
            rpc menv.conn ref_unblocked_time ServerCommandTypes.RAGE
          in
          let add i =
            add
              {
                title = i.ServerRageTypes.title;
                data = i.ServerRageTypes.data;
              }
          in
          List.iter items ~f:add;
          Lwt.return (Ok ()))
      | _ -> Lwt.return (Error "server rage - not in main loop")
    in
    let timeout_promise =
      let%lwt () = Lwt_unix.sleep 30. in
      (* 30s *)
      Lwt.return (Error "server rage - timeout 30s")
    in
    let%lwt server_rage_result =
      try%lwt Lwt.pick [server_promise; timeout_promise]
      with e ->
        let message = Exn.to_string e in
        let stack = Printexc.get_backtrace () in
        Lwt.return
          (Error (Printf.sprintf "server rage - %s\n%s" message stack))
    in
    (* Don't start waiting on these until the end because we want all of our LWT requests to be in
     * flight simultaneously. *)
    let%lwt () = get_log_files in
    let%lwt current_version = current_version in
    add_data ("Version previously read from .hhconfig: " ^ !hhconfig_version);
    add_data ("Version in .hhconfig: " ^ current_version);
    if
      Str.string_match
        (Str.regexp "^\\^[0-9]+\\.[0-9]+\\.[0-9]+")
        current_version
        0
    then
      add_data
        ( "Version source control: hg update remote/releases/hack/v"
        ^ String_utils.lstrip current_version "^" );
    Result.iter_error server_rage_result ~f:add_data;

    (* that's it! *)
    Lwt.return !items)

let do_toggleTypeCoverage
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : ToggleTypeCoverage.params) : unit Lwt.t =
  (* Currently, the only thing to do on toggling type coverage is turn on dynamic view *)
  let command =
    ServerCommandTypes.DYNAMIC_VIEW params.ToggleTypeCoverage.toggle
  in
  cached_toggle_state := params.ToggleTypeCoverage.toggle;
  rpc conn ref_unblocked_time command

let do_didOpen
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : DidOpen.params) : unit Lwt.t =
  DidOpen.(
    TextDocumentItem.(
      let filename = lsp_uri_to_path params.textDocument.uri in
      let text = params.textDocument.text in
      let command = ServerCommandTypes.OPEN_FILE (filename, text) in
      rpc conn ref_unblocked_time command))

let do_didClose
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : DidClose.params) : unit Lwt.t =
  DidClose.(
    TextDocumentIdentifier.(
      let filename = lsp_uri_to_path params.textDocument.uri in
      let command = ServerCommandTypes.CLOSE_FILE filename in
      rpc conn ref_unblocked_time command))

let do_didChange
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : DidChange.params) : unit Lwt.t =
  VersionedTextDocumentIdentifier.(
    Lsp.DidChange.(
      let lsp_change_to_ide (lsp : DidChange.textDocumentContentChangeEvent) :
          Ide_api_types.text_edit =
        {
          Ide_api_types.range = Option.map lsp.range lsp_range_to_ide;
          text = lsp.text;
        }
      in
      let filename = lsp_uri_to_path params.textDocument.uri in
      let changes = List.map params.contentChanges ~f:lsp_change_to_ide in
      let command = ServerCommandTypes.EDIT_FILE (filename, changes) in
      rpc conn ref_unblocked_time command))

let do_hover_common (infos : HoverService.hover_info list) : Hover.result =
  let contents =
    infos
    |> List.map ~f:(fun hoverInfo ->
           (* Hack server uses None to indicate absence of a result. *)
           (* We're also catching the non-result "" just in case...               *)
           match hoverInfo with
           | { HoverService.snippet = ""; _ } -> []
           | { HoverService.snippet; addendum; _ } ->
             MarkedCode ("hack", snippet)
             :: List.map ~f:(fun s -> MarkedString s) addendum)
    |> List.concat
  in
  (* We pull the position from the SymbolOccurrence.t record, so I would be
     surprised if there were any different ones in here. Just take the first
     non-None one. *)
  let range =
    infos
    |> List.filter_map ~f:(fun { HoverService.pos; _ } -> pos)
    |> List.hd
    |> Option.map ~f:hack_pos_to_lsp_range
  in
  if contents = [] then
    None
  else
    Some { Hover.contents; range }

let do_hover
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : Hover.params) : Hover.result Lwt.t =
  let (file, line, column) = lsp_file_position_to_hack params in
  let command = ServerCommandTypes.IDE_HOVER (file, line, column) in
  let%lwt infos = rpc conn ref_unblocked_time command in
  Lwt.return (do_hover_common infos)

let do_hover_local
    (ide_service : ClientIdeService.t)
    (editor_open_files : Lsp.TextDocumentItem.t SMap.t)
    (params : Hover.params) : Hover.result Lwt.t =
  let document_location = get_document_location editor_open_files params in
  let%lwt infos =
    ClientIdeService.rpc ide_service (ClientIdeMessage.Hover document_location)
  in
  match infos with
  | Ok infos ->
    let infos = do_hover_common infos in
    Lwt.return infos
  | Error error_message ->
    failwith (Printf.sprintf "Local hover failed: %s" error_message)

let do_typeDefinition
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : Definition.params) : TypeDefinition.result Lwt.t =
  let (file, line, column) = lsp_file_position_to_hack params in
  let command =
    ServerCommandTypes.(IDENTIFY_TYPES (FileName file, line, column))
  in
  let%lwt results = rpc conn ref_unblocked_time command in
  Lwt.return
    (List.map results ~f:(fun nast_sid ->
         hack_pos_definition_to_lsp_identifier_location
           nast_sid
           ~default_path:file))

let do_typeDefinition_local
    (ide_service : ClientIdeService.t)
    (editor_open_files : Lsp.TextDocumentItem.t SMap.t)
    (params : Definition.params) : TypeDefinition.result Lwt.t =
  let document_location = get_document_location editor_open_files params in
  let%lwt results =
    ClientIdeService.rpc
      ide_service
      (ClientIdeMessage.Type_definition document_location)
  in
  match results with
  | Ok results ->
    let file = Path.to_string document_location.ClientIdeMessage.file_path in
    let results =
      List.map results ~f:(fun nast_sid ->
          hack_pos_definition_to_lsp_identifier_location
            nast_sid
            ~default_path:file)
    in
    Lwt.return results
  | Error error_message ->
    failwith
      (Printf.sprintf "Local go-to-type-definition failed: %s" error_message)

let do_definition
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (editor_open_files : Lsp.TextDocumentItem.t SMap.t)
    (params : Definition.params) : Definition.result Lwt.t =
  let (filename, line, column) = lsp_file_position_to_hack params in
  let uri =
    params.TextDocumentPositionParams.textDocument.TextDocumentIdentifier.uri
  in
  let labelled_file =
    match SMap.get uri editor_open_files with
    | Some document ->
      ServerCommandTypes.(
        LabelledFileContent
          { filename; content = document.TextDocumentItem.text })
    | None -> ServerCommandTypes.(LabelledFileName filename)
  in
  let command =
    ServerCommandTypes.GO_TO_DEFINITION (labelled_file, line, column)
  in
  let%lwt results = rpc conn ref_unblocked_time command in
  Lwt.return
    (List.map results ~f:(fun (_occurrence, definition) ->
         hack_symbol_definition_to_lsp_identifier_location
           definition
           ~default_path:filename))

let do_definition_local
    (ide_service : ClientIdeService.t)
    (editor_open_files : Lsp.TextDocumentItem.t SMap.t)
    (params : Definition.params) : Definition.result Lwt.t =
  let document_location = get_document_location editor_open_files params in
  let%lwt results =
    ClientIdeService.rpc
      ide_service
      (ClientIdeMessage.Definition document_location)
  in
  match results with
  | Ok results ->
    let results =
      List.map results ~f:(fun (_occurrence, definition) ->
          hack_symbol_definition_to_lsp_identifier_location
            definition
            ~default_path:
              (document_location.ClientIdeMessage.file_path |> Path.to_string))
    in
    Lwt.return results
  | Error error_message ->
    failwith (Printf.sprintf "Local go-to-definition failed: %s" error_message)

let make_ide_completion_response
    (result : AutocompleteTypes.ide_result) (filename : string) :
    Completion.completionList Lwt.t =
  AutocompleteTypes.(
    Completion.(
      (* We use snippets to provide parentheses+arguments when autocompleting     *)
      (* method calls e.g. "$c->|" ==> "$c->foo($arg1)". But we'll only do this   *)
      (* there's nothing after the caret: no "$c->|(1)" -> "$c->foo($arg1)(1)"    *)
      let is_caret_followed_by_lparen = result.char_at_pos = '(' in
      let p = initialize_params_exc () in
      let hack_to_itemType (completion : complete_autocomplete_result) :
          string option =
        (* TODO: we're using itemType (left column) for function return types, and *)
        (* the inlineDetail (right column) for variable/field types. Is that good? *)
        Option.map completion.func_details ~f:(fun details ->
            details.return_ty)
      in
      let hack_to_detail (completion : complete_autocomplete_result) : string =
        (* TODO: retrieve the actual signature including name+modifiers     *)
        (* For now we just return the type of the completion. In the case   *)
        (* of functions, their function-types have parentheses around them  *)
        (* which we want to strip. In other cases like tuples, no strip.    *)
        match completion.func_details with
        | None -> completion.res_ty
        | Some _ ->
          String_utils.rstrip (String_utils.lstrip completion.res_ty "(") ")"
      in
      let hack_to_inline_detail (completion : complete_autocomplete_result) :
          string =
        match completion.func_details with
        | None -> hack_to_detail completion
        | Some details ->
          (* "(type1 $param1, ...)" *)
          let f param =
            Printf.sprintf "%s %s" param.param_ty param.param_name
          in
          let params = String.concat ~sep:", " (List.map details.params ~f) in
          Printf.sprintf "(%s)" params
        (* Returns a tuple of (insertText, insertTextFormat, textEdits). *)
      in
      let hack_to_insert (completion : complete_autocomplete_result) :
          [ `InsertText of string | `TextEdit of TextEdit.t list ]
          * Completion.insertTextFormat =
        let use_textedits =
          Initialize.(p.initializationOptions.useTextEditAutocomplete)
        in
        match (completion.func_details, use_textedits) with
        | (Some details, _)
          when Lsp_helpers.supports_snippets p
               && (not is_caret_followed_by_lparen)
               && completion.res_kind <> SearchUtils.SI_LocalVariable ->
          (* "method(${1:arg1}, ...)" but for args we just use param names. *)
          let f i param = Printf.sprintf "${%i:%s}" (i + 1) param.param_name in
          let params = String.concat ~sep:", " (List.mapi details.params ~f) in
          ( `InsertText (Printf.sprintf "%s(%s)" completion.res_name params),
            SnippetFormat )
        | (_, false) -> (`InsertText completion.res_name, PlainText)
        | (_, true) ->
          ( `TextEdit
              [
                TextEdit.
                  {
                    range = ide_range_to_lsp completion.res_replace_pos;
                    newText = completion.res_name;
                  };
              ],
            PlainText )
      in
      let hack_completion_to_lsp (completion : complete_autocomplete_result) :
          Completion.completionItem =
        let (insertText, insertTextFormat, textEdits) =
          match hack_to_insert completion with
          | (`InsertText text, format) -> (Some text, format, [])
          | (`TextEdit edits, format) -> (None, format, edits)
        in
        let pos =
          if Pos.filename completion.res_pos = "" then
            Pos.set_file filename completion.res_pos
          else
            completion.res_pos
        in
        let data =
          let (line, start, _) = Pos.info_pos pos in
          let filename = Pos.filename pos in
          let base_class =
            match completion.res_base_class with
            | Some base_class -> Hh_json.JSON_String base_class
            | None -> Hh_json.JSON_Null
          in
          Some
            (Hh_json.JSON_Object
               [
                 (* Fullname is needed for namespaces.  We often trim namespaces to make
                  * the results more readable, such as showing "ad__breaks" instead of
                  * "Thrift\Packages\cf\ad__breaks".
                  *)
                   ("fullname", Hh_json.JSON_String completion.res_fullname);
                 (* Filename/line/char/base_class are used to handle class methods.
                  * We could unify this with fullname in the future.
                  *)
                   ("filename", Hh_json.JSON_String filename);
                 ("line", Hh_json.int_ line);
                 ("char", Hh_json.int_ start);
                 ("base_class", base_class);
               ])
        in
        {
          label =
            ( completion.res_name
            ^
            if completion.res_kind = SearchUtils.SI_Namespace then
              "\\"
            else
              "" );
          kind =
            si_kind_to_completion_kind completion.AutocompleteTypes.res_kind;
          detail = Some (hack_to_detail completion);
          inlineDetail = Some (hack_to_inline_detail completion);
          itemType = hack_to_itemType completion;
          documentation = None;
          (* This will be filled in by completionItem/resolve. *)
          sortText = None;
          filterText = None;
          insertText;
          insertTextFormat = Some insertTextFormat;
          textEdits;
          command = None;
          data;
        }
      in
      Lwt.return
        {
          isIncomplete = not result.is_complete;
          items = List.map result.completions ~f:hack_completion_to_lsp;
        }))

let do_completion_ffp
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : Completion.params) : Completion.result Lwt.t =
  Completion.(
    TextDocumentIdentifier.(
      let pos =
        lsp_position_to_ide params.loc.TextDocumentPositionParams.position
      in
      let filename =
        lsp_uri_to_path params.loc.TextDocumentPositionParams.textDocument.uri
      in
      let command = ServerCommandTypes.IDE_FFP_AUTOCOMPLETE (filename, pos) in
      let%lwt result = rpc conn ref_unblocked_time command in
      make_ide_completion_response result filename))

let do_completion_legacy
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : Completion.params) : Completion.result Lwt.t =
  Completion.(
    TextDocumentIdentifier.(
      let pos =
        lsp_position_to_ide params.loc.TextDocumentPositionParams.position
      in
      let filename =
        lsp_uri_to_path params.loc.TextDocumentPositionParams.textDocument.uri
      in
      let is_manually_invoked =
        match params.context with
        | None -> false
        | Some c -> c.triggerKind = Invoked
      in
      let command =
        ServerCommandTypes.IDE_AUTOCOMPLETE (filename, pos, is_manually_invoked)
      in
      let%lwt result = rpc conn ref_unblocked_time command in
      make_ide_completion_response result filename))

let do_completion_local
    (ide_service : ClientIdeService.t)
    (editor_open_files : Lsp.TextDocumentItem.t SMap.t)
    (params : Completion.params) : Completion.result Lwt.t =
  Completion.(
    let document_location =
      get_document_location editor_open_files params.loc
    in
    (* Other parameters *)
    let is_manually_invoked =
      match params.context with
      | None -> false
      | Some c -> c.triggerKind = Invoked
    in
    (* this is what I want to fix *)
    let request =
      ClientIdeMessage.Completion
        { ClientIdeMessage.Completion.document_location; is_manually_invoked }
    in
    let%lwt result = ClientIdeService.rpc ide_service request in
    match result with
    | Ok infos ->
      let filename =
        document_location.ClientIdeMessage.file_path |> Path.to_string
      in
      let%lwt response = make_ide_completion_response infos filename in
      Lwt.return response
    | Error error_message ->
      failwith (Printf.sprintf "Local completion failed: %s" error_message))

exception NoLocationFound

let docblock_to_markdown (raw_docblock : DocblockService.result) :
    markedString list option =
  match raw_docblock with
  | [] -> None
  | docblock ->
    Some
      (Core_kernel.List.fold docblock ~init:[] ~f:(fun acc elt ->
           match elt with
           | DocblockService.Markdown txt -> MarkedString txt :: acc
           | DocblockService.HackSnippet txt -> MarkedCode ("hack", txt) :: acc
           | DocblockService.XhpSnippet txt -> MarkedCode ("html", txt) :: acc))

let do_completionItemResolve
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : CompletionItemResolve.params) :
    CompletionItemResolve.result Lwt.t =
  (* No matter what, we need the kind *)
  let raw_kind = params.Completion.kind in
  let kind = completion_kind_to_si_kind raw_kind in
  (* First try fetching position data from json *)
  let%lwt raw_docblock =
    try
      match params.Completion.data with
      | None -> raise NoLocationFound
      | Some _ as data ->
        (* Some docblocks are for class methods.  Class methods need to know
         * file/line/column/base_class to find the docblock. *)
        let filename = Jget.string_exn data "filename" in
        let line = Jget.int_exn data "line" in
        let column = Jget.int_exn data "char" in
        let base_class = Jget.string_opt data "base_class" in
        (* If not found ... *)
        if line = 0 && column = 0 then (
          (* For global symbols such as functions, classes, enums, etc, we
           * need to know the full name INCLUDING all namespaces.  Once
           * we know that, we can look up its file/line/column. *)
          let fullname = Jget.string_exn data "fullname" in
          if fullname = "" then raise NoLocationFound;
          let fullname = Utils.add_ns fullname in
          let command =
            ServerCommandTypes.DOCBLOCK_FOR_SYMBOL (fullname, kind)
          in
          let%lwt raw_docblock = rpc conn ref_unblocked_time command in
          Lwt.return raw_docblock
        ) else
          (* Okay let's get a docblock for this specific location *)
          let command =
            ServerCommandTypes.DOCBLOCK_AT
              (filename, line, column, base_class, kind)
          in
          let%lwt raw_docblock = rpc conn ref_unblocked_time command in
          Lwt.return raw_docblock
      (* If that failed, fetch docblock using just the symbol name *)
    with _ ->
      let symbolname = params.Completion.label in
      let command =
        ServerCommandTypes.DOCBLOCK_FOR_SYMBOL (symbolname, kind)
      in
      let%lwt raw_docblock = rpc conn ref_unblocked_time command in
      Lwt.return raw_docblock
  in
  (* Convert to markdown and return *)
  let documentation = docblock_to_markdown raw_docblock in
  Lwt.return { params with Completion.documentation }

(*
 * Note that resolve does not depend on having previously executed completion in
 * the same process.  The LSP resolve request takes, as input, a single item
 * produced by any previously executed completion request.  So it's okay for
 * one process to respond to another, because they'll both know the answers
 * to the same symbol requests.
 *
 * And it's totally okay to mix and match requests to serverless IDE and
 * hh_server.
 *)
let do_resolve_local
    (ide_service : ClientIdeService.t)
    (editor_open_files : Lsp.TextDocumentItem.t SMap.t)
    (params : CompletionItemResolve.params) :
    CompletionItemResolve.result Lwt.t =
  let raw_kind = params.Completion.kind in
  let kind = completion_kind_to_si_kind raw_kind in
  (* Some docblocks are for class methods.  Class methods need to know
  * file/line/column/base_class to find the docblock. *)
  let%lwt result =
    try
      match params.Completion.data with
      | None -> raise NoLocationFound
      | Some _ as data ->
        let filename = Jget.string_exn data "filename" in
        let uri = "file://" ^ filename in
        let file_path = Path.make filename in
        let line = Jget.int_exn data "line" in
        let column = Jget.int_exn data "char" in
        let file_contents = get_document_contents editor_open_files uri in
        if line = 0 && column = 0 then failwith "NoFileLineColumnData";
        let request =
          ClientIdeMessage.Completion_resolve_location
            {
              ClientIdeMessage.Completion_resolve_location.document_location =
                {
                  ClientIdeMessage.file_path;
                  ClientIdeMessage.file_contents;
                  ClientIdeMessage.line;
                  ClientIdeMessage.column;
                };
              kind;
            }
        in
        let%lwt location_result = ClientIdeService.rpc ide_service request in
        (match location_result with
        | Ok raw_docblock ->
          let documentation = docblock_to_markdown raw_docblock in
          Lwt.return { params with Completion.documentation }
        | Error error_message ->
          failwith (Printf.sprintf "Local resolve failed: %s" error_message))
      (* If that fails, next try using symbol *)
    with _ ->
      let symbolname = params.Completion.label in
      let request =
        ClientIdeMessage.Completion_resolve
          { ClientIdeMessage.Completion_resolve.symbol = symbolname; kind }
      in
      let%lwt resolve_result = ClientIdeService.rpc ide_service request in
      (match resolve_result with
      | Ok raw_docblock ->
        let documentation = docblock_to_markdown raw_docblock in
        Lwt.return { params with Completion.documentation }
      | Error error_message ->
        failwith (Printf.sprintf "Local resolve failed: %s" error_message))
  in
  Lwt.return result

let do_workspaceSymbol
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : WorkspaceSymbol.params) : WorkspaceSymbol.result Lwt.t =
  WorkspaceSymbol.(
    SearchUtils.(
      let query = params.query in
      let query_type = "" in
      let command = ServerCommandTypes.SEARCH (query, query_type) in
      let%lwt results = rpc conn ref_unblocked_time command in
      let hack_to_lsp_kind = function
        | SearchUtils.SI_Class -> SymbolInformation.Class
        | SearchUtils.SI_Interface -> SymbolInformation.Interface
        | SearchUtils.SI_Trait -> SymbolInformation.Interface
        (* LSP doesn't have traits, so we approximate with interface *)
        | SearchUtils.SI_Enum -> SymbolInformation.Enum
        (* TODO(T36697624): Add SymbolInformation.Record *)
        | SearchUtils.SI_ClassMethod -> SymbolInformation.Method
        | SearchUtils.SI_Function -> SymbolInformation.Function
        | SearchUtils.SI_Typedef -> SymbolInformation.Class
        (* LSP doesn't have typedef, so we approximate with class *)
        | SearchUtils.SI_GlobalConstant -> SymbolInformation.Constant
        | SearchUtils.SI_Namespace -> SymbolInformation.Namespace
        | SearchUtils.SI_Mixed -> SymbolInformation.Variable
        | SearchUtils.SI_XHP -> SymbolInformation.Class
        | SearchUtils.SI_Literal -> SymbolInformation.Variable
        | SearchUtils.SI_ClassConstant -> SymbolInformation.Constant
        | SearchUtils.SI_Property -> SymbolInformation.Property
        | SearchUtils.SI_LocalVariable -> SymbolInformation.Variable
        | SearchUtils.SI_Constructor -> SymbolInformation.Constructor
        (* Do these happen in practice? *)
        | SearchUtils.SI_Keyword
        | SearchUtils.SI_Unknown ->
          failwith "Unknown symbol kind"
      in
      (* Hack sometimes gives us back items with an empty path, by which it       *)
      (* intends "whichever path you asked me about". That would be meaningless   *)
      (* here. If it does, then it'll pick up our default path (also empty),      *)
      (* which will throw and go into our telemetry. That's the best we can do.   *)
      let hack_symbol_to_lsp (symbol : SearchUtils.symbol) =
        {
          SymbolInformation.name = Utils.strip_ns symbol.name;
          kind = hack_to_lsp_kind symbol.result_type;
          location = hack_pos_to_lsp_location symbol.pos ~default_path:"";
          containerName = None;
        }
      in
      Lwt.return (List.map results ~f:hack_symbol_to_lsp)))

let rec hack_symbol_tree_to_lsp
    ~(filename : string)
    ~(accu : Lsp.SymbolInformation.t list)
    ~(container_name : string option)
    (defs : FileOutline.outline) : Lsp.SymbolInformation.t list =
  SymbolDefinition.(
    let hack_to_lsp_kind = function
      | SymbolDefinition.Function -> SymbolInformation.Function
      | SymbolDefinition.Class -> SymbolInformation.Class
      | SymbolDefinition.Method -> SymbolInformation.Method
      | SymbolDefinition.Property -> SymbolInformation.Property
      | SymbolDefinition.Const -> SymbolInformation.Constant
      | SymbolDefinition.Enum -> SymbolInformation.Enum
      | SymbolDefinition.Interface -> SymbolInformation.Interface
      | SymbolDefinition.Trait -> SymbolInformation.Interface
      (* LSP doesn't have traits, so we approximate with interface *)
      | SymbolDefinition.LocalVar -> SymbolInformation.Variable
      | SymbolDefinition.Typeconst -> SymbolInformation.Class
      (* e.g. "const type Ta = string;" -- absent from LSP *)
      | SymbolDefinition.Typedef -> SymbolInformation.Class
      (* e.g. top level type alias -- absent from LSP *)
      | SymbolDefinition.Param -> SymbolInformation.Variable
      (* We never return a param from a document-symbol-search *)
    in
    let hack_symbol_to_lsp definition containerName =
      {
        SymbolInformation.name = definition.name;
        kind = hack_to_lsp_kind definition.kind;
        location =
          hack_symbol_definition_to_lsp_construct_location
            definition
            ~default_path:filename;
        containerName;
      }
    in
    match defs with
    (* Flattens the recursive list of symbols *)
    | [] -> List.rev accu
    | def :: defs ->
      let children = Option.value def.children ~default:[] in
      let accu = hack_symbol_to_lsp def container_name :: accu in
      let accu =
        hack_symbol_tree_to_lsp
          ~filename
          ~accu
          ~container_name:(Some def.name)
          children
      in
      hack_symbol_tree_to_lsp ~filename ~accu ~container_name defs)

let do_documentSymbol
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : DocumentSymbol.params) : DocumentSymbol.result Lwt.t =
  DocumentSymbol.(
    TextDocumentIdentifier.(
      let filename = lsp_uri_to_path params.textDocument.uri in
      let command = ServerCommandTypes.OUTLINE filename in
      let%lwt outline = rpc conn ref_unblocked_time command in
      let converted =
        hack_symbol_tree_to_lsp ~filename ~accu:[] ~container_name:None outline
      in
      Lwt.return converted))

(* for serverless ide *)
let do_documentSymbol_local
    (ide_service : ClientIdeService.t)
    (editor_open_files : Lsp.TextDocumentItem.t SMap.t)
    (params : DocumentSymbol.params) : DocumentSymbol.result Lwt.t =
  DocumentSymbol.(
    TextDocumentIdentifier.(
      let filename = lsp_uri_to_path params.textDocument.uri in
      let file_contents =
        get_document_contents editor_open_files params.textDocument.uri
      in
      let request =
        ClientIdeMessage.Document_symbol
          { ClientIdeMessage.Document_symbol.file_contents }
      in
      let%lwt results = ClientIdeService.rpc ide_service request in
      match results with
      | Ok outline ->
        let converted =
          hack_symbol_tree_to_lsp
            ~filename
            ~accu:[]
            ~container_name:None
            outline
        in
        Lwt.return converted
      | Error error_message ->
        failwith
          (Printf.sprintf "Local document-symbol failed: %s" error_message)))

let do_findReferences
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : FindReferences.params) : FindReferences.result Lwt.t =
  FindReferences.(
    TextDocumentPositionParams.(
      let { Ide_api_types.line; column } =
        lsp_position_to_ide params.loc.position
      in
      let filename =
        Lsp_helpers.lsp_textDocumentIdentifier_to_filename
          params.loc.textDocument
      in
      let include_defs = params.context.includeDeclaration in
      let labelled_file = ServerCommandTypes.LabelledFileName filename in
      let command =
        ServerCommandTypes.IDE_FIND_REFS
          (labelled_file, line, column, include_defs)
      in
      let%lwt results = rpc_with_retry conn ref_unblocked_time command in
      (* TODO: respect params.context.include_declaration *)
      match results with
      | None -> Lwt.return []
      | Some (_name, positions) ->
        Lwt.return
          (List.map
             positions
             ~f:(hack_pos_to_lsp_location ~default_path:filename))))

(* Shared function for hack range conversion *)
let hack_range_to_lsp_highlight range =
  { DocumentHighlight.range = ide_range_to_lsp range; kind = None }

let do_documentHighlight
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : DocumentHighlight.params) : DocumentHighlight.result Lwt.t =
  let (file, line, column) = lsp_file_position_to_hack params in
  let command =
    ServerCommandTypes.(IDE_HIGHLIGHT_REFS (FileName file, line, column))
  in
  let%lwt results = rpc conn ref_unblocked_time command in
  Lwt.return (List.map results ~f:hack_range_to_lsp_highlight)

(* Serverless IDE implementation of highlight *)
let do_highlight_local
    (ide_service : ClientIdeService.t)
    (editor_open_files : Lsp.TextDocumentItem.t SMap.t)
    (params : DocumentHighlight.params) : DocumentHighlight.result Lwt.t =
  let document_location = get_document_location editor_open_files params in
  let%lwt result =
    ClientIdeService.rpc
      ide_service
      (ClientIdeMessage.Document_highlight document_location)
  in
  match result with
  | Ok ranges -> Lwt.return (List.map ranges ~f:hack_range_to_lsp_highlight)
  | Error error_message ->
    failwith (Printf.sprintf "Local highlight failed: %s" error_message)

let format_typeCoverage_result results counts =
  TypeCoverage.(
    let coveredPercent = Coverage_level.get_percent counts in
    let hack_coverage_to_lsp (pos, level) =
      let range = hack_pos_to_lsp_range pos in
      match level with
      (* We only show diagnostics for completely untypechecked code. *)
      | Ide_api_types.Checked
      | Ide_api_types.Partial ->
        None
      | Ide_api_types.Unchecked -> Some { range; message = None }
    in
    {
      coveredPercent;
      uncoveredRanges = List.filter_map results ~f:hack_coverage_to_lsp;
      defaultMessage =
        "Un-type checked code. Consider adding type annotations.";
    })

let do_typeCoverage
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : TypeCoverage.params) : TypeCoverage.result Lwt.t =
  TypeCoverage.(
    let filename =
      Lsp_helpers.lsp_textDocumentIdentifier_to_filename params.textDocument
    in
    let command =
      ServerCommandTypes.COVERAGE_LEVELS (ServerCommandTypes.FileName filename)
    in
    let%lwt (results, counts) : Coverage_level_defs.result =
      rpc conn ref_unblocked_time command
    in
    let formatted = format_typeCoverage_result results counts in
    Lwt.return formatted)

let do_typeCoverage_local
    (ide_service : ClientIdeService.t)
    (editor_open_files : Lsp.TextDocumentItem.t SMap.t)
    (params : TypeCoverage.params) : TypeCoverage.result Lwt.t =
  TypeCoverage.(
    let document_contents =
      get_document_contents
        editor_open_files
        params.textDocument.TextDocumentIdentifier.uri
    in
    match document_contents with
    | None -> failwith "Local type coverage failed, file could not be found."
    | Some file_contents ->
      let file_path =
        params.textDocument.TextDocumentIdentifier.uri
        |> lsp_uri_to_path
        |> Path.make
      in
      let request =
        ClientIdeMessage.Type_coverage
          { ClientIdeMessage.file_path; ClientIdeMessage.file_contents }
      in
      let%lwt result = ClientIdeService.rpc ide_service request in
      (match result with
      | Ok (results, counts) ->
        let formatted = format_typeCoverage_result results counts in
        Lwt.return formatted
      | Error error_message ->
        failwith
          (Printf.sprintf "Local type coverage failed: %s" error_message)))

let do_formatting_common
    (editor_open_files : Lsp.TextDocumentItem.t SMap.t)
    (action : ServerFormatTypes.ide_action)
    (options : DocumentFormatting.formattingOptions) : TextEdit.t list =
  ServerFormatTypes.(
    let response : ServerFormatTypes.ide_result =
      ServerFormat.go_ide editor_open_files action options
    in
    match response with
    | Error "File failed to parse without errors" ->
      (* If LSP issues a formatting request at a given line+char, but we can't *)
      (* calculate a better format for the file due to syntax errors in it,    *)
      (* then we should return "success and there are no edits to apply"       *)
      (* rather than "error".                                                  *)
      (* TODO: let's eliminate hh_format, and incorporate hackfmt into the     *)
      (* hh_client binary itself, and make make "hackfmt" just a wrapper for   *)
      (* "hh_client format", and then make it return proper error that we can  *)
      (* pattern-match upon, rather than hard-coding the string...             *)
      []
    | Error message -> raise (Error.InternalError message)
    | Ok r ->
      let range = ide_range_to_lsp r.range in
      let newText = r.new_text in
      [{ TextEdit.range; newText }])

let do_documentRangeFormatting
    (editor_open_files : Lsp.TextDocumentItem.t SMap.t)
    (params : DocumentRangeFormatting.params) : DocumentRangeFormatting.result
    =
  DocumentRangeFormatting.(
    TextDocumentIdentifier.(
      let action =
        ServerFormatTypes.Range
          {
            Ide_api_types.range_filename =
              lsp_uri_to_path params.textDocument.uri;
            file_range = lsp_range_to_ide params.range;
          }
      in
      do_formatting_common editor_open_files action params.options))

let do_signatureHelp
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : SignatureHelp.params) : SignatureHelp.result Lwt.t =
  let (file, line, column) = lsp_file_position_to_hack params in
  let command =
    ServerCommandTypes.IDE_SIGNATURE_HELP
      (ServerCommandTypes.FileName file, line, column)
  in
  rpc conn ref_unblocked_time command

(* Serverless IDE version of signature help *)
let do_signatureHelp_local
    (ide_service : ClientIdeService.t)
    (editor_open_files : Lsp.TextDocumentItem.t SMap.t)
    (params : SignatureHelp.params) : SignatureHelp.result Lwt.t =
  let document_location = get_document_location editor_open_files params in
  let%lwt result =
    ClientIdeService.rpc
      ide_service
      (ClientIdeMessage.Signature_help document_location)
  in
  match result with
  | Ok signatures -> Lwt.return signatures
  | Error error_message ->
    failwith (Printf.sprintf "Local highlight failed: %s" error_message)

let patch_to_workspace_edit_change (patch : ServerRefactorTypes.patch) :
    string * TextEdit.t =
  ServerRefactorTypes.(
    Pos.(
      let text_edit =
        match patch with
        | Insert insert_patch
        | Replace insert_patch ->
          {
            TextEdit.range = hack_pos_to_lsp_range insert_patch.pos;
            newText = insert_patch.text;
          }
        | Remove pos ->
          { TextEdit.range = hack_pos_to_lsp_range pos; newText = "" }
      in
      let uri =
        match patch with
        | Insert insert_patch
        | Replace insert_patch ->
          File_url.create (filename insert_patch.pos)
        | Remove pos -> File_url.create (filename pos)
      in
      (uri, text_edit)))

let patches_to_workspace_edit (patches : ServerRefactorTypes.patch list) :
    WorkspaceEdit.t =
  let changes = List.map patches ~f:patch_to_workspace_edit_change in
  let changes =
    List.fold changes ~init:SMap.empty ~f:(fun acc (uri, text_edit) ->
        let current_edits = Option.value ~default:[] (SMap.get uri acc) in
        let new_edits = text_edit :: current_edits in
        SMap.add uri new_edits acc)
  in
  { WorkspaceEdit.changes }

let do_documentRename
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : Rename.params) : WorkspaceEdit.t Lwt.t =
  let (filename, line, char) =
    lsp_file_position_to_hack (rename_params_to_document_position params)
  in
  Rename.(
    let new_name = params.newName in
    let command =
      ServerCommandTypes.IDE_REFACTOR
        { ServerCommandTypes.Ide_refactor_type.filename; line; char; new_name }
    in
    let%lwt patches = rpc_with_retry conn ref_unblocked_time command in
    let patches =
      match patches with
      | Ok patches -> patches
      | Error message -> raise (Error.InvalidRequest message)
    in
    Lwt.return (patches_to_workspace_edit patches))

let do_documentOnTypeFormatting
    (editor_open_files : Lsp.TextDocumentItem.t SMap.t)
    (params : DocumentOnTypeFormatting.params) :
    DocumentOnTypeFormatting.result =
  DocumentOnTypeFormatting.(
    TextDocumentIdentifier.(
      (*
    In LSP, positions do not point directly to characters, but to spaces in between characters.
    Thus, the LSP position that the cursor points to after typing a character is the space
    immediately after the character.

    For example:
          Character positions:      0 1 2 3 4 5 6
                                    f o o ( ) { }
          LSP positions:           0 1 2 3 4 5 6 7

          The cursor is at LSP position 7 after typing the "}" of "foo(){}"
          But the character position of "}" is 6.

    Nuclide currently sends positions according to LSP, but everything else in the server
    and in hack formatting assumes that positions point directly to characters.

    Thus, to send the position of the character itself for formatting,
      we must subtract one.
  *)
      let fixup_position position =
        { position with character = position.character - 1 }
      in
      let action =
        ServerFormatTypes.Position
          {
            Ide_api_types.filename = lsp_uri_to_path params.textDocument.uri;
            position = lsp_position_to_ide (fixup_position params.position);
          }
      in
      do_formatting_common editor_open_files action params.options))

let do_documentFormatting
    (editor_open_files : Lsp.TextDocumentItem.t SMap.t)
    (params : DocumentFormatting.params) : DocumentFormatting.result =
  DocumentFormatting.(
    TextDocumentIdentifier.(
      let action =
        ServerFormatTypes.Document (lsp_uri_to_path params.textDocument.uri)
      in
      do_formatting_common editor_open_files action params.options))

let do_server_busy (state : state) (status : ServerCommandTypes.busy_status) :
    state =
  Main_env.(
    ServerCommandTypes.(
      let (type_, shortMessage, message) =
        match status with
        | Needs_local_typecheck ->
          (MessageType.InfoMessage, None, "Hack: preparing to check edits")
        | Doing_local_typecheck ->
          (MessageType.InfoMessage, None, "Hack: checking edits")
        | Done_local_typecheck ->
          ( MessageType.InfoMessage,
            None,
            "hh_server is initialized and running correctly." )
        | Doing_global_typecheck Blocking ->
          ( MessageType.WarningMessage,
            Some "checking project",
            "Hack: checking entire project (blocking)" )
        | Doing_global_typecheck Interruptible ->
          ( MessageType.InfoMessage,
            None,
            "Hack: checking entire project (interruptible)" )
        | Doing_global_typecheck (Remote_blocking message) ->
          ( MessageType.WarningMessage,
            Some (Printf.sprintf "checking project: %s" message),
            Printf.sprintf
              "Hack: checking entire project (remote, blocking) - %s"
              message )
        | Done_global_typecheck _ ->
          ( MessageType.InfoMessage,
            None,
            "hh_server is initialized and running correctly." )
      in
      match state with
      | Main_loop menv ->
        Main_loop { menv with status = { type_; shortMessage; message } }
      | _ -> state))

(* do_diagnostics: sends notifications for all reported diagnostics; also     *)
(* returns an updated "files_with_diagnostics" set of all files for which     *)
(* our client currently has non-empty diagnostic reports.                     *)
let do_diagnostics
    (uris_with_diagnostics : SSet.t)
    (file_reports : Pos.absolute Errors.error_ list SMap.t) : SSet.t =
  (* Hack sometimes reports a diagnostic on an empty file when it can't       *)
  (* figure out which file to report. In this case we'll report on the root.  *)
  (* Nuclide and VSCode both display this fine, though they obviously don't   *)
  (* let you click-to-go-to-file on it.                                       *)
  let default_path =
    match get_root_opt () with
    | None -> failwith "expected root"
    | Some root -> Path.to_string root
  in
  let file_reports =
    match SMap.get "" file_reports with
    | None -> file_reports
    | Some errors ->
      SMap.remove "" file_reports
      |> SMap.add ~combine:( @ ) default_path errors
  in
  let per_file file errors =
    hack_errors_to_lsp_diagnostic file errors
    |> print_diagnostics
    |> notify_jsonrpc ~powered_by:Hh_server "textDocument/publishDiagnostics"
  in
  SMap.iter per_file file_reports;

  let is_error_free _uri errors = List.is_empty errors in
  (* reports_without/reports_with are maps of filename->ErrorList. *)
  let (reports_without, reports_with) =
    SMap.partition is_error_free file_reports
  in
  (* files_without/files_with are sets of filenames *)
  let files_without = SMap.bindings reports_without |> List.map ~f:fst in
  let files_with = SMap.bindings reports_with |> List.map ~f:fst in
  (* uris_without/uris_with are sets of uris *)
  let uris_without =
    List.map files_without ~f:(path_to_lsp_uri ~default_path) |> SSet.of_list
  in
  let uris_with =
    List.map files_with ~f:(path_to_lsp_uri ~default_path) |> SSet.of_list
  in
  (* this is "(uris_with_diagnostics \ uris_without) U uris_with" *)
  SSet.union (SSet.diff uris_with_diagnostics uris_without) uris_with

let report_connect_progress (ienv : In_init_env.t) : unit =
  In_init_env.(
    ShowStatus.(
      ShowMessageRequest.(
        let time = Unix.time () in
        let delay_in_secs = int_of_float (time -. ienv.first_start_time) in
        (* TODO: better to report time that hh_server has spent initializing *)
        let root =
          match get_root_opt () with
          | Some root -> root
          | None -> failwith "we should have root by now"
        in
        let (progress, warning) =
          match ServerUtils.server_progress ~timeout:3 root with
          | Error _ -> (None, None)
          | Ok (progress, warning) -> (progress, warning)
        in
        let progress =
          Option.value progress ~default:ClientConnect.default_progress_message
        in
        let message =
          if Option.is_some warning then
            Printf.sprintf
              "hh_server initializing (load-state not found - will take a while): %s [%i seconds]"
              progress
              delay_in_secs
          else
            Printf.sprintf
              "hh_server initializing: %s [%i seconds]"
              progress
              delay_in_secs
        in
        request_showStatus
          {
            request =
              { type_ = MessageType.WarningMessage; message; actions = [] };
            progress = None;
            total = None;
            shortMessage =
              Some (Printf.sprintf "[%s] %s" progress (status_tick ()));
          })))

let report_connect_end (ienv : In_init_env.t) : state =
  Hh_logger.log "report_connect_end";
  In_init_env.(
    Main_env.(
      let _state = dismiss_ui (In_init ienv) in
      let menv =
        {
          Main_env.conn = ienv.In_init_env.conn;
          needs_idle = true;
          editor_open_files = ienv.editor_open_files;
          uris_with_diagnostics = SSet.empty;
          uris_with_unsaved_changes =
            ienv.In_init_env.uris_with_unsaved_changes;
          status =
            {
              type_ = MessageType.InfoMessage;
              message = "hh_server is initialized and running correctly.";
              shortMessage = None;
            };
        }
      in
      Main_loop menv))

(* After the server has sent 'hello', it means the persistent connection is   *)
(* ready, so we can send our backlog of file-edits to the server.             *)
let connect_after_hello
    (server_conn : server_conn) (file_edits : Hh_json.json ImmQueue.t) :
    unit Lwt.t =
  Hh_logger.log "connect_after_hello";
  let ignore = ref 0.0 in
  let%lwt () =
    try%lwt
      (* tell server we want persistent connection *)
      let oc = server_conn.oc in
      ServerCommandLwt.send_connection_type oc ServerCommandTypes.Persistent;
      let fd =
        oc |> Unix.descr_of_out_channel |> Lwt_unix.of_unix_file_descr
      in
      let%lwt (response : 'a ServerCommandTypes.message_type) =
        Marshal_tools_lwt.from_fd_with_preamble fd
      in
      begin
        match response with
        | ServerCommandTypes.Response (ServerCommandTypes.Connected, _) ->
          set_hh_server_state Hh_server_handling_or_ready
        | _ -> failwith "Didn't get server Connected response"
      end;

      (* tell server we want diagnostics *)
      Hh_logger.log "Diag_subscribe: clientLsp subscribing diagnostic 0";
      let%lwt () =
        rpc server_conn ignore (ServerCommandTypes.SUBSCRIBE_DIAGNOSTIC 0)
      in
      (* send open files and unsaved buffers to server *)
      let handle_file_edit (json : Hh_json.json) =
        Jsonrpc.(
          let c = Jsonrpc.parse_message ~json ~timestamp:0.0 in
          let%lwt () =
            match c.method_ with
            | "textDocument/didOpen" ->
              let%lwt () =
                parse_didOpen c.params |> do_didOpen server_conn ignore
              in
              Lwt.return_unit
            | "textDocument/didChange" ->
              let%lwt () =
                parse_didChange c.params |> do_didChange server_conn ignore
              in
              Lwt.return_unit
            | "textDocument/didClose" ->
              let%lwt () =
                parse_didClose c.params |> do_didClose server_conn ignore
              in
              Lwt.return_unit
            | _ -> failwith "should only buffer up didOpen/didChange/didClose"
          in
          Lwt.return_unit)
      in
      let%lwt () =
        file_edits
        |> ImmQueue.to_list
        (* Note: do serially since these involve RPC calls. *)
        |> Lwt_list.iter_s handle_file_edit
      in
      Lwt.return_unit
    with e ->
      let message = Exn.to_string e in
      let stack = Printexc.get_backtrace () in
      Hh_logger.log "connect_after_hello exception %s\n%s" message stack;
      raise
        (Server_fatal_connection_exception { Marshal_tools.message; stack })
  in
  Lwt.return_unit

let rec connect_client (root : Path.t) ~(autostart : bool) : server_conn Lwt.t
    =
  Hh_logger.log "connect_client";
  Exit_status.(
    (* This basically does the same connection attempt as "hh_client check":  *)
    (* it makes repeated attempts to connect; it prints useful messages to    *)
    (* stderr; in case of failure it will raise an exception. Below we're     *)
    (* catching the main exceptions so we can give a good user-facing error   *)
    (* text. For other exceptions, they'll end up showing to the user just    *)
    (* "internal error" with the error code.                                  *)
    let env_connect =
      {
        ClientConnect.root;
        from = !ref_from;
        autostart;
        force_dormant_start = false;
        watchman_debug_logging = false;
        (* If you want this, start the server manually in terminal. *)
        deadline = Some (Unix.time () +. 3.);
        (* limit to 3 seconds *)
        no_load = false;
        (* only relevant when autostart=true *)
        log_inference_constraints = false;
        (* irrelevant *)
        profile_log = false;
        (* irrelevant *)
        remote = false;
        (* irrelevant *)
        ai_mode = None;
        (* only relevant when autostart=true *)
        progress_callback = ClientConnect.null_progress_reporter;
        (* we're fast! *)
        do_post_handoff_handshake = false;
        ignore_hh_version = false;
        saved_state_ignore_hhconfig = false;
        use_priority_pipe = true;
        prechecked = None;
        config = [];
        allow_non_opt_build = false;
      }
    in
    try%lwt
      let%lwt ClientConnect.{ channels = (ic, oc); server_finale_file; _ } =
        ClientConnect.connect env_connect
      in
      can_autostart_after_mismatch := false;
      let pending_messages = Queue.create () in
      Lwt.return { ic; oc; pending_messages; server_finale_file }
    with Exit_with Build_id_mismatch when !can_autostart_after_mismatch ->
      (* Raised when the server was running an old version. We'll retry once.   *)
      Hh_logger.log "connect_client: build_id_mismatch";
      can_autostart_after_mismatch := false;
      connect_client root ~autostart:true)

let do_initialize () : Initialize.result =
  Initialize.
    {
      server_capabilities =
        {
          textDocumentSync =
            {
              want_openClose = true;
              want_change = IncrementalSync;
              want_willSave = false;
              want_willSaveWaitUntil = false;
              want_didSave = Some { includeText = false };
            };
          hoverProvider = true;
          completionProvider =
            Some
              {
                resolveProvider = true;
                completion_triggerCharacters =
                  ["$"; ">"; "\\"; ":"; "<"; "["; "'"; "\""];
              };
          signatureHelpProvider =
            Some { sighelp_triggerCharacters = ["("; ","] };
          definitionProvider = true;
          typeDefinitionProvider = true;
          referencesProvider = true;
          documentHighlightProvider = true;
          documentSymbolProvider = true;
          workspaceSymbolProvider = true;
          codeActionProvider = false;
          codeLensProvider = None;
          documentFormattingProvider = true;
          documentRangeFormattingProvider = true;
          documentOnTypeFormattingProvider =
            Some { firstTriggerCharacter = ";"; moreTriggerCharacter = ["}"] };
          renameProvider = true;
          documentLinkProvider = None;
          executeCommandProvider = None;
          typeCoverageProvider = true;
          rageProvider = true;
        };
    }

let do_didChangeWatchedFiles_registerCapability () : Lsp.lsp_request =
  let registration_options =
    DidChangeWatchedFilesRegistrationOptions
      {
        DidChangeWatchedFiles.watchers =
          [
            {
              DidChangeWatchedFiles.globPattern
              (* We could be more precise here, but some language clients (such as
          LanguageClient-neovim) don't currently support rich glob patterns.
          We'll do further filtering at a later stage. *) =
                "**";
            };
          ];
      }
  in
  let registration =
    Lsp.RegisterCapability.make_registration registration_options
  in
  Lsp.RegisterCapabilityRequest
    { RegisterCapability.registrations = [registration] }

let set_up_hh_logger_for_client_lsp () : unit =
  (* Log to a file on disk. Note that calls to `Hh_logger` will always write to
  `stderr`; this is in addition to that. *)
  let root =
    match get_root_opt () with
    | Some root -> root
    | None ->
      failwith
        ( "set_up_hh_logger_for_client_lsp should only be called "
        ^ "after having been initialized with a root" )
  in
  let client_lsp_log_fn = ServerFiles.client_lsp_log root in
  begin
    try Sys.rename client_lsp_log_fn (client_lsp_log_fn ^ ".old")
    with _e -> ()
  end;
  Hh_logger.set_log
    client_lsp_log_fn
    (Out_channel.create client_lsp_log_fn ~append:true);
  log "Starting clientLsp at %s" client_lsp_log_fn

let start_server (root : Path.t) : unit =
  (* This basically does "hh_client start": a single attempt to open the     *)
  (* socket, send+read version and compare for mismatch, send handoff and    *)
  (* read response. It will print information to stderr. If the server is in *)
  (* an unresponsive or invalid state then it will kill the server. Next if  *)
  (* necessary it tries to spawn the server and wait until the monitor is    *)
  (* responsive enough to print "ready". It will do a hard program exit if   *)
  (* there were spawn problems.                                              *)
  let env_start =
    {
      ClientStart.root;
      from = !ref_from;
      no_load = false;
      watchman_debug_logging = false;
      log_inference_constraints = false;
      profile_log = false;
      ai_mode = None;
      silent = true;
      exit_on_failure = false;
      debug_port = None;
      ignore_hh_version = false;
      saved_state_ignore_hhconfig = false;
      dynamic_view = !cached_toggle_state;
      prechecked = None;
      config = [];
      allow_non_opt_build = false;
    }
  in
  let _exit_status = ClientStart.main env_start in
  ()

(* connect: this method either connects to the monitor and leaves in an       *)
(* In_init state waiting for the server hello, or it fails to connect and     *)
(* leaves in a Lost_server state. You might call this from Pre_init or        *)
(* Lost_server states, obviously. But you can also call it from In_init state *)
(* if you want to give up on the prior attempt at connection and try again.   *)
let rec connect (state : state) : state Lwt.t =
  let root =
    match get_root_opt () with
    | Some root -> root
    | None -> assert false
  in
  begin
    match state with
    | In_init { In_init_env.conn; _ } ->
      begin
        try
          Timeout.shutdown_connection conn.ic;
          Timeout.close_in_noerr conn.ic
        with _ -> ()
      end
    | Pre_init
    | Lost_server _ ->
      ()
    | _ -> failwith "connect only in Pre_init, In_init or Lost_server state"
  end;
  try%lwt
    let%lwt conn = connect_client root ~autostart:false in
    set_hh_server_state Hh_server_initializing;
    match state with
    | In_init ienv ->
      Lwt.return
        (In_init
           {
             ienv with
             In_init_env.conn;
             most_recent_start_time = Unix.time ();
           })
    | _ ->
      let state = dismiss_ui state in
      Lwt.return
        (In_init
           {
             In_init_env.conn;
             first_start_time = Unix.time ();
             most_recent_start_time = Unix.time ();
             editor_open_files =
               Option.value (get_editor_open_files state) ~default:SMap.empty;
             (* uris_with_unsaved_changes should always be empty here: *)
             (* Pre_init will of course be empty; *)
             (* Lost_server will exit rather than reconnect with unsaved changes. *)
             uris_with_unsaved_changes = get_uris_with_unsaved_changes state;
             (* TODO(ljw): if a file is already open, and we connect here, and then *)
             (* the user closes the file -- then at that time we'll send CLOSE to the *)
             (* server without it having first received OPEN. I've seen erratic behavior *)
             (* from the server in that situation but haven't been able to repro it. *)
             file_edits = ImmQueue.empty;
           })
  with e ->
    (* Exit_with Out_of_retries, Exit_with Out_of_time: raised when we        *)
    (*   couldn't complete the handshake up to handoff within 3 attempts over *)
    (*   3 seconds. Maybe the informant is stopping anything from happening   *)
    (*   until a rebase has settled?                                          *)
    (* Exit_with No_server_running: raised when (1) the server's simply not   *)
    (*   running, or there's some other reason why the connection was refused *)
    (*   or timed-out and no lockfile is present; (2) the server was dormant  *)
    (*   and had already received too many pending connection requests;       *)
    (*   (3) server failed to load saved-state but was required to do so.     *)
    (* Exit_with Monitor_connection_failure: raised when the lockfile is      *)
    (*   present but connection-attempt to the monitor times out - maybe it's *)
    (*   under DDOS, or maybe it's declining to answer new connections.       *)
    let stack = Printexc.get_backtrace () in
    let { Lsp.Error.code; message; _ } = Lsp_fmt.error_of_exn e in
    let longMessage =
      Printf.sprintf "connect failed: %s [%i]\n%s" message code stack
    in
    let () = Lsp_helpers.telemetry_error to_stdout longMessage in
    Exit_status.(
      let new_hh_server_state =
        match e with
        | Exit_with Build_id_mismatch
        | Exit_with No_server_running_should_retry
        | Exit_with Server_hung_up_should_retry
        | Exit_with Server_hung_up_should_abort ->
          Hh_server_stopped
        | Exit_with Out_of_retries
        | Exit_with Out_of_time ->
          Hh_server_denying_connection
        | _ -> Hh_server_unknown
      in
      let explanation =
        match e with
        | Exit_with Out_of_retries
        | Exit_with Out_of_time ->
          "hh_server is waiting for things to settle"
        | Exit_with No_server_running_should_retry ->
          ClientMessages.lsp_explanation_for_no_server_running
        | _ -> "hh_server: " ^ message
      in
      let%lwt state =
        do_lost_server
          state
          ~allow_immediate_reconnect:false
          {
            Lost_env.explanation;
            new_hh_server_state;
            start_on_click = true;
            trigger_on_lock_file = true;
            trigger_on_lsp = false;
          }
      in
      Lwt.return state)

and reconnect_from_lost_if_necessary
    (state : state) (reason : [> `Event of event | `Force_regain ]) :
    state Lwt.t =
  Lost_env.(
    let should_reconnect =
      match (state, reason) with
      | (Lost_server _, `Force_regain) -> true
      | (Lost_server lenv, `Event (Client_message c))
        when lenv.p.trigger_on_lsp && c.Jsonrpc.kind <> Jsonrpc.Response ->
        true
      | (Lost_server lenv, `Event Tick) when lenv.p.trigger_on_lock_file ->
        MonitorConnection.server_exists lenv.lock_file
      | (_, _) -> false
    in
    if should_reconnect then
      let has_unsaved_changes =
        not (SSet.is_empty (get_uris_with_unsaved_changes state))
      in
      let%lwt current_version = read_hhconfig_version () in
      let needs_to_terminate =
        has_unsaved_changes || !hhconfig_version <> current_version
      in
      if needs_to_terminate then (
        (* In these cases we have to terminate our LSP server, and trust the    *)
        (* client to restart us. Note that we can't do clientStart because that *)
        (* would start our (old) version of hh_server, not the new one!         *)
        let unsaved = get_uris_with_unsaved_changes state |> SSet.elements in
        let unsaved_str =
          if unsaved = [] then
            "[None]"
          else
            String.concat ~sep:"\n" unsaved
        in
        let message =
          "Unsaved files:\n"
          ^ unsaved_str
          ^ "\nVersion in hhconfig that spawned the current hh_client: "
          ^ !hhconfig_version
          ^ "\nVersion in hhconfig currently: "
          ^ current_version
          ^ "\n"
        in
        Lsp_helpers.telemetry_log to_stdout message;
        exit_fail ()
      ) else
        let%lwt state = connect state in
        Lwt.return state
    else
      Lwt.return state)

(* do_lost_server: handles the various ways we might lose hh_server. We keep  *)
(* the LSP server alive, and will (elsewhere) listen for the various triggers *)
(* of getting the server back.                                                *)
and do_lost_server
    (state : state) ?(allow_immediate_reconnect = true) (p : Lost_env.params) :
    state Lwt.t =
  Lost_env.(
    set_hh_server_state p.new_hh_server_state;

    let state = dismiss_ui state in
    let uris_with_unsaved_changes = get_uris_with_unsaved_changes state in
    let editor_open_files =
      Option.value (get_editor_open_files state) ~default:SMap.empty
    in
    let lock_file =
      match get_root_opt () with
      | None -> assert false
      | Some root -> ServerFiles.lock_file root
    in
    let reconnect_immediately =
      allow_immediate_reconnect
      && p.trigger_on_lock_file
      && MonitorConnection.server_exists lock_file
    in
    if reconnect_immediately then (
      let lost_state =
        Lost_server
          {
            Lost_env.p;
            editor_open_files;
            uris_with_unsaved_changes;
            lock_file;
          }
      in
      Lsp_helpers.telemetry_log
        to_stdout
        "Reconnecting immediately to hh_server";
      let%lwt new_state =
        reconnect_from_lost_if_necessary lost_state `Force_regain
      in
      Lwt.return new_state
    ) else
      Lwt.return
        (Lost_server
           {
             Lost_env.p;
             editor_open_files;
             uris_with_unsaved_changes;
             lock_file;
           }))

let handle_idle_if_necessary (state : state) (event : event) : state =
  match state with
  | Main_loop menv when event <> Tick ->
    Main_loop { menv with Main_env.needs_idle = true }
  | _ -> state

let track_open_files (state : state) (event : event) : state =
  Jsonrpc.(
    (* We'll keep track of which files are opened by the editor. *)
    let prev_opened_files =
      Option.value (get_editor_open_files state) ~default:SMap.empty
    in
    let editor_open_files =
      match event with
      | Client_message c when c.method_ = "textDocument/didOpen" ->
        let params = parse_didOpen c.params in
        let doc = params.DidOpen.textDocument in
        let uri = params.DidOpen.textDocument.TextDocumentItem.uri in
        SMap.add uri doc prev_opened_files
      | Client_message c when c.method_ = "textDocument/didChange" ->
        let params = parse_didChange c.params in
        let uri =
          params.DidChange.textDocument.VersionedTextDocumentIdentifier.uri
        in
        let doc = SMap.get uri prev_opened_files in
        Lsp.TextDocumentItem.(
          (match doc with
          | Some doc ->
            let doc' =
              {
                doc with
                version =
                  params.DidChange.textDocument
                    .VersionedTextDocumentIdentifier.version;
                text =
                  Lsp_helpers.apply_changes_unsafe
                    doc.text
                    params.DidChange.contentChanges;
              }
            in
            SMap.add uri doc' prev_opened_files
          | None -> prev_opened_files))
      | Client_message c when c.method_ = "textDocument/didClose" ->
        let params = parse_didClose c.params in
        let uri = params.DidClose.textDocument.TextDocumentIdentifier.uri in
        SMap.remove uri prev_opened_files
      | _ -> prev_opened_files
    in
    match state with
    | Main_loop menv -> Main_loop { menv with Main_env.editor_open_files }
    | In_init ienv -> In_init { ienv with In_init_env.editor_open_files }
    | Lost_server lenv -> Lost_server { lenv with Lost_env.editor_open_files }
    | _ -> state)

let track_edits_if_necessary (state : state) (event : event) : state =
  Jsonrpc.(
    (* We'll keep track of which files have unsaved edits. Note that not all    *)
    (* clients send didSave messages; for those we only rely on didClose.       *)
    let previous = get_uris_with_unsaved_changes state in
    let uris_with_unsaved_changes =
      match event with
      | Client_message ({ method_ = "textDocument/didChange"; _ } as c) ->
        let params = parse_didChange c.params in
        let uri =
          params.DidChange.textDocument.VersionedTextDocumentIdentifier.uri
        in
        SSet.add uri previous
      | Client_message ({ method_ = "textDocument/didClose"; _ } as c) ->
        let params = parse_didClose c.params in
        let uri = params.DidClose.textDocument.TextDocumentIdentifier.uri in
        SSet.remove uri previous
      | Client_message ({ method_ = "textDocument/didSave"; _ } as c) ->
        let params = parse_didSave c.params in
        let uri = params.DidSave.textDocument.TextDocumentIdentifier.uri in
        SSet.remove uri previous
      | _ -> previous
    in
    match state with
    | Main_loop menv ->
      Main_loop { menv with Main_env.uris_with_unsaved_changes }
    | In_init ienv ->
      In_init { ienv with In_init_env.uris_with_unsaved_changes }
    | Lost_server lenv ->
      Lost_server { lenv with Lost_env.uris_with_unsaved_changes }
    | _ -> state)

let track_ide_service_open_files
    (ide_service : ClientIdeService.t) (event : event) : unit Lwt.t =
  Jsonrpc.(
    match event with
    | Client_message { method_ = "textDocument/didOpen"; params; _ } ->
      let params = parse_didOpen params in
      let file_path =
        params.DidOpen.textDocument.TextDocumentItem.uri
        |> lsp_uri_to_path
        |> Path.make
      in
      let file_contents = params.DidOpen.textDocument.TextDocumentItem.text in
      let%lwt (_ : (unit, string) result) =
        ClientIdeService.rpc
          ide_service
          (ClientIdeMessage.File_opened
             { ClientIdeMessage.file_path; file_contents })
      in
      Lwt.return_unit
    | _ ->
      (* Don't handle other events for now. When we show typechecking errors for
    the open file, we'll start handling them. *)
      Lwt.return_unit)

let log_response_if_necessary
    (event : event)
    (response : Hh_json.json option)
    (unblocked_time : float)
    (env : env) : unit =
  Jsonrpc.(
    match event with
    | Client_message c ->
      let json = c.json |> Hh_json.json_truncate |> Hh_json.json_to_string in
      let json_response =
        match response with
        | None -> ""
        | Some json -> json |> Hh_json.json_truncate |> Hh_json.json_to_string
      in
      HackEventLogger.client_lsp_method_handled
        ~root:(get_root_opt ())
        ~method_:
          ( if c.kind = Response then
            "[response]"
          else
            c.method_ )
        ~kind:(kind_to_string c.kind)
        ~start_queue_time:c.timestamp
        ~start_hh_server_state:
          (get_older_hh_server_state c.timestamp |> hh_server_state_to_string)
        ~start_handle_time:unblocked_time
        ~serverless_ide_flag:env.use_serverless_ide
        ~json
        ~json_response
    | _ -> ())

let hack_log_error
    (event : event option)
    (message : string)
    (stack : string)
    (source : string)
    (unblocked_time : float)
    (env : env) : unit =
  let root = get_root_opt () in
  log "Exception: message: %s, stack trace: %s" message stack;
  match event with
  | Some (Client_message c) ->
    Jsonrpc.(
      let json = c.json |> Hh_json.json_truncate |> Hh_json.json_to_string in
      HackEventLogger.client_lsp_method_exception
        ~root
        ~method_:c.method_
        ~kind:(kind_to_string c.kind)
        ~start_queue_time:c.timestamp
        ~start_hh_server_state:
          (get_older_hh_server_state c.timestamp |> hh_server_state_to_string)
        ~start_handle_time:unblocked_time
        ~serverless_ide_flag:env.use_serverless_ide
        ~json
        ~message
        ~stack
        ~source)
  | _ -> HackEventLogger.client_lsp_exception ~root ~message ~stack ~source

(* cancel_if_stale: If a message is stale, throw the necessary exception to
   cancel it. A message is considered stale if it's sufficiently old and there
   are other messages in the queue that are newer than it. *)
let short_timeout = 2.5

let long_timeout = 15.0

let cancel_if_stale
    (client : Jsonrpc.queue) (message : Jsonrpc.message) (timeout : float) :
    unit Lwt.t =
  let message_received_time = message.Jsonrpc.timestamp in
  let time_elapsed = Unix.gettimeofday () -. message_received_time in
  if time_elapsed >= timeout then
    if Jsonrpc.has_message client then
      raise (Error.RequestCancelled "request timed out")
    else
      Lwt.return_unit
  else
    Lwt.return_unit

let tick_showStatus ~(state : state ref) : unit Lwt.t =
  Main_env.(
    Lost_env.(
      ShowStatus.(
        ShowMessageRequest.(
          let show_status () : unit Lwt.t =
            let%lwt () = Lwt_unix.sleep 1.0 in
            begin
              match !state with
              | Main_loop menv ->
                request_showStatus
                  {
                    request =
                      {
                        type_ = menv.status.type_;
                        message = menv.status.message;
                        actions = [];
                      };
                    progress = None;
                    total = None;
                    shortMessage =
                      (match menv.status.shortMessage with
                      | Some msg -> Some (msg ^ " " ^ status_tick ())
                      | None -> None);
                  }
              | Lost_server { p; _ } ->
                let restart_command = "Restart Hack Server" in
                let on_result ~result state =
                  let result = Jget.string_d result "title" ~default:"" in
                  match (result, state) with
                  | (command, Lost_server _) ->
                    if command = restart_command then (
                      let root =
                        match get_root_opt () with
                        | None -> failwith "we should have root by now"
                        | Some root -> root
                      in
                      (* Belt-and-braces kill the server. This is in case the server was *)
                      (* stuck in some weird state. It's also what 'hh restart' does. *)
                      if MonitorConnection.server_exists (Path.to_string root)
                      then
                        ClientStop.kill_server root !ref_from;

                      (* After that it's safe to try to reconnect! *)
                      start_server root;
                      let%lwt state =
                        reconnect_from_lost_if_necessary state `Force_regain
                      in
                      Lwt.return state
                    ) else
                      Lwt.return state
                  | _ -> Lwt.return state
                in
                request_showStatus
                  ~on_result
                  {
                    request =
                      {
                        type_ = MessageType.ErrorMessage;
                        message =
                          p.explanation
                          ^ " Language features such as errors and go-to-def are currently unavailable.";
                        actions = [{ title = "Restart Hack Server" }];
                      };
                    progress = None;
                    total = None;
                    shortMessage = None;
                  }
              | _ -> ()
            end;
            Lwt.return_unit
          in
          let rec loop () : unit Lwt.t =
            let%lwt () = show_status () in
            loop ()
          in
          loop ()))))

(************************************************************************)
(* Message handling                                                     *)
(************************************************************************)

(* handle_event: Process and respond to a message, and update the LSP state
   machine accordingly. In case the message was a request, it returns the
   json it responded with, so the caller can log it. *)
let handle_event
    ~(env : env)
    ~(state : state ref)
    ~(client : Jsonrpc.queue)
    ~(ide_service : ClientIdeService.t)
    ~(event : event)
    ~(ref_unblocked_time : float ref) : unit Lwt.t =
  Jsonrpc.(
    Main_env.(
      let%lwt () =
        (* make sure to wrap any exceptions below in the promise *)
        match (!state, event) with
        (* response *)
        | (_, Client_message c) when c.kind = Jsonrpc.Response ->
          let id =
            match c.id with
            | Some (Hh_json.JSON_Number id) -> NumberId (int_of_string id)
            | Some (Hh_json.JSON_String id) -> StringId id
            | _ -> failwith "malformed response id"
          in
          let (on_result, on_error) =
            match IdMap.get id !callbacks_outstanding with
            | Some callbacks -> callbacks
            | None ->
              failwith
                "response id doesn't correspond to an outstanding request"
          in
          if Option.is_some c.error then (
            let code = Jget.int_exn c.error "code" in
            let message = Jget.string_exn c.error "message" in
            let data = Jget.val_opt c.error "data" in
            let%lwt new_state = on_error code message data !state in
            state := new_state;
            Lwt.return_unit
          ) else
            let%lwt new_state = on_result c.result !state in
            state := new_state;
            Lwt.return_unit
        (* shutdown request *)
        | (_, Client_message c) when c.method_ = "shutdown" ->
          let%lwt new_state =
            do_shutdown !state ide_service ref_unblocked_time
          in
          state := new_state;
          print_shutdown () |> respond_jsonrpc ~powered_by:Language_server c;
          Lwt.return_unit
        (* cancel notification *)
        | (_, Client_message c) when c.method_ = "$/cancelRequest" ->
          (* For now, we'll ignore it. *)
          Lwt.return_unit
        (* exit notification *)
        | (_, Client_message c) when c.method_ = "exit" ->
          if !state = Post_shutdown then
            exit_ok ()
          else
            exit_fail ()
        (* rage request *)
        | (_, Client_message c) when c.method_ = "telemetry/rage" ->
          let%lwt result = do_rage !state ref_unblocked_time in
          result |> print_rage |> respond_jsonrpc ~powered_by:Language_server c;
          Lwt.return_unit
        | (_, Client_message c)
          when env.use_serverless_ide
               && c.method_ = "workspace/didChangeWatchedFiles" ->
          DidChangeWatchedFiles.(
            let notification = parse_didChangeWatchedFiles c.params in
            List.iter notification.changes ~f:(fun change ->
                let path = lsp_uri_to_path change.uri in
                let path = Path.make path in
                ClientIdeService.notify_file_changed ide_service path);
            Lwt.return_unit)
        (* initialize request *)
        | (Pre_init, Client_message c) when c.method_ = "initialize" ->
          let initialize_params = c.params |> parse_initialize in
          Lwt.wakeup_later initialize_params_resolver initialize_params;
          set_up_hh_logger_for_client_lsp ();

          let%lwt version = read_hhconfig_version () in
          hhconfig_version := version;
          let%lwt new_state = connect !state in
          state := new_state;
          do_initialize ()
          |> print_initialize
          |> respond_jsonrpc ~powered_by:Language_server c;

          if env.use_serverless_ide then (
            Relative_path.set_path_prefix
              Relative_path.Root
              (Path.make (Lsp_helpers.get_root initialize_params));

            let id = NumberId (Jsonrpc.get_next_request_id ()) in
            let message = do_didChangeWatchedFiles_registerCapability () in
            to_stdout (print_lsp_request id message);
            let on_result ~result:_ state = Lwt.return state in
            let on_error ~code:_ ~message:_ ~data:_ state = Lwt.return state in
            callbacks_outstanding :=
              IdMap.add id (on_result, on_error) !callbacks_outstanding
          );

          if not @@ Sys_utils.is_test_mode () then
            Lsp_helpers.telemetry_log
              to_stdout
              ("Version in hhconfig=" ^ !hhconfig_version);
          Lwt.return_unit
        (* any request/notification if we haven't yet initialized *)
        | (Pre_init, Client_message _c) ->
          raise (Error.ServerNotInitialized "Server not yet initialized")
        (* Text document completion: "AutoComplete!" *)
        | ( ( In_init { In_init_env.editor_open_files; _ }
            | Main_loop { Main_env.editor_open_files; _ }
            | Lost_server { Lost_env.editor_open_files; _ } ),
            Client_message c )
          when env.use_serverless_ide && c.method_ = "textDocument/completion"
          ->
          let%lwt () = cancel_if_stale client c short_timeout in
          let%lwt result =
            parse_completion c.params
            |> do_completion_local ide_service editor_open_files
          in
          result
          |> print_completion
          |> respond_jsonrpc ~powered_by:Serverless_ide c;
          Lwt.return_unit
        (* Resolve documentation for a symbol: "Autocomplete Docblock!" *)
        | ( ( In_init { In_init_env.editor_open_files; _ }
            | Main_loop { Main_env.editor_open_files; _ }
            | Lost_server { Lost_env.editor_open_files; _ } ),
            Client_message c )
          when env.use_serverless_ide && c.method_ = "completionItem/resolve"
          ->
          let%lwt () = cancel_if_stale client c short_timeout in
          let%lwt result =
            parse_completionItem c.params
            |> do_resolve_local ide_service editor_open_files
          in
          result
          |> print_completionItem
          |> respond_jsonrpc ~powered_by:Serverless_ide c;
          Lwt.return_unit
        (* Document highlighting in serverless IDE *)
        | ( ( In_init { In_init_env.editor_open_files; _ }
            | Main_loop { Main_env.editor_open_files; _ }
            | Lost_server { Lost_env.editor_open_files; _ } ),
            Client_message c )
          when env.use_serverless_ide
               && c.method_ = "textDocument/documentHighlight" ->
          let%lwt () = cancel_if_stale client c short_timeout in
          let%lwt result =
            parse_documentHighlight c.params
            |> do_highlight_local ide_service editor_open_files
          in
          result |> print_documentHighlight |> Jsonrpc.respond to_stdout c;
          Lwt.return_unit
        (* Type coverage in serverless IDE *)
        | ( ( In_init { In_init_env.editor_open_files; _ }
            | Main_loop { Main_env.editor_open_files; _ }
            | Lost_server { Lost_env.editor_open_files; _ } ),
            Client_message c )
          when env.use_serverless_ide
               && c.method_ = "textDocument/typeCoverage" ->
          let%lwt () = cancel_if_stale client c short_timeout in
          let%lwt result =
            parse_typeCoverage c.params
            |> do_typeCoverage_local ide_service editor_open_files
          in
          result |> print_typeCoverage |> Jsonrpc.respond to_stdout c;
          Lwt.return_unit
        (* Hover docblocks in serverless IDE *)
        | ( ( In_init { In_init_env.editor_open_files; _ }
            | Main_loop { Main_env.editor_open_files; _ }
            | Lost_server { Lost_env.editor_open_files; _ } ),
            Client_message c )
          when env.use_serverless_ide && c.method_ = "textDocument/hover" ->
          let%lwt () = cancel_if_stale client c short_timeout in
          let%lwt result =
            parse_hover c.params
            |> do_hover_local ide_service editor_open_files
          in
          result |> print_hover |> respond_jsonrpc ~powered_by:Serverless_ide c;
          Lwt.return_unit
        | ( ( In_init { In_init_env.editor_open_files; _ }
            | Main_loop { Main_env.editor_open_files; _ }
            | Lost_server { Lost_env.editor_open_files; _ } ),
            Client_message c )
          when env.use_serverless_ide
               && c.method_ = "textDocument/documentSymbol" ->
          let%lwt () = cancel_if_stale client c short_timeout in
          let%lwt result =
            parse_documentSymbol c.params
            |> do_documentSymbol_local ide_service editor_open_files
          in
          result
          |> print_documentSymbol
          |> respond_jsonrpc ~powered_by:Serverless_ide c;
          Lwt.return_unit
        | ( ( In_init { In_init_env.editor_open_files; _ }
            | Main_loop { Main_env.editor_open_files; _ }
            | Lost_server { Lost_env.editor_open_files; _ } ),
            Client_message c )
          when env.use_serverless_ide && c.method_ = "textDocument/definition"
          ->
          let%lwt () = cancel_if_stale client c short_timeout in
          let%lwt result =
            parse_definition c.params
            |> do_definition_local ide_service editor_open_files
          in
          result
          |> print_definition
          |> respond_jsonrpc ~powered_by:Serverless_ide c;
          Lwt.return_unit
        | ( ( In_init { In_init_env.editor_open_files; _ }
            | Main_loop { Main_env.editor_open_files; _ }
            | Lost_server { Lost_env.editor_open_files; _ } ),
            Client_message c )
          when env.use_serverless_ide
               && c.method_ = "textDocument/typeDefinition" ->
          let%lwt () = cancel_if_stale client c short_timeout in
          let%lwt result =
            parse_definition c.params
            |> do_typeDefinition_local ide_service editor_open_files
          in
          result
          |> print_definition
          |> respond_jsonrpc ~powered_by:Serverless_ide c;
          Lwt.return_unit
        (* Resolve documentation for a symbol: "Autocomplete Docblock!" *)
        | ( ( In_init { In_init_env.editor_open_files; _ }
            | Main_loop { Main_env.editor_open_files; _ }
            | Lost_server { Lost_env.editor_open_files; _ } ),
            Client_message c )
          when env.use_serverless_ide
               && c.method_ = "textDocument/signatureHelp" ->
          let%lwt () = cancel_if_stale client c short_timeout in
          let%lwt result =
            parse_textDocumentPositionParams c.params
            |> do_signatureHelp_local ide_service editor_open_files
          in
          result
          |> print_signatureHelp
          |> respond_jsonrpc ~powered_by:Serverless_ide c;
          Lwt.return_unit
        (* any request/notification if we're not yet ready *)
        | (In_init ienv, Client_message c) ->
          In_init_env.(
            begin
              match c.method_ with
              | "textDocument/didOpen"
              | "textDocument/didChange"
              | "textDocument/didClose" ->
                (* These three crucial-for-correctness notifications will be buffered *)
                (* up so we'll be able to handle them when we're ready.               *)
                state :=
                  In_init
                    {
                      ienv with
                      file_edits = ImmQueue.push ienv.file_edits c.json;
                    };
                Lwt.return_unit
              | _ ->
                raise
                  (Error.RequestCancelled
                     (Hh_server_initializing |> hh_server_state_to_string))
                (* We deny all other requests. Operation_cancelled is the only *)
                (* error-response that won't produce logs/warnings on most clients. *)
            end)
        (* idle tick while waiting for server to complete initialization *)
        | (In_init ienv, Tick) ->
          In_init_env.(
            let time = Unix.time () in
            let delay_in_secs =
              int_of_float (time -. ienv.most_recent_start_time)
            in
            let%lwt () =
              if delay_in_secs <= 10 then (
                report_connect_progress ienv;
                Lwt.return_unit
              ) else
                (* terminate + retry the connection *)
                  let%lwt new_state = connect !state in
                  state := new_state;
                  Lwt.return_unit
            in
            Lwt.return_unit)
        (* server completes initialization *)
        | (In_init ienv, Server_hello) ->
          let%lwt () =
            connect_after_hello
              ienv.In_init_env.conn
              ienv.In_init_env.file_edits
          in
          state := report_connect_end ienv;
          Lwt.return_unit
        (* any "hello" from the server when we weren't expecting it. This is so *)
        (* egregious that we can't trust anything more from the server.         *)
        | (_, Server_hello) ->
          let message = "Unexpected hello" in
          let stack = "" in
          raise
            (Server_fatal_connection_exception { Marshal_tools.message; stack })
        (* Tick when we're connected to the server *)
        | (Main_loop menv, Tick) ->
          if menv.needs_idle then (
            (* If we're connected to a server and have no more messages in the queue, *)
            (* then we must let the server know we're idle, so it will be free to     *)
            (* handle command-line requests.                                          *)
            state := Main_loop { menv with needs_idle = false };
            rpc menv.conn ref_unblocked_time ServerCommandTypes.IDE_IDLE
          ) else
            Lwt.return_unit
        | (Main_loop _menv, Client_message c) when c.method_ = "initialized" ->
          (* Currently, we don't do anything in response to this notification. *)
          Lwt.return_unit
        (* textDocument/hover request *)
        | (Main_loop menv, Client_message c)
          when c.method_ = "textDocument/hover" ->
          let%lwt () = cancel_if_stale client c short_timeout in
          let%lwt result =
            parse_hover c.params |> do_hover menv.conn ref_unblocked_time
          in
          result |> print_hover |> respond_jsonrpc ~powered_by:Hh_server c;
          Lwt.return_unit
        (* textDocument/typeDefinition request *)
        | (Main_loop menv, Client_message c)
          when c.method_ = "textDocument/typeDefinition" ->
          let%lwt () = cancel_if_stale client c short_timeout in
          let%lwt result =
            parse_definition c.params
            |> do_typeDefinition menv.conn ref_unblocked_time
          in
          result |> print_definition |> respond_jsonrpc ~powered_by:Hh_server c;
          Lwt.return_unit
        (* textDocument/definition request *)
        | (Main_loop { conn; editor_open_files; _ }, Client_message c)
          when c.method_ = "textDocument/definition" ->
          let%lwt () = cancel_if_stale client c short_timeout in
          let%lwt result =
            parse_definition c.params
            |> do_definition conn ref_unblocked_time editor_open_files
          in
          result |> print_definition |> respond_jsonrpc ~powered_by:Hh_server c;
          Lwt.return_unit
        (* textDocument/completion request *)
        | (Main_loop menv, Client_message c)
          when c.method_ = "textDocument/completion" ->
          let do_completion =
            if env.use_ffp_autocomplete then
              do_completion_ffp
            else
              do_completion_legacy
          in
          let%lwt () = cancel_if_stale client c short_timeout in
          let%lwt result =
            parse_completion c.params
            |> do_completion menv.conn ref_unblocked_time
          in
          result |> print_completion |> respond_jsonrpc ~powered_by:Hh_server c;
          Lwt.return_unit
        (* completionItem/resolve request *)
        | (Main_loop menv, Client_message c)
          when c.method_ = "completionItem/resolve" ->
          let%lwt () = cancel_if_stale client c short_timeout in
          let%lwt result =
            parse_completionItem c.params
            |> do_completionItemResolve menv.conn ref_unblocked_time
          in
          result
          |> print_completionItem
          |> respond_jsonrpc ~powered_by:Hh_server c;
          Lwt.return_unit
        (* workspace/symbol request *)
        | (Main_loop menv, Client_message c)
          when c.method_ = "workspace/symbol" ->
          let%lwt result =
            parse_workspaceSymbol c.params
            |> do_workspaceSymbol menv.conn ref_unblocked_time
          in
          result
          |> print_workspaceSymbol
          |> respond_jsonrpc ~powered_by:Hh_server c;
          Lwt.return_unit
        (* textDocument/documentSymbol request *)
        | (Main_loop menv, Client_message c)
          when c.method_ = "textDocument/documentSymbol" ->
          let%lwt result =
            parse_documentSymbol c.params
            |> do_documentSymbol menv.conn ref_unblocked_time
          in
          result
          |> print_documentSymbol
          |> respond_jsonrpc ~powered_by:Hh_server c;
          Lwt.return_unit
        (* textDocument/references request *)
        | (Main_loop menv, Client_message c)
          when c.method_ = "textDocument/references" ->
          let%lwt () = cancel_if_stale client c long_timeout in
          let%lwt result =
            parse_findReferences c.params
            |> do_findReferences menv.conn ref_unblocked_time
          in
          result
          |> print_findReferences
          |> respond_jsonrpc ~powered_by:Hh_server c;
          Lwt.return_unit
        (* textDocument/rename *)
        | (Main_loop menv, Client_message c)
          when c.method_ = "textDocument/rename" ->
          let%lwt result =
            parse_documentRename c.params
            |> do_documentRename menv.conn ref_unblocked_time
          in
          result
          |> print_documentRename
          |> respond_jsonrpc ~powered_by:Hh_server c;
          Lwt.return_unit
        (* textDocument/documentHighlight *)
        | (Main_loop menv, Client_message c)
          when c.method_ = "textDocument/documentHighlight" ->
          let%lwt () = cancel_if_stale client c short_timeout in
          let%lwt result =
            parse_documentHighlight c.params
            |> do_documentHighlight menv.conn ref_unblocked_time
          in
          result
          |> print_documentHighlight
          |> respond_jsonrpc ~powered_by:Hh_server c;
          Lwt.return_unit
        (* textDocument/typeCoverage *)
        | (Main_loop menv, Client_message c)
          when c.method_ = "textDocument/typeCoverage" ->
          let%lwt result =
            parse_typeCoverage c.params
            |> do_typeCoverage menv.conn ref_unblocked_time
          in
          result
          |> print_typeCoverage
          |> respond_jsonrpc ~powered_by:Hh_server c;
          Lwt.return_unit
        (* textDocument/formatting *)
        | (Main_loop menv, Client_message c)
          when c.method_ = "textDocument/formatting" ->
          parse_documentFormatting c.params
          |> do_documentFormatting menv.editor_open_files
          |> print_documentFormatting
          |> respond_jsonrpc ~powered_by:Language_server c;
          Lwt.return_unit
        (* textDocument/formatting *)
        | (Main_loop menv, Client_message c)
          when c.method_ = "textDocument/rangeFormatting" ->
          parse_documentRangeFormatting c.params
          |> do_documentRangeFormatting menv.editor_open_files
          |> print_documentRangeFormatting
          |> respond_jsonrpc ~powered_by:Language_server c;
          Lwt.return_unit
        (* textDocument/onTypeFormatting *)
        | (Main_loop menv, Client_message c)
          when c.method_ = "textDocument/onTypeFormatting" ->
          let%lwt () = cancel_if_stale client c short_timeout in
          parse_documentOnTypeFormatting c.params
          |> do_documentOnTypeFormatting menv.editor_open_files
          |> print_documentOnTypeFormatting
          |> respond_jsonrpc ~powered_by:Language_server c;
          Lwt.return_unit
        (* textDocument/didOpen notification *)
        | (Main_loop menv, Client_message c)
          when c.method_ = "textDocument/didOpen" ->
          parse_didOpen c.params |> do_didOpen menv.conn ref_unblocked_time
        | (Main_loop menv, Client_message c)
          when c.method_ = "workspace/toggleTypeCoverage" ->
          parse_toggleTypeCoverage c.params
          |> do_toggleTypeCoverage menv.conn ref_unblocked_time
        (* textDocument/didClose notification *)
        | (Main_loop menv, Client_message c)
          when c.method_ = "textDocument/didClose" ->
          parse_didClose c.params |> do_didClose menv.conn ref_unblocked_time
        (* textDocument/didChange notification *)
        | (Main_loop menv, Client_message c)
          when c.method_ = "textDocument/didChange" ->
          parse_didChange c.params |> do_didChange menv.conn ref_unblocked_time
        (* textDocument/didSave notification *)
        | (Main_loop _menv, Client_message c)
          when c.method_ = "textDocument/didSave" ->
          Lwt.return_unit
        (* textDocument/signatureHelp notification *)
        | (Main_loop menv, Client_message c)
          when c.method_ = "textDocument/signatureHelp" ->
          let%lwt result =
            parse_textDocumentPositionParams c.params
            |> do_signatureHelp menv.conn ref_unblocked_time
          in
          result
          |> print_signatureHelp
          |> respond_jsonrpc ~powered_by:Hh_server c;
          Lwt.return_unit
        (* server busy status *)
        | ( _,
            Server_message { push = ServerCommandTypes.BUSY_STATUS status; _ }
          ) ->
          let should_send_status =
            match Lwt.poll initialize_params_promise with
            | None -> false
            | Some p ->
              Lsp.Initialize.(p.initializationOptions.sendServerStatusEvents)
          in
          ( if should_send_status then
            let status_message =
              ServerCommandTypes.(
                match status with
                | Needs_local_typecheck -> "needs_local_typecheck"
                | Doing_local_typecheck -> "doing_local_typecheck"
                | Done_local_typecheck -> "done_local_typecheck"
                | Doing_global_typecheck _ -> "doing_global_typecheck"
                | Done_global_typecheck _ -> "done_global_typecheck")
            in
            Lsp_helpers.telemetry_log to_stdout status_message );
          state := do_server_busy !state status;
          Lwt.return_unit
        (* textDocument/publishDiagnostics notification *)
        | ( Main_loop menv,
            Server_message
              { push = ServerCommandTypes.DIAGNOSTIC (_, errors); _ } ) ->
          let uris_with_diagnostics =
            do_diagnostics menv.uris_with_diagnostics errors
          in
          state := Main_loop { menv with uris_with_diagnostics };
          Lwt.return_unit
        (* any server diagnostics that come after we've shut down *)
        | (_, Server_message { push = ServerCommandTypes.DIAGNOSTIC _; _ }) ->
          Lwt.return_unit
        | (_, Client_ide_notification ClientIdeMessage.Done_processing) ->
          Lsp_helpers.telemetry_log
            to_stdout
            "[client-ide] Done processing file changes";
          Lwt.return_unit
        (* catch-all for client reqs/notifications we haven't yet implemented *)
        | (Main_loop _menv, Client_message c) ->
          let message = Printf.sprintf "not implemented: %s" c.method_ in
          raise (Error.MethodNotFound message)
        (* catch-all for requests/notifications after shutdown request *)
        | (Post_shutdown, Client_message _c) ->
          raise (Error.InvalidRequest "already received shutdown request")
        (* server shut-down request *)
        | ( Main_loop _menv,
            Server_message
              { push = ServerCommandTypes.NEW_CLIENT_CONNECTED; _ } ) ->
          let%lwt new_state =
            do_lost_server
              !state
              {
                Lost_env.explanation = "hh_server is active in another window.";
                new_hh_server_state = Hh_server_stolen;
                start_on_click = false;
                trigger_on_lock_file = false;
                trigger_on_lsp = true;
              }
          in
          state := new_state;
          Lwt.return_unit
        (* server shut-down request, unexpected *)
        | ( _,
            Server_message
              { push = ServerCommandTypes.NEW_CLIENT_CONNECTED; _ } ) ->
          let message = "unexpected close of absent server" in
          let stack = "" in
          raise
            (Server_fatal_connection_exception { Marshal_tools.message; stack })
        (* server fatal shutdown *)
        | (_, Server_message { push = ServerCommandTypes.FATAL_EXCEPTION e; _ })
          ->
          raise (Server_fatal_connection_exception e)
        (* server non-fatal exception *)
        | ( _,
            Server_message
              { push = ServerCommandTypes.NONFATAL_EXCEPTION e; _ } ) ->
          raise (Server_nonfatal_exception e)
        (* idle tick. No-op. *)
        | (_, Tick) ->
          EventLogger.flush ();
          Lwt.return_unit
        (* client message when we've lost the server *)
        | (Lost_server lenv, Client_message _c) ->
          Lost_env.(
            (* if trigger_on_lsp_method is set, our caller should already have        *)
            (* transitioned away from this state.                                     *)
            assert (not lenv.p.trigger_on_lsp);

            (* We deny all other requests. This is the only response that won't       *)
            (* produce logs/warnings on most clients...                               *)
            raise
              (Error.RequestCancelled
                 (lenv.p.new_hh_server_state |> hh_server_state_to_string)))
      in
      Lwt.return_unit))

let run_ide_service (env : env) (ide_service : ClientIdeService.t) : unit Lwt.t
    =
  if env.use_serverless_ide then (
    let%lwt root = get_root_wait () in
    let initialize_params = initialize_params_exc () in
    if
      Lsp.Initialize.(
        initialize_params.client_capabilities.workspace.didChangeWatchedFiles
          .dynamicRegistration)
    then
      log "Language client reports that it supports file-watching"
    else
      log
        ( "Warning: the language client does not report "
        ^^ "that it supports file-watching; "
        ^^ "file change notifications may not be processed, "
        ^^ "and consequently, IDE queries may return stale results." );

    let naming_table_saved_state_path =
      Lsp.Initialize.(
        initialize_params.initializationOptions.namingTableSavedStatePath)
      |> Option.map ~f:Path.make
    in
    let%lwt result =
      ClientIdeService.initialize_from_saved_state
        ide_service
        ~root
        ~naming_table_saved_state_path
        ~wait_for_initialization:(Option.is_some naming_table_saved_state_path)
    in
    match result with
    | Ok () ->
      let%lwt () = ClientIdeService.serve ide_service in
      Lwt.return_unit
    | Error message ->
      log "IDE services could not be initialized: %s" message;
      Lwt.return_unit
  ) else
    Lwt.return_unit

let shutdown_ide_service (ide_service : ClientIdeService.t) : unit Lwt.t =
  log "Shutting down IDE service process...";
  let%lwt () = ClientIdeService.destroy ide_service in
  Lwt.return_unit

(* main: this is the main loop for processing incoming Lsp client requests,
   and incoming server notifications. Never returns. *)
let main (env : env) : Exit_status.t Lwt.t =
  Printexc.record_backtrace true;
  ref_from := env.from;

  HackEventLogger.set_from env.from;
  let client = Jsonrpc.make_queue () in
  let ide_service = ClientIdeService.make () in
  let deferred_action : (unit -> unit Lwt.t) option ref = ref None in
  let state = ref Pre_init in
  let ref_event = ref None in
  let ref_unblocked_time = ref (Unix.gettimeofday ()) in
  (* ref_unblocked_time is the time at which we're no longer blocked on either *)
  (* clientLsp message-loop or hh_server, and can start actually handling.  *)
  (* Everything that blocks will update this variable.                      *)
  let process_next_event () : unit Lwt.t =
    try%lwt
      let%lwt () =
        match !deferred_action with
        | Some deferred_action ->
          let%lwt () = deferred_action () in
          Lwt.return_unit
        | None -> Lwt.return_unit
      in
      deferred_action := None;
      let%lwt event = get_next_event !state client ide_service in
      ref_event := Some event;
      ref_unblocked_time := Unix.gettimeofday ();

      (* maybe set a flag to indicate that we'll need to send an idle message *)
      state := handle_idle_if_necessary !state event;

      (* if we're in a lost-server state, some triggers cause us to reconnect *)
      let%lwt new_state =
        reconnect_from_lost_if_necessary !state (`Event event)
      in
      state := new_state;

      (* we keep track of all open files and their contents *)
      state := track_open_files !state event;

      (* we keep track of all files that have unsaved changes in them *)
      state := track_edits_if_necessary !state event;

      (* if a message comes from the server, maybe update our record of server state *)
      update_hh_server_state_if_necessary event;

      let%lwt () =
        (* update the IDE service with the new file contents, if any *)
        if env.use_serverless_ide then
          let%lwt () = track_ide_service_open_files ide_service event in
          Lwt.return_unit
        else
          Lwt.return_unit
      in
      (* this is the main handler for each message*)
      Jsonrpc.clear_last_sent ();
      let%lwt () =
        handle_event
          ~env
          ~state
          ~client
          ~ide_service
          ~event
          ~ref_unblocked_time
      in
      let response = Jsonrpc.last_sent () in
      (* for LSP requests and notifications, we keep a log of what+when we responded *)
      log_response_if_necessary event response !ref_unblocked_time env;
      Lwt.return_unit
    with
    | Server_fatal_connection_exception { Marshal_tools.stack; message } ->
      if !state <> Post_shutdown then (
        (* The server never tells us why it closed the connection - it simply   *)
        (* closes. We don't have privilege to inspect its exit status.          *)
        (* But in some cases of a controlled exit, the server does write to a   *)
        (* "finale file" to explain its reason for exit...                      *)
        let server_finale_data =
          match !state with
          | Main_loop { Main_env.conn; _ }
          | In_init { In_init_env.conn; _ } ->
            ClientConnect.get_finale_data conn.server_finale_file
          | _ -> None
        in
        let server_finale_stack =
          match server_finale_data with
          | Some { ServerCommandTypes.stack = Utils.Callstack s; _ } -> s
          | _ -> ""
        in
        let stack =
          Printf.sprintf
            "%s\n---\n%s\n---\n%s"
            stack
            (Printexc.get_backtrace ())
            server_finale_stack
        in
        (* Log all the things! *)
        hack_log_error
          !ref_event
          message
          stack
          "from_server"
          !ref_unblocked_time
          env;
        Lsp_helpers.telemetry_error
          to_stdout
          (message ^ ", from_server\n" ^ stack);

        (* The monitor is responsible for detecting server closure and exit     *)
        (* status, and restarting the server if necessary (that's not our job). *)
        (* All we'll do is put up a dialog telling the user that the server is  *)
        (* down and giving them a button to restart.                            *)
        let explanation =
          match server_finale_data with
          | Some { ServerCommandTypes.msg; _ } -> msg
          | _ -> "hh_server has stopped."
        in
        (* When would be a good time to auto-dismiss the dialog and attempt     *)
        (* a proper re-connection? it's not our job to ascertain with certainty *)
        (* whether that re-connection will succeed - it's impossible to know,   *)
        (* but also our re-connection attempt is pretty forceful.               *)
        (* First: if the server determined in its finale that there shouldn't   *)
        (* be automatic retry then we won't. Otherwise, we'll sleep for 1 sec   *)
        (* and then look for the presence of the lock file. The sleep is        *)
        (* because typically if you do "hh stop" then the persistent connection *)
        (* shuts down instantly but the monitor takes a short time to release   *)
        (* its lockfile.                                                        *)
        let trigger_on_lock_file =
          match server_finale_data with
          | Some
              {
                ServerCommandTypes.exit_status =
                  Exit_status.Failed_to_load_should_abort;
                _;
              } ->
            false
          | _ -> true
        in
        Unix.sleep 1;

        (* We're right now inside an exception handler. We don't want to do     *)
        (* work that might itself throw. So instead we'll leave that to the     *)
        (* next time around the loop.                                           *)
        deferred_action :=
          Some
            (fun () ->
              let%lwt new_state =
                do_lost_server
                  !state
                  {
                    Lost_env.explanation;
                    new_hh_server_state = Hh_server_stopped;
                    start_on_click = true;
                    trigger_on_lock_file;
                    trigger_on_lsp = false;
                  }
              in
              state := new_state;
              Lwt.return_unit)
      );
      Lwt.return_unit
    | Client_fatal_connection_exception { Marshal_tools.stack; message } ->
      let stack = stack ^ "---\n" ^ Printexc.get_backtrace () in
      hack_log_error
        !ref_event
        message
        stack
        "from_client"
        !ref_unblocked_time
        env;
      Lsp_helpers.telemetry_error
        to_stdout
        (message ^ ", from_client\n" ^ stack);
      let () = exit_fail () in
      Lwt.return_unit
    | Client_recoverable_connection_exception { Marshal_tools.stack; message }
      ->
      let stack = stack ^ "---\n" ^ Printexc.get_backtrace () in
      hack_log_error
        !ref_event
        message
        stack
        "from_client"
        !ref_unblocked_time
        env;
      Lsp_helpers.telemetry_error
        to_stdout
        (message ^ ", from_client\n" ^ stack);
      Lwt.return_unit
    | Server_nonfatal_exception { Marshal_tools.stack; message } ->
      let stack = stack ^ "---\n" ^ Printexc.get_backtrace () in
      hack_log_error
        !ref_event
        message
        stack
        "from_server"
        !ref_unblocked_time
        env;
      respond_to_error !ref_event (Error.Unknown message) stack;
      Lwt.return_unit
    | Error.RequestCancelled _ as e ->
      let stack = Printexc.get_backtrace () in
      (* Note: we may send back a request-cancelled error if we decided to
      cancel the user's request (such as if `hh_server` was not ready to accept
      it, or if it timed out). This happens fairly frequently and are pretty
      benign, so don't log them. *)
      respond_to_error !ref_event e stack;
      Lwt.return_unit
    | e ->
      let stack = Printexc.get_backtrace () in
      let message = Exn.to_string e in
      respond_to_error !ref_event e stack;
      hack_log_error
        !ref_event
        message
        stack
        "from_lsp"
        !ref_unblocked_time
        env;
      Lwt.return_unit
  in
  let rec main_loop () : unit Lwt.t =
    let%lwt () = process_next_event () in
    main_loop ()
  in
  let%lwt () = Lwt.pick [main_loop (); tick_showStatus state]
  and () = run_ide_service env ide_service in
  Lwt.return Exit_status.No_error
