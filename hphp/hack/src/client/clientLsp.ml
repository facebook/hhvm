(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Lsp
open Lsp_fmt
open Hh_json_helpers

(* All hack-specific code relating to LSP goes in here. *)

type env = {
  from: string;
  config: (string * string) list;
  use_ffp_autocomplete: bool;
  use_ranked_autocomplete: bool;
  use_serverless_ide: bool;
  verbose: bool;
  init_id: string;
}

(** This is env.from, but maybe modified in the light of the initialize request *)
let from = ref "[init]"

(** We cache the state of the typecoverageToggle button, so that when Hack restarts,
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
[@@deriving eq]

let hh_server_restart_button_text = "Restart hh_server"

let client_ide_restart_button_text = "Restart Hack IDE"

let see_output_hack = " See Output\xE2\x80\xBAHack for details." (* chevron *)

type incoming_metadata = {
  timestamp: float;  (** time this message arrived at stdin *)
  tracking_id: string;
      (** a unique random string of our own creation, which we can use for logging *)
}

(** A push message from the server might come while we're waiting for a server-rpc
   response, or while we're free. The current architecture allows us to have
   arbitrary responses to push messages while we're free, but only a limited set
   of responses while we're waiting for a server-rpc - e.g. we can update our
   notion of the server_state, or send a message to the client, but we can't
   update our own state monad. The has_* fields are ad-hoc push-specific indicators
   of whether we've done some part of the response during the rpc. *)
type server_message = {
  push: ServerCommandTypes.push;
  has_updated_server_state: bool;
}

type server_conn = {
  ic: Timeout.in_channel;
  oc: Out_channel.t;
  server_finale_file: string;
  pending_messages: server_message Queue.t;
      (** ones that arrived during current rpc *)
}

module Main_env = struct
  type t = {
    conn: server_conn;
    needs_idle: bool;
    most_recent_file: documentUri option;
    editor_open_files: Lsp.TextDocumentItem.t UriMap.t;
    uris_with_diagnostics: UriSet.t;
    uris_with_unsaved_changes: UriSet.t;
        (** see comment in get_uris_with_unsaved_changes *)
    hh_server_status: ShowStatusFB.params;
        (** is updated by [handle_server_message] > [do_server_busy]. Shows status of
        a connected hh_server, whether it's busy typechecking or ready:
        (1) type_=InfoMessage when done typechecking, or WarningMessage during.
        (2) shortMessage="Hack" if IDE is available, or "Hack: busy" if not
        (3) message is a descriptive status about what it's doing. *)
  }
end

module In_init_env = struct
  type t = {
    conn: server_conn;
    first_start_time: float;  (** our first attempt to connect *)
    most_recent_start_time: float;  (** for subsequent retries *)
    most_recent_file: documentUri option;
    editor_open_files: Lsp.TextDocumentItem.t UriMap.t;
    uris_with_unsaved_changes: UriSet.t;
        (** see comment in get_uris_with_unsaved_changes *)
    hh_server_status_diagnostic: PublishDiagnostics.params option;
  }
end

module Lost_env = struct
  type t = {
    p: params;
    most_recent_file: documentUri option;
    editor_open_files: Lsp.TextDocumentItem.t UriMap.t;
    uris_with_unsaved_changes: UriSet.t;
        (** see comment in get_uris_with_unsaved_changes *)
    lock_file: string;
    hh_server_status_diagnostic: PublishDiagnostics.params option;
  }

  and params = {
    explanation: string;
    new_hh_server_state: hh_server_state;
    start_on_click: bool;
        (** if user clicks Restart, do we ClientStart before reconnecting? *)
    trigger_on_lsp: bool;
        (** reconnect if we receive any LSP request/notification *)
    trigger_on_lock_file: bool;  (** reconnect if lockfile is created *)
  }
end

type state =
  | Pre_init  (** Pre_init: we haven't yet received the initialize request. *)
  | In_init of In_init_env.t
      (** In_init: we did respond to the initialize request, and now we're
          waiting for a "Hello" from the server. When that comes we'll
          request a permanent connection from the server, and process the
          file_changes backlog, and switch to Main_loop. *)
  | Main_loop of Main_env.t
      (** Main_loop: we have a working connection to both server and client. *)
  | Lost_server of Lost_env.t
      (** Lost_server: someone stole the persistent connection from us.
          We might choose to grab it back if prompted... *)
  | Post_shutdown
      (** Post_shutdown: we received a shutdown request from the client, and
          therefore shut down our connection to the server. We can't handle
          any more requests from the client and will close as soon as it
          notifies us that we can exit. *)

let is_post_shutdown (state : state) : bool =
  match state with
  | Post_shutdown -> true
  | Pre_init
  | In_init _
  | Main_loop _
  | Lost_server _ ->
    false

let is_pre_init (state : state) : bool =
  match state with
  | Pre_init -> true
  | Post_shutdown
  | In_init _
  | Main_loop _
  | Lost_server _ ->
    false

type result_handler = lsp_result -> state -> state Lwt.t

type result_telemetry = {
  (* how many results did we send back to the user? *)
  result_count: int;
  (* other message-specific data *)
  result_extra_telemetry: Telemetry.t option;
}

let initialize_params_ref : Lsp.Initialize.params option ref = ref None

let initialize_params_exc () : Lsp.Initialize.params =
  match !initialize_params_ref with
  | None -> failwith "initialize_params not yet received"
  | Some initialize_params -> initialize_params

let get_root_opt () : Path.t option =
  match !initialize_params_ref with
  | None -> None
  | Some initialize_params ->
    let path = Some (Lsp_helpers.get_root initialize_params) in
    Some (Wwwroot.get path)

let get_root_exn () : Path.t = Option.value_exn (get_root_opt ())

(** We remember the last version of .hhconfig, and hack_rc_mode switch,
so that if they change then we know we must terminate and be restarted. *)
let hhconfig_version_and_switch : string ref = ref "[NotYetInitialized]"

(** This flag is used to control how much will be written
to log-files. It can be turned on initially by --verbose at the command-line or
setting "trace:Verbose" in initializationParams. Thereafter, it can
be changed by the user dynamically via $/setTraceNotification.
Don't alter this reference directly; instead use [set_verbose_to_file]
so as to pass the message on to ide_service as well.
Note: control for how much will be written to stderr is solely
controlled by --verbose at the command-line, stored in env.verbose. *)
let verbose_to_file : bool ref = ref false

let can_autostart_after_mismatch : bool ref = ref true

let requests_outstanding : (lsp_request * result_handler) IdMap.t ref =
  ref IdMap.empty

let get_outstanding_request_exn (id : lsp_id) : lsp_request =
  match IdMap.find_opt id !requests_outstanding with
  | Some (request, _) -> request
  | None -> failwith "response id doesn't correspond to an outstanding request"

(** hh_server pushes BUSY_STATUS messages over the persistent connection
to mark key milestones like "begin typechecking". We handle them in two
ways. First, update_hh_server_state_if_necessary updates a historical
record of state transitions over the past two minutes, called both from
the main message loop and also during rpc progress callbacks. It's
stored in a global variable because we don't want rpc callbacks to have
to be part of the state monad. It's kept as an ordered list of states
over the past two minutes (head is newest) so that if we were busy at
the time a jsonrpc request arrived on stdin, we can still know what was
the server state at the time. The second way we handle them is inside
handle_server_message, called as part of the main message loop, whose
job is to update the Main_env representation of current hh_server status. *)
let hh_server_state_log : (float * hh_server_state) list ref = ref []

(** hh_server pushes a different form of its state to the monitor e.g.
"busy typechecking 1/50 files" or "init is slow due to lack of saved state".
It does this for the sake of clients who don't have a persistent connection;
they can ask the monitor what was the latest that hh_server pushed. We use
this during In_init when we don't have a persistent connection; other
command-line clients of hh_client do this when they're waiting their turn.
On idle, i.e. during [Tick] events, we update the following global
variable which synthesizes our best knowledge about the current hh_server
state; during Main_loop it's obtained from the latest Main_env representation
of the current hh_server state, and during In_init it's obtained by
asking the monitor. We store this in a global value so we can access
it during rpc callbacks, without requiring them to have the state monad. *)
let latest_hh_server_status : ShowStatusFB.params option ref = ref None

(** Have we already sent a status message over LSP? If so, and our new
status will be just the same as the previous one, we won't need to send it
again. This stores the most recent status that the LSP client has. *)
let showStatus_outstanding : string ref = ref ""

let log s = Hh_logger.log ("[client-lsp] " ^^ s)

let log_debug s = Hh_logger.debug ("[client-lsp] " ^^ s)

let log_error s = Hh_logger.error ("[client-lsp] " ^^ s)

let set_up_hh_logger_for_client_lsp (root : Path.t) : unit =
  (* Log to a file on disk. Note that calls to `Hh_logger` will always write to
  `stderr`; this is in addition to that. *)
  let client_lsp_log_fn = ServerFiles.client_lsp_log root in
  begin
    try Sys.rename client_lsp_log_fn (client_lsp_log_fn ^ ".old")
    with _e -> ()
  end;
  Hh_logger.set_log client_lsp_log_fn;
  log "Starting clientLsp at %s" client_lsp_log_fn

let to_stdout (json : Hh_json.json) : unit =
  let s = Hh_json.json_to_string json ^ "\r\n\r\n" in
  Http_lite.write_message stdout s

let get_editor_open_files (state : state) :
    Lsp.TextDocumentItem.t UriMap.t option =
  match state with
  | Pre_init
  | Post_shutdown ->
    None
  | Main_loop menv -> Some menv.Main_env.editor_open_files
  | In_init ienv -> Some ienv.In_init_env.editor_open_files
  | Lost_server lenv -> Some lenv.Lost_env.editor_open_files

(** This is the most recent file that was subject of an LSP request
from the client. There's no guarantee that the file is still open. *)
let get_most_recent_file (state : state) : documentUri option =
  match state with
  | Pre_init
  | Post_shutdown ->
    None
  | Main_loop menv -> menv.Main_env.most_recent_file
  | In_init ienv -> ienv.In_init_env.most_recent_file
  | Lost_server lenv -> lenv.Lost_env.most_recent_file

type event =
  | Server_hello
  | Server_message of server_message
  | Client_message of incoming_metadata * lsp_message
      (** Client_message stores raw json, and the parsed form of it *)
  | Client_ide_notification of ClientIdeMessage.notification
  | Tick  (** once per second, on idle *)

let event_to_string (event : event) : string =
  match event with
  | Server_hello -> "Server_hello"
  | Server_message _ -> "Server_message(_)"
  | Client_message (metadata, m) ->
    Printf.sprintf
      "Client_message(#%s: %s)"
      metadata.tracking_id
      (Lsp_fmt.denorm_message_to_string m)
  | Client_ide_notification n ->
    Printf.sprintf
      "Client_ide_notification(%s)"
      (ClientIdeMessage.notification_to_string n)
  | Tick -> "Tick"

let is_tick (event : event) : bool =
  match event with
  | Tick -> true
  | Server_hello
  | Server_message _
  | Client_message _
  | Client_ide_notification _ ->
    false

(* Here are some exit points. *)
let exit_ok () = exit 0

let exit_fail () = exit 1

(* The following connection exceptions inform the main LSP event loop how to
respond to an exception: was the exception a connection-related exception
(one of these) or did it arise during other logic (not one of these)? Can
we report the exception to the LSP client? Can we continue handling
further LSP messages or must we quit? If we quit, can we do so immediately
or must we delay?  --  Separately, they also help us marshal callstacks
across daemon- and process-boundaries. *)

exception
  Client_fatal_connection_exception of Marshal_tools.remote_exception_data

exception
  Client_recoverable_connection_exception of Marshal_tools.remote_exception_data

exception
  Server_fatal_connection_exception of Marshal_tools.remote_exception_data

exception Server_nonfatal_exception of Lsp.Error.t

(** Helper function to construct an Lsp.Error. Its goal is to gather
useful information in the optional freeform 'data' field. It assembles
that data out of any data already provided, the provided stack, and the
current stack. A typical scenario is that we got an error marshalled
from a remote server with its remote stack where the error was generated,
and we also want to record the stack where we received it. *)
let make_lsp_error
    ?(data : Hh_json.json option = None)
    ?(stack : string option)
    ?(current_stack : bool = true)
    ?(code : Lsp.Error.code = Lsp.Error.UnknownErrorCode)
    (message : string) : Lsp.Error.t =
  let elems =
    match data with
    | None -> []
    | Some (Hh_json.JSON_Object elems) -> elems
    | Some json -> [("data", json)]
  in
  let elems =
    match stack with
    | Some stack when not (List.Assoc.mem ~equal:String.equal elems "stack") ->
      ("stack", stack |> Exception.clean_stack |> Hh_json.string_) :: elems
    | _ -> elems
  in
  let elems =
    match current_stack with
    | true when not (List.Assoc.mem ~equal:String.equal elems "current_stack")
      ->
      ( "current_stack",
        Exception.get_current_callstack_string 99
        |> Exception.clean_stack
        |> Hh_json.string_ )
      :: elems
    | _ -> elems
  in
  { Lsp.Error.code; message; data = Some (Hh_json.JSON_Object elems) }

(** Use ignore_promise_but_handle_failure when you want don't care about awaiting
results of an async piece of work, but still want any exceptions to be logged.
This is similar to Lwt.async except (1) it logs to our HackEventLogger and
Hh_logger rather than stderr, (2) it you can decide on a case-by-case basis what
should happen to exceptions rather than having them all share the same
Lwt.async_exception_hook, (3) while Lwt.async takes a lambda for creating the
promise and so catches exceptions during promise creation, this function takes
an already-existing promise and so the caller has to handle such exceptions
themselves - I resent using lambdas as a control-flow primitive.

You can think of this function as similar to [ignore], but enhanced because
it's poor practice to ignore a promise. *)
let ignore_promise_but_handle_failure
    ~(desc : string) ~(terminate_on_failure : bool) (promise : unit Lwt.t) :
    unit =
  Lwt.async (fun () ->
      try%lwt
        let%lwt () = promise in
        Lwt.return_unit
      with exn ->
        let open Hh_json in
        let exn = Exception.wrap exn in
        let message = "Unhandled exception: " ^ Exception.get_ctor_string exn in
        let stack =
          Exception.get_backtrace_string exn |> Exception.clean_stack
        in
        let data =
          JSON_Object
            [
              ("description", string_ desc);
              ("message", string_ message);
              ("stack", string_ stack);
            ]
        in
        HackEventLogger.client_lsp_exception
          ~root:(get_root_opt ())
          ~message:"Unhandled exception"
          ~data_opt:(Some data)
          ~source:"lsp_misc";
        log_error "%s\n%s\n%s" message desc stack;
        if terminate_on_failure then
          (* exit 2 is the same as used by Lwt.async *)
          exit 2;

        Lwt.return_unit)

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

(** This conversion is imprecise.  Comments indicate potential gaps *)
let completion_kind_to_si_kind
    (completion_kind : Completion.completionItemKind option) :
    SearchUtils.si_kind =
  let open Lsp in
  let open SearchUtils in
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
  | _ -> SI_Unknown

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
  | SearchUtils.SI_RecordDef -> Some Completion.Struct
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
    | (time, state) :: rest when Float.(time >= new_time -. 120.0) ->
      (time, state) :: retain rest
    | (time, state) :: _rest -> [(time, state)]
    (* retain only the first that's older *)
  in
  hh_server_state_log :=
    match !hh_server_state_log with
    | (prev_time, prev_hh_server_state) :: rest
      when equal_hh_server_state prev_hh_server_state new_hh_server_state ->
      (prev_time, prev_hh_server_state) :: retain rest
    | rest -> (new_time, new_hh_server_state) :: retain rest

let get_older_hh_server_state (requested_time : float) : hh_server_state =
  (* find the first item which is older than the specified time. *)
  match
    List.find !hh_server_state_log ~f:(fun (time, _) ->
        Float.(time <= requested_time))
  with
  | None -> Hh_server_forgot
  | Some (_, hh_server_state) -> hh_server_state

let read_hhconfig_version () : string Lwt.t =
  match get_root_opt () with
  | None -> Lwt.return "[NoRoot]"
  | Some root ->
    let file = Filename.concat (Path.to_string root) ".hhconfig" in
    let%lwt config = Config_file_lwt.parse_hhconfig file in
    (match config with
    | Ok (_hash, config) ->
      let version =
        SMap.find_opt "version" config
        |> Config_file_lwt.parse_version
        |> Config_file_lwt.version_to_string_opt
        |> Option.value ~default:"[NoVersion]"
      in
      Lwt.return version
    | Error message -> Lwt.return (Printf.sprintf "[NoHhconfig:%s]" message))

let read_hhconfig_version_and_switch () : string Lwt.t =
  let%lwt hack_rc_mode_result =
    Lwt_utils.read_all (Sys_utils.expanduser "~/.hack_rc_mode")
  in
  let hack_rc_mode =
    match hack_rc_mode_result with
    | Ok s -> " hack_rc_mode=" ^ s
    | Error _ -> ""
  in
  let hh_home =
    match Sys.getenv_opt "HH_HOME" with
    | Some s -> " HH_HOME=" ^ s
    | None -> ""
  in
  let%lwt hhconfig_version = read_hhconfig_version () in
  Lwt.return (hhconfig_version ^ hack_rc_mode ^ hh_home)

(** get_uris_with_unsaved_changes is the set of files for which we've
received didChange but haven't yet received didSave/didOpen. It is purely
a description of what we've heard of the editor, and is independent of
whether or not they've yet been synced with hh_server.
As it happens: in Main_loop state all these files will already have been
sent to hh_server; in In_init state all these files will have been queued
up inside editor_open_files ready to be sent when we receive the hello; in
Lost_server state they're not even queued up, and if ever we see hh_server
ready then we'll terminate the LSP server and trust the client to relaunch
us and resend a load of didOpen/didChange events. *)
let get_uris_with_unsaved_changes (state : state) : UriSet.t =
  match state with
  | Main_loop menv -> menv.Main_env.uris_with_unsaved_changes
  | In_init ienv -> ienv.In_init_env.uris_with_unsaved_changes
  | Lost_server lenv -> lenv.Lost_env.uris_with_unsaved_changes
  | _ -> UriSet.empty

let update_hh_server_state_if_necessary (event : event) : unit =
  let open ServerCommandTypes in
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
  | _ -> ()

(** This cancellable async function will block indefinitely until a notification is
available from ide_service. *)
let pop_from_ide_service (ide_service : ClientIdeService.t ref option) :
    event Lwt.t =
  match ide_service with
  | None -> Lwt.wait () |> fst (* a never-fulfilled promise *)
  | Some ide_service ->
    let%lwt notification_opt =
      Lwt_message_queue.pop (ClientIdeService.get_notifications !ide_service)
    in
    (match notification_opt with
    | None -> Lwt.wait () |> fst (* a never-fulfilled promise *)
    | Some notification -> Lwt.return (Client_ide_notification notification))

(** Determine whether to read a message from the client (the editor) or the
server (hh_server), or whether neither is ready within 1s. *)
let get_message_source
    (server : server_conn)
    (client : Jsonrpc.queue)
    (ide_service : ClientIdeService.t ref option) :
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
          (let%lwt () = Lwt_unix.wait_read server_read_fd in
           Lwt.return `From_server);
          (let%lwt () = Lwt_unix.wait_read client_read_fd in
           Lwt.return `From_client);
          (let%lwt notification = pop_from_ide_service ide_service in
           Lwt.return (`From_ide_service notification));
        ]
    in
    Lwt.return message_source

(** A simplified version of get_message_source which only looks at client *)
let get_client_message_source
    (client : Jsonrpc.queue) (ide_service : ClientIdeService.t ref option) :
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
          (let%lwt notification = pop_from_ide_service ide_service in
           Lwt.return (`From_ide_service notification));
        ]
    in
    Lwt.return message_source

(**  Read a message unmarshaled from the server's out_channel. *)
let read_message_from_server (server : server_conn) : event Lwt.t =
  let open ServerCommandTypes in
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
    raise (Server_fatal_connection_exception { Marshal_tools.message; stack })

(** get_next_event: picks up the next available message from either client or
server. The way it's implemented, at the first character of a message
from either client or server, we block until that message is completely
received. Note: if server is None (meaning we haven't yet established
connection with server) then we'll just block waiting for client. *)
let get_next_event
    (state : state)
    (client : Jsonrpc.queue)
    (ide_service : ClientIdeService.t ref option) : event Lwt.t =
  let from_server (server : server_conn) : event Lwt.t =
    if Queue.is_empty server.pending_messages then
      read_message_from_server server
    else
      Lwt.return (Server_message (Queue.dequeue_exn server.pending_messages))
  in
  let from_client (client : Jsonrpc.queue) : event Lwt.t =
    let%lwt message = Jsonrpc.get_message client in
    match message with
    | `Message { Jsonrpc.json; timestamp } ->
      begin
        try
          let message = Lsp_fmt.parse_lsp json get_outstanding_request_exn in
          let rnd = Random_id.short_string () in
          let tracking_id =
            match message with
            | RequestMessage (id, _) -> rnd ^ "." ^ Lsp_fmt.id_to_string id
            | _ -> rnd
          in
          Lwt.return (Client_message ({ tracking_id; timestamp }, message))
        with e ->
          let e = Exception.wrap e in
          let edata =
            {
              Marshal_tools.stack = Exception.get_backtrace_string e;
              message = Exception.get_ctor_string e;
            }
          in
          raise (Client_recoverable_connection_exception edata)
      end
    | `Fatal_exception edata -> raise (Client_fatal_connection_exception edata)
    | `Recoverable_exception edata ->
      raise (Client_recoverable_connection_exception edata)
  in
  match state with
  | Main_loop { Main_env.conn; _ }
  | In_init { In_init_env.conn; _ } ->
    let%lwt message_source = get_message_source conn client ide_service in
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

let add_powered_by ~(powered_by : powered_by) (json : Hh_json.json) :
    Hh_json.json =
  let open Hh_json in
  match (json, powered_by) with
  | (JSON_Object props, Serverless_ide) ->
    JSON_Object (("powered_by", JSON_String "serverless_ide") :: props)
  | (_, _) -> json

let respond_jsonrpc
    ~(powered_by : powered_by) (id : lsp_id) (result : lsp_result) : unit =
  print_lsp_response id result |> add_powered_by ~powered_by |> to_stdout

let notify_jsonrpc ~(powered_by : powered_by) (notification : lsp_notification)
    : unit =
  print_lsp_notification notification |> add_powered_by ~powered_by |> to_stdout

(** respond_to_error: if we threw an exception during the handling of a request,
report the exception to the client as the response to their request. *)
let respond_to_error (event : event option) (e : Lsp.Error.t) : unit =
  let result = ErrorResult e in
  match event with
  | Some (Client_message (_, RequestMessage (id, _request))) ->
    respond_jsonrpc ~powered_by:Language_server id result
  | _ ->
    (* We want to report LSP error 'e' over jsonrpc. But jsonrpc only allows
    errors to be reported in response to requests. So we'll stick the information
    in a telemetry/event. The format of this event isn't defined. We're going to
    roll our own, using ad-hoc json fields to emit all the data out of 'e' *)
    let open Lsp.Error in
    let extras =
      ("code", e.code |> Error.show_code |> Hh_json.string_)
      :: Option.value_map e.data ~default:[] ~f:(fun data -> [("data", data)])
    in
    Lsp_helpers.telemetry_error to_stdout e.message ~extras

(** request_showStatusFB: pops up a dialog *)
let request_showStatusFB
    ?(on_result : ShowStatusFB.result -> state -> state Lwt.t =
      (fun _ state -> Lwt.return state))
    ?(on_error : Error.t -> state -> state Lwt.t =
      (fun _ state -> Lwt.return state))
    (params : ShowStatusFB.params) : unit =
  let initialize_params = initialize_params_exc () in
  if not (Lsp_helpers.supports_status initialize_params) then
    ()
  else
    (* We try not to send duplicate statuses.
    That means: if you call request_showStatus but your message is the same as
    what's already up, then you won't be shown, and your callbacks won't be shown. *)
    let msg = params.ShowStatusFB.request.ShowMessageRequest.message in
    if String.equal msg !showStatus_outstanding then
      ()
    else (
      showStatus_outstanding := msg;
      let id = NumberId (Jsonrpc.get_next_request_id ()) in
      let request = ShowStatusRequestFB params in
      to_stdout (print_lsp_request id request);

      let handler (result : lsp_result) (state : state) : state Lwt.t =
        if String.equal msg !showStatus_outstanding then
          showStatus_outstanding := "";
        match result with
        | ShowStatusResultFB result -> on_result result state
        | ErrorResult error -> on_error error state
        | _ ->
          let error =
            {
              Error.code = Error.ParseError;
              message = "expected ShowStatusResult";
              data = None;
            }
          in
          on_error error state
      in
      requests_outstanding :=
        IdMap.add id (request, handler) !requests_outstanding
    )

(** request_showMessage: pops up a dialog *)
let request_showMessage
    (on_result : ShowMessageRequest.result -> state -> state Lwt.t)
    (on_error : Error.t -> state -> state Lwt.t)
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
  to_stdout (print_lsp_request id request);

  let handler (result : lsp_result) (state : state) : state Lwt.t =
    match result with
    | ShowMessageRequestResult result -> on_result result state
    | ErrorResult error -> on_error error state
    | _ ->
      let error =
        {
          Error.code = Error.ParseError;
          message = "expected ShowMessageRequestResult";
          data = None;
        }
      in
      on_error error state
  in
  requests_outstanding := IdMap.add id (request, handler) !requests_outstanding;

  (* return a token *)
  ShowMessageRequest.Present { id }

(** dismiss_showMessageRequest: sends a cancellation-request for the dialog *)
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

(** These functions are not currently used, but may be useful in the future. *)
let (_ : 'a -> 'b) = request_showMessage

and (_ : 'c -> 'd) = dismiss_showMessageRequest

(** dismiss_diagnostics: dismisses all diagnostics from a state,
both the error diagnostics in Main_loop and the hh_server_status
diagnostics in In_init and Lost_server. *)
let dismiss_diagnostics (state : state) : state =
  let dismiss_one ~isStatusFB uri =
    let params = { PublishDiagnostics.uri; diagnostics = []; isStatusFB } in
    let notification = PublishDiagnosticsNotification params in
    notification |> print_lsp_notification |> to_stdout
  in
  let dismiss_status diagnostic =
    dismiss_one ~isStatusFB:true diagnostic.PublishDiagnostics.uri
  in
  match state with
  | In_init ienv ->
    let open In_init_env in
    Option.iter ienv.hh_server_status_diagnostic ~f:dismiss_status;
    In_init { ienv with hh_server_status_diagnostic = None }
  | Main_loop menv ->
    let open Main_env in
    UriSet.iter (dismiss_one ~isStatusFB:false) menv.uris_with_diagnostics;
    Main_loop { menv with uris_with_diagnostics = UriSet.empty }
  | Lost_server lenv ->
    let open Lost_env in
    Option.iter lenv.hh_server_status_diagnostic ~f:dismiss_status;
    Lost_server { lenv with hh_server_status_diagnostic = None }
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
  let open Lsp.TextDocumentPositionParams in
  let { Ide_api_types.line; column } = lsp_position_to_ide params.position in
  let filename =
    Lsp_helpers.lsp_textDocumentIdentifier_to_filename params.textDocument
  in
  (filename, line, column)

let rename_params_to_document_position (params : Lsp.Rename.params) :
    Lsp.TextDocumentPositionParams.t =
  Rename.
    {
      TextDocumentPositionParams.textDocument = params.textDocument;
      position = params.position;
    }

let hack_pos_to_lsp_range ~(equal : 'a -> 'a -> bool) (pos : 'a Pos.pos) :
    Lsp.range =
  (* .hhconfig errors are Positions with a filename, but dummy start/end
   * positions. Handle that case - and Pos.none - specially, as the LSP
   * specification requires line and character >= 0, and VSCode silently
   * drops diagnostics that violate the spec in this way *)
  if Pos.equal_pos equal pos (Pos.make_from (Pos.filename pos)) then
    { start = { line = 0; character = 0 }; end_ = { line = 0; character = 0 } }
  else
    let (line1, col1, line2, col2) = Pos.destruct_range pos in
    {
      start = { line = line1 - 1; character = col1 - 1 };
      end_ = { line = line2 - 1; character = col2 - 1 };
    }

let hack_pos_to_lsp_location (pos : Pos.absolute) ~(default_path : string) :
    Lsp.Location.t =
  Lsp.Location.
    {
      uri = path_to_lsp_uri (Pos.filename pos) ~default_path;
      range = hack_pos_to_lsp_range ~equal:String.equal pos;
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
  let open SymbolDefinition in
  hack_pos_to_lsp_location symbol.span ~default_path

let hack_pos_definition_to_lsp_identifier_location
    (sid : Pos.absolute * string) ~(default_path : string) :
    Lsp.DefinitionLocation.t =
  let (pos, title) = sid in
  let location = hack_pos_to_lsp_location pos ~default_path in
  Lsp.DefinitionLocation.{ location; title = Some title }

let hack_symbol_definition_to_lsp_identifier_location
    (symbol : string SymbolDefinition.t) ~(default_path : string) :
    Lsp.DefinitionLocation.t =
  let open SymbolDefinition in
  let location = hack_pos_to_lsp_location symbol.pos ~default_path in
  Lsp.DefinitionLocation.
    {
      location;
      title = Some (Utils.strip_ns symbol.SymbolDefinition.full_name);
    }

let hack_errors_to_lsp_diagnostic
    (filename : string) (errors : Pos.absolute Errors.error_ list) :
    PublishDiagnostics.params =
  let open Lsp.Location in
  let location_message (error : Pos.absolute * string) : Lsp.Location.t * string
      =
    let (pos, message) = error in
    let { uri; range } = hack_pos_to_lsp_location pos ~default_path:filename in
    ({ Location.uri; range }, Markdown_lite.render message)
  in
  let hack_error_to_lsp_diagnostic (error : Pos.absolute Errors.error_) =
    let all_messages = Errors.to_list error |> List.map ~f:location_message in
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
    isStatusFB = false;
    diagnostics = List.map errors ~f:hack_error_to_lsp_diagnostic;
  }

let get_document_contents
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t) (uri : documentUri) :
    string option =
  match UriMap.find_opt uri editor_open_files with
  | Some document -> Some document.TextDocumentItem.text
  | None -> None

let get_document_location
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : Lsp.TextDocumentPositionParams.t) :
    ClientIdeMessage.document_location =
  let (file_path, line, column) = lsp_file_position_to_hack params in
  let uri =
    params.TextDocumentPositionParams.textDocument.TextDocumentIdentifier.uri
  in
  let file_path = Path.make file_path in
  let file_contents = get_document_contents editor_open_files uri in
  { ClientIdeMessage.file_path; file_contents; line; column }

(************************************************************************)
(* Connection and rpc                                                   *)
(************************************************************************)

let start_server ~(env : env) (root : Path.t) : unit =
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
      from = !from;
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
      config = env.config;
      custom_telemetry_data = [];
      allow_non_opt_build = false;
    }
  in
  let _exit_status = ClientStart.main env_start in
  ()

let rec connect_client ~(env : env) (root : Path.t) ~(autostart : bool) :
    server_conn Lwt.t =
  log "connect_client";
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
        from = !from;
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
        progress_callback = None;
        (* we're fast! *)
        do_post_handoff_handshake = false;
        ignore_hh_version = false;
        saved_state_ignore_hhconfig = false;
        (* priority_pipe delivers good experience for hh_server, but has a bug,
        and doesn't provide benefits in serverless-ide. *)
        use_priority_pipe = not env.use_serverless_ide;
        prechecked = None;
        config = env.config;
        custom_telemetry_data = [];
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
      log "connect_client: build_id_mismatch";
      can_autostart_after_mismatch := false;
      connect_client ~env root ~autostart:true)

(* connect: this method either connects to the monitor and leaves in an       *)
(* In_init state waiting for the server hello, or it fails to connect and     *)
(* leaves in a Lost_server state. You might call this from Pre_init or        *)
(* Lost_server states, obviously. But you can also call it from In_init state *)
(* if you want to give up on the prior attempt at connection and try again.   *)
let rec connect ~(env : env) (state : state) : state Lwt.t =
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
    let%lwt conn = connect_client ~env (get_root_exn ()) ~autostart:false in
    set_hh_server_state Hh_server_initializing;
    match state with
    | In_init ienv ->
      Lwt.return
        (In_init
           { ienv with In_init_env.conn; most_recent_start_time = Unix.time () })
    | _ ->
      let state = dismiss_diagnostics state in
      Lwt.return
        (In_init
           {
             In_init_env.conn;
             first_start_time = Unix.time ();
             most_recent_start_time = Unix.time ();
             most_recent_file = get_most_recent_file state;
             editor_open_files =
               Option.value (get_editor_open_files state) ~default:UriMap.empty;
             (* uris_with_unsaved_changes should always be empty here: *)
             (* Pre_init will of course be empty; *)
             (* Lost_server will exit rather than reconnect with unsaved changes. *)
             uris_with_unsaved_changes = get_uris_with_unsaved_changes state;
             hh_server_status_diagnostic = None;
           })
  with exn ->
    let exn = Exception.wrap exn in
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
    let message =
      match Exception.unwrap exn with
      | Exit_status.Exit_with code -> Exit_status.show code
      | _ -> Exception.get_ctor_string exn
    in
    let longMessage =
      Printf.sprintf
        "connect failed: %s\n%s"
        message
        (Exception.get_backtrace_string exn |> Exception.clean_stack)
    in
    let () = Lsp_helpers.telemetry_error to_stdout longMessage in
    let open Exit_status in
    let new_hh_server_state =
      match Exception.unwrap exn with
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
      match Exception.unwrap exn with
      | Exit_with Out_of_retries
      | Exit_with Out_of_time ->
        "hh_server is waiting for things to settle"
      | Exit_with No_server_running_should_retry -> "hh_server: stopped."
      | _ -> "hh_server: " ^ message
    in
    let%lwt state =
      do_lost_server
        state
        ~allow_immediate_reconnect:false
        ~env
        {
          Lost_env.explanation;
          new_hh_server_state;
          start_on_click = true;
          trigger_on_lock_file = true;
          trigger_on_lsp = false;
        }
    in
    Lwt.return state

and reconnect_from_lost_if_necessary
    ~(env : env) (state : state) (reason : [> `Event of event | `Force_regain ])
    : state Lwt.t =
  Lost_env.(
    let should_reconnect =
      match (state, reason) with
      | (Lost_server _, `Force_regain) -> true
      | ( Lost_server { p = { trigger_on_lsp = true; _ }; _ },
          `Event
            (Client_message (_, (RequestMessage _ | NotificationMessage _))) )
        ->
        true
      | ( Lost_server { p = { trigger_on_lock_file = true; _ }; lock_file; _ },
          `Event Tick ) ->
        MonitorConnection.server_exists lock_file
      | (_, _) -> false
    in
    if should_reconnect then
      let%lwt current_version_and_switch =
        read_hhconfig_version_and_switch ()
      in
      let needs_to_terminate =
        not
          (String.equal !hhconfig_version_and_switch current_version_and_switch)
      in
      if needs_to_terminate then (
        (* In these cases we have to terminate our LSP server, and trust the    *)
        (* client to restart us. Note that we can't do clientStart because that *)
        (* would start our (old) version of hh_server, not the new one!         *)
        let unsaved = get_uris_with_unsaved_changes state |> UriSet.elements in
        let unsaved_str =
          if List.is_empty unsaved then
            "[None]"
          else
            unsaved |> List.map ~f:string_of_uri |> String.concat ~sep:"\n"
        in
        let message =
          "Unsaved files:\n"
          ^ unsaved_str
          ^ "\nVersion in hhconfig and switch that spawned the current hh_client: "
          ^ !hhconfig_version_and_switch
          ^ "\nVersion in hhconfig and switch currently: "
          ^ current_version_and_switch
          ^ "\n"
        in
        Lsp_helpers.telemetry_log to_stdout message;
        exit_fail ()
      ) else
        let%lwt state = connect ~env state in
        Lwt.return state
    else
      Lwt.return state)

(* do_lost_server: handles the various ways we might lose hh_server. We keep  *)
(* the LSP server alive, and will (elsewhere) listen for the various triggers *)
(* of getting the server back.                                                *)
and do_lost_server
    (state : state)
    ~(env : env)
    ?(allow_immediate_reconnect = true)
    (p : Lost_env.params) : state Lwt.t =
  Lost_env.(
    set_hh_server_state p.new_hh_server_state;

    let state = dismiss_diagnostics state in
    let uris_with_unsaved_changes = get_uris_with_unsaved_changes state in
    let most_recent_file = get_most_recent_file state in
    let editor_open_files =
      Option.value (get_editor_open_files state) ~default:UriMap.empty
    in
    let lock_file = ServerFiles.lock_file (get_root_exn ()) in
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
            most_recent_file;
            editor_open_files;
            uris_with_unsaved_changes;
            lock_file;
            hh_server_status_diagnostic = None;
          }
      in
      Lsp_helpers.telemetry_log
        to_stdout
        "Reconnecting immediately to hh_server";
      let%lwt new_state =
        reconnect_from_lost_if_necessary ~env lost_state `Force_regain
      in
      Lwt.return new_state
    ) else
      Lwt.return
        (Lost_server
           {
             Lost_env.p;
             most_recent_file;
             editor_open_files;
             uris_with_unsaved_changes;
             lock_file;
             hh_server_status_diagnostic = None;
           }))

let report_connect_end (ienv : In_init_env.t) : state =
  log "report_connect_end";
  In_init_env.(
    let _state = dismiss_diagnostics (In_init ienv) in
    let menv =
      {
        Main_env.conn = ienv.In_init_env.conn;
        needs_idle = true;
        most_recent_file = ienv.most_recent_file;
        editor_open_files = ienv.editor_open_files;
        uris_with_diagnostics = UriSet.empty;
        uris_with_unsaved_changes = ienv.In_init_env.uris_with_unsaved_changes;
        hh_server_status =
          {
            ShowStatusFB.request =
              {
                ShowMessageRequest.type_ = MessageType.InfoMessage;
                message = "hh_server: ready.";
                actions = [];
              };
            progress = None;
            total = None;
            shortMessage = None;
          };
      }
    in
    Main_loop menv)

let announce_ide_failure (error_data : ClientIdeMessage.stopped_reason) :
    unit Lwt.t =
  let open ClientIdeMessage in
  log
    "IDE services could not be initialized.\n%s\n%s"
    error_data.long_user_message
    error_data.debug_details;

  let input =
    Printf.sprintf
      "%s\n\n%s"
      error_data.long_user_message
      error_data.debug_details
  in
  let%lwt upload_result =
    Clowder_paste.clowder_upload_and_get_url ~timeout:10. input
  in
  let append_to_log =
    match upload_result with
    | Ok url -> Printf.sprintf "\nMore details: %s" url
    | Error message ->
      Printf.sprintf
        "\n\nMore details:\n%s\n\nTried to upload those details but it didn't work...\n%s"
        error_data.debug_details
        message
  in
  Lsp_helpers.log_error to_stdout (error_data.long_user_message ^ append_to_log);
  if error_data.is_actionable then
    Lsp_helpers.showMessage_error
      to_stdout
      (error_data.medium_user_message ^ see_output_hack);
  Lwt.return_unit

(** Like all async methods, this method has a synchronous preamble up
to its first await point, at which point it returns a promise to its
caller; the rest of the method will be scheduled asynchronously.
The synchronous preamble sends an "initialize" request to the ide_service.
The asynchronous continuation is triggered when the response comes back;
it then pumps messages to and from the ide service.
Note: the fact that the request is sent in the synchronous preamble, is
important for correctness - the rest of the codebase can send other requests
to the ide_service at any time, safe in the knowledge that such requests will
necessarily be delivered after the initialize request. *)
let run_ide_service
    (env : env)
    (ide_service : ClientIdeService.t)
    (initialize_params : Lsp.Initialize.params)
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t option) : unit Lwt.t =
  let open Lsp.Initialize in
  let root = Some (Lsp_helpers.get_root initialize_params) |> Wwwroot.get in
  if
    not
      initialize_params.client_capabilities.workspace.didChangeWatchedFiles
        .dynamicRegistration
  then
    log_error "client doesn't support file-watching";

  let naming_table_load_info =
    match initialize_params.initializationOptions.namingTableSavedStatePath with
    | None -> None
    | Some path ->
      Some
        {
          ClientIdeMessage.Initialize_from_saved_state.path = Path.make path;
          test_delay =
            initialize_params.initializationOptions
              .namingTableSavedStateTestDelay;
        }
  in
  let open_files =
    editor_open_files
    |> Option.value ~default:UriMap.empty
    |> UriMap.keys
    |> List.map ~f:(fun uri -> uri |> lsp_uri_to_path |> Path.make)
  in
  log_debug "initialize_from_saved_state";
  let%lwt result =
    ClientIdeService.initialize_from_saved_state
      ide_service
      ~root
      ~naming_table_load_info
      ~use_ranked_autocomplete:env.use_ranked_autocomplete
      ~config:env.config
      ~open_files
  in
  log_debug "initialize_from_saved_state.done";
  match result with
  | Ok () ->
    let%lwt () = ClientIdeService.serve ide_service in
    Lwt.return_unit
  | Error error_data ->
    let%lwt () = announce_ide_failure error_data in
    Lwt.return_unit

let stop_ide_service
    (ide_service : ClientIdeService.t)
    ~(tracking_id : string)
    ~(stop_reason : ClientIdeService.Stop_reason.t) : unit Lwt.t =
  log
    "Stopping IDE service process: %s"
    (ClientIdeService.Stop_reason.to_string stop_reason);
  let%lwt () =
    ClientIdeService.stop ide_service ~tracking_id ~stop_reason ~exn:None
  in
  Lwt.return_unit

let on_status_restart_action
    ~(env : env)
    ~(ide_service : ClientIdeService.t ref option)
    (result : ShowStatusFB.result)
    (state : state) : state Lwt.t =
  let open ShowMessageRequest in
  match (result, state, ide_service) with
  | (Some { title }, Lost_server _, _)
    when String.equal title hh_server_restart_button_text ->
    let root = get_root_exn () in
    (* Belt-and-braces kill the server. This is in case the server was *)
    (* stuck in some weird state. It's also what 'hh restart' does. *)
    if MonitorConnection.server_exists (Path.to_string root) then
      ClientStop.kill_server root !from;

    (* After that it's safe to try to reconnect! *)
    start_server ~env root;
    let%lwt state = reconnect_from_lost_if_necessary ~env state `Force_regain in
    Lwt.return state
  | (Some { title }, _, Some ide_service)
    when String.equal title client_ide_restart_button_text ->
    log "Restarting IDE service";

    (* It's possible that [destroy] takes a while to finish, so make
    sure to assign the new IDE service to the [ref] before attempting
    to do an asynchronous operation with the old one. *)
    let ide_args =
      {
        ClientIdeMessage.init_id = env.init_id;
        verbose_to_stderr = env.verbose;
        verbose_to_file = !verbose_to_file;
      }
    in
    let new_ide_service = ClientIdeService.make ide_args in
    let old_ide_service = !ide_service in
    ide_service := new_ide_service;
    (* Note: the env.verbose passed on init controls verbosity for stderr
    and is only ever controlled by --verbose command line, stored in env.
    But verbosity-to-file can be altered dynamically by the user. *)
    let (promise : unit Lwt.t) =
      run_ide_service
        env
        new_ide_service
        (initialize_params_exc ())
        (get_editor_open_files state)
    in
    ignore_promise_but_handle_failure
      promise
      ~desc:"run-ide-after-restart"
      ~terminate_on_failure:true;
    (* Invariant: at all times after InitializeRequest, ide_service has
    already been sent an "initialize" message. *)
    let%lwt () =
      stop_ide_service
        old_ide_service
        ~tracking_id:"restart"
        ~stop_reason:ClientIdeService.Stop_reason.Restarting
    in
    Lwt.return state
  | _ -> Lwt.return state

let get_client_ide_status (ide_service : ClientIdeService.t) :
    ShowStatusFB.params option =
  let (type_, shortMessage, message, actions) =
    match ClientIdeService.get_status ide_service with
    | ClientIdeService.Status.Initializing ->
      ( MessageType.WarningMessage,
        "Hack: initializing",
        "Hack IDE: initializing.",
        [] )
    | ClientIdeService.Status.Processing_files p ->
      let open ClientIdeMessage.Processing_files in
      ( MessageType.WarningMessage,
        "Hack",
        Printf.sprintf "Hack IDE: processing %d files." p.total,
        [] )
    | ClientIdeService.Status.Rpc ->
      (MessageType.WarningMessage, "Hack", "Hack IDE: working...", [])
    | ClientIdeService.Status.Ready ->
      (MessageType.InfoMessage, "Hack: ready", "Hack IDE: ready.", [])
    | ClientIdeService.Status.Stopped s ->
      let open ClientIdeMessage in
      ( MessageType.ErrorMessage,
        "Hack: " ^ s.short_user_message,
        s.medium_user_message ^ see_output_hack,
        [{ ShowMessageRequest.title = client_ide_restart_button_text }] )
  in
  Some
    {
      ShowStatusFB.shortMessage = Some shortMessage;
      request = { ShowMessageRequest.type_; message; actions };
      progress = None;
      total = None;
    }

(** This function blocks while it attempts to connect to the monitor to read status.
It normally it gets status quickly, but has a 3s timeout just in case. *)
let get_hh_server_status (state : state) : ShowStatusFB.params option =
  let open ShowStatusFB in
  let open ShowMessageRequest in
  match state with
  | Pre_init
  | Post_shutdown ->
    None
  | In_init ienv ->
    let open In_init_env in
    let time = Unix.time () in
    let delay_in_secs =
      if Sys_utils.is_test_mode () then
        (* we avoid raciness in our tests by not showing a real time *)
        "<test>"
      else
        int_of_float (time -. ienv.first_start_time) |> string_of_int
    in
    (* TODO: better to report time that hh_server has spent initializing *)
    let tracker = Connection_tracker.create () in
    let (progress, warning) =
      match
        ServerUtils.server_progress ~tracker ~timeout:3 (get_root_exn ())
      with
      | Error _ -> ("connecting", None)
      | Ok (progress, warning) -> (progress, warning)
    in
    (* [progress] comes from ServerProgress.ml, sent to the monitor, and now we've fetched
    it from the monitor. It's a string "op X/Y units (%)" e.g. "typechecking 5/16 files (78%)",
    or "connecting", if there is no relevant progress to show.
    [warning] comes from the same place, and if pressent is a human-readable string
    that warns about saved-state-init failure. *)
    let warning =
      if Option.is_some warning then
        " (saved-state not found - will take a while)"
      else
        ""
    in
    let message =
      Printf.sprintf
        "hh_server initializing%s: %s [%s seconds]"
        warning
        progress
        delay_in_secs
    in
    Some
      {
        request = { type_ = MessageType.WarningMessage; message; actions = [] };
        progress = None;
        total = None;
        shortMessage = Some "Hack: initializing";
      }
  | Main_loop { Main_env.hh_server_status; _ } ->
    (* This shows whether the connected hh_server is busy or ready.
    It's produced in clientLsp.do_server_busy upon receipt of a status
    enum from the server. See comments on hh_server_status for invariants. *)
    Some hh_server_status
  | Lost_server { Lost_env.p; _ } ->
    Some
      {
        shortMessage = Some "Hack: stopped";
        request =
          {
            type_ = MessageType.ErrorMessage;
            message = p.Lost_env.explanation;
            actions = [{ title = hh_server_restart_button_text }];
          };
        progress = None;
        total = None;
      }

let hh_server_status_to_diagnostic
    (uri : documentUri option) (hh_server_status : ShowStatusFB.params) :
    PublishDiagnostics.params option =
  let open ShowStatusFB in
  let open ShowMessageRequest in
  let open PublishDiagnostics in
  let diagnostic =
    {
      PublishDiagnostics.range =
        {
          start = { line = 0; character = 0 };
          end_ = { line = 0; character = 1 };
        };
      severity = None;
      code = NoCode;
      source = Some "hh_server";
      message = "";
      relatedInformation = [];
      relatedLocations = [];
    }
  in
  match (uri, hh_server_status.request.type_) with
  | (None, _)
  | (_, (MessageType.InfoMessage | MessageType.LogMessage)) ->
    None
  | (Some uri, MessageType.ErrorMessage) ->
    Some
      {
        uri;
        isStatusFB = true;
        diagnostics =
          [
            {
              diagnostic with
              message =
                "hh_server isn't running, so there may be undetected errors. Try `hh` at the command line... "
                ^ hh_server_status.request.message;
              severity = Some Error;
            };
          ];
      }
  | (Some uri, MessageType.WarningMessage) ->
    Some
      {
        uri;
        isStatusFB = true;
        diagnostics =
          [
            {
              diagnostic with
              message =
                "hh_server isn't yet ready, so there may be undetected errors... "
                ^ hh_server_status.request.message;
              severity = Some Warning;
            };
          ];
      }

(** Manages the state of which diagnostics have been shown to the user
about hh_server status: removes the old one if necessary, and adds a new one
if necessary. Note that we only display hh_server_status diagnostics
during In_init and Lost_server states, neither of which have diagnostics
of their own. *)
let publish_hh_server_status_diagnostic
    (state : state) (hh_server_status : ShowStatusFB.params option) : state =
  let uri =
    match (get_most_recent_file state, get_editor_open_files state) with
    | (Some uri, Some open_files) when UriMap.mem uri open_files -> Some uri
    | (_, Some open_files) when not (UriMap.is_empty open_files) ->
      Some (UriMap.choose open_files |> fst)
    | (_, _) -> None
  in
  let desired_diagnostic =
    Option.bind hh_server_status ~f:(hh_server_status_to_diagnostic uri)
  in
  let get_existing_diagnostic state =
    match state with
    | In_init ienv -> ienv.In_init_env.hh_server_status_diagnostic
    | Lost_server lenv -> lenv.Lost_env.hh_server_status_diagnostic
    | _ -> None
  in
  let publish_and_update_diagnostic state diagnostic =
    let notification = PublishDiagnosticsNotification diagnostic in
    notification |> print_lsp_notification |> to_stdout;
    match state with
    | In_init ienv ->
      In_init
        { ienv with In_init_env.hh_server_status_diagnostic = Some diagnostic }
    | Lost_server lenv ->
      Lost_server
        { lenv with Lost_env.hh_server_status_diagnostic = Some diagnostic }
    | _ -> state
  in
  let open PublishDiagnostics in
  (* The following match emboodies these rules:
  (1) we only publish hh_server_status diagnostics in In_init and Lost_server states,
  (2) we'll remove the old PublishDiagnostic if necessary and add a new one if necessary
  (3) to avoid extra LSP messages, if the diagnostic hasn't changed then we won't send anything
  (4) to avoid flicker, if the diagnostic has changed but is still in the same file, then
  we refrain from sending an "erase old" message and it will be implied by sending "new". *)
  match (get_existing_diagnostic state, desired_diagnostic, state) with
  | (_, _, Main_loop _)
  | (_, _, Pre_init)
  | (_, _, Post_shutdown)
  | (None, None, _) ->
    state
  | (Some _, None, _) -> dismiss_diagnostics state
  | (Some existing, Some desired, _)
    when Lsp.equal_documentUri existing.uri desired.uri
         && Option.equal
              PublishDiagnostics.equal_diagnostic
              (List.hd existing.diagnostics)
              (List.hd desired.diagnostics) ->
    state
  | (Some existing, Some desired, _)
    when Lsp.equal_documentUri existing.uri desired.uri ->
    publish_and_update_diagnostic state desired
  | (Some _, Some desired, _) ->
    let state = dismiss_diagnostics state in
    publish_and_update_diagnostic state desired
  | (None, Some desired, _) -> publish_and_update_diagnostic state desired

(** Here are the rules for merging status. They embody the principle that the spinner
shows if initializing/typechecking is in progress, the error icon shows if error,
and the status bar word is "Hack" if IDE services are available or "Hack: xyz" if not.
Note that if Hack IDE is up but hh_server is down, then the hh_server failure message
is conveyed via a publishDiagnostic; it's not conveyed via status.
  [ok] Hack -- if ide_service is up and hh_server is ready
  [spin] Hack -- if ide_service is processing-files or hh_server is initializing/typechecking
  [spin] Hack: initializing -- if ide_service is initializing
  [err] Hack: failure -- if ide_service is down
If client_ide_service isn't enabled, then we show thing differently:
  [ok] Hack  -- if hh_server is ready (Main_loop)
  [spin] Hack -- if hh_server is doing local or global typechecks (Main_loop)
  [spin] Hack: busy -- if hh_server is doing non-interruptible typechecks (Main_loop)
  [spin] Hack: initializing -- if hh_server is initializing (In_init)
  [err] hh_server: stopped -- hh_server is down (Lost_server)
As for the tooltip and actions, they are combined from both ide_service and hh_server. *)
let merge_statuses
    ~(client_ide_status : ShowStatusFB.params option)
    ~(hh_server_status : ShowStatusFB.params option) :
    ShowStatusFB.params option =
  (* The correctness of the following match is a bit subtle. This is how to think of it.
  From the spec in the docblock, (1) if there's no client_ide_service, then the result
  of this function is simply the same as hh_server_status, since that's how it was constructed
  by get_hh_server_status (for In_init and Lost_server) and do_server_busy; (2) if there
  is a client_ide_service then the result is almost always simply the same as ide_service
  since that's how it was constructed by get_client_ide_status; (3) the only exception to
  rule 2 is that, if client_ide_status would have shown "[ok] Hack" and hh_server_status
  would have been a spinner, then we change to "[spin] Hack". *)
  match (client_ide_status, hh_server_status) with
  | (None, None) -> None
  | (None, Some _) -> hh_server_status
  | (Some _, None) -> client_ide_status
  | (Some client_ide_status, Some hh_server_status) ->
    let open Lsp.ShowStatusFB in
    let open Lsp.ShowMessageRequest in
    let request =
      {
        client_ide_status.request with
        message =
          client_ide_status.request.message
          ^ "\n"
          ^ hh_server_status.request.message;
        actions =
          client_ide_status.request.actions @ hh_server_status.request.actions;
      }
    in
    if
      MessageType.equal client_ide_status.request.type_ MessageType.InfoMessage
      && MessageType.equal
           hh_server_status.request.type_
           MessageType.WarningMessage
    then
      let request = { request with type_ = MessageType.WarningMessage } in
      Some { client_ide_status with request; shortMessage = Some "Hack" }
    else
      Some { client_ide_status with request }

let refresh_status ~(env : env) ~(ide_service : ClientIdeService.t ref option) :
    unit =
  let client_ide_status =
    match ide_service with
    | None -> None
    | Some ide_service -> get_client_ide_status !ide_service
  in
  let status =
    merge_statuses ~hh_server_status:!latest_hh_server_status ~client_ide_status
  in
  Option.iter
    status
    ~f:
      (request_showStatusFB
         ~on_result:(on_status_restart_action ~env ~ide_service));
  ()

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
        let start_time = Unix.gettimeofday () in
        let%lwt result =
          ServerCommandLwt.rpc_persistent
            (server_conn.ic, server_conn.oc)
            ()
            callback
            command
        in
        let end_time = Unix.gettimeofday () in
        let duration = end_time -. start_time in
        let msg = ServerCommandTypesUtils.debug_describe_t command in
        log_debug "hh_server rpc: [%s] [%0.3f]" msg duration;
        match result with
        | Ok ((), res, tracker) ->
          ref_unblocked_time :=
            Connection_tracker.get_server_unblocked_time tracker;
          Lwt.return res
        | Error
            ( (),
              Utils.Callstack _,
              ServerCommandLwt.Remote_fatal_exception remote_e_data ) ->
          raise (Server_fatal_connection_exception remote_e_data)
        | Error
            ( (),
              Utils.Callstack _,
              ServerCommandLwt.Remote_nonfatal_exception
                { Marshal_tools.message; stack } ) ->
          raise (Server_nonfatal_exception (make_lsp_error message ~stack))
        | Error ((), Utils.Callstack stack, e) ->
          let message = Exn.to_string e in
          raise
            (Server_fatal_connection_exception { Marshal_tools.message; stack }))
  in
  Lwt.return result

let rpc_with_retry server_conn ref_unblocked_time command =
  ServerCommandTypes.Done_or_retry.call ~f:(fun () ->
      rpc server_conn ref_unblocked_time command)

(** A thin wrapper around ClientIdeMessage which turns errors into exceptions *)
let ide_rpc
    (ide_service : ClientIdeService.t ref)
    ~(env : env)
    ~(tracking_id : string)
    ~(ref_unblocked_time : float ref)
    (message : 'a ClientIdeMessage.t) : 'a Lwt.t =
  let progress () = refresh_status ~env ~ide_service:(Some ide_service) in
  let%lwt result =
    ClientIdeService.rpc
      !ide_service
      ~tracking_id
      ~ref_unblocked_time
      ~progress
      message
  in
  match result with
  | Ok result -> Lwt.return result
  | Error error_data -> raise (Server_nonfatal_exception error_data)

(************************************************************************)
(* Protocol                                                             *)
(************************************************************************)

let do_shutdown
    (state : state)
    (ide_service : ClientIdeService.t ref option)
    (tracking_id : string)
    (ref_unblocked_time : float ref) : state Lwt.t =
  log "Received shutdown request";
  let state = dismiss_diagnostics state in
  let%lwt () =
    match state with
    | Main_loop menv ->
      (* In Main_loop state, we're expected to unsubscribe diagnostics and tell *)
      (* server to disconnect so it can revert the state of its unsaved files.  *)
      Main_env.(
        log "Diag_subscribe: clientLsp do_shutdown unsubscribing diagnostic 0 ";
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
  and () =
    match ide_service with
    | None -> Lwt.return_unit
    | Some ide_service ->
      stop_ide_service
        !ide_service
        ~tracking_id
        ~stop_reason:ClientIdeService.Stop_reason.Editor_exited
  in
  Lwt.return Post_shutdown

let state_to_rage (state : state) : string =
  let uris_to_string uris =
    List.map uris ~f:(fun (DocumentUri uri) -> uri) |> String.concat ~sep:","
  in
  let details =
    match state with
    | Pre_init -> ""
    | Post_shutdown -> ""
    | Main_loop menv ->
      let open Main_env in
      Printf.sprintf
        ( "needs_idle: %b\n"
        ^^ "editor_open_files: %s\n"
        ^^ "uris_with_diagnostics: %s\n"
        ^^ "uris_with_unsaved_changes: %s\n"
        ^^ "hh_server_status.message: %s\n"
        ^^ "hh_server_status.shortMessage: %s\n" )
        menv.needs_idle
        (menv.editor_open_files |> UriMap.keys |> uris_to_string)
        (menv.uris_with_diagnostics |> UriSet.elements |> uris_to_string)
        (menv.uris_with_unsaved_changes |> UriSet.elements |> uris_to_string)
        menv.hh_server_status.ShowStatusFB.request.ShowMessageRequest.message
        (Option.value
           menv.hh_server_status.ShowStatusFB.shortMessage
           ~default:"[absent]")
    | In_init ienv ->
      let open In_init_env in
      Printf.sprintf
        ( "first_start_time: %f\n"
        ^^ "most_recent_sstart_time: %f\n"
        ^^ "editor_open_files: %s\n"
        ^^ "uris_with_unsaved_changes: %s\n" )
        ienv.first_start_time
        ienv.most_recent_start_time
        (ienv.editor_open_files |> UriMap.keys |> uris_to_string)
        (ienv.uris_with_unsaved_changes |> UriSet.elements |> uris_to_string)
    | Lost_server lenv ->
      let open Lost_env in
      Printf.sprintf
        ( "editor_open_files: %s\n"
        ^^ "uris_with_unsaved_changes: %s\n"
        ^^ "lock_file: %s\n"
        ^^ "explanation: %s\n"
        ^^ "new_hh_server_state: %s\n"
        ^^ "start_on_click: %b\n"
        ^^ "trigger_on_lsp: %b\n"
        ^^ "trigger_on_lock_file: %b\n" )
        (lenv.editor_open_files |> UriMap.keys |> uris_to_string)
        (lenv.uris_with_unsaved_changes |> UriSet.elements |> uris_to_string)
        lenv.lock_file
        lenv.p.explanation
        (lenv.p.new_hh_server_state |> hh_server_state_to_string)
        lenv.p.start_on_click
        lenv.p.trigger_on_lsp
        lenv.p.trigger_on_lock_file
  in
  Printf.sprintf "clientLsp state: %s\n%s\n" (state_to_string state) details

let do_rageFB (state : state) : RageFB.result Lwt.t =
  (* clientLsp status *)
  let tnow = Unix.gettimeofday () in
  let server_state_to_string (tstate, state) =
    let tdiff = tnow -. tstate in
    let state = hh_server_state_to_string state in
    let tm = Unix.localtime tstate in
    let ms = int_of_float (tstate *. 1000.) mod 1000 in
    Printf.sprintf
      "[%02d:%02d:%02d.%03d] [%03.3fs ago] %s"
      tm.Unix.tm_hour
      tm.Unix.tm_min
      tm.Unix.tm_sec
      ms
      tdiff
      state
  in
  let server_state =
    List.map ~f:server_state_to_string !hh_server_state_log
    |> String.concat ~sep:"\n"
  in
  let%lwt current_version_and_switch = read_hhconfig_version_and_switch () in

  (* spawn hh rage --rageid <rageid>, and write <rageid> in our own rage output *)
  let rageid = Random_id.short_string () in
  let devnull = Unix.openfile Sys_utils.null_path [Unix.O_RDWR] 0 in
  let rage =
    try
      let (_pid : int) =
        Unix.create_process
          (Exec_command.to_string Exec_command.Hh)
          [|
            Exec_command.to_string Exec_command.Hh;
            "rage";
            "--rageid";
            rageid;
            "--from";
            !from;
            "--desc";
            "lsp_rage";
            get_root_exn () |> Path.to_string;
          |]
          devnull
          devnull
          devnull
      in
      Printf.sprintf
        ( "Detailed hack information can't be included here for technical reasons.\n"
        ^^ "Please look it up by rageid. (This will take several minutes to be ready.)\n"
        ^^ "rageid: %s\nlook up rageid: %s\n" )
        rageid
        (HackEventLogger.Rage.get_telemetry_url rageid)
    with e -> Exception.wrap e |> Exception.to_string
  in

  (* We kicked off a potentially long-running `hh rage` command. What happens if we ourselves
  die before it has finished? Will it be allowed to finish?
  Unix behavior is that the worst that can happen is that the rage process gets reparented onto
  parent pid 1, and its stdin/out/err get closed, and it gets sent SIGHUP. The behavior of
  hh rage when it receives the `--rageid` argument is to ignore all these things. Hence it
  will run to completion. *)

  (* that's it! *)
  let data =
    Printf.sprintf
      ( "%s\n\n"
      ^^ "%s\n\n"
      ^^ "version previously read from .hhconfig and switch: %s\n"
      ^^ "version in .hhconfig and switch: %s\n\n"
      ^^ "clientLsp belief of hh_server_state:\n%s\n" )
      rage
      (state_to_rage state)
      !hhconfig_version_and_switch
      current_version_and_switch
      server_state
  in
  Lwt.return [{ RageFB.title = None; data }]

let do_toggleTypeCoverageFB
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : ToggleTypeCoverageFB.params) : unit Lwt.t =
  (* Currently, the only thing to do on toggling type coverage is turn on dynamic view *)
  let command =
    ServerCommandTypes.DYNAMIC_VIEW params.ToggleTypeCoverageFB.toggle
  in
  cached_toggle_state := params.ToggleTypeCoverageFB.toggle;
  rpc conn ref_unblocked_time command

let do_didOpen
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : DidOpen.params) : unit Lwt.t =
  let open DidOpen in
  let open TextDocumentItem in
  let filename = lsp_uri_to_path params.textDocument.uri in
  let text = params.textDocument.text in
  let command = ServerCommandTypes.OPEN_FILE (filename, text) in
  rpc conn ref_unblocked_time command

let do_didClose
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : DidClose.params) : unit Lwt.t =
  let open DidClose in
  let open TextDocumentIdentifier in
  let filename = lsp_uri_to_path params.textDocument.uri in
  let command = ServerCommandTypes.CLOSE_FILE filename in
  rpc conn ref_unblocked_time command

let do_didChange
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : DidChange.params) : unit Lwt.t =
  let open VersionedTextDocumentIdentifier in
  let open Lsp.DidChange in
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
  rpc conn ref_unblocked_time command

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
    |> Option.map ~f:(hack_pos_to_lsp_range ~equal:Relative_path.equal)
  in
  if List.is_empty contents then
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
    (ide_service : ClientIdeService.t ref)
    (env : env)
    (tracking_id : string)
    (ref_unblocked_time : float ref)
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : Hover.params) : Hover.result Lwt.t =
  let document_location = get_document_location editor_open_files params in
  let%lwt infos =
    ide_rpc
      ide_service
      ~env
      ~tracking_id
      ~ref_unblocked_time
      (ClientIdeMessage.Hover document_location)
  in
  Lwt.return (do_hover_common infos)

let do_typeDefinition
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : Definition.params) : TypeDefinition.result Lwt.t =
  let (file, line, column) = lsp_file_position_to_hack params in
  let command =
    ServerCommandTypes.(IDENTIFY_TYPES (LabelledFileName file, line, column))
  in
  let%lwt results = rpc conn ref_unblocked_time command in
  Lwt.return
    (List.map results ~f:(fun nast_sid ->
         hack_pos_definition_to_lsp_identifier_location
           nast_sid
           ~default_path:file))

let do_typeDefinition_local
    (ide_service : ClientIdeService.t ref)
    (env : env)
    (tracking_id : string)
    (ref_unblocked_time : float ref)
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : Definition.params) : TypeDefinition.result Lwt.t =
  let document_location = get_document_location editor_open_files params in
  let%lwt results =
    ide_rpc
      ide_service
      ~env
      ~tracking_id
      ~ref_unblocked_time
      (ClientIdeMessage.Type_definition document_location)
  in
  let file = Path.to_string document_location.ClientIdeMessage.file_path in
  let results =
    List.map results ~f:(fun nast_sid ->
        hack_pos_definition_to_lsp_identifier_location
          nast_sid
          ~default_path:file)
  in
  Lwt.return results

let do_definition
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : Definition.params) : Definition.result Lwt.t =
  let (filename, line, column) = lsp_file_position_to_hack params in
  let uri =
    params.TextDocumentPositionParams.textDocument.TextDocumentIdentifier.uri
  in
  let labelled_file =
    match UriMap.find_opt uri editor_open_files with
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
    (ide_service : ClientIdeService.t ref)
    (env : env)
    (tracking_id : string)
    (ref_unblocked_time : float ref)
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : Definition.params) : Definition.result Lwt.t =
  let document_location = get_document_location editor_open_files params in
  let%lwt results =
    ide_rpc
      ide_service
      ~env
      ~tracking_id
      ~ref_unblocked_time
      (ClientIdeMessage.Definition document_location)
  in
  let results =
    List.map results ~f:(fun (_occurrence, definition) ->
        hack_symbol_definition_to_lsp_identifier_location
          definition
          ~default_path:
            (document_location.ClientIdeMessage.file_path |> Path.to_string))
  in
  Lwt.return results

let snippet_re = Str.regexp {|[\$}]|} (* snippets must backslash-escape "$\}" *)

let make_ide_completion_response
    (result : AutocompleteTypes.ide_result) (filename : string) :
    Completion.completionList Lwt.t =
  let open AutocompleteTypes in
  let open Completion in
  (* We use snippets to provide parentheses+arguments when autocompleting     *)
  (* method calls e.g. "$c->|" ==> "$c->foo($arg1)". But we'll only do this   *)
  (* there's nothing after the caret: no "$c->|(1)" -> "$c->foo($arg1)(1)"    *)
  let is_caret_followed_by_lparen = Char.equal result.char_at_pos '(' in
  let p = initialize_params_exc () in
  let hack_to_itemType (completion : complete_autocomplete_result) :
      string option =
    (* TODO: we're using itemType (left column) for function return types, and *)
    (* the inlineDetail (right column) for variable/field types. Is that good? *)
    Option.map completion.func_details ~f:(fun details -> details.return_ty)
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
  let hack_to_inline_detail (completion : complete_autocomplete_result) : string
      =
    match completion.func_details with
    | None -> hack_to_detail completion
    | Some details ->
      (* "(type1 $param1, ...)" *)
      let f param = Printf.sprintf "%s %s" param.param_ty param.param_name in
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
           && not
                (SearchUtils.equal_si_kind
                   completion.res_kind
                   SearchUtils.SI_LocalVariable) ->
      (* "method(${1:arg1}, ...)" but for args we just use param names. *)
      let f i param =
        let name = Str.global_replace snippet_re "\\\\\\0" param.param_name in
        Printf.sprintf "${%i:%s}" (i + 1) name
      in
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
      if String.equal (Pos.filename completion.res_pos) "" then
        Pos.set_file filename completion.res_pos
      else
        completion.res_pos
    in
    let data =
      let (line, start, _) = Pos.info_pos pos in
      let filename = Pos.filename pos in
      let base_class =
        match completion.res_base_class with
        | Some base_class -> [("base_class", Hh_json.JSON_String base_class)]
        | None -> []
      in
      let ranking_detail =
        match completion.ranking_details with
        | Some details ->
          [
            ("ranking_detail", Hh_json.JSON_String details.detail);
            ("ranking_source", Hh_json.JSON_Number details.kind);
          ]
        | None -> []
      in
      (* If we do not have a correct file position, skip sending that data *)
      if Int.equal line 0 && Int.equal start 0 then
        Some
          (Hh_json.JSON_Object
             ( [("fullname", Hh_json.JSON_String completion.res_fullname)]
             @ base_class
             @ ranking_detail ))
      else
        Some
          (Hh_json.JSON_Object
             ( [
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
               ]
             @ base_class
             @ ranking_detail ))
    in
    let hack_to_sort_text (completion : complete_autocomplete_result) :
        string option =
      let label = completion.res_name in
      let should_downrank label =
        String.length label > 2
        && String.equal (Str.string_before label 2) "__"
        || Str.string_match (Str.regexp_case_fold ".*do_not_use.*") label 0
      in
      let downranked_result_prefix_character = "~" in
      if should_downrank label then
        Some (downranked_result_prefix_character ^ label)
      else
        Some label
    in
    {
      label =
        ( completion.res_name
        ^
        if
          SearchUtils.equal_si_kind completion.res_kind SearchUtils.SI_Namespace
        then
          "\\"
        else
          "" );
      kind =
        (match completion.ranking_details with
        | Some _ -> Some Completion.Event
        | None ->
          si_kind_to_completion_kind completion.AutocompleteTypes.res_kind);
      detail = Some (hack_to_detail completion);
      inlineDetail = Some (hack_to_inline_detail completion);
      itemType = hack_to_itemType completion;
      documentation = None;
      (* This will be filled in by completionItem/resolve. *)
      sortText =
        (match completion.ranking_details with
        | Some detail -> Some detail.sort_text
        | None -> hack_to_sort_text completion);
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
    }

let do_completion_ffp
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : Completion.params) : Completion.result Lwt.t =
  let open Completion in
  let open TextDocumentIdentifier in
  let pos =
    lsp_position_to_ide params.loc.TextDocumentPositionParams.position
  in
  let filename =
    lsp_uri_to_path params.loc.TextDocumentPositionParams.textDocument.uri
  in
  let command = ServerCommandTypes.IDE_FFP_AUTOCOMPLETE (filename, pos) in
  let%lwt result = rpc conn ref_unblocked_time command in
  make_ide_completion_response result filename

let do_completion_legacy
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : Completion.params) : Completion.result Lwt.t =
  let open Completion in
  let open TextDocumentIdentifier in
  let pos =
    lsp_position_to_ide params.loc.TextDocumentPositionParams.position
  in
  let filename =
    lsp_uri_to_path params.loc.TextDocumentPositionParams.textDocument.uri
  in
  let is_manually_invoked =
    match params.context with
    | None -> false
    | Some c -> is_invoked c.triggerKind
  in
  let command =
    ServerCommandTypes.IDE_AUTOCOMPLETE (filename, pos, is_manually_invoked)
  in
  let%lwt result = rpc conn ref_unblocked_time command in
  make_ide_completion_response result filename

let do_completion_local
    (ide_service : ClientIdeService.t ref)
    (env : env)
    (tracking_id : string)
    (ref_unblocked_time : float ref)
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : Completion.params) : Completion.result Lwt.t =
  let open Completion in
  let document_location = get_document_location editor_open_files params.loc in
  (* Other parameters *)
  let is_manually_invoked =
    match params.context with
    | None -> false
    | Some c -> is_invoked c.triggerKind
  in
  (* this is what I want to fix *)
  let request =
    ClientIdeMessage.Completion
      { ClientIdeMessage.Completion.document_location; is_manually_invoked }
  in
  let%lwt infos =
    ide_rpc ide_service ~env ~tracking_id ~ref_unblocked_time request
  in
  let filename =
    document_location.ClientIdeMessage.file_path |> Path.to_string
  in
  let%lwt response = make_ide_completion_response infos filename in
  Lwt.return response

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

let docblock_with_ranking_detail
    (raw_docblock : DocblockService.result) (ranking_detail : string option) :
    DocblockService.result =
  match ranking_detail with
  | Some detail -> raw_docblock @ [DocblockService.Markdown detail]
  | None -> raw_docblock

let resolve_ranking_source
    (kind : SearchUtils.si_kind) (ranking_source : int option) :
    SearchUtils.si_kind =
  match ranking_source with
  | Some x -> SearchUtils.int_to_kind x
  | None -> kind

let do_completionItemResolve
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : CompletionItemResolve.params) : CompletionItemResolve.result Lwt.t
    =
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
        let ranking_detail = Jget.string_opt data "ranking_detail" in
        let ranking_source = Jget.int_opt data "ranking_source" in
        (* If not found ... *)
        if line = 0 && column = 0 then (
          (* For global symbols such as functions, classes, enums, etc, we
           * need to know the full name INCLUDING all namespaces.  Once
           * we know that, we can look up its file/line/column. *)
          let fullname = Jget.string_exn data "fullname" in
          if String.equal fullname "" then raise NoLocationFound;
          let fullname = Utils.add_ns fullname in
          let command =
            ServerCommandTypes.DOCBLOCK_FOR_SYMBOL
              (fullname, resolve_ranking_source kind ranking_source)
          in
          let%lwt raw_docblock = rpc conn ref_unblocked_time command in
          Lwt.return (docblock_with_ranking_detail raw_docblock ranking_detail)
        ) else
          (* Okay let's get a docblock for this specific location *)
          let command =
            ServerCommandTypes.DOCBLOCK_AT
              ( filename,
                line,
                column,
                base_class,
                resolve_ranking_source kind ranking_source )
          in
          let%lwt raw_docblock = rpc conn ref_unblocked_time command in
          Lwt.return (docblock_with_ranking_detail raw_docblock ranking_detail)
      (* If that failed, fetch docblock using just the symbol name *)
    with _ ->
      let symbolname = params.Completion.label in
      let ranking_source =
        try Jget.int_opt params.Completion.data "ranking_source"
        with _ -> None
      in
      let command =
        ServerCommandTypes.DOCBLOCK_FOR_SYMBOL
          (symbolname, resolve_ranking_source kind ranking_source)
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
    (ide_service : ClientIdeService.t ref)
    (env : env)
    (tracking_id : string)
    (ref_unblocked_time : float ref)
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : CompletionItemResolve.params) : CompletionItemResolve.result Lwt.t
    =
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
        let uri = File_url.create filename |> Lsp.uri_of_string in
        let file_path = Path.make filename in
        let line = Jget.int_exn data "line" in
        let column = Jget.int_exn data "char" in
        let file_contents = get_document_contents editor_open_files uri in
        let ranking_detail = Jget.string_opt data "ranking_detail" in
        let ranking_source = Jget.int_opt data "ranking_source" in
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
              kind = resolve_ranking_source kind ranking_source;
            }
        in
        let%lwt raw_docblock =
          ide_rpc ide_service ~env ~tracking_id ~ref_unblocked_time request
        in
        let documentation =
          docblock_with_ranking_detail raw_docblock ranking_detail
          |> docblock_to_markdown
        in
        Lwt.return { params with Completion.documentation }
      (* If that fails, next try using symbol *)
    with _ ->
      (* The "fullname" value includes the fully qualified namespace, so
       * we want to use that.  However, if it's missing (it shouldn't be)
       * let's default to using the label which doesn't include the
       * namespace. *)
      let symbolname =
        try Jget.string_exn params.Completion.data "fullname"
        with _ -> params.Completion.label
      in
      let ranking_source =
        try Jget.int_opt params.Completion.data "ranking_source"
        with _ -> None
      in
      let request =
        ClientIdeMessage.Completion_resolve
          {
            ClientIdeMessage.Completion_resolve.symbol = symbolname;
            kind = resolve_ranking_source kind ranking_source;
          }
      in
      let%lwt raw_docblock =
        ide_rpc ide_service ~env ~tracking_id ~ref_unblocked_time request
      in
      let documentation = docblock_to_markdown raw_docblock in
      Lwt.return { params with Completion.documentation }
  in
  Lwt.return result

let hack_symbol_to_lsp (symbol : SearchUtils.symbol) =
  let open SearchUtils in
  (* Hack sometimes gives us back items with an empty path, by which it
  intends "whichever path you asked me about". That would be meaningless
  here. If it does, then it'll pick up our default path (also empty),
  which will throw and go into our telemetry. That's the best we can do. *)
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
    | SearchUtils.SI_RecordDef -> SymbolInformation.Struct
    (* Do these happen in practice? *)
    | SearchUtils.SI_Keyword
    | SearchUtils.SI_Unknown ->
      failwith "Unknown symbol kind"
  in
  {
    SymbolInformation.name = Utils.strip_ns symbol.name;
    kind = hack_to_lsp_kind symbol.result_type;
    location = hack_pos_to_lsp_location symbol.pos ~default_path:"";
    containerName = None;
  }

let do_workspaceSymbol
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : WorkspaceSymbol.params) : WorkspaceSymbol.result Lwt.t =
  let query_type = "" in
  let command =
    ServerCommandTypes.SEARCH (params.WorkspaceSymbol.query, query_type)
  in
  let%lwt results = rpc conn ref_unblocked_time command in
  Lwt.return (List.map results ~f:hack_symbol_to_lsp)

let do_workspaceSymbol_local
    (ide_service : ClientIdeService.t ref)
    (env : env)
    (tracking_id : string)
    (ref_unblocked_time : float ref)
    (params : WorkspaceSymbol.params) : WorkspaceSymbol.result Lwt.t =
  let query = params.WorkspaceSymbol.query in
  let request = ClientIdeMessage.Workspace_symbol query in
  let%lwt results =
    ide_rpc ide_service ~env ~tracking_id ~ref_unblocked_time request
  in
  Lwt.return (List.map results ~f:hack_symbol_to_lsp)

let rec hack_symbol_tree_to_lsp
    ~(filename : string)
    ~(accu : Lsp.SymbolInformation.t list)
    ~(container_name : string option)
    (defs : FileOutline.outline) : Lsp.SymbolInformation.t list =
  let open SymbolDefinition in
  let hack_to_lsp_kind = function
    | SymbolDefinition.Function -> SymbolInformation.Function
    | SymbolDefinition.Class -> SymbolInformation.Class
    | SymbolDefinition.Method -> SymbolInformation.Method
    | SymbolDefinition.Property -> SymbolInformation.Property
    | SymbolDefinition.RecordDef -> SymbolInformation.Struct
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
    hack_symbol_tree_to_lsp ~filename ~accu ~container_name defs

let do_documentSymbol
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : DocumentSymbol.params) : DocumentSymbol.result Lwt.t =
  let open DocumentSymbol in
  let open TextDocumentIdentifier in
  let filename = lsp_uri_to_path params.textDocument.uri in
  let command = ServerCommandTypes.OUTLINE filename in
  let%lwt outline = rpc conn ref_unblocked_time command in
  let converted =
    hack_symbol_tree_to_lsp ~filename ~accu:[] ~container_name:None outline
  in
  Lwt.return converted

(* for serverless ide *)
let do_documentSymbol_local
    (ide_service : ClientIdeService.t ref)
    (env : env)
    (tracking_id : string)
    (ref_unblocked_time : float ref)
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : DocumentSymbol.params) : DocumentSymbol.result Lwt.t =
  let open DocumentSymbol in
  let open TextDocumentIdentifier in
  let filename = lsp_uri_to_path params.textDocument.uri in
  let document_location =
    {
      ClientIdeMessage.file_path = Path.make filename;
      file_contents =
        get_document_contents editor_open_files params.textDocument.uri;
      line = 0;
      column = 0;
    }
  in
  let request = ClientIdeMessage.Document_symbol document_location in
  let%lwt outline =
    ide_rpc ide_service ~env ~tracking_id ~ref_unblocked_time request
  in
  let converted =
    hack_symbol_tree_to_lsp ~filename ~accu:[] ~container_name:None outline
  in
  Lwt.return converted

let do_findReferences
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : FindReferences.params) : FindReferences.result Lwt.t =
  let { Ide_api_types.line; column } =
    lsp_position_to_ide
      params.FindReferences.loc.TextDocumentPositionParams.position
  in
  let filename =
    Lsp_helpers.lsp_textDocumentIdentifier_to_filename
      params.FindReferences.loc.TextDocumentPositionParams.textDocument
  in
  let include_defs =
    params.FindReferences.context.FindReferences.includeDeclaration
  in
  let labelled_file = ServerCommandTypes.LabelledFileName filename in
  let command =
    ServerCommandTypes.IDE_FIND_REFS (labelled_file, line, column, include_defs)
  in
  let%lwt results = rpc_with_retry conn ref_unblocked_time command in
  (* TODO: respect params.context.include_declaration *)
  match results with
  | None -> Lwt.return []
  | Some (_name, positions) ->
    Lwt.return
      (List.map positions ~f:(hack_pos_to_lsp_location ~default_path:filename))

let do_goToImplementation
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : Implementation.params) : Implementation.result Lwt.t =
  let { Ide_api_types.line; column } =
    lsp_position_to_ide params.TextDocumentPositionParams.position
  in
  let filename =
    Lsp_helpers.lsp_textDocumentIdentifier_to_filename
      params.TextDocumentPositionParams.textDocument
  in
  let labelled_file = ServerCommandTypes.LabelledFileName filename in
  let command =
    ServerCommandTypes.IDE_GO_TO_IMPL (labelled_file, line, column)
  in
  let%lwt results = rpc_with_retry conn ref_unblocked_time command in
  match results with
  | None -> Lwt.return []
  | Some (_name, positions) ->
    Lwt.return
      (List.map positions ~f:(hack_pos_to_lsp_location ~default_path:filename))

(* Shared function for hack range conversion *)
let hack_range_to_lsp_highlight range =
  { DocumentHighlight.range = ide_range_to_lsp range; kind = None }

let do_documentHighlight
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : DocumentHighlight.params) : DocumentHighlight.result Lwt.t =
  let (file, line, column) = lsp_file_position_to_hack params in
  let command =
    ServerCommandTypes.(IDE_HIGHLIGHT_REFS (file, FileName file, line, column))
  in
  let%lwt results = rpc conn ref_unblocked_time command in
  Lwt.return (List.map results ~f:hack_range_to_lsp_highlight)

(* Serverless IDE implementation of highlight *)
let do_highlight_local
    (ide_service : ClientIdeService.t ref)
    (env : env)
    (tracking_id : string)
    (ref_unblocked_time : float ref)
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : DocumentHighlight.params) : DocumentHighlight.result Lwt.t =
  let document_location = get_document_location editor_open_files params in
  let%lwt ranges =
    ide_rpc
      ide_service
      ~env
      ~tracking_id
      ~ref_unblocked_time
      (ClientIdeMessage.Document_highlight document_location)
  in
  Lwt.return (List.map ranges ~f:hack_range_to_lsp_highlight)

let format_typeCoverage_result ~(equal : 'a -> 'a -> bool) results counts =
  TypeCoverageFB.(
    let coveredPercent = Coverage_level.get_percent counts in
    let hack_coverage_to_lsp (pos, level) =
      let range = hack_pos_to_lsp_range ~equal pos in
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
      defaultMessage = "Un-type checked code. Consider adding type annotations.";
    })

let do_typeCoverageFB
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : TypeCoverageFB.params) : TypeCoverageFB.result Lwt.t =
  TypeCoverageFB.(
    let filename =
      Lsp_helpers.lsp_textDocumentIdentifier_to_filename params.textDocument
    in
    let command =
      ServerCommandTypes.COVERAGE_LEVELS
        (filename, ServerCommandTypes.FileName filename)
    in
    let%lwt (results, counts) : Coverage_level_defs.result =
      rpc conn ref_unblocked_time command
    in
    let formatted =
      format_typeCoverage_result ~equal:String.equal results counts
    in
    Lwt.return formatted)

let do_typeCoverage_localFB
    (ide_service : ClientIdeService.t ref)
    (env : env)
    (tracking_id : string)
    (ref_unblocked_time : float ref)
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : TypeCoverageFB.params) : TypeCoverageFB.result Lwt.t =
  let open TypeCoverageFB in
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
    let%lwt result =
      ide_rpc ide_service ~env ~tracking_id ~ref_unblocked_time request
    in
    let (results, counts) = result in
    let formatted =
      format_typeCoverage_result ~equal:String.equal results counts
    in
    Lwt.return formatted

let do_formatting_common
    (uri : Lsp.documentUri)
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (action : ServerFormatTypes.ide_action)
    (options : DocumentFormatting.formattingOptions) : TextEdit.t list =
  let open ServerFormatTypes in
  let filename_for_logging = lsp_uri_to_path uri in
  (* Following line will throw if the document isn't already open, so we'll *)
  (* return an error code to the LSP client. The spec doesn't spell out if we *)
  (* should be expected to handle formatting requests on unopened files. *)
  let lsp_doc = UriMap.find uri editor_open_files in
  let content = lsp_doc.Lsp.TextDocumentItem.text in
  let response =
    ServerFormat.go_ide ~filename_for_logging ~content ~action ~options
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
  | Error message ->
    raise
      (Error.LspException
         { Error.code = Error.UnknownErrorCode; message; data = None })
  | Ok r ->
    let range = ide_range_to_lsp r.range in
    let newText = r.new_text in
    [{ TextEdit.range; newText }]

let do_documentRangeFormatting
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : DocumentRangeFormatting.params) : DocumentRangeFormatting.result =
  let open DocumentRangeFormatting in
  let open TextDocumentIdentifier in
  let action = ServerFormatTypes.Range (lsp_range_to_ide params.range) in
  do_formatting_common
    params.textDocument.uri
    editor_open_files
    action
    params.options

let do_documentOnTypeFormatting
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : DocumentOnTypeFormatting.params) : DocumentOnTypeFormatting.result
    =
  let open DocumentOnTypeFormatting in
  let open TextDocumentIdentifier in
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
  let position =
    { params.position with character = params.position.character - 1 }
  in
  let action = ServerFormatTypes.Position (lsp_position_to_ide position) in
  do_formatting_common
    params.textDocument.uri
    editor_open_files
    action
    params.options

let do_documentFormatting
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : DocumentFormatting.params) : DocumentFormatting.result =
  let open DocumentFormatting in
  let open TextDocumentIdentifier in
  let action = ServerFormatTypes.Document in
  do_formatting_common
    params.textDocument.uri
    editor_open_files
    action
    params.options

let do_signatureHelp
    (conn : server_conn)
    (ref_unblocked_time : float ref)
    (params : SignatureHelp.params) : SignatureHelp.result Lwt.t =
  let (file, line, column) = lsp_file_position_to_hack params in
  let command = ServerCommandTypes.IDE_SIGNATURE_HELP (file, line, column) in
  rpc conn ref_unblocked_time command

(* Serverless IDE version of signature help *)
let do_signatureHelp_local
    (ide_service : ClientIdeService.t ref)
    (env : env)
    (tracking_id : string)
    (ref_unblocked_time : float ref)
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : SignatureHelp.params) : SignatureHelp.result Lwt.t =
  let document_location = get_document_location editor_open_files params in
  let%lwt signatures =
    ide_rpc
      ide_service
      ~env
      ~tracking_id
      ~ref_unblocked_time
      (ClientIdeMessage.Signature_help document_location)
  in
  Lwt.return signatures

let patch_to_workspace_edit_change (patch : ServerRefactorTypes.patch) :
    string * TextEdit.t =
  let open ServerRefactorTypes in
  let open Pos in
  let text_edit =
    match patch with
    | Insert insert_patch
    | Replace insert_patch ->
      {
        TextEdit.range =
          hack_pos_to_lsp_range ~equal:String.equal insert_patch.pos;
        newText = insert_patch.text;
      }
    | Remove pos ->
      {
        TextEdit.range = hack_pos_to_lsp_range ~equal:String.equal pos;
        newText = "";
      }
  in
  let uri =
    match patch with
    | Insert insert_patch
    | Replace insert_patch ->
      File_url.create (filename insert_patch.pos)
    | Remove pos -> File_url.create (filename pos)
  in
  (uri, text_edit)

let patches_to_workspace_edit (patches : ServerRefactorTypes.patch list) :
    WorkspaceEdit.t =
  let changes = List.map patches ~f:patch_to_workspace_edit_change in
  let changes =
    List.fold changes ~init:SMap.empty ~f:(fun acc (uri, text_edit) ->
        let current_edits = Option.value ~default:[] (SMap.find_opt uri acc) in
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
  let open Rename in
  let new_name = params.newName in
  let command =
    ServerCommandTypes.IDE_REFACTOR
      { ServerCommandTypes.Ide_refactor_type.filename; line; char; new_name }
  in
  let%lwt patches = rpc_with_retry conn ref_unblocked_time command in
  let patches =
    match patches with
    | Ok patches -> patches
    | Error message ->
      raise
        (Error.LspException
           { Error.code = Error.InvalidRequest; message; data = None })
  in
  Lwt.return (patches_to_workspace_edit patches)

(** This updates Main_env.hh_server_status according to the status message
we just received from hh_server. See comments on hh_server_status for
the invariants on its fields. *)
let do_server_busy (state : state) (status : ServerCommandTypes.busy_status) :
    state =
  let open Main_env in
  let open ServerCommandTypes in
  let (type_, shortMessage, message) =
    match status with
    | Needs_local_typecheck ->
      ( MessageType.InfoMessage,
        "Hack: ready",
        "hh_server is preparing to check edits" )
    | Doing_local_typecheck ->
      (MessageType.WarningMessage, "Hack", "hh_server is checking edits")
    | Done_local_typecheck ->
      ( MessageType.InfoMessage,
        "Hack: ready",
        "hh_server is initialized and running correctly." )
    | Doing_global_typecheck Blocking ->
      ( MessageType.WarningMessage,
        "Hack: busy",
        "hh_server is typechecking the entire project (blocking)" )
    | Doing_global_typecheck Interruptible ->
      ( MessageType.WarningMessage,
        "Hack",
        "hh_server is typechecking entire project" )
    | Doing_global_typecheck (Remote_blocking message) ->
      ( MessageType.WarningMessage,
        "Hack: remote",
        "hh_server is remote-typechecking the entire project - " ^ message )
    | Done_global_typecheck _ ->
      ( MessageType.InfoMessage,
        "Hack: ready",
        "hh_server is initialized and running correctly." )
  in
  match state with
  | Main_loop menv ->
    let hh_server_status =
      {
        ShowStatusFB.shortMessage = Some shortMessage;
        request = { ShowMessageRequest.type_; message; actions = [] };
        total = None;
        progress = None;
      }
    in
    Main_loop { menv with hh_server_status }
  | _ -> state

(* do_diagnostics: sends notifications for all reported diagnostics; also     *)
(* returns an updated "uris_with_diagnostics" set of all files for which     *)
(* our client currently has non-empty diagnostic reports.                     *)
let do_diagnostics
    (uris_with_diagnostics : UriSet.t)
    (file_reports : Pos.absolute Errors.error_ list SMap.t) : UriSet.t =
  (* Hack sometimes reports a diagnostic on an empty file when it can't       *)
  (* figure out which file to report. In this case we'll report on the root.  *)
  (* Nuclide and VSCode both display this fine, though they obviously don't   *)
  (* let you click-to-go-to-file on it.                                       *)
  let default_path = get_root_exn () |> Path.to_string in
  let file_reports =
    match SMap.find_opt "" file_reports with
    | None -> file_reports
    | Some errors ->
      SMap.remove "" file_reports |> SMap.add ~combine:( @ ) default_path errors
  in
  let per_file file errors =
    let params = hack_errors_to_lsp_diagnostic file errors in
    let notification = PublishDiagnosticsNotification params in
    notify_jsonrpc ~powered_by:Hh_server notification
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
    List.map files_without ~f:(path_to_lsp_uri ~default_path) |> UriSet.of_list
  in
  let uris_with =
    List.map files_with ~f:(path_to_lsp_uri ~default_path) |> UriSet.of_list
  in
  (* this is "(uris_with_diagnostics \ uris_without) U uris_with" *)
  UriSet.union (UriSet.diff uris_with_diagnostics uris_without) uris_with

let do_initialize ~(env : env) (root : Path.t) : Initialize.result =
  let server_args = ServerArgs.default_options ~root:(Path.to_string root) in
  let server_args = ServerArgs.set_config server_args env.config in
  let server_local_config =
    snd @@ ServerConfig.load ~silent:true ServerConfig.filename server_args
  in
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
          implementationProvider =
            server_local_config.ServerLocalConfig.go_to_implementation;
          typeCoverageProviderFB = true;
          rageProviderFB = true;
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

let handle_idle_if_necessary (state : state) (event : event) : state =
  match state with
  | Main_loop menv when not (is_tick event) ->
    Main_loop { menv with Main_env.needs_idle = true }
  | _ -> state

let track_open_and_recent_files (state : state) (event : event) : state =
  (* We'll keep track of which files are opened by the editor. *)
  let prev_opened_files =
    Option.value (get_editor_open_files state) ~default:UriMap.empty
  in
  let editor_open_files =
    match event with
    | Client_message (_, NotificationMessage (DidOpenNotification params)) ->
      let doc = params.DidOpen.textDocument in
      let uri = params.DidOpen.textDocument.TextDocumentItem.uri in
      UriMap.add uri doc prev_opened_files
    | Client_message (_, NotificationMessage (DidChangeNotification params)) ->
      let uri =
        params.DidChange.textDocument.VersionedTextDocumentIdentifier.uri
      in
      let doc = UriMap.find_opt uri prev_opened_files in
      let open Lsp.TextDocumentItem in
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
        UriMap.add uri doc' prev_opened_files
      | None -> prev_opened_files)
    | Client_message (_, NotificationMessage (DidCloseNotification params)) ->
      let uri = params.DidClose.textDocument.TextDocumentIdentifier.uri in
      UriMap.remove uri prev_opened_files
    | _ -> prev_opened_files
  in
  (* We'll track which was the most recent file to have an event *)
  let most_recent_file =
    match event with
    | Client_message (_metadata, message) ->
      let uri = Lsp_fmt.get_uri_opt message in
      if Option.is_some uri then
        uri
      else
        get_most_recent_file state
    | _ -> get_most_recent_file state
  in
  match state with
  | Main_loop menv ->
    Main_loop { menv with Main_env.editor_open_files; most_recent_file }
  | In_init ienv ->
    In_init { ienv with In_init_env.editor_open_files; most_recent_file }
  | Lost_server lenv ->
    Lost_server { lenv with Lost_env.editor_open_files; most_recent_file }
  | _ -> state

let track_edits_if_necessary (state : state) (event : event) : state =
  (* We'll keep track of which files have unsaved edits. Note that not all    *)
  (* clients send didSave messages; for those we only rely on didClose.       *)
  let previous = get_uris_with_unsaved_changes state in
  let uris_with_unsaved_changes =
    match event with
    | Client_message (_, NotificationMessage (DidChangeNotification params)) ->
      let uri =
        params.DidChange.textDocument.VersionedTextDocumentIdentifier.uri
      in
      UriSet.add uri previous
    | Client_message (_, NotificationMessage (DidCloseNotification params)) ->
      let uri = params.DidClose.textDocument.TextDocumentIdentifier.uri in
      UriSet.remove uri previous
    | Client_message (_, NotificationMessage (DidSaveNotification params)) ->
      let uri = params.DidSave.textDocument.TextDocumentIdentifier.uri in
      UriSet.remove uri previous
    | _ -> previous
  in
  match state with
  | Main_loop menv -> Main_loop { menv with Main_env.uris_with_unsaved_changes }
  | In_init ienv -> In_init { ienv with In_init_env.uris_with_unsaved_changes }
  | Lost_server lenv ->
    Lost_server { lenv with Lost_env.uris_with_unsaved_changes }
  | _ -> state

let get_filename_in_message_for_logging (message : lsp_message) :
    Relative_path.t option =
  let uri_opt = Lsp_fmt.get_uri_opt message in
  match uri_opt with
  | None -> None
  | Some uri ->
    (try
       let path = Lsp_helpers.lsp_uri_to_path uri in
       Some (Relative_path.create_detect_prefix path)
     with _ ->
       Some (Relative_path.create Relative_path.Dummy (Lsp.string_of_uri uri)))

(* Historical quirk: we log kind and method-name a bit idiosyncratically... *)
let get_message_kind_and_method_for_logging (message : lsp_message) :
    string * string =
  match message with
  | ResponseMessage (_, _) -> ("Response", "[response]")
  | RequestMessage (_, r) -> ("Request", Lsp_fmt.request_name_to_string r)
  | NotificationMessage n ->
    ("Notification", Lsp_fmt.notification_name_to_string n)

let log_response_if_necessary
    (env : env)
    (event : event)
    (result_telemetry_opt : result_telemetry option)
    (unblocked_time : float) : unit =
  match event with
  | Client_message (metadata, message) ->
    let (kind, method_) = get_message_kind_and_method_for_logging message in
    let t = Unix.gettimeofday () in
    log_debug
      "lsp-message [%s] queue time [%0.3f] execution time [%0.3f]"
      method_
      (unblocked_time -. metadata.timestamp)
      (t -. unblocked_time);
    let (result_count, result_extra_telemetry) =
      match result_telemetry_opt with
      | None -> (None, None)
      | Some { result_count; result_extra_telemetry } ->
        (Some result_count, result_extra_telemetry)
    in
    HackEventLogger.client_lsp_method_handled
      ~root:(get_root_opt ())
      ~method_
      ~kind
      ~path_opt:(get_filename_in_message_for_logging message)
      ~result_count
      ~result_extra_telemetry
      ~tracking_id:metadata.tracking_id
      ~start_queue_time:metadata.timestamp
      ~start_hh_server_state:
        ( get_older_hh_server_state metadata.timestamp
        |> hh_server_state_to_string )
      ~start_handle_time:unblocked_time
      ~serverless_ide_flag:env.use_serverless_ide
  | _ -> ()

type error_source =
  | Error_from_server_fatal
  | Error_from_client_fatal
  | Error_from_client_recoverable
  | Error_from_server_recoverable
  | Error_from_lsp_cancelled
  | Error_from_lsp_misc

let hack_log_error
    (event : event option)
    (e : Lsp.Error.t)
    (source : error_source)
    (unblocked_time : float)
    (env : env) : unit =
  let root = get_root_opt () in
  let is_expected =
    match source with
    | Error_from_lsp_cancelled -> true
    | Error_from_server_fatal
    | Error_from_client_fatal
    | Error_from_client_recoverable
    | Error_from_server_recoverable
    | Error_from_lsp_misc ->
      false
  in
  let source =
    match source with
    | Error_from_server_fatal -> "server_fatal"
    | Error_from_client_fatal -> "client_fatal"
    | Error_from_client_recoverable -> "client_recoverable"
    | Error_from_server_recoverable -> "server_recoverable"
    | Error_from_lsp_cancelled -> "lsp_cancelled"
    | Error_from_lsp_misc -> "lsp_misc"
  in
  if not is_expected then log "%s" (Lsp_fmt.error_to_log_string e);
  match event with
  | Some (Client_message (metadata, message)) ->
    let start_hh_server_state =
      get_older_hh_server_state metadata.timestamp |> hh_server_state_to_string
    in
    let (kind, method_) = get_message_kind_and_method_for_logging message in
    HackEventLogger.client_lsp_method_exception
      ~root
      ~method_
      ~kind
      ~path_opt:(get_filename_in_message_for_logging message)
      ~tracking_id:metadata.tracking_id
      ~start_queue_time:metadata.timestamp
      ~start_hh_server_state
      ~start_handle_time:unblocked_time
      ~serverless_ide_flag:env.use_serverless_ide
      ~message:e.Error.message
      ~data_opt:e.Error.data
      ~source
  | _ ->
    HackEventLogger.client_lsp_exception
      ~root
      ~message:e.Error.message
      ~data_opt:e.Error.data
      ~source

(* cancel_if_stale: If a message is stale, throw the necessary exception to
   cancel it. A message is considered stale if it's sufficiently old and there
   are other messages in the queue that are newer than it. *)
let short_timeout = 2.5

let long_timeout = 15.0

let cancel_if_stale
    (client : Jsonrpc.queue) (timestamp : float) (timeout : float) : unit Lwt.t
    =
  let time_elapsed = Unix.gettimeofday () -. timestamp in
  if Float.(time_elapsed >= timeout) then
    if Jsonrpc.has_message client then
      raise
        (Error.LspException
           {
             Error.code = Error.RequestCancelled;
             message = "request timed out";
             data = None;
           })
    else
      Lwt.return_unit
  else
    Lwt.return_unit

(************************************************************************)
(* Message handling                                                     *)
(************************************************************************)

(** send DidOpen/Close/Change/Save to hh_server and ide_service as needed *)
let handle_editor_buffer_message
    ~(state : state)
    ~(ide_service : ClientIdeService.t ref option)
    ~(env : env)
    ~(metadata : incoming_metadata)
    ~(ref_unblocked_time : float ref)
    ~(message : lsp_message) : unit Lwt.t =
  let uri_to_path uri = uri |> lsp_uri_to_path |> Path.make in
  let ref_hh_unblocked_time = ref 0. in
  let ref_ide_unblocked_time = ref 0. in

  (* send to hh_server as necessary *)
  let (hh_server_promise : unit Lwt.t) =
    let open Main_env in
    match (state, message) with
    (* textDocument/didOpen notification *)
    | (Main_loop menv, NotificationMessage (DidOpenNotification params)) ->
      let%lwt () = do_didOpen menv.conn ref_hh_unblocked_time params in
      Lwt.return_unit
    (* textDocument/didClose notification *)
    | (Main_loop menv, NotificationMessage (DidCloseNotification params)) ->
      let%lwt () = do_didClose menv.conn ref_hh_unblocked_time params in
      Lwt.return_unit
    (* textDocument/didChange notification *)
    | (Main_loop menv, NotificationMessage (DidChangeNotification params)) ->
      let%lwt () = do_didChange menv.conn ref_hh_unblocked_time params in
      Lwt.return_unit
    (* textDocument/didSave notification *)
    | (Main_loop _menv, NotificationMessage (DidSaveNotification _params)) ->
      Lwt.return_unit
    | (_, _) -> Lwt.return_unit
  in

  (* send to ide_service as necessary *)
  (* For now 'ide_service_promise' is immediately fulfilled, but in future it will
  be fulfilled only when the ide_service has finished processing the message. *)
  let (ide_service_promise : unit Lwt.t) =
    match (ide_service, message) with
    | (Some ide_service, NotificationMessage (DidOpenNotification params)) ->
      let file_path =
        uri_to_path params.DidOpen.textDocument.TextDocumentItem.uri
      in
      let file_contents = params.DidOpen.textDocument.TextDocumentItem.text in
      (* The ClientIdeDaemon only delivers answers for open files, which is why it's vital
      never to let is miss a DidOpen. *)
      let%lwt () =
        ide_rpc
          ide_service
          ~env
          ~tracking_id:metadata.tracking_id
          ~ref_unblocked_time:ref_ide_unblocked_time
          ClientIdeMessage.(Ide_file_opened { file_path; file_contents })
      in
      Lwt.return_unit
    | (Some ide_service, NotificationMessage (DidChangeNotification params)) ->
      let file_path =
        uri_to_path
          params.DidChange.textDocument.VersionedTextDocumentIdentifier.uri
      in
      let%lwt () =
        ide_rpc
          ide_service
          ~env
          ~tracking_id:metadata.tracking_id
          ~ref_unblocked_time:ref_ide_unblocked_time
          ClientIdeMessage.(Ide_file_changed { Ide_file_changed.file_path })
      in
      Lwt.return_unit
    | (Some ide_service, NotificationMessage (DidCloseNotification params)) ->
      let file_path =
        uri_to_path params.DidClose.textDocument.TextDocumentIdentifier.uri
      in
      let%lwt () =
        ide_rpc
          ide_service
          ~env
          ~tracking_id:metadata.tracking_id
          ~ref_unblocked_time:ref_ide_unblocked_time
          ClientIdeMessage.(Ide_file_closed file_path)
      in
      Lwt.return_unit
    | _ ->
      (* Don't handle other events for now. When we show typechecking errors for
    the open file, we'll start handling them. *)
      Lwt.return_unit
  in

  (* Our asynchrony deal is (1) we want to kick off notifications to
  hh_server and ide_service at the same time, (2) we want to wait until
  both are done, (3) an exception in one shouldn't jeapordize the other,
  (4) our failure model only allows us to record at most one exception
  so we'll pick one arbitrarily. *)
  let%lwt (hh_server_e : Exception.t option) =
    try%lwt
      let%lwt () = hh_server_promise in
      Lwt.return_none
    with e -> Lwt.return_some (Exception.wrap e)
  and (ide_service_e : Exception.t option) =
    try%lwt
      let%lwt () = ide_service_promise in
      Lwt.return_none
    with e -> Lwt.return_some (Exception.wrap e)
  in
  ref_unblocked_time := Float.max !ref_hh_unblocked_time !ref_ide_unblocked_time;
  match (hh_server_e, ide_service_e) with
  | (_, Some e)
  | (Some e, _) ->
    Exception.reraise e
  | _ -> Lwt.return_unit

let set_verbose_to_file
    ~(ide_service : ClientIdeService.t ref option)
    ~(env : env)
    ~(tracking_id : string)
    (value : bool) : unit =
  verbose_to_file := value;
  if !verbose_to_file then
    Hh_logger.Level.set_min_level_file Hh_logger.Level.Debug
  else
    Hh_logger.Level.set_min_level_file Hh_logger.Level.Info;
  match ide_service with
  | Some ide_service ->
    let ref_unblocked_time = ref 0. in
    let (promise : unit Lwt.t) =
      ide_rpc
        ide_service
        ~env
        ~tracking_id
        ~ref_unblocked_time
        (ClientIdeMessage.Verbose_to_file !verbose_to_file)
    in
    ignore_promise_but_handle_failure
      promise
      ~desc:"verbose-ide-rpc"
      ~terminate_on_failure:false
  | None -> ()

(* handle_event: Process and respond to a message, and update the LSP state
   machine accordingly. In case the message was a request, it returns the
   json it responded with, so the caller can log it. *)
let handle_client_message
    ~(env : env)
    ~(state : state ref)
    ~(client : Jsonrpc.queue)
    ~(ide_service : ClientIdeService.t ref option)
    ~(metadata : incoming_metadata)
    ~(message : lsp_message)
    ~(ref_unblocked_time : float ref) : result_telemetry option Lwt.t =
  let open Main_env in
  let%lwt result_telemetry_opt =
    (* make sure to wrap any exceptions below in the promise *)
    let tracking_id = metadata.tracking_id in
    let timestamp = metadata.timestamp in
    let editor_open_files =
      match get_editor_open_files !state with
      | Some files -> files
      | None -> UriMap.empty
    in
    match (!state, ide_service, message) with
    (* response *)
    | (_, _, ResponseMessage (id, response)) ->
      let (_, handler) = IdMap.find id !requests_outstanding in
      let%lwt new_state = handler response !state in
      state := new_state;
      Lwt.return_none
    (* shutdown request *)
    | (_, _, RequestMessage (id, ShutdownRequest)) ->
      let%lwt new_state =
        do_shutdown !state ide_service tracking_id ref_unblocked_time
      in
      state := new_state;
      respond_jsonrpc ~powered_by:Language_server id ShutdownResult;
      Lwt.return_none
    (* cancel notification *)
    | (_, _, NotificationMessage (CancelRequestNotification _)) ->
      (* For now, we'll ignore it. *)
      Lwt.return_none
    (* exit notification *)
    | (_, _, NotificationMessage ExitNotification) ->
      if is_post_shutdown !state then
        exit_ok ()
      else
        exit_fail ()
    (* setTrace notification *)
    | (_, _, NotificationMessage (SetTraceNotification params)) ->
      let value =
        match params with
        | SetTraceNotification.Verbose -> true
        | SetTraceNotification.Off -> false
      in
      set_verbose_to_file ~ide_service ~env ~tracking_id value;
      Lwt.return_none
    (* test entrypoint: shutdown client_ide_service *)
    | ( _,
        Some ide_service,
        RequestMessage (id, HackTestShutdownServerlessRequestFB) ) ->
      let%lwt () =
        stop_ide_service
          !ide_service
          ~tracking_id
          ~stop_reason:ClientIdeService.Stop_reason.Testing
      in
      respond_jsonrpc
        ~powered_by:Serverless_ide
        id
        HackTestShutdownServerlessResultFB;
      Lwt.return_none
    (* test entrypoint: stop hh_server *)
    | (_, _, RequestMessage (id, HackTestStopServerRequestFB)) ->
      let root_folder =
        Path.make (Relative_path.path_of_prefix Relative_path.Root)
      in
      ClientStop.kill_server root_folder !from;
      respond_jsonrpc ~powered_by:Serverless_ide id HackTestStopServerResultFB;
      Lwt.return_none
    (* test entrypoint: start hh_server *)
    | (_, _, RequestMessage (id, HackTestStartServerRequestFB)) ->
      let root_folder =
        Path.make (Relative_path.path_of_prefix Relative_path.Root)
      in
      start_server ~env root_folder;
      respond_jsonrpc ~powered_by:Serverless_ide id HackTestStartServerResultFB;
      Lwt.return_none
    (* initialize request *)
    | (Pre_init, _, RequestMessage (id, InitializeRequest initialize_params)) ->
      let open Initialize in
      initialize_params_ref := Some initialize_params;
      let root = get_root_exn () in
      (* calculated from initialize_params_ref *)
      set_up_hh_logger_for_client_lsp root;
      (* Following is a hack. Atom incorrectly passes '--from vscode', rendering us
      unable to distinguish Atom from VSCode. But Atom is now frozen at vscode client
      v3.14. So by looking at the version, we can at least distinguish that it's old. *)
      if
        (not
           initialize_params.client_capabilities.textDocument.declaration
             .declarationLinkSupport)
        && String.equal env.from "vscode"
      then begin
        from := "vscode_pre314";
        HackEventLogger.set_from !from
      end;

      let%lwt version = read_hhconfig_version () in
      HackEventLogger.set_hhconfig_version
        (Some (String_utils.lstrip version "^"));
      let%lwt version_and_switch = read_hhconfig_version_and_switch () in
      hhconfig_version_and_switch := version_and_switch;
      let%lwt new_state = connect ~env !state in
      state := new_state;
      Relative_path.set_path_prefix Relative_path.Root root;
      (* If editor sent 'trace: on' then that will turn on verbose_to_file. But we won't turn off
      verbose here, since the command-line argument --verbose trumps initialization params. *)
      begin
        match initialize_params.Initialize.trace with
        | Initialize.Off -> ()
        | Initialize.Messages
        | Initialize.Verbose ->
          set_verbose_to_file ~ide_service ~env ~tracking_id true
      end;
      let result = do_initialize ~env root in
      respond_jsonrpc ~powered_by:Language_server id (InitializeResult result);

      begin
        match ide_service with
        | None -> ()
        | Some ide_service ->
          let (promise : unit Lwt.t) =
            run_ide_service env !ide_service initialize_params None
          in
          ignore_promise_but_handle_failure
            promise
            ~desc:"run-ide-after-init"
            ~terminate_on_failure:true;
          (* Invariant: at all times after InitializeRequest, ide_service has
          already been sent an "initialize" message. *)
          let id = NumberId (Jsonrpc.get_next_request_id ()) in
          let request = do_didChangeWatchedFiles_registerCapability () in
          to_stdout (print_lsp_request id request);
          (* TODO: our handler should really handle an error response properly *)
          let handler _response state = Lwt.return state in
          requests_outstanding :=
            IdMap.add id (request, handler) !requests_outstanding
      end;

      if not @@ Sys_utils.is_test_mode () then
        Lsp_helpers.telemetry_log
          to_stdout
          ("Version in hhconfig and switch=" ^ !hhconfig_version_and_switch);
      Lwt.return_some { result_count = 0; result_extra_telemetry = None }
    (* any request/notification if we haven't yet initialized *)
    | (Pre_init, _, _) ->
      raise
        (Error.LspException
           {
             Error.code = Error.ServerNotInitialized;
             message = "Server not yet initialized";
             data = None;
           })
    | (Post_shutdown, _, _c) ->
      raise
        (Error.LspException
           {
             Error.code = Error.InvalidRequest;
             message = "already received shutdown request";
             data = None;
           })
    (* initialized notification *)
    | (_, _, NotificationMessage InitializedNotification) -> Lwt.return_none
    (* rage request *)
    | (_, _, RequestMessage (id, RageRequestFB)) ->
      let%lwt result = do_rageFB !state in
      respond_jsonrpc ~powered_by:Language_server id (RageResultFB result);
      Lwt.return_some
        { result_count = List.length result; result_extra_telemetry = None }
    | ( _,
        Some ide_service,
        NotificationMessage (DidChangeWatchedFilesNotification notification) )
      ->
      let open DidChangeWatchedFiles in
      let changes =
        List.map notification.changes ~f:(fun change ->
            ClientIdeMessage.Changed_file (lsp_uri_to_path change.uri))
      in
      let%lwt () =
        ide_rpc
          ide_service
          ~env
          ~tracking_id
          ~ref_unblocked_time
          ClientIdeMessage.(Disk_files_changed changes)
      in
      Lwt.return_none
    (* Text document completion: "AutoComplete!" *)
    | (_, Some ide_service, RequestMessage (id, CompletionRequest params)) ->
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let%lwt result =
        do_completion_local
          ide_service
          env
          tracking_id
          ref_unblocked_time
          editor_open_files
          params
      in
      respond_jsonrpc ~powered_by:Serverless_ide id (CompletionResult result);
      Lwt.return_some
        {
          result_count = List.length result.Completion.items;
          result_extra_telemetry = None;
        }
    (* Resolve documentation for a symbol: "Autocomplete Docblock!" *)
    | ( _,
        Some ide_service,
        RequestMessage (id, CompletionItemResolveRequest params) ) ->
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let%lwt result =
        do_resolve_local
          ide_service
          env
          tracking_id
          ref_unblocked_time
          editor_open_files
          params
      in
      respond_jsonrpc
        ~powered_by:Serverless_ide
        id
        (CompletionItemResolveResult result);
      Lwt.return_some { result_count = 1; result_extra_telemetry = None }
    (* Document highlighting in serverless IDE *)
    | (_, Some ide_service, RequestMessage (id, DocumentHighlightRequest params))
      ->
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let%lwt result =
        do_highlight_local
          ide_service
          env
          tracking_id
          ref_unblocked_time
          editor_open_files
          params
      in
      respond_jsonrpc
        ~powered_by:Serverless_ide
        id
        (DocumentHighlightResult result);
      Lwt.return_some
        { result_count = List.length result; result_extra_telemetry = None }
    (* Type coverage in serverless IDE *)
    | (_, Some ide_service, RequestMessage (id, TypeCoverageRequestFB params))
      ->
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let%lwt result =
        do_typeCoverage_localFB
          ide_service
          env
          tracking_id
          ref_unblocked_time
          editor_open_files
          params
      in
      respond_jsonrpc
        ~powered_by:Serverless_ide
        id
        (TypeCoverageResultFB result);
      Lwt.return_some
        {
          result_count = List.length result.TypeCoverageFB.uncoveredRanges;
          result_extra_telemetry = None;
        }
    (* Hover docblocks in serverless IDE *)
    | (_, Some ide_service, RequestMessage (id, HoverRequest params)) ->
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let%lwt result =
        do_hover_local
          ide_service
          env
          tracking_id
          ref_unblocked_time
          editor_open_files
          params
      in
      respond_jsonrpc ~powered_by:Serverless_ide id (HoverResult result);
      let result_count =
        match result with
        | None -> 0
        | Some { Hover.contents; _ } -> List.length contents
      in
      Lwt.return_some { result_count; result_extra_telemetry = None }
    | (_, Some ide_service, RequestMessage (id, DocumentSymbolRequest params))
      ->
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let%lwt result =
        do_documentSymbol_local
          ide_service
          env
          tracking_id
          ref_unblocked_time
          editor_open_files
          params
      in
      respond_jsonrpc
        ~powered_by:Serverless_ide
        id
        (DocumentSymbolResult result);
      Lwt.return_some
        { result_count = List.length result; result_extra_telemetry = None }
    | (_, Some ide_service, RequestMessage (id, WorkspaceSymbolRequest params))
      ->
      let%lwt result =
        do_workspaceSymbol_local
          ide_service
          env
          tracking_id
          ref_unblocked_time
          params
      in
      respond_jsonrpc
        ~powered_by:Serverless_ide
        id
        (WorkspaceSymbolResult result);
      Lwt.return_some
        { result_count = List.length result; result_extra_telemetry = None }
    | (_, Some ide_service, RequestMessage (id, DefinitionRequest params)) ->
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let%lwt result =
        do_definition_local
          ide_service
          env
          tracking_id
          ref_unblocked_time
          editor_open_files
          params
      in
      respond_jsonrpc ~powered_by:Serverless_ide id (DefinitionResult result);
      Lwt.return_some
        { result_count = List.length result; result_extra_telemetry = None }
    | (_, Some ide_service, RequestMessage (id, TypeDefinitionRequest params))
      ->
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let%lwt result =
        do_typeDefinition_local
          ide_service
          env
          tracking_id
          ref_unblocked_time
          editor_open_files
          params
      in
      respond_jsonrpc
        ~powered_by:Serverless_ide
        id
        (TypeDefinitionResult result);
      Lwt.return_some
        { result_count = List.length result; result_extra_telemetry = None }
    (* Resolve documentation for a symbol: "Autocomplete Docblock!" *)
    | (_, Some ide_service, RequestMessage (id, SignatureHelpRequest params)) ->
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let%lwt result =
        do_signatureHelp_local
          ide_service
          env
          tracking_id
          ref_unblocked_time
          editor_open_files
          params
      in
      respond_jsonrpc ~powered_by:Serverless_ide id (SignatureHelpResult result);
      let result_count =
        match result with
        | None -> 0
        | Some { SignatureHelp.signatures; _ } -> List.length signatures
      in
      Lwt.return_some { result_count; result_extra_telemetry = None }
    (* textDocument/formatting *)
    | (_, _, RequestMessage (id, DocumentFormattingRequest params)) ->
      let result = do_documentFormatting editor_open_files params in
      respond_jsonrpc
        ~powered_by:Language_server
        id
        (DocumentFormattingResult result);
      Lwt.return_some
        { result_count = List.length result; result_extra_telemetry = None }
    (* textDocument/rangeFormatting *)
    | (_, _, RequestMessage (id, DocumentRangeFormattingRequest params)) ->
      let result = do_documentRangeFormatting editor_open_files params in
      respond_jsonrpc
        ~powered_by:Language_server
        id
        (DocumentRangeFormattingResult result);
      Lwt.return_some
        { result_count = List.length result; result_extra_telemetry = None }
    (* textDocument/onTypeFormatting *)
    | (_, _, RequestMessage (id, DocumentOnTypeFormattingRequest params)) ->
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let result = do_documentOnTypeFormatting editor_open_files params in
      respond_jsonrpc
        ~powered_by:Language_server
        id
        (DocumentOnTypeFormattingResult result);
      Lwt.return_some
        { result_count = List.length result; result_extra_telemetry = None }
    (* editor buffer events *)
    | ( _,
        _,
        NotificationMessage
          ( DidOpenNotification _ | DidChangeNotification _
          | DidCloseNotification _ | DidSaveNotification _ ) ) ->
      let%lwt () =
        handle_editor_buffer_message
          ~state:!state
          ~ide_service
          ~env
          ~metadata
          ~ref_unblocked_time
          ~message
      in
      Lwt.return_none
    (* any request/notification that we can't handle yet *)
    | (In_init _, _, message) ->
      (* we respond with Operation_cancelled so that clients don't produce *)
      (* user-visible logs/warnings. *)
      raise
        (Error.LspException
           {
             Error.code = Error.RequestCancelled;
             message = Hh_server_initializing |> hh_server_state_to_string;
             data =
               Some
                 (Hh_json.JSON_Object
                    [
                      ("state", !state |> state_to_string |> Hh_json.string_);
                      ( "message",
                        Hh_json.string_
                          (Lsp_fmt.denorm_message_to_string message) );
                    ]);
           })
    (* textDocument/hover request *)
    | (Main_loop menv, _, RequestMessage (id, HoverRequest params)) ->
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let%lwt result = do_hover menv.conn ref_unblocked_time params in
      respond_jsonrpc ~powered_by:Hh_server id (HoverResult result);
      let result_count =
        match result with
        | None -> 0
        | Some { Hover.contents; _ } -> List.length contents
      in
      Lwt.return_some { result_count; result_extra_telemetry = None }
    (* textDocument/typeDefinition request *)
    | (Main_loop menv, _, RequestMessage (id, TypeDefinitionRequest params)) ->
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let%lwt result = do_typeDefinition menv.conn ref_unblocked_time params in
      respond_jsonrpc ~powered_by:Hh_server id (TypeDefinitionResult result);
      Lwt.return_some
        { result_count = List.length result; result_extra_telemetry = None }
    (* textDocument/definition request *)
    | (Main_loop menv, _, RequestMessage (id, DefinitionRequest params)) ->
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let%lwt result =
        do_definition menv.conn ref_unblocked_time editor_open_files params
      in
      respond_jsonrpc ~powered_by:Hh_server id (DefinitionResult result);
      Lwt.return_some
        { result_count = List.length result; result_extra_telemetry = None }
    (* textDocument/completion request *)
    | (Main_loop menv, _, RequestMessage (id, CompletionRequest params)) ->
      let do_completion =
        if env.use_ffp_autocomplete then
          do_completion_ffp
        else
          do_completion_legacy
      in
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let%lwt result = do_completion menv.conn ref_unblocked_time params in
      respond_jsonrpc ~powered_by:Hh_server id (CompletionResult result);
      Lwt.return_some
        {
          result_count = List.length result.Completion.items;
          result_extra_telemetry = None;
        }
    (* completionItem/resolve request *)
    | ( Main_loop menv,
        _,
        RequestMessage (id, CompletionItemResolveRequest params) ) ->
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let%lwt result =
        do_completionItemResolve menv.conn ref_unblocked_time params
      in
      respond_jsonrpc
        ~powered_by:Hh_server
        id
        (CompletionItemResolveResult result);
      Lwt.return_some { result_count = 1; result_extra_telemetry = None }
    (* workspace/symbol request *)
    | (Main_loop menv, _, RequestMessage (id, WorkspaceSymbolRequest params)) ->
      let%lwt result = do_workspaceSymbol menv.conn ref_unblocked_time params in
      respond_jsonrpc ~powered_by:Hh_server id (WorkspaceSymbolResult result);
      Lwt.return_some
        { result_count = List.length result; result_extra_telemetry = None }
    (* textDocument/documentSymbol request *)
    | (Main_loop menv, _, RequestMessage (id, DocumentSymbolRequest params)) ->
      let%lwt result = do_documentSymbol menv.conn ref_unblocked_time params in
      respond_jsonrpc ~powered_by:Hh_server id (DocumentSymbolResult result);
      Lwt.return_some
        { result_count = List.length result; result_extra_telemetry = None }
    (* textDocument/references request *)
    | (Main_loop menv, _, RequestMessage (id, FindReferencesRequest params)) ->
      let%lwt () = cancel_if_stale client timestamp long_timeout in
      let%lwt result = do_findReferences menv.conn ref_unblocked_time params in
      respond_jsonrpc ~powered_by:Hh_server id (FindReferencesResult result);
      Lwt.return_some
        { result_count = List.length result; result_extra_telemetry = None }
    (* textDocument/implementation request *)
    | (Main_loop menv, _, RequestMessage (id, ImplementationRequest params)) ->
      let%lwt () = cancel_if_stale client timestamp long_timeout in
      let%lwt result =
        do_goToImplementation menv.conn ref_unblocked_time params
      in
      respond_jsonrpc ~powered_by:Hh_server id (ImplementationResult result);
      Lwt.return_some
        { result_count = List.length result; result_extra_telemetry = None }
    (* textDocument/rename *)
    | (Main_loop menv, _, RequestMessage (id, RenameRequest params)) ->
      let%lwt result = do_documentRename menv.conn ref_unblocked_time params in
      respond_jsonrpc ~powered_by:Hh_server id (RenameResult result);
      let result_count =
        SMap.fold
          (fun _file changes tot -> tot + List.length changes)
          result.WorkspaceEdit.changes
          0
      in
      let result_extra_telemetry =
        Telemetry.create ()
        |> Telemetry.int_
             ~key:"files"
             ~value:(SMap.cardinal result.WorkspaceEdit.changes)
      in
      Lwt.return_some
        { result_count; result_extra_telemetry = Some result_extra_telemetry }
    (* textDocument/documentHighlight *)
    | (Main_loop menv, _, RequestMessage (id, DocumentHighlightRequest params))
      ->
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let%lwt result =
        do_documentHighlight menv.conn ref_unblocked_time params
      in
      respond_jsonrpc ~powered_by:Hh_server id (DocumentHighlightResult result);
      Lwt.return_some
        { result_count = List.length result; result_extra_telemetry = None }
    (* textDocument/typeCoverage *)
    | (Main_loop menv, _, RequestMessage (id, TypeCoverageRequestFB params)) ->
      let%lwt result = do_typeCoverageFB menv.conn ref_unblocked_time params in
      respond_jsonrpc ~powered_by:Hh_server id (TypeCoverageResultFB result);
      Lwt.return_some
        {
          result_count = List.length result.TypeCoverageFB.uncoveredRanges;
          result_extra_telemetry = None;
        }
    (* textDocument/toggleTypeCoverage *)
    | ( Main_loop menv,
        _,
        NotificationMessage (ToggleTypeCoverageNotificationFB params) ) ->
      let%lwt () =
        do_toggleTypeCoverageFB menv.conn ref_unblocked_time params
      in
      Lwt.return_none
    (* textDocument/signatureHelp notification *)
    | (Main_loop menv, _, RequestMessage (id, SignatureHelpRequest params)) ->
      let%lwt result = do_signatureHelp menv.conn ref_unblocked_time params in
      respond_jsonrpc ~powered_by:Hh_server id (SignatureHelpResult result);
      let result_count =
        match result with
        | None -> 0
        | Some result -> List.length result.SignatureHelp.signatures
      in
      Lwt.return_some { result_count; result_extra_telemetry = None }
    (* catch-all for client reqs/notifications we haven't yet implemented *)
    | (Main_loop _menv, _, message) ->
      let method_ = Lsp_fmt.message_name_to_string message in
      raise
        (Error.LspException
           {
             Error.code = Error.MethodNotFound;
             message = Printf.sprintf "not implemented: %s" method_;
             data = None;
           })
    (* catch-all for requests/notifications after shutdown request *)
    (* client message when we've lost the server *)
    | (Lost_server lenv, _, _) ->
      let open Lost_env in
      (* if trigger_on_lsp_method is set, our caller should already have        *)
      (* transitioned away from this state.                                     *)
      assert (not lenv.p.trigger_on_lsp);

      (* We deny all other requests. This is the only response that won't       *)
      (* produce logs/warnings on most clients...                               *)
      raise
        (Error.LspException
           {
             Error.code = Error.RequestCancelled;
             message = lenv.p.new_hh_server_state |> hh_server_state_to_string;
             data =
               Some
                 (Hh_json.JSON_Object
                    [
                      ("state", !state |> state_to_string |> Hh_json.string_);
                      ( "message",
                        Hh_json.string_
                          (Lsp_fmt.denorm_message_to_string message) );
                    ]);
           })
  in
  Lwt.return result_telemetry_opt

let handle_server_message
    ~(env : env) ~(state : state ref) ~(message : server_message) :
    result_telemetry option Lwt.t =
  let open Main_env in
  let%lwt () =
    match (!state, message) with
    (* server busy status *)
    | (_, { push = ServerCommandTypes.BUSY_STATUS status; _ }) ->
      (* if we're connected to hh_server, that can only be because
      we know its root, which can only be because we received initializeParams.
      So the following call won't fail! *)
      let p = initialize_params_exc () in
      let should_send_status =
        Lsp.Initialize.(p.initializationOptions.sendServerStatusEvents)
      in
      ( if should_send_status then
        let status_message =
          let open ServerCommandTypes in
          match status with
          | Needs_local_typecheck -> "needs_local_typecheck"
          | Doing_local_typecheck -> "doing_local_typecheck"
          | Done_local_typecheck -> "done_local_typecheck"
          | Doing_global_typecheck _ -> "doing_global_typecheck"
          | Done_global_typecheck _ -> "done_global_typecheck"
        in
        Lsp_helpers.telemetry_log to_stdout status_message );
      state := do_server_busy !state status;
      Lwt.return_unit
    (* textDocument/publishDiagnostics notification *)
    | (Main_loop menv, { push = ServerCommandTypes.DIAGNOSTIC (_, errors); _ })
      ->
      let uris_with_diagnostics =
        do_diagnostics menv.uris_with_diagnostics errors
      in
      state := Main_loop { menv with uris_with_diagnostics };
      Lwt.return_unit
    (* any server diagnostics that come after we've shut down *)
    | (_, { push = ServerCommandTypes.DIAGNOSTIC _; _ }) -> Lwt.return_unit
    (* server shut-down request *)
    | (Main_loop _menv, { push = ServerCommandTypes.NEW_CLIENT_CONNECTED; _ })
      ->
      let%lwt new_state =
        do_lost_server
          ~env
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
    | (_, { push = ServerCommandTypes.NEW_CLIENT_CONNECTED; _ }) ->
      let message = "unexpected close of absent server" in
      let stack = "" in
      raise (Server_fatal_connection_exception { Marshal_tools.message; stack })
    (* server fatal shutdown *)
    | (_, { push = ServerCommandTypes.FATAL_EXCEPTION e; _ }) ->
      raise (Server_fatal_connection_exception e)
    (* server non-fatal exception *)
    | ( _,
        {
          push =
            ServerCommandTypes.NONFATAL_EXCEPTION
              { Marshal_tools.message; stack };
          _;
        } ) ->
      raise (Server_nonfatal_exception (make_lsp_error message ~stack))
  in
  Lwt.return_none

(* After the server has sent 'hello', it means the persistent connection is   *)
(* ready, so we can send our backlog of file-edits to the server.             *)
let connect_after_hello (server_conn : server_conn) (state : state) : unit Lwt.t
    =
  log "connect_after_hello";
  let ignore = ref 0.0 in
  let%lwt () =
    try%lwt
      (* tell server we want persistent connection *)
      let oc = server_conn.oc in
      ServerCommandLwt.send_connection_type oc ServerCommandTypes.Persistent;
      let fd = oc |> Unix.descr_of_out_channel |> Lwt_unix.of_unix_file_descr in
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
      log "Diag_subscribe: clientLsp subscribing diagnostic 0";
      let%lwt () =
        rpc server_conn ignore (ServerCommandTypes.SUBSCRIBE_DIAGNOSTIC 0)
      in
      (* Extract the list of file changes we're tracking *)
      let editor_open_files =
        UriMap.elements
          (match state with
          | Main_loop menv -> Main_env.(menv.editor_open_files)
          | In_init ienv -> In_init_env.(ienv.editor_open_files)
          | Lost_server lenv -> Lost_env.(lenv.editor_open_files)
          | _ -> UriMap.empty)
      in
      (* send open files and unsaved buffers to server *)
      let float_unblocked_time = ref 0.0 in
      (* Note: do serially since these involve RPC calls. *)
      let%lwt () =
        Lwt_list.iter_s
          (fun (uri, textDocument) ->
            let filename = lsp_uri_to_path uri in
            let command =
              ServerCommandTypes.OPEN_FILE
                (filename, textDocument.TextDocumentItem.text)
            in
            rpc server_conn float_unblocked_time command)
          editor_open_files
      in
      Lwt.return_unit
    with e ->
      let message = Exn.to_string e in
      let stack = Printexc.get_backtrace () in
      log "connect_after_hello exception %s\n%s" message stack;
      raise (Server_fatal_connection_exception { Marshal_tools.message; stack })
  in
  Lwt.return_unit

let handle_server_hello ~(state : state ref) : result_telemetry option Lwt.t =
  let%lwt () =
    match !state with
    (* server completes initialization *)
    | In_init ienv ->
      let%lwt () = connect_after_hello ienv.In_init_env.conn !state in
      state := report_connect_end ienv;
      Lwt.return_unit
    (* any "hello" from the server when we weren't expecting it. This is so *)
    (* egregious that we can't trust anything more from the server.         *)
    | _ ->
      let message = "Unexpected hello" in
      let stack = "" in
      raise (Server_fatal_connection_exception { Marshal_tools.message; stack })
  in
  Lwt.return_none

let handle_client_ide_notification
    ~(notification : ClientIdeMessage.notification) :
    result_telemetry option Lwt.t =
  (* In response to ide_service notifications we have three goals:
  (1) in case of Done_init, we might have to announce the failure to the user
  (2) in a few other cases, we send telemetry events so that test harnesses
  get insight into the internal state of the ide_service
  (3) after every single event, includinng client_ide_notification events,
  our caller queries the ide_service for what status it wants to display to
  the user, so these notifications have the goal of triggering that refresh. *)
  match notification with
  | ClientIdeMessage.Done_init (Ok p) ->
    Lsp_helpers.telemetry_log to_stdout "[client-ide] Finished init: ok";
    Lsp_helpers.telemetry_log
      to_stdout
      (Printf.sprintf
         "[client-ide] Initialized; %d file changes to process"
         p.ClientIdeMessage.Processing_files.total);
    Lwt.return_none
  | ClientIdeMessage.Done_init (Error error_data) ->
    log_debug "<-- done_init";
    Lsp_helpers.telemetry_log to_stdout "[client-ide] Finished init: failure";
    let%lwt () = announce_ide_failure error_data in
    Lwt.return_none
  | ClientIdeMessage.Processing_files _ ->
    (* used solely for triggering a refresh of status by our caller; nothing
    for us to do here. *)
    Lwt.return_none
  | ClientIdeMessage.Done_processing ->
    Lsp_helpers.telemetry_log
      to_stdout
      "[client-ide] Done processing file changes";
    Lwt.return_none

let handle_tick
    ~(env : env) ~(state : state ref) ~(ref_unblocked_time : float ref) :
    result_telemetry option Lwt.t =
  EventLogger.recheck_disk_files ();
  (* Update the hh_server_status global variable, either by asking the monitor
  during In_init, or reading it from Main_env: *)
  latest_hh_server_status := get_hh_server_status !state;
  let%lwt () =
    match !state with
    (* idle tick while waiting for server to complete initialization *)
    | In_init ienv ->
      let open In_init_env in
      let time = Unix.time () in
      let delay_in_secs = int_of_float (time -. ienv.most_recent_start_time) in
      let%lwt () =
        if delay_in_secs <= 10 then
          Lwt.return_unit
        else
          (* terminate + retry the connection *)
            let%lwt new_state = connect ~env !state in
            state := new_state;
            Lwt.return_unit
      in
      Lwt.return_unit
    (* Tick when we're connected to the server *)
    | Main_loop menv ->
      let open Main_env in
      let%lwt () =
        if menv.needs_idle then begin
          (* If we're connected to a server and have no more messages in the queue, *)
          (* then we must let the server know we're idle, so it will be free to     *)
          (* handle command-line requests.                                          *)
          state := Main_loop { menv with needs_idle = false };
          let%lwt () =
            rpc menv.conn ref_unblocked_time ServerCommandTypes.IDE_IDLE
          in
          Lwt.return_unit
        end else
          Lwt.return_unit
      in
      Lwt.return_unit
    (* idle tick. No-op. *)
    | _ -> Lwt.return_unit
  in
  let (promise : unit Lwt.t) = EventLoggerLwt.flush () in
  ignore_promise_but_handle_failure
    promise
    ~desc:"tick-event-flush"
    ~terminate_on_failure:false;
  Lwt.return_none

let main (env : env) : Exit_status.t Lwt.t =
  Printexc.record_backtrace true;
  from := env.from;
  HackEventLogger.set_from !from;

  if env.verbose then begin
    Hh_logger.Level.set_min_level_stderr Hh_logger.Level.Debug;
    Hh_logger.Level.set_min_level_file Hh_logger.Level.Debug
  end else begin
    Hh_logger.Level.set_min_level_stderr Hh_logger.Level.Error;
    Hh_logger.Level.set_min_level_file Hh_logger.Level.Info
  end;
  (* The --verbose flag in env.verbose is the only thing that controls verbosity
  to stderr. Meanwhile, verbosity-to-file can be altered dynamically by the user.
  Why are they different? because we should write to stderr under a test harness,
  but we should never write to stderr when invoked by VSCode - it's not even guaranteed
  to drain the stderr pipe. *)
  let ide_service =
    if env.use_serverless_ide then
      Some
        (ref
           (ClientIdeService.make
              {
                ClientIdeMessage.init_id = env.init_id;
                verbose_to_stderr = env.verbose;
                verbose_to_file = env.verbose;
              }))
    else
      None
  in

  let client = Jsonrpc.make_queue () in
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
      if not (is_tick event) then
        log_debug "next event: %s" (event_to_string event);
      ref_event := Some event;
      ref_unblocked_time := Unix.gettimeofday ();

      (* maybe set a flag to indicate that we'll need to send an idle message *)
      state := handle_idle_if_necessary !state event;

      (* if we're in a lost-server state, some triggers cause us to reconnect *)
      let%lwt new_state =
        reconnect_from_lost_if_necessary ~env !state (`Event event)
      in
      state := new_state;

      (* we keep track of all open files and their contents *)
      state := track_open_and_recent_files !state event;

      (* we keep track of all files that have unsaved changes in them *)
      state := track_edits_if_necessary !state event;

      (* if a message comes from the server, maybe update our record of server state *)
      update_hh_server_state_if_necessary event;

      (* update status immediately if warranted *)
      if not (is_pre_init !state || is_post_shutdown !state) then begin
        state :=
          publish_hh_server_status_diagnostic !state !latest_hh_server_status;
        refresh_status ~env ~ide_service
      end;

      (* this is the main handler for each message*)
      let%lwt result_telemetry_opt =
        match event with
        | Client_message (metadata, message) ->
          handle_client_message
            ~env
            ~state
            ~client
            ~ide_service
            ~metadata
            ~message
            ~ref_unblocked_time
        | Client_ide_notification notification ->
          handle_client_ide_notification ~notification
        | Server_message message -> handle_server_message ~env ~state ~message
        | Server_hello -> handle_server_hello ~state
        | Tick -> handle_tick ~env ~state ~ref_unblocked_time
      in
      (* for LSP requests and notifications, we keep a log of what+when we responded.
      INVARIANT: every LSP request gets either a response logged here,
      or an error logged by one of the handlers below. *)
      log_response_if_necessary
        env
        event
        result_telemetry_opt
        !ref_unblocked_time;
      Lwt.return_unit
    with
    | Server_fatal_connection_exception { Marshal_tools.stack; message } ->
      if not (is_post_shutdown !state) then (
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
          | Some { Exit.stack = Utils.Callstack s; _ } ->
            s |> Exception.clean_stack
          | _ -> ""
        in
        let data =
          Some
            (Hh_json.JSON_Object
               [("server_finale_stack", Hh_json.string_ server_finale_stack)])
        in
        let e = make_lsp_error ~stack ~data message in
        (* Log all the things! *)
        hack_log_error
          !ref_event
          e
          Error_from_server_fatal
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
          | Some { Exit.msg = Some msg; _ } -> msg
          | Some { Exit.msg = None; exit_status; _ } ->
            Printf.sprintf
              "hh_server: stopped [%s]"
              (Exit_status.show exit_status)
          | _ -> "hh_server: stopped."
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
              { Exit.exit_status = Exit_status.Failed_to_load_should_abort; _ }
            ->
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
                  ~env
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
      let e = make_lsp_error ~stack message in
      hack_log_error
        !ref_event
        e
        Error_from_client_fatal
        !ref_unblocked_time
        env;
      Lsp_helpers.telemetry_error to_stdout (message ^ ", from_client\n" ^ stack);
      let () = exit_fail () in
      Lwt.return_unit
    | Client_recoverable_connection_exception { Marshal_tools.stack; message }
      ->
      let e = make_lsp_error ~stack message in
      hack_log_error
        !ref_event
        e
        Error_from_client_recoverable
        !ref_unblocked_time
        env;
      Lsp_helpers.telemetry_error to_stdout (message ^ ", from_client\n" ^ stack);
      Lwt.return_unit
    | (Server_nonfatal_exception e | Error.LspException e) as exn ->
      let exn = Exception.wrap exn in
      let error_source =
        match (e.Error.code, Exception.unwrap exn) with
        | (Error.RequestCancelled, _) -> Error_from_lsp_cancelled
        | (_, Server_nonfatal_exception _) -> Error_from_server_recoverable
        | (_, _) -> Error_from_lsp_misc
      in
      let e =
        make_lsp_error ~data:e.Error.data ~code:e.Error.code e.Error.message
      in
      respond_to_error !ref_event e;
      hack_log_error !ref_event e error_source !ref_unblocked_time env;
      Lwt.return_unit
    | exn ->
      let exn = Exception.wrap exn in
      let e =
        make_lsp_error
          ~stack:(Exception.get_backtrace_string exn)
          ~current_stack:false
          (Exception.get_ctor_string exn)
      in
      respond_to_error !ref_event e;
      hack_log_error !ref_event e Error_from_lsp_misc !ref_unblocked_time env;
      Lwt.return_unit
  in
  let rec main_loop () : unit Lwt.t =
    let%lwt () = process_next_event () in
    main_loop ()
  in
  let%lwt () = main_loop () in
  Lwt.return Exit_status.No_error
