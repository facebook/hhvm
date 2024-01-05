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

type args = {
  from: string;
  config: (string * string) list;
  ignore_hh_version: bool;
  naming_table: string option;
  notebook_mode: bool;
  verbose: bool;
  root_from_cli: Path.t;
}

type env = {
  args: args;
  root: Path.t;
  hhconfig_version_and_switch: string;
  local_config: ServerLocalConfig.t;
}

(** We're storing [env] as global state. It gets initialized right at the start of [main].
But we have to fill it with some kind of dummy default, for sake of the type system. *)
let env =
  ref
    {
      args =
        {
          from = "[init]";
          config = [];
          ignore_hh_version = false;
          naming_table = None;
          notebook_mode = false;
          verbose = false;
          root_from_cli = Path.dummy_path;
        };
      root = Path.dummy_path;
      hhconfig_version_and_switch = "[init]";
      local_config = ServerLocalConfig.default;
    }

(** When did this binary start? *)
let binary_start_time = Unix.gettimeofday ()

(************************************************************************)
(* Protocol orchestration & helpers                                     *)
(************************************************************************)

let see_output_hack = " See Output\xE2\x80\xBAHack for details." (* chevron *)

let fix_by_running_hh = "Try running `hh` at the command-line."

type incoming_metadata = {
  timestamp: float;  (** time this message arrived at stdin *)
  activity_id: string;
      (** comes from VSCode for the user-initiated action, which might include several LSP requests. Might
      be "_" for LSP requests/notifications that weren't initiated by VSCode-tracked actions. *)
  tracking_suffix: string;
      (** additional unique key, since [activity_id] might be shared between several LSP messages, or empty. *)
  tracking_id: string;
      (** This is just [activity_id].[tracking_suffix]. Is handy to keep it in one place. *)
}

type errors_from =
  | Errors_from_clientIdeDaemon of {
      errors: Errors.finalized_error list;
      validated: bool;
    }
      (** TEMPORARY, FOR VALIDATION ONLY. The validated flag says whether we've validated that
      errors-file contained the exact same errors as this list. Once we've done validation,
      we'll need neither. *)
  | Errors_from_errors_file
[@@deriving show { with_path = false }]

(** This type describes our connection to the errors.bin file, where hh_server writes
errors discovered during its current typecheck.

Here's an overview of how errors_conn works:
1. [handle_tick] attempts to update the global mutable [errors_conn] from [SeekingErrors] to [TailingErrors]
   every (idle) second, by trying to open the errors file.
2. If the global [errors_conn] is [TailingErrors q], when getting next event for any state,
   an [Errors_file of read_result] event may be produced if there were new errors (read_result) in q
  (and not other higher pri events).
3. handling the [Errors_file] event consists in calling [handle_errors_file_item].
4. [handle_errors_file_item] takes a [read_result] and updates the diagnostic map [Run_env.uris_with_standalone_diagnostics],
   or switches the global mutable [error_conn] from [TrailingErrors] to [SeekingErrors] in case of error or end of file. *)
type errors_conn =
  | SeekingErrors of {
      prev_st_ino: int option;
          (** The inode of the last errors.bin file we have dealt with, if any --
          we'll only attempt another seek (hence, update [seek_reason] or switch to [TailingErrors])
          once we find a new, different errors.bin file. *)
      seek_reason:
        ServerProgress.errors_file_error * ServerProgress.ErrorsRead.log_message;
          (** This contains the latest explanation of why we entered or remain in [SeekingErrors] state.
          "Latest" in the sense that every time [handle_tick] is unable to open or find a new errors-file,
          or when [handle_errors_file_item] enters [SeekingErrors] mode, then it will will update [seek_reason].
          * [ServerProgress.NothingYet] is used at initialization, before anything's yet been tried.
          * [ServerProgress.NothingYet] is also used when [handle_tick] couldn't open an errors-file.
          * [ServerProgress.Completed] is used when we were in [TailingErrors], and then [handle_errors_file_item]
            discovered that the errors-file was complete, and so switched over to [SeekingErrors]
          * Other cases are used when [handle_tick] calls [ServerProgress.ErrorsRead.openfile]
            and gets an error. The only possibilities are [ServerProgress.{Killed,Build_id_mismatch}].

          It's a bit naughty of us to report our own "seek reasons" (completed/nothing-yet) through
          the error codes that properly belong to [ServerProgress.ErrorsRead]. But they fit so well... *)
    }  (** Haven't yet found a suitable errors.bin, but looking! *)
  | TailingErrors of {
      start_time: float;
      fd: Unix.file_descr;
      q: ServerProgress.ErrorsRead.read_result Lwt_stream.t;
    }  (** We are tailing this errors.bin file *)

(** What triggered the errors that we're sending right now? *)
type errors_trigger =
  | Initialize_trigger
      (** After initialization is done, we send errors for all open files *)
  | Message_trigger of {
      message: lsp_message;
      metadata: incoming_metadata;
    }  (** In receipt of didOpen/didChange and similar, we update live errors *)

module Run_env = struct
  type stream_file = {
    file: Path.t;
    q: FindRefsWireFormat.half_open_one_based list Lwt_stream.t;
    partial_result_token: Lsp.partial_result_token;
  }

  type shellout_standard_response = {
    symbol: string;
    find_refs_action: ServerCommandTypes.Find_refs.action; [@opaque]
    ide_calculated_positions: Pos.absolute list UriMap.t; [@opaque]
        (** These are results calculated by clientIdeDaemon from its open files, one entry for each open file *)
    stream_file: stream_file option; [@opaque]
        (** If we want streaming results out of a shellout, go here *)
    hint_suffixes: string list; [@opaque]
        (** We got this list of hints from clientIdeDaemon, and pass it on to our shellout *)
  }
  [@@deriving show]

  (** This identifies an LSP *request* that triggered something (typically a shellout).
  It's strongly typed to be bound to a request, i.e. something that has an lsp_id,
  so we know that an LSP response is needed, and know how to respond. *)
  type triggering_request = {
    id: lsp_id;
    metadata: incoming_metadata;
    start_time_local_handle: float;
    request: lsp_request;
  }

  type shellable_type =
    | FindRefs of shellout_standard_response
    | GoToImpl of shellout_standard_response
    | Rename of {
        symbol_definition: Relative_path.t SymbolDefinition.t; [@opaque]
        find_refs_action: ServerCommandTypes.Find_refs.action; [@opaque]
        new_name: string;
        ide_calculated_patches: ServerRenameTypes.patch list; [@opaque]
      }
  [@@deriving show]

  type current_hh_shell = {
    process:
      (Lwt_utils.Process_success.t, Lwt_utils.Process_failure.t) Lwt_result.t;
    cancellation_token: unit Lwt.u;
    triggering_request: triggering_request;
    shellable_type: shellable_type;
  }

  type t = {
    editor_open_files: Lsp.TextDocumentItem.t UriMap.t;
    uris_that_need_check: errors_trigger UriMap.t;
        (** files which we believe should have their errors rechecked, maybe
        because they had a didOpen/didChange, maybe because a file on disk
        changed. We'll recheck them once the queue is empty. Most recent
        triggering request goes at the front of the list. *)
    uris_with_unsaved_changes: UriSet.t;
        (** files which have received didOpen but not didClose.
        This isn't needed for correctness; only for validation...
        validation that error squiggles from clientIdeDaemon are the
        same *for unmodified files* as what hh_server's errors.bin says. *)
    uris_with_standalone_diagnostics: (float * errors_from) UriMap.t;
        (** these are diagnostics which arrived from serverless-ide or by tailing hh_server's
        errors.bin file, each with a timestamp of when they were discovered.
        THis isnt needed for correctness; only for validation...
        It's used during validating error squiggles from clientIdeDaemon vs errors.bin,
        using the timestamp to let us calculate whether a diagnostic discovered by
        clientIdeDaemon is more recent than one read from errors.bin.
        (It's also used if we fail to read a new errors.bin file to at least clean up
        after ourselves by deleting all the diagnostics we read from the previous one.
        And similarly to clean up if we get a shutdown request. We'd be fine without
        these two forms of clean up. *)
    current_hh_shell: current_hh_shell option;
        (** If a shell-out to "hh --ide-find-refs" or similar is currently underway.
        Invariant: if this is Some, then...
        (1) an LSP response will be delivered for it; when it is turned to None,
        either the LSP response has just been sent, or the obligation has been handed
        off to a method that will assuredly respond.
        (2) the streaming-find-refs file exists; whoever turns it to None they must
        also delete the file, which they can assume exists. *)
  }
end

type state =
  | Pre_init  (** Pre_init: we haven't yet received the initialize request. *)
  | Running of Run_env.t
      (** Running : this is the main state that we'll be in (after initialize request,
          before shutdown request) under ide_standalone. *)
  | Post_shutdown
      (** Post_shutdown: we received a shutdown request from the client, and
          therefore shut down our connection to the server. We can't handle
          any more requests from the client and will close as soon as it
          notifies us that we can exit. *)

let is_post_shutdown (state : state) : bool =
  match state with
  | Post_shutdown -> true
  | Pre_init
  | Running _ ->
    false

type result_handler = lsp_result -> state -> state Lwt.t

type result_telemetry = {
  result_count: int;  (** how many results did we send back to the user? *)
  result_extra_data: Telemetry.t option;  (** other message-specific data *)
  log_immediately: bool;
      (** Should we log telemetry about this response to an LSP action immediately?
      (default true, in case result_telemetry isn't provided.)
      Or should we defer logging until some later time, e.g. [handle_shell_out_complete]? *)
}

let make_result_telemetry
    ?(result_extra_data : Telemetry.t option)
    ?(log_immediately : bool = true)
    (count : int) : result_telemetry =
  { result_count = count; result_extra_data; log_immediately }

(* --ide-rename-by-symbol returns a list of patches *)
type ide_refactor_patch = {
  filename: string;
  line: int;
  char_start: int;
  char_end: int;
  patch_type: string;
  replacement: string;
}

let initialize_params_ref : Lsp.Initialize.params option ref = ref None

(** This flag is used to control how much will be written
to log-files. It can be turned on initially by --verbose at the command-line or
setting "trace:Verbose" in initializationParams. Thereafter, it can
be changed by the user dynamically via $/setTraceNotification.
Don't alter this reference directly; instead use [set_verbose_to_file]
so as to pass the message on to ide_service as well.
Note: control for how much will be written to stderr is solely
controlled by --verbose at the command-line, stored in env.verbose. *)
let verbose_to_file : bool ref = ref false

let requests_outstanding : (lsp_request * result_handler) IdMap.t ref =
  ref IdMap.empty

let get_outstanding_request_exn (id : lsp_id) : lsp_request =
  match IdMap.find_opt id !requests_outstanding with
  | Some (request, _) -> request
  | None -> failwith "response id doesn't correspond to an outstanding request"

(** hh_server pushes errors to an errors.bin file, which we tail along.
(Only in ide_standalone mode; we don't do anything in other modes.)
This variable is where we store our progress in seeking an errors.bin, or tailing
one we already found. It's updated
(1) by [handle_tick] which runs every second and may call [try_open_errors_file] to seek errors.bin;
(2) by [handle_errors_file_item] which tails an errors.bin file. *)
let latest_hh_server_errors : errors_conn ref =
  ref
    (SeekingErrors
       { prev_st_ino = None; seek_reason = (ServerProgress.NothingYet, "init") })

(** This is the latest state of the progress.json file, updated once a second
in a background Lwt process [background_status_refresher]. This file is where the
monitor reports its status like "starting up", and hh_server reports its status
like "typechecking". *)
let latest_hh_server_progress : ServerProgress.t ref =
  ref
    ServerProgress.
      {
        pid = 0;
        disposition = DWorking;
        message = "connecting";
        timestamp = Unix.gettimeofday ();
      }

(** Have we already sent a status message over LSP? If so, and our new
status will be just the same as the previous one, we won't need to send it
again. This stores the most recent status that the LSP client has. *)
let showStatus_outstanding : string ref = ref ""

let log s = Hh_logger.log ("[client-lsp] " ^^ s)

let log_debug s = Hh_logger.debug ("[client-lsp] " ^^ s)

let log_error s = Hh_logger.error ("[client-lsp] " ^^ s)

let to_stdout (json : Hh_json.json) : unit =
  let s = Hh_json.json_to_string json ^ "\r\n\r\n" in
  Http_lite.write_message stdout s

let get_editor_open_files (state : state) :
    Lsp.TextDocumentItem.t UriMap.t option =
  match state with
  | Pre_init
  | Post_shutdown ->
    None
  | Running lenv -> Some lenv.Run_env.editor_open_files

let ensure_cancelled (cancellation_token : unit Lwt.u) : unit =
  try Lwt.wakeup_later cancellation_token () with
  | Invalid_argument _ ->
    (* [wakeup_later] throws [Invalid_argument]] if the promise wasn't pending,
       e.g. if someone else had requested cancellation, but the cancellation token
       is still around and now we request cancellation on it too.
       (In other languages, this is typically a no-op rather than an exception...) *)
    ()

(** The architecture of clientLsp is a single-threaded message loop inside
[ClientLsp.main]. The loop calls [get_next_event] to wait for the next
event from a variety of sources, dispatches it according to what kind of event
it is, and repeats until it finally receives an event that an "exit" lsp notification
has been received from the client. *)
type event =
  | Client_message of incoming_metadata * lsp_message
      (** The editor e.g. VSCode might send a request or notification LSP message to us
      at any time. This event represents those LSP messages. The fields store
      raw json as well as the parsed form of it. Handled by [handle_client_message]. *)
  | Daemon_notification of ClientIdeMessage.notification
      (** This event represents whenever clientIdeDaemon
      pushes a notification to us, e.g. a progress or status update.
      Handled by [handle_client_ide_notification]. *)
  | Errors_file of ServerProgress.ErrorsRead.read_result option
      (** Under [--config ide_standalone=true], we once a second seek out a new errors.bin
      file that the server has produced to accumulate errors in the current typecheck.
      Once we have one, we "tail -f" it until it's finished. This event signals
      with [Some Ok _] that new errors have been appended to the file in the current typecheck,
      or [Some Error _] that either the typecheck completed or hh_server failed.
      The [None] case never arises; it represents a logic bug, an unexpected close
      of the underlying [Lwt_stream.t]. All handled in [handle_errors_file_item]. *)
  | Refs_file of FindRefsWireFormat.half_open_one_based list option
      (** If the editor sent a find-refs-request with a partialResultsToken, then
      we create a partial-results file and "tail -f" it until the shell-out to "hh --ide-find-refs-by-symbol3"
      has finished. This event signals with [Some] that new refs have been appended to the file.
      The [None] case never arises; it represents a logic bug, an unexpected close
      of the underlying [Lwt_stream.t]. *)
  | Shell_out_complete of
      ((Lwt_utils.Process_success.t, Lwt_utils.Process_failure.t) result
      * Run_env.triggering_request
      * Run_env.shellable_type)
      (** Under [--config ide_standlone=true], LSP requests for rename or
      find-refs are accomplished by kicking off an asynchronous shell-out to
      "hh --refactor" or "hh --ide-find-refs". This event signals that the
      shell-out has completed and it's now time to process the results.
      Handled in [handle_shell_out_complete].
      Invariant: if we have one of these events, then we must eventually produce
      an LSP response for it. *)
  | Deferred_check of DocumentUri.t * errors_trigger
      (** If there had been a didOpen/didChange without a subsequent didClose,
      and no other events are pending, we will recompute squiggles for it. *)
  | Tick
      (** Once a second, if no other events are pending, we synthesize a Tick
      event. Handled in [handle_tick]. It does things like see if there's a new
      errors.bin streaming-error file to tail, and flush HackEventLogger telemetry. *)

let event_to_string (event : event) : string =
  match event with
  | Client_message (metadata, m) ->
    Printf.sprintf
      "Client_message(#%s: %s)"
      metadata.tracking_id
      (Lsp_fmt.denorm_message_to_string m)
  | Daemon_notification n ->
    Printf.sprintf
      "Daemon_notification(%s)"
      (ClientIdeMessage.notification_to_string n)
  | Tick -> "Tick"
  | Refs_file None -> "Refs_file(anomalous end-of-stream)"
  | Refs_file (Some refs) ->
    Printf.sprintf "Refs_file(%d refs)" (List.length refs)
  | Errors_file None -> "Errors_file(anomalous end-of-stream)"
  | Errors_file (Some (Ok (ServerProgress.Telemetry _))) ->
    "Errors_file: telemetry"
  | Errors_file (Some (Ok (ServerProgress.Errors { errors; timestamp = _ }))) ->
  begin
    match Relative_path.Map.choose_opt errors with
    | None -> "Errors_file(anomalous empty report)"
    | Some (file, errors_in_file) ->
      Printf.sprintf
        "Errors_file(%d errors in %s, ...)"
        (List.length errors_in_file)
        (Relative_path.suffix file)
  end
  | Errors_file (Some (Error (e, log_message))) ->
    Printf.sprintf
      "Errors_file: %s [%s]"
      (ServerProgress.show_errors_file_error e)
      log_message
  | Shell_out_complete _ -> "Shell_out_complete"
  | Deferred_check (uri, _trigger) ->
    Printf.sprintf "Deferred_check(%s)" (Lsp.string_of_uri uri)

let is_tick (event : event) : bool =
  match event with
  | Tick -> true
  | Errors_file _
  | Refs_file _
  | Shell_out_complete _
  | Client_message _
  | Daemon_notification _
  | Deferred_check _ ->
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

exception Daemon_nonfatal_exception of Lsp.Error.t

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
      with
      | exn ->
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
          ~root:!env.root
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
  | Running _ -> "Running "
  | Post_shutdown -> "Post_shutdown"

(** This conversion is imprecise.  Comments indicate potential gaps *)
let completion_kind_to_si_kind
    (completion_kind : Completion.completionItemKind option) : FileInfo.si_kind
    =
  let open Lsp in
  let open FileInfo in
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

let si_kind_to_completion_kind (kind : FileInfo.si_kind) :
    Completion.completionItemKind option =
  match kind with
  | FileInfo.SI_XHP
  | FileInfo.SI_Class ->
    Some Completion.Class
  | FileInfo.SI_ClassMethod -> Some Completion.Method
  | FileInfo.SI_Function -> Some Completion.Function
  | FileInfo.SI_Mixed
  | FileInfo.SI_LocalVariable ->
    Some Completion.Variable
  | FileInfo.SI_Property -> Some Completion.Field
  | FileInfo.SI_ClassConstant -> Some Completion.Constant
  | FileInfo.SI_Interface
  | FileInfo.SI_Trait ->
    Some Completion.Interface
  | FileInfo.SI_Enum -> Some Completion.Enum
  | FileInfo.SI_Namespace -> Some Completion.Module
  | FileInfo.SI_Constructor -> Some Completion.Constructor
  | FileInfo.SI_Keyword -> Some Completion.Keyword
  | FileInfo.SI_Literal -> Some Completion.Value
  | FileInfo.SI_GlobalConstant -> Some Completion.Constant
  | FileInfo.SI_Typedef -> Some Completion.TypeParameter
  | FileInfo.SI_Unknown -> None

let read_hhconfig_version (root : Path.t) : string Lwt.t =
  let file = Filename.concat (Path.to_string root) ".hhconfig" in
  let%lwt config = Config_file_lwt.parse_hhconfig file in
  match config with
  | Ok (_hash, config) ->
    let version =
      config
      |> Config_file.Getters.string_opt "version"
      |> Config_file_lwt.parse_version
      |> Config_file_lwt.version_to_string_opt
      |> Option.value ~default:"[NoVersion]"
    in
    Lwt.return version
  | Error message -> Lwt.return (Printf.sprintf "[NoHhconfig:%s]" message)

let read_hhconfig_version_and_switch (root : Path.t) : string Lwt.t =
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
  let%lwt hhconfig_version = read_hhconfig_version root in
  Lwt.return (hhconfig_version ^ hack_rc_mode ^ hh_home)

let terminate_if_version_changed_since_start_of_lsp () : unit Lwt.t =
  let%lwt current_version_and_switch =
    read_hhconfig_version_and_switch !env.root
  in
  let is_ok =
    String.equal !env.hhconfig_version_and_switch current_version_and_switch
  in
  if is_ok then
    Lwt.return_unit
  else
    (* In these cases we have to terminate our LSP server, and trust
       VSCode to restart us. Note that we can't do clientStart because that
       would start our (old) version of hh_server, not the new one! *)
    let message =
      ""
      ^ "Version in hhconfig+switch that spawned the current hh_client: "
      ^ !env.hhconfig_version_and_switch
      ^ "\nVersion in hhconfig+switch currently: "
      ^ current_version_and_switch
      ^ "\n"
    in
    Lsp_helpers.telemetry_log to_stdout message;
    exit_fail ()

(** get_uris_with_unsaved_changes is the set of files for which we've
received didChange but haven't yet received didSave/didClose. *)
let get_uris_with_unsaved_changes (state : state) : UriSet.t =
  match state with
  | Running lenv -> lenv.Run_env.uris_with_unsaved_changes
  | Pre_init
  | Post_shutdown ->
    UriSet.empty

(** This cancellable async function will block indefinitely until a notification is
available from ide_service, and return. *)
let pop_from_ide_service (ide_service : ClientIdeService.t ref) : event Lwt.t =
  let%lwt notification_opt =
    Lwt_message_queue.pop (ClientIdeService.get_notifications !ide_service)
  in
  match notification_opt with
  | None -> Lwt.task () |> fst (* a never-fulfilled, cancellable promise *)
  | Some notification -> Lwt.return (Daemon_notification notification)

(** This cancellable async function will block indefinitely until data is
available from the client, but won't read from it. If there's no client
then it awaits indefinitely. *)
let wait_until_client_has_data (client : Jsonrpc.t option) : unit Lwt.t =
  match client with
  | None -> Lwt.task () |> fst (* a never-fulfilled, cancellable promise *)
  | Some client ->
    let%lwt () =
      match Jsonrpc.await_until_message client with
      | `Already_has_message -> Lwt.return_unit
      | `Wait_for_data_here fd ->
        let fd = Lwt_unix.of_unix_file_descr fd in
        let%lwt () = Lwt_unix.wait_read fd in
        Lwt.return_unit
    in
    Lwt.return_unit

(** Determine whether to read a message from the client (the editor) if
we've yet received an initialize message, or from
clientIdeDaemon, or whether neither is ready within 1s. *)
let get_client_message_source
    (client : Jsonrpc.t option)
    (ide_service : ClientIdeService.t ref)
    (q_opt : ServerProgress.ErrorsRead.read_result Lwt_stream.t option)
    (refs_q_opt :
      FindRefsWireFormat.half_open_one_based list Lwt_stream.t option)
    (uri_needs_check : (DocumentUri.t * errors_trigger) option) :
    [ `From_client
    | `From_ide_service of event
    | `From_q of ServerProgress.ErrorsRead.read_result option
    | `From_refs_q of FindRefsWireFormat.half_open_one_based list option
    | `From_uris_that_need_check of DocumentUri.t * errors_trigger
    | `No_source
    ]
    Lwt.t =
  (* This code embodies the following rules:
     1. If something is available from the client, then pick it
     2. Otherwise if client/ide_service/errors-q/refs-q is available right away then pick one arbitrarily
     3. Otherwise if there's a uri-that-needs-check then pick it
     4. Otherwise wait at most 1 second for any of the above
     5. Otherwise return an idle [Tick].

     Alas Lwt.pick doesn't let us prioritize amongst the items we're awaiting.
     So we do it a bit hacky, using "0.01 seconds" as an artificial delay
     to make file-that-needs-check be avoided if anything else is available immediately. *)
  let delay_to_force_lower_priority = 0.01 in
  if Option.value_map client ~default:false ~f:Jsonrpc.has_message then
    Lwt.return `From_client
  else
    let%lwt message_source =
      Lwt.pick
        ([
           (let%lwt () = Lwt_unix.sleep 1.0 in
            Lwt.return `No_source);
           (let%lwt () = wait_until_client_has_data client in
            Lwt.return `From_client);
           (let%lwt notification = pop_from_ide_service ide_service in
            Lwt.return (`From_ide_service notification));
         ]
        @ Option.value_map q_opt ~default:[] ~f:(fun q ->
              [Lwt_stream.get q |> Lwt.map (fun item -> `From_q item)])
        @ Option.value_map refs_q_opt ~default:[] ~f:(fun q ->
              [Lwt_stream.get q |> Lwt.map (fun item -> `From_refs_q item)])
        @
        match uri_needs_check with
        | None -> []
        | Some uri ->
          [
            (let%lwt () = Lwt_unix.sleep delay_to_force_lower_priority in
             Lwt.return (`From_uris_that_need_check uri));
          ])
    in
    Lwt.return message_source

(** get_next_event: picks up the next available message.
The way it's implemented, at the first character of a message,
we block until that message is completely received. *)
let get_next_event
    (state : state ref)
    (client : Jsonrpc.t)
    (ide_service : ClientIdeService.t ref) : event Lwt.t =
  let can_use_client =
    match !initialize_params_ref with
    | Some { Initialize.initializationOptions; _ }
      when initializationOptions.Initialize.delayUntilDoneInit -> begin
      match ClientIdeService.get_status !ide_service with
      | ClientIdeService.Status.(Initializing | Rpc _) -> false
      | ClientIdeService.Status.(Ready | Stopped _) -> true
    end
    | _ -> true
  in
  let client = Option.some_if can_use_client client in
  let from_client (client : Jsonrpc.t) : event Lwt.t =
    let%lwt message = Jsonrpc.get_message client in
    match message with
    | `Message { Jsonrpc.json; timestamp } -> begin
      try
        let message = Lsp_fmt.parse_lsp json get_outstanding_request_exn in
        let activity_key = Jget.obj_opt (Some json) "activityKey" in
        let activity_id =
          Jget.string_d
            activity_key
            "id"
            ~default:("[hh]" ^ Random_id.short_string ())
        in
        (* activity_id is used in telemetry for joins on other tables;
           tracking_suffix is used as extra disambiguation since activity-ids
           are sometimes shared between events *)
        let rnd = Random_id.short_string () in
        let tracking_suffix =
          match message with
          | RequestMessage (id, _) -> rnd ^ "." ^ Lsp_fmt.id_to_string id
          | _ -> rnd
        in
        let tracking_id = activity_id ^ "." ^ tracking_suffix in
        Lwt.return
          (Client_message
             ({ tracking_id; activity_id; tracking_suffix; timestamp }, message))
      with
      | e ->
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
  match !state with
  | Running ({ Run_env.current_hh_shell = Some sh; _ } as lenv)
    when not (Lwt.is_sleeping sh.Run_env.process) ->
    (* Invariant is that if current_hh_shell is Some, then we will eventually
       produce an LSP response for it. We're turning it into None here;
       the obligation to return an LSP response has been transferred onto
       our Shell_out_complete result. *)
    state := Running { lenv with Run_env.current_hh_shell = None };
    let%lwt result = sh.Run_env.process in
    Lwt.return
      (Shell_out_complete
         (result, sh.Run_env.triggering_request, sh.Run_env.shellable_type))
  | Pre_init
  | Running _
  | Post_shutdown ->
    (* invariant used by [handle_tick_event]: Errors_file events solely arise
       in state [Running ] in conjunction with [TailingErrors]. *)
    let q_opt =
      match (!latest_hh_server_errors, !state) with
      | (TailingErrors { q; _ }, Running _) -> Some q
      | _ -> None
    in
    let refs_q_opt =
      match !state with
      | Running
          Run_env.
            {
              current_hh_shell =
                Some
                  {
                    shellable_type = FindRefs { stream_file = Some { q; _ }; _ };
                    _;
                  };
              _;
            } ->
        Some q
      | _ -> None
    in
    let uri_needs_check =
      match !state with
      | Running Run_env.{ uris_that_need_check; _ } ->
        UriMap.choose_opt uris_that_need_check
      | _ -> None
    in
    let%lwt message_source =
      get_client_message_source
        client
        ide_service
        q_opt
        refs_q_opt
        uri_needs_check
    in
    (match message_source with
    | `From_client ->
      let%lwt message = from_client (Option.value_exn client) in
      Lwt.return message
    | `From_ide_service message -> Lwt.return message
    | `From_q message -> Lwt.return (Errors_file message)
    | `From_refs_q message -> Lwt.return (Refs_file message)
    | `No_source -> Lwt.return Tick
    | `From_uris_that_need_check (uri, trigger) ->
      Lwt.return (Deferred_check (uri, trigger)))

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
    (params : ShowStatusFB.params) ~(initialize_params : Lsp.Initialize.params)
    : unit =
  if not (Lsp_helpers.supports_status initialize_params) then
    ()
  else
    (* We try not to send duplicate statuses.
       That means: if you call request_showStatus but your message is the same as
       what's already up, then you won't be shown, and your callbacks won't be shown. *)
    let msg = params.ShowStatusFB.request.ShowStatusFB.message in
    if String.equal msg !showStatus_outstanding then
      ()
    else (
      showStatus_outstanding := msg;
      let id = NumberId (Jsonrpc.get_next_request_id ()) in
      let request = ShowStatusRequestFB params in
      to_stdout (print_lsp_request id request);

      let handler (_ : lsp_result) (state : state) : state Lwt.t =
        Lwt.return state
      in
      requests_outstanding :=
        IdMap.add id (request, handler) !requests_outstanding
    )

(** Dismiss all diagnostics. *)
let dismiss_diagnostics (state : state) : state =
  match state with
  | Running lenv ->
    let open Run_env in
    (* [uris_with_standalone_diagnostics] are the files-with-squiggles that we've reported
       to the editor; these came either from clientIdeDaemon or from tailing
       the errors.bin file in which hh_server accumulates diagnostics for the current typecheck. *)
    UriMap.iter
      (fun uri _time ->
        let params =
          { PublishDiagnostics.uri; diagnostics = []; isStatusFB = false }
        in
        let notification = PublishDiagnosticsNotification params in
        notification |> print_lsp_notification |> to_stdout)
      lenv.uris_with_standalone_diagnostics;
    Running { lenv with uris_with_standalone_diagnostics = UriMap.empty }
  | Pre_init -> Pre_init
  | Post_shutdown -> Post_shutdown

(************************************************************************)
(* Conversions - ad-hoc ones written as needed them, not systematic     *)
(************************************************************************)

let lsp_uri_to_path = Lsp_helpers.lsp_uri_to_path

let path_string_to_lsp_uri = Lsp_helpers.path_string_to_lsp_uri

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

let ide_shell_out_pos_to_lsp_location
    (pos : FindRefsWireFormat.half_open_one_based) : Lsp.Location.t =
  let open FindRefsWireFormat in
  Lsp.Location.
    {
      uri = path_string_to_lsp_uri pos.filename ~default_path:pos.filename;
      range =
        {
          start = { line = pos.line - 1; character = pos.char_start - 1 };
          end_ = { line = pos.line - 1; character = pos.char_end - 1 };
        };
    }

let hack_pos_to_lsp_location (pos : Pos.absolute) ~(default_path : string) :
    Lsp.Location.t =
  Lsp.Location.
    {
      uri = path_string_to_lsp_uri (Pos.filename pos) ~default_path;
      range = Lsp_helpers.hack_pos_to_lsp_range ~equal:String.equal pos;
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

(** See documentation for `args` field `notebook_mode` *)
let diagnostics_to_exclude_from_notebooks =
  SSet.of_list
    [
      "Hack does not support top level statements. Use the __EntryPoint attribute on a function instead";
      "The function prep is deprecated: use await, see https://fburl.com/goodbye-prep";
      "await cannot be used in a toplevel statement";
    ]

let hack_errors_to_lsp_diagnostic
    (filename : string) (errors : Errors.finalized_error list) :
    PublishDiagnostics.params =
  let open Lsp.Location in
  let location_message (message : Pos.absolute * string) :
      Lsp.Location.t * string =
    let (pos, message) = message in
    (* It's known and expected that hh_server sometimes sends an error
       with empty filename in its list of messages. These implicitly
       refer to the file in which the error is reported. *)
    let pos =
      if String.is_empty (Pos.filename pos) then
        Pos.set_file filename pos
      else
        pos
    in
    let { uri; range } = hack_pos_to_lsp_location pos ~default_path:filename in
    ({ Location.uri; range }, Markdown_lite.render message)
  in
  let hack_error_to_lsp_diagnostic (error : Errors.finalized_error) =
    let all_messages =
      User_error.to_list error |> List.map ~f:location_message
    in
    let (first_message, additional_messages) =
      match all_messages with
      | hd :: tl -> (hd, tl)
      | [] -> failwith "Expected at least one error in the error list"
    in
    let ( {
            range;
            uri =
              (* This is the file of the first message of the error which is supposed to correspond to [filename] *)
              _;
          },
          message ) =
      first_message
    in
    let relatedInformation =
      additional_messages
      |> List.map ~f:(fun (location, message) ->
             {
               PublishDiagnostics.relatedLocation = location;
               relatedMessage = message;
             })
    in
    let first_loc = fst first_message in
    let custom_errors =
      List.map error.User_error.custom_msgs ~f:(fun relatedMessage ->
          PublishDiagnostics.{ relatedLocation = first_loc; relatedMessage })
    in
    let relatedInformation = relatedInformation @ custom_errors in
    {
      Lsp.PublishDiagnostics.range;
      severity = Some Lsp.PublishDiagnostics.Error;
      code = PublishDiagnostics.IntCode (User_error.get_code error);
      source = Some "Hack";
      message;
      relatedInformation;
      relatedLocations = relatedInformation (* legacy FB extension *);
    }
  in
  let should_include_diagnostic { Lsp.PublishDiagnostics.message; _ } =
    not
      (!env.args.notebook_mode
      && SSet.mem message diagnostics_to_exclude_from_notebooks)
  in
  (* The caller is required to give us a non-empty filename. If it is empty,
     the following path_to_lsp_uri will fall back to the default path - which
     is also empty - and throw, logging appropriate telemetry. *)
  let diagnostics =
    errors
    |> List.map ~f:hack_error_to_lsp_diagnostic
    |> List.filter ~f:should_include_diagnostic
  in

  {
    Lsp.PublishDiagnostics.uri =
      path_string_to_lsp_uri filename ~default_path:"";
    isStatusFB = false;
    diagnostics;
  }

(** Retrieves a TextDocumentItem for a given URI from editor_open_files,
 or raises LSP [InvalidRequest] if not open *)
let get_text_document_item
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t) (uri : DocumentUri.t)
    : TextDocumentItem.t =
  match UriMap.find_opt uri editor_open_files with
  | Some document -> document
  | None ->
    raise
      (Error.LspException
         {
           Error.code = Error.InvalidRequest;
           message = "action on a file not open in LSP server";
           data = None;
         })

(** Retrieves the content of this file, or raises an LSP [InvalidRequest] exception
if the file isn't currently open *)
let get_document_contents
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t) (uri : DocumentUri.t)
    : string =
  let document = get_text_document_item editor_open_files uri in
  document.TextDocumentItem.text

(** Converts a single TextDocumentItem (an element in editor_open_files, for example) into a document that ClientIdeDaemon uses*)
let lsp_document_to_ide (text_document : TextDocumentItem.t) :
    ClientIdeMessage.document =
  let fn = text_document.TextDocumentItem.uri in
  {
    ClientIdeMessage.file_path = Path.make @@ Lsp_helpers.lsp_uri_to_path fn;
    file_contents = text_document.TextDocumentItem.text;
  }

(** Turns the complete set of editor_open_files into a format
suitable for clientIdeDaemon *)
let get_documents_from_open_files
    (editor_open_files : TextDocumentItem.t UriMap.t) :
    ClientIdeMessage.document list =
  editor_open_files |> UriMap.values |> List.map ~f:lsp_document_to_ide

(** Turns this Lsp uri+line+col into a format suitable for clientIdeDaemon.
Raises an LSP [InvalidRequest] exception if the file isn't currently open. *)
let get_document_location
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : Lsp.TextDocumentPositionParams.t) :
    ClientIdeMessage.document * ClientIdeMessage.location =
  let (file_path, line, column) = lsp_file_position_to_hack params in
  let uri =
    params.TextDocumentPositionParams.textDocument.TextDocumentIdentifier.uri
  in
  let file_path = Path.make file_path in
  let file_contents = get_document_contents editor_open_files uri in
  ( { ClientIdeMessage.file_path; file_contents },
    { ClientIdeMessage.line; column } )

(** Parses output of "hh --ide-find-references" and "hh --ide-go-to-impl".
If the output is malformed, raises an exception. *)
let shellout_locations_to_lsp_locations_exn (stdout : string) :
    Lsp.Location.t list =
  FindRefsWireFormat.IdeShellout.from_string_exn stdout
  |> List.map ~f:ide_shell_out_pos_to_lsp_location

let edit_of_patch (patch : ide_refactor_patch) : Lsp.TextEdit.t =
  let text =
    match patch.patch_type with
    | "insert"
    | "replace" ->
      patch.replacement
    | "remove" -> ""
    | e -> failwith ("invalid patch type: " ^ e)
  in
  let loc =
    let ide_shell_out_pos =
      {
        FindRefsWireFormat.filename = patch.filename;
        char_start = patch.char_start;
        char_end = patch.char_end + 1;
        (* This end is inclusive for range replacement *)
        line = patch.line;
      }
    in
    ide_shell_out_pos_to_lsp_location ide_shell_out_pos
  in
  { TextEdit.range = loc.Lsp.Location.range; newText = text }

(** Parses output of "hh --ide-rename-by-symbol".
If the output is malformed, raises an exception. *)
let shellout_patch_list_to_lsp_edits_exn (stdout : string) : Lsp.WorkspaceEdit.t
    =
  let patches =
    let json = Yojson.Safe.from_string stdout in
    match json with
    | `List positions ->
      List.bind positions ~f:(fun pos ->
          let open Yojson.Basic.Util in
          let lst_json = Yojson.Safe.to_basic pos in
          let filename = lst_json |> member "filename" |> to_string in
          let raw_patches = lst_json |> member "patches" |> to_list in
          List.map raw_patches ~f:(fun raw_patch ->
              let char_start = raw_patch |> member "col_start" |> to_int in
              let char_end = raw_patch |> member "col_end" |> to_int in
              let line = raw_patch |> member "line" |> to_int in
              let patch_type = raw_patch |> member "patch_type" |> to_string in
              let replacement =
                raw_patch |> member "replacement" |> to_string
              in
              { filename; char_start; char_end; line; patch_type; replacement }))
    | `Int _
    | `Tuple _
    | `Bool _
    | `Intlit _
    | `Null
    | `Variant _
    | `Assoc _
    | `Float _
    | `String _ ->
      failwith ("Expected list, got json like " ^ stdout)
  in
  let changes =
    List.fold patches ~init:Lsp.DocumentUri.Map.empty ~f:(fun acc patch ->
        let uri =
          Lsp_helpers.path_string_to_lsp_uri
            patch.filename
            ~default_path:patch.filename
        in
        let new_edits =
          let edit = edit_of_patch patch in
          let current_edits =
            Option.value ~default:[] (Lsp.DocumentUri.Map.find_opt uri acc)
          in
          edit :: current_edits
        in
        Lsp.DocumentUri.Map.add uri new_edits acc)
  in
  { WorkspaceEdit.changes }

(************************************************************************)
(* Connection and rpc                                                   *)
(************************************************************************)

let start_server () : unit =
  (* This basically does "hh_client start": a single attempt to open the     *)
  (* socket, send+read version and compare for mismatch, send handoff and    *)
  (* read response. It will print information to stderr. If the server is in *)
  (* an unresponsive or invalid state then it will kill the server. Next if  *)
  (* necessary it tries to spawn the server and wait until the monitor is    *)
  (* responsive enough to print "ready". It will do a hard program exit if   *)
  (* there were spawn problems.                                              *)
  let env_start =
    ClientStart.
      {
        root = !env.root;
        from = !env.args.from;
        no_load = false;
        watchman_debug_logging = false;
        log_inference_constraints = false;
        silent = true;
        exit_on_failure = false;
        ignore_hh_version = false;
        saved_state_ignore_hhconfig = false;
        mini_state = None;
        save_64bit = None;
        save_human_readable_64bit_dep_map = None;
        prechecked = None;
        config = !env.args.config;
        custom_hhi_path = None;
        custom_telemetry_data = [];
        allow_non_opt_build = false;
      }
  in
  let _exit_status = ClientStart.main env_start in
  ()

let announce_ide_failure (error_data : ClientIdeMessage.rich_error) : unit Lwt.t
    =
  let open ClientIdeMessage in
  let debug_details =
    Printf.sprintf
      "%s - %s"
      error_data.category
      (Option.value_map error_data.data ~default:"" ~f:Hh_json.json_to_string)
  in
  log
    "IDE services could not be initialized.\n%s\n%s"
    error_data.long_user_message
    debug_details;
  let input =
    Printf.sprintf "%s\n\n%s" error_data.long_user_message debug_details
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
        debug_details
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
    (ide_service : ClientIdeService.t)
    (initialize_params : Lsp.Initialize.params)
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t option) : unit Lwt.t =
  let open Lsp.Initialize in
  if
    not
      initialize_params.client_capabilities.workspace.didChangeWatchedFiles
        .dynamicRegistration
  then
    log_error "client doesn't support file-watching";

  let naming_table_load_info =
    match
      ( initialize_params.initializationOptions.namingTableSavedStatePath,
        !env.args.naming_table )
    with
    | (Some _, Some _) ->
      log_error
        "naming table path supplied from both LSP initialization param and command line";
      exit_fail ()
    | (Some path, _)
    | (_, Some path) ->
      Some
        {
          ClientIdeMessage.Initialize_from_saved_state.path = Path.make path;
          test_delay =
            initialize_params.initializationOptions
              .namingTableSavedStateTestDelay;
        }
    | (None, None) -> None
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
      ~root:!env.root
      ~naming_table_load_info
      ~config:!env.args.config
      ~ignore_hh_version:!env.args.ignore_hh_version
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
    (ClientIdeService.Stop_reason.to_log_string stop_reason);
  let%lwt () =
    ClientIdeService.stop ide_service ~tracking_id ~stop_reason ~e:None
  in
  Lwt.return_unit

(** This function looks at three sources of information (1) hh_server progress.json file,
(2) whether we're currently tailing an errors.bin file or whether we failed to open one,
(3) status of clientIdeDaemon. It synthesizes a status message suitable for display
in the VSCode status bar. *)
let merge_statuses_standalone
    (server : ServerProgress.t)
    (errors : errors_conn)
    (ide : ClientIdeService.t) : Lsp.ShowStatusFB.params =
  let ( (client_ide_disposition, client_ide_is_initializing),
        client_ide_message,
        client_ide_tooltip ) =
    match ClientIdeService.get_status ide with
    | ClientIdeService.Status.Initializing ->
      ( (ServerProgress.DWorking, true),
        "Hack: initializing",
        "Hack IDE support is initializing (loading saved state)" )
    | ClientIdeService.Status.Rpc _ ->
      ( (ServerProgress.DWorking, false),
        "Hack",
        "Hack is working on IDE requests" )
    | ClientIdeService.Status.Ready ->
      ((ServerProgress.DReady, false), "Hack", "Hack IDE support is ready")
    | ClientIdeService.Status.Stopped s ->
      ( (ServerProgress.DStopped, false),
        "Hack: " ^ s.ClientIdeMessage.short_user_message,
        s.ClientIdeMessage.medium_user_message ^ see_output_hack )
  in
  let (hh_server_disposition, hh_server_message, hh_server_tooltip) =
    let open ServerProgress in
    match (server, errors) with
    | ({ disposition = DStopped; message; _ }, _) ->
      ( DStopped,
        "Hack: hh_server " ^ message,
        Printf.sprintf "hh_server is %s. %s" message fix_by_running_hh )
    | ( _,
        SeekingErrors
          { seek_reason = (ServerProgress.Build_id_mismatch, log_message); _ }
      ) ->
      ( DStopped,
        "Hack: hh_server wrong version",
        Printf.sprintf
          "hh_server is the wrong version [%s]. %s"
          log_message
          fix_by_running_hh )
    | ({ disposition = DWorking; message; _ }, _) ->
      ( DWorking,
        (* following is little hack because "Hack: typechecking 1234/5678" is too big to fit
           in the VSCode status bar. So we shorten this special case. *)
        (if String.is_substring message ~substring:"typechecking" then
          message
        else
          "Hack: " ^ message),
        Printf.sprintf "hh_server is busy [%s]" message )
    | ( { disposition = DReady; _ },
        SeekingErrors
          {
            seek_reason = (ServerProgress.(NothingYet | Killed _), log_message);
            _;
          } ) ->
      ( DStopped,
        "Hack: hh_server stopped",
        Printf.sprintf
          "hh_server is stopped. [%s]. %s"
          log_message
          fix_by_running_hh )
    | ( { disposition = DReady; _ },
        SeekingErrors { seek_reason = (e, log_message); _ } )
      when not (ServerProgress.is_complete e) ->
      let e = ServerProgress.show_errors_file_error e in
      ( DStopped,
        "Hack: hh_server " ^ e,
        Printf.sprintf
          "hh_server error %s [%s]. %s"
          e
          log_message
          fix_by_running_hh )
    | ({ disposition = DReady; _ }, TailingErrors _) ->
      (DWorking, "Hack: typechecking", "hh_server is busy [typechecking].")
    | ({ disposition = DReady; _ }, _) -> (DReady, "Hack", "hh_server is ready")
  in
  let (disposition, message) =
    match
      (client_ide_disposition, client_ide_is_initializing, hh_server_disposition)
    with
    | (ServerProgress.DWorking, true, _) ->
      (ServerProgress.DWorking, client_ide_message)
    | (_, _, ServerProgress.DWorking) ->
      (ServerProgress.DWorking, hh_server_message)
    | (ServerProgress.DWorking, false, ServerProgress.DStopped) ->
      (ServerProgress.DWorking, hh_server_message)
    | (ServerProgress.DWorking, false, _) ->
      (ServerProgress.DWorking, client_ide_message)
    | (ServerProgress.DStopped, _, _) ->
      (ServerProgress.DStopped, client_ide_message)
    | (_, _, ServerProgress.DStopped) ->
      (ServerProgress.DStopped, hh_server_message)
    | (ServerProgress.DReady, _, ServerProgress.DReady) ->
      (ServerProgress.DReady, "Hack")
  in
  let root_tooltip =
    if Sys_utils.deterministic_behavior_for_tests () then
      "<ROOT>"
    else
      !env.root |> Path.to_string
  in
  let tooltip =
    Printf.sprintf
      "%s\n\n%s\n\n%s"
      root_tooltip
      client_ide_tooltip
      hh_server_tooltip
  in
  let type_ =
    match disposition with
    | ServerProgress.DStopped -> MessageType.ErrorMessage
    | ServerProgress.DWorking -> MessageType.WarningMessage
    | ServerProgress.DReady -> MessageType.InfoMessage
  in
  {
    ShowStatusFB.shortMessage = Some message;
    request = { ShowStatusFB.type_; message = tooltip };
    progress = None;
    total = None;
    telemetry = None;
  }

let refresh_status ~(ide_service : ClientIdeService.t ref) : unit =
  match !initialize_params_ref with
  | Some initialize_params ->
    let status =
      merge_statuses_standalone
        !latest_hh_server_progress
        !latest_hh_server_errors
        !ide_service
    in
    request_showStatusFB status ~initialize_params
  | None ->
    (* [initialize_params_ref] gets set in the Initialize message. We can't report
       status unless we know what client capabilities were sent in that message. *)
    ()

(** This kicks off a permanent process which, once a second, reads progress.json
and calls [refresh_status]. Because it's an Lwt process, it's able to update
status even when we're stuck waiting for RPC. *)
let background_status_refresher (ide_service : ClientIdeService.t ref) : unit =
  let rec loop () =
    latest_hh_server_progress := ServerProgress.read ();
    refresh_status ~ide_service;
    let%lwt () = Lwt_unix.sleep 1.0 in
    loop ()
  in
  let _future = loop () in
  ()

(** A thin wrapper around ClientIdeMessage which turns errors into exceptions. *)
let ide_rpc
    (ide_service : ClientIdeService.t ref)
    ~(tracking_id : string)
    ~(ref_unblocked_time : float ref)
    (message : 'a ClientIdeMessage.t) : 'a Lwt.t =
  let%lwt result =
    ClientIdeService.rpc
      !ide_service
      ~tracking_id
      ~ref_unblocked_time
      ~progress:(fun () -> refresh_status ~ide_service)
      message
  in
  match result with
  | Ok result -> Lwt.return result
  | Error error_data -> raise (Daemon_nonfatal_exception error_data)

(** This sets up a background watcher which polls the file for size changes due
to it being appended to; each file size change, it writes to an [Lwt_stream.t] everything that
has been appended since last append. (Except: the first item it places on the stream
comes not from the file, but rather from the [open_file_results] parameter; moreover
subsequent items that it reads from the file are culled if they refer to files
mentioned in [open_file_results]. This exception is a vehicle for (1) the rest of the
code to read IDE-results and file-results in a uniform way, (2) an easy way for us
to "mask" file-results by culling out all file-results that pertain to files open in the IDE.)

Once the file has been unlinked, then
the background watcher will gather everything that has been appended until
that point, write it to the stream, close the stream, and terminate.
Note: if you have a watcher, and unlink the file, and start a new file
with the same name without first waiting for the stream to close, then
there might be a race condition. Suggestion: wait for the stream to close
before starting a new file! *)
let watch_refs_stream_file
    (file : Path.t)
    ~(open_file_results : (string * Pos.absolute) list Lsp.UriMap.t) :
    FindRefsWireFormat.half_open_one_based list Lwt_stream.t =
  let (q, add) = Lwt_stream.create () in
  let ide_results =
    open_file_results
    |> UriMap.values
    |> List.concat
    |> List.map ~f:(fun (_name, pos) -> FindRefsWireFormat.from_absolute pos)
  in
  if not (List.is_empty ide_results) then add (Some ide_results);
  let rec watch_loop fd file_pos =
    let%lwt fstat = Lwt_unix.fstat fd in
    match fstat.Unix.st_size with
    | size when file_pos = size && not (Path.file_exists file) ->
      (* The stream only gets closed if we've read everything up to the end... *)
      add None;
      Lwt.return_unit
    | size when file_pos = size ->
      let%lwt () = Lwt_unix.sleep 0.2 in
      watch_loop fd file_pos
    | _ ->
      let%lwt (results, file_pos) =
        Lwt_utils.with_lock fd Unix.F_RLOCK ~f:(fun () ->
            let r =
              FindRefsWireFormat.Ide_stream.read_from_locked_file
                (Lwt_unix.unix_file_descr fd)
                ~pos:file_pos
            in
            Lwt.return r)
      in
      let results =
        List.filter results ~f:(fun { FindRefsWireFormat.filename; _ } ->
            let uri = path_string_to_lsp_uri filename ~default_path:filename in
            not (UriMap.mem uri open_file_results))
      in
      if not (List.is_empty results) then add (Some results);
      watch_loop fd file_pos
  in
  let watcher () =
    let%lwt fd =
      Lwt_unix.openfile (Path.to_string file) [Unix.O_RDONLY] 0o666
    in
    let%lwt () = watch_loop fd 0 in
    let%lwt () = Lwt_unix.close fd in
    Lwt.return_unit
  in
  let (_watcher_future : unit Lwt.t) = watcher () in
  q

(** If there was a streaming find-refs result file, this function will unlink it.
We do this first just to clean up after ourselves, but second because doing this will
terminate the [watch_refs_stream_file] background thread. *)
let unlink_refs_stream_file_if_present (shellable_type : Run_env.shellable_type)
    : unit =
  match shellable_type with
  | Run_env.(FindRefs { stream_file = Some { file; _ }; _ }) ->
    (try
       Unix.unlink (Path.to_string file);
       log "unlinked streaming-find-refs file %s" (Path.to_string file)
     with
    | exn ->
      let e = Exception.wrap exn in
      (* The [current_hh_shell] invariant is that if and only if it is Some, then the stream-file exists:
         whoever changes [current_hh_shell] to None should be responsible for deleting it -- by calling this
         function -- and can thereby be sure that the file exists when they try to delete it.
         Hence: going down this codepath via ENOENT is unexpected. Some might say it's worth logging
         as an indication that something has gone wrong. On the other hand, the name+contract of this
         function is to unlink the file "if present". I side with the first, which is why I'm logging. *)
      HackEventLogger.invariant_violation_bug
        ~data:(Exception.to_string e)
        "Streaming find-refs file unlinking error";
      Hh_logger.log "Streaming find-refs unlinking error")
  | _ -> ()

(** Historical quirk: we log kind and method-name a bit idiosyncratically... *)
let get_message_kind_and_method_for_logging (message : lsp_message) :
    string * string =
  match message with
  | ResponseMessage (_id, result) ->
    ("Response", Lsp.lsp_result_to_log_string result)
  | RequestMessage (_id, r) -> ("Request", Lsp_fmt.request_name_to_string r)
  | NotificationMessage n ->
    ("Notification", Lsp_fmt.notification_name_to_string n)

let get_filename_in_message_for_logging (message : lsp_message) :
    Relative_path.t option =
  let uri_opt = Lsp_fmt.get_uri_opt message in
  match uri_opt with
  | None -> None
  | Some uri ->
    (try
       let path = Lsp_helpers.lsp_uri_to_path uri in
       Some (Relative_path.create_detect_prefix path)
     with
    | _ ->
      Some (Relative_path.create Relative_path.Dummy (Lsp.string_of_uri uri)))

let log_response_if_necessary
    (event : event)
    (result_telemetry_opt : result_telemetry option)
    (unblocked_time : float) : unit =
  let (result_count, result_extra_telemetry, log_immediately) =
    match result_telemetry_opt with
    | None -> (None, None, true)
    | Some { result_count; result_extra_data; log_immediately } ->
      (Some result_count, result_extra_data, log_immediately)
  in
  let to_log =
    match event with
    | _ when not log_immediately -> None
    | Client_message (metadata, message) ->
      Some (metadata, unblocked_time, message)
    | Shell_out_complete (_result, triggering_request, _shellable_type) ->
      let { Run_env.id; metadata; start_time_local_handle; request } =
        triggering_request
      in
      Some (metadata, start_time_local_handle, RequestMessage (id, request))
    | Tick
    | Daemon_notification _
    | Errors_file _
    | Refs_file _
    | Deferred_check _ ->
      None
  in
  match to_log with
  | None -> ()
  | Some
      ( { timestamp; activity_id; tracking_suffix = _; tracking_id = _ },
        start_handle_time,
        message ) ->
    let (kind, method_) = get_message_kind_and_method_for_logging message in
    HackEventLogger.client_lsp_method_handled
      ~root:!env.root
      ~method_
      ~kind
      ~path_opt:(get_filename_in_message_for_logging message)
      ~result_count
      ~result_extra_telemetry
      ~activity_id
      ~start_queue_time:timestamp
      ~start_hh_server_state:""
      ~start_handle_time
      ~serverless_ide_flag:"Ide_standalone"

type error_source =
  | Error_from_client_fatal
  | Error_from_client_recoverable
  | Error_from_daemon_recoverable
  | Error_from_lsp_cancelled
  | Error_from_lsp_misc

let hack_log_error
    (event : event option)
    (e : Lsp.Error.t)
    (source : error_source)
    (unblocked_time : float) : unit =
  let is_expected =
    match source with
    | Error_from_lsp_cancelled -> true
    | Error_from_client_fatal
    | Error_from_client_recoverable
    | Error_from_daemon_recoverable
    | Error_from_lsp_misc ->
      false
  in
  let source =
    match source with
    | Error_from_client_fatal -> "client_fatal"
    | Error_from_client_recoverable -> "client_recoverable"
    | Error_from_daemon_recoverable -> "daemon_recoverable"
    | Error_from_lsp_cancelled -> "lsp_cancelled"
    | Error_from_lsp_misc -> "lsp_misc"
  in
  if not is_expected then log "%s" (Lsp_fmt.error_to_log_string e);
  match event with
  | Some (Client_message (metadata, message)) ->
    let (kind, method_) = get_message_kind_and_method_for_logging message in
    HackEventLogger.client_lsp_method_exception
      ~root:!env.root
      ~method_
      ~kind
      ~path_opt:(get_filename_in_message_for_logging message)
      ~activity_id:metadata.activity_id
      ~start_queue_time:metadata.timestamp
      ~start_hh_server_state:""
      ~start_handle_time:unblocked_time
      ~message:e.Error.message
      ~data_opt:e.Error.data
      ~source
  | _ ->
    HackEventLogger.client_lsp_exception
      ~root:!env.root
      ~message:e.Error.message
      ~data_opt:e.Error.data
      ~source

let kickoff_shell_out_and_maybe_cancel
    (state : state)
    (shellable_type : Run_env.shellable_type)
    ~(triggering_request : Run_env.triggering_request) :
    (state * result_telemetry) Lwt.t =
  let%lwt () = terminate_if_version_changed_since_start_of_lsp () in
  let compose_shellout_cmd ~(from : string) (args : string list) : string array
      =
    args
    @ [
        "--from";
        Printf.sprintf "clientLsp:%s" from;
        "--json";
        "--autostart-server";
        "false";
        !env.root |> Path.to_string;
      ]
    |> Array.of_list
  in
  match state with
  | Running ({ Run_env.current_hh_shell; _ } as lenv) ->
    (* Cancel any existing shell-out, if there is one. *)
    Option.iter
      current_hh_shell
      ~f:(fun
           {
             Run_env.cancellation_token;
             shellable_type;
             triggering_request =
               {
                 Run_env.id = prev_id;
                 metadata = prev_metadata;
                 request = prev_request;
                 start_time_local_handle = prev_start_time_local_handle;
               };
             _;
           }
         ->
        unlink_refs_stream_file_if_present shellable_type;
        (* Send SIGTERM to the underlying process.
           We won't wait for it -- that'd make it too hard to reason about concurrency. *)
        ensure_cancelled cancellation_token;
        (* We still have to respond to the LSP's prev request. We'll do so now
           without waiting for the underlying process to finish. In this way we fulfill
           the invariant that "current_hh_shell must always give rise to an LSP response".
           (Later in the function we overwrite current_hh_shell with the new shellout, so right here
           right now is the only place that can fulfill that invariant for the prev shellout). *)
        let lsp_error =
          make_lsp_error
            ~code:Lsp.Error.RequestCancelled
            "Cancelled (displaced by another request)"
            ~data:
              (Some
                 (Hh_json.JSON_Object
                    [
                      ( "new_request_id",
                        Hh_json.string_
                          (Lsp_fmt.id_to_string triggering_request.Run_env.id)
                      );
                    ]))
        in
        respond_jsonrpc
          ~powered_by:Language_server
          prev_id
          (ErrorResult lsp_error);
        hack_log_error
          (Some
             (Client_message
                (prev_metadata, RequestMessage (prev_id, prev_request))))
          lsp_error
          Error_from_daemon_recoverable
          prev_start_time_local_handle;
        ());
    let (cancel, cancellation_token) = Lwt.wait () in
    let cmd =
      begin
        let open Run_env in
        match shellable_type with
        | FindRefs
            {
              symbol;
              find_refs_action;
              hint_suffixes;
              stream_file;
              ide_calculated_positions = _;
            } ->
          let (action, stream_file, hints) =
            FindRefsWireFormat.CliArgs.to_string_triple
              {
                FindRefsWireFormat.CliArgs.symbol_name = symbol;
                action = find_refs_action;
                stream_file =
                  Option.map stream_file ~f:(fun sf -> sf.Run_env.file);
                hint_suffixes;
              }
          in
          let cmd =
            compose_shellout_cmd
              ~from:"find-refs-by-symbol"
              ["--ide-find-refs-by-symbol3"; action; stream_file; hints]
          in
          cmd
        | GoToImpl { symbol; find_refs_action; _ } ->
          let symbol_action_arg =
            FindRefsWireFormat.CliArgs.to_string
              {
                FindRefsWireFormat.CliArgs.symbol_name = symbol;
                action = find_refs_action;
                stream_file = None;
                hint_suffixes = [];
              }
          in
          let cmd =
            compose_shellout_cmd
              ~from:"go-to-impl-by-symbol"
              ["--ide-go-to-impl-by-symbol"; symbol_action_arg]
          in
          cmd
        | Rename { symbol_definition; find_refs_action; new_name; _ } ->
          let name_action_definition_string =
            ServerCommandTypes.Rename.arguments_to_string_exn
              new_name
              find_refs_action
              symbol_definition
          in
          let cmd =
            compose_shellout_cmd
              ~from:"rename"
              ["--ide-rename-by-symbol"; name_action_definition_string]
          in
          cmd
      end
    in
    (* Two environment variables FIND_HH_START_TIME and FIND_HH_RETRIED are
       used in HackEventLogger.ml for telemetry, expected to be set by the
       caller find_hh.sh, but [Exec_command.Current_executable] bypasses
       find_hh.sh and hence we need to override them ourselves. There's no
       need for removing existing definitions from [Unix.environment ()]
       because glibc putenv/getenv will grab the first one. *)
    let env =
      Array.append
        [|
          Printf.sprintf "FIND_HH_START_TIME=%0.3f" (Unix.gettimeofday ());
          "FIND_HH_RETRIED=0";
        |]
        (Unix.environment ())
    in
    let process =
      Lwt_utils.exec_checked Exec_command.Current_executable cmd ~cancel ~env
    in
    log "kickoff_shell_out: %s" (String.concat ~sep:" " (Array.to_list cmd));
    let state =
      Running
        {
          lenv with
          Run_env.current_hh_shell =
            Some
              {
                Run_env.process;
                cancellation_token;
                shellable_type;
                triggering_request;
              };
        }
    in
    let result_telemetry = make_result_telemetry 0 ~log_immediately:false in
    Lwt.return (state, result_telemetry)
  | _ ->
    HackEventLogger.invariant_violation_bug "kickoff when not in Run_env";
    Lwt.return (state, make_result_telemetry 0)

(** If there's a current_hh_shell with [id], then send it a SIGTERM.
All we're doing is triggering the cancellation; we rely upon normal
[handle_shell_out_complete] for once the process has terminated. *)
let cancel_shellout_if_applicable (state : state) (id : lsp_id) : unit =
  match state with
  | Running
      {
        Run_env.current_hh_shell =
          Some
            {
              Run_env.cancellation_token;
              triggering_request = { Run_env.id = peek_id; _ };
              _;
            };
        _;
      }
    when Lsp.IdKey.compare id peek_id = 0 ->
    ensure_cancelled cancellation_token
  | _ -> ()

(************************************************************************)
(* Protocol                                                             *)
(************************************************************************)

let do_shutdown
    (state : state)
    (ide_service : ClientIdeService.t ref)
    (tracking_id : string) : state Lwt.t =
  log "Received shutdown request";
  let _state = dismiss_diagnostics state in
  let%lwt () =
    stop_ide_service
      !ide_service
      ~tracking_id
      ~stop_reason:ClientIdeService.Stop_reason.Editor_exited
  in
  Lwt.return Post_shutdown

let state_to_rage (state : state) : string =
  let uris_to_string uris =
    List.map uris ~f:(fun (DocumentUri.Uri uri) -> uri)
    |> String.concat ~sep:","
  in
  let timestamped_uris_to_string uris =
    List.map uris ~f:(fun (DocumentUri.Uri uri, (timestamp, errors_from)) ->
        Printf.sprintf
          "%s [%s] @ %0.2f"
          (show_errors_from errors_from)
          uri
          timestamp)
    |> String.concat ~sep:","
  in
  let details =
    match state with
    | Pre_init -> ""
    | Post_shutdown -> ""
    | Running lenv ->
      let open Run_env in
      Printf.sprintf
        ("editor_open_files: %s\n"
        ^^ "uris_with_unsaved_changes: %s\n"
        ^^ "urs_with_standalone_diagnostics: %s\n")
        (lenv.editor_open_files |> UriMap.keys |> uris_to_string)
        (lenv.uris_with_unsaved_changes |> UriSet.elements |> uris_to_string)
        (lenv.uris_with_standalone_diagnostics
        |> UriMap.elements
        |> timestamped_uris_to_string)
  in
  Printf.sprintf "clientLsp state: %s\n%s\n" (state_to_string state) details

let do_rageFB (state : state) : RageFB.result Lwt.t =
  let%lwt current_version_and_switch =
    read_hhconfig_version_and_switch !env.root
  in
  let data =
    Printf.sprintf
      ("%s\n\n"
      ^^ "version previously read from .hhconfig and switch: %s\n"
      ^^ "version in .hhconfig and switch: %s\n\n")
      (state_to_rage state)
      !env.hhconfig_version_and_switch
      current_version_and_switch
  in
  Lwt.return [{ RageFB.title = None; data }]

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
    |> Option.map
         ~f:(Lsp_helpers.hack_pos_to_lsp_range ~equal:Relative_path.equal)
  in
  if List.is_empty contents then
    None
  else
    Some { Hover.contents; range }

let do_hover
    (ide_service : ClientIdeService.t ref)
    (tracking_id : string)
    (ref_unblocked_time : float ref)
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : Hover.params) : Hover.result Lwt.t =
  let (document, location) = get_document_location editor_open_files params in
  let%lwt infos =
    ide_rpc
      ide_service
      ~tracking_id
      ~ref_unblocked_time
      (ClientIdeMessage.Hover (document, location))
  in
  Lwt.return (do_hover_common infos)

let do_typeDefinition
    (ide_service : ClientIdeService.t ref)
    (tracking_id : string)
    (ref_unblocked_time : float ref)
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : Definition.params) : TypeDefinition.result Lwt.t =
  let (document, location) = get_document_location editor_open_files params in
  let%lwt results =
    ide_rpc
      ide_service
      ~tracking_id
      ~ref_unblocked_time
      (ClientIdeMessage.Type_definition (document, location))
  in
  let file = Path.to_string document.ClientIdeMessage.file_path in
  let results =
    List.map results ~f:(fun nast_sid ->
        hack_pos_definition_to_lsp_identifier_location
          nast_sid
          ~default_path:file)
  in
  Lwt.return results

let do_definition
    (ide_service : ClientIdeService.t ref)
    (tracking_id : string)
    (ref_unblocked_time : float ref)
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : Definition.params) : (Definition.result * bool) Lwt.t =
  let (document, location) = get_document_location editor_open_files params in
  let%lwt results =
    ide_rpc
      ide_service
      ~tracking_id
      ~ref_unblocked_time
      (ClientIdeMessage.Definition (document, location))
  in
  let locations =
    List.map results ~f:(fun (_, definition) ->
        hack_symbol_definition_to_lsp_identifier_location
          definition
          ~default_path:(document.ClientIdeMessage.file_path |> Path.to_string))
  in
  let has_xhp_attribute =
    List.exists results ~f:(fun (occurence, _) ->
        SymbolOccurrence.is_xhp_literal_attr occurence)
  in
  Lwt.return (locations, has_xhp_attribute)

let make_ide_completion_response
    (result : AutocompleteTypes.ide_result) (filename : string) :
    Completion.completionList Lwt.t =
  let open AutocompleteTypes in
  let open Completion in
  let hack_to_insert (completion : autocomplete_item) :
      TextEdit.t * Completion.insertTextFormat * TextEdit.t list =
    let additional_edits =
      List.map completion.res_additional_edits ~f:(fun (text, range) ->
          TextEdit.{ range = ide_range_to_lsp range; newText = text })
    in
    let range = ide_range_to_lsp completion.res_replace_pos in
    match completion.res_insert_text with
    | InsertAsSnippet { snippet; fallback } ->
      let initialize_params = Option.value_exn !initialize_params_ref in
      if Lsp_helpers.supports_snippets initialize_params then
        (TextEdit.{ range; newText = snippet }, SnippetFormat, additional_edits)
      else
        (TextEdit.{ range; newText = fallback }, PlainText, additional_edits)
    | InsertLiterally text ->
      (TextEdit.{ range; newText = text }, PlainText, additional_edits)
  in

  let hack_completion_to_lsp (completion : autocomplete_item) :
      Completion.completionItem =
    let (textEdit, insertTextFormat, additionalTextEdits) =
      hack_to_insert completion
    in
    let pos =
      if String.equal (Pos.filename completion.res_decl_pos) "" then
        Pos.set_file filename completion.res_decl_pos
      else
        completion.res_decl_pos
    in
    let data =
      let (line, start, _) = Pos.info_pos pos in
      let filename = Pos.filename pos in
      let base_class =
        match completion.res_base_class with
        | Some base_class -> [("base_class", Hh_json.JSON_String base_class)]
        | None -> []
      in
      (* If we do not have a correct file position, skip sending that data *)
      if Int.equal line 0 && Int.equal start 0 then
        Some
          (Hh_json.JSON_Object
             ([("fullname", Hh_json.JSON_String completion.res_fullname)]
             @ base_class))
      else
        Some
          (Hh_json.JSON_Object
             ([
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
             @ base_class))
    in
    let hack_to_sort_text (completion : autocomplete_item) : string option =
      let sort_text = completion.res_sortText in
      let label = completion.res_label in
      let should_downrank label =
        String.length label > 2
        && String.equal (Str.string_before label 2) "__"
        || Str.string_match (Str.regexp_case_fold ".*do_not_use.*") label 0
      in
      let downranked_result_prefix_character = "~" in
      if should_downrank label then
        Some (downranked_result_prefix_character ^ label)
      else if
        Option.is_some sort_text
        && !env.local_config.ServerLocalConfig.autocomplete_sort_text
      then
        sort_text
      else
        Some label
    in
    {
      label = completion.res_label;
      kind = si_kind_to_completion_kind completion.AutocompleteTypes.res_kind;
      detail = Some completion.res_detail;
      documentation =
        Option.map completion.res_documentation ~f:(fun s ->
            MarkedStringsDocumentation [MarkedString s]);
      (* This will be filled in by completionItem/resolve. *)
      sortText = hack_to_sort_text completion;
      filterText = completion.res_filter_text;
      insertText = None;
      insertTextFormat = Some insertTextFormat;
      textEdit = Some textEdit;
      additionalTextEdits;
      command = None;
      data;
    }
  in
  Lwt.return
    {
      isIncomplete = not result.is_complete;
      items = List.map result.completions ~f:hack_completion_to_lsp;
    }

let do_completion
    (ide_service : ClientIdeService.t ref)
    (tracking_id : string)
    (ref_unblocked_time : float ref)
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : Completion.params) : Completion.result Lwt.t =
  let open Completion in
  let (document, location) =
    get_document_location editor_open_files params.loc
  in
  (* Other parameters *)
  let is_manually_invoked =
    match params.context with
    | None -> false
    | Some c -> is_invoked c.triggerKind
  in
  (* this is what I want to fix *)
  let request =
    ClientIdeMessage.Completion
      (document, location, { ClientIdeMessage.is_manually_invoked })
  in
  let%lwt infos =
    ide_rpc ide_service ~tracking_id ~ref_unblocked_time request
  in
  let filename = document.ClientIdeMessage.file_path |> Path.to_string in
  let%lwt response = make_ide_completion_response infos filename in
  Lwt.return response

exception NoLocationFound

let docblock_to_markdown (raw_docblock : DocblockService.result) :
    Completion.completionDocumentation option =
  match raw_docblock with
  | [] -> None
  | docblock ->
    Some
      (Completion.MarkedStringsDocumentation
         (Core.List.fold docblock ~init:[] ~f:(fun acc elt ->
              match elt with
              | DocblockService.Markdown txt -> MarkedString txt :: acc
              | DocblockService.HackSnippet txt ->
                MarkedCode ("hack", txt) :: acc
              | DocblockService.XhpSnippet txt ->
                MarkedCode ("html", txt) :: acc)))

let docblock_with_ranking_detail
    (raw_docblock : DocblockService.result) (ranking_detail : string option) :
    DocblockService.result =
  match ranking_detail with
  | Some detail -> raw_docblock @ [DocblockService.Markdown detail]
  | None -> raw_docblock

let resolve_ranking_source
    (kind : FileInfo.si_kind) (ranking_source : int option) : FileInfo.si_kind =
  match ranking_source with
  | Some x -> SearchTypes.int_to_kind x
  | None -> kind

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
let do_resolve
    (ide_service : ClientIdeService.t ref)
    (tracking_id : string)
    (ref_unblocked_time : float ref)
    (params : CompletionItemResolve.params) : CompletionItemResolve.result Lwt.t
    =
  if Option.is_some params.Completion.documentation then
    Lwt.return params
  else
    let raw_kind = params.Completion.kind in
    let kind = completion_kind_to_si_kind raw_kind in
    (* [do_completion] happens to provide line+col+filename for members.
       Since docblock resolution needs line+col+filename, and we've already computed them,
       let's just use them! *)
    let%lwt result =
      try
        match params.Completion.data with
        | None -> raise NoLocationFound
        | Some _ as data ->
          let fullname = Jget.string_exn params.Completion.data "fullname" in
          let filename = Jget.string_exn data "filename" in
          let file_path = Path.make filename in
          let line = Jget.int_exn data "line" in
          let column = Jget.int_exn data "char" in
          let ranking_detail = Jget.string_opt data "ranking_detail" in
          let ranking_source = Jget.int_opt data "ranking_source" in
          if line = 0 && column = 0 then failwith "NoFileLineColumnData";
          let request =
            ClientIdeMessage.Completion_resolve_location
              ( file_path,
                ClientIdeMessage.Full_name fullname,
                { ClientIdeMessage.line; column },
                resolve_ranking_source kind ranking_source )
          in
          let%lwt raw_docblock =
            ide_rpc ide_service ~tracking_id ~ref_unblocked_time request
          in
          let documentation =
            docblock_with_ranking_detail
              raw_docblock.ClientIdeMessage.Completion_resolve.docblock
              ranking_detail
            |> docblock_to_markdown
          in
          (match raw_docblock.ClientIdeMessage.Completion_resolve.signature with
          | None -> Lwt.return { params with Completion.documentation }
          | Some signature ->
            Lwt.return
              { params with Completion.documentation; detail = Some signature })
        (* If that fails, next try using symbol *)
      with
      | _ ->
        (* Completion.data was produced by [make_ide_completion_response]
         * which in all codepaths provides a "fullname" field to data *)
        let fullname = Jget.string_exn params.Completion.data "fullname" in
        let request =
          ClientIdeMessage.Completion_resolve
            ClientIdeMessage.Completion_resolve.{ fullname; kind }
        in

        let%lwt raw_docblock =
          ide_rpc ide_service ~tracking_id ~ref_unblocked_time request
        in
        let documentation =
          docblock_to_markdown
            raw_docblock.ClientIdeMessage.Completion_resolve.docblock
        in
        (match raw_docblock.ClientIdeMessage.Completion_resolve.signature with
        | None -> Lwt.return { params with Completion.documentation }
        | Some signature ->
          Lwt.return
            { params with Completion.documentation; detail = Some signature })
    in
    Lwt.return result

let hack_symbol_to_lsp (symbol : SearchUtils.symbol) =
  (* Hack sometimes gives us back items with an empty path, by which it
     intends "whichever path you asked me about". That would be meaningless
     here. If it does, then it'll pick up our default path (also empty),
     which will throw and go into our telemetry. That's the best we can do. *)
  let hack_to_lsp_kind = function
    | FileInfo.SI_Class -> SymbolInformation.Class
    | FileInfo.SI_Interface -> SymbolInformation.Interface
    | FileInfo.SI_Trait -> SymbolInformation.Interface
    (* LSP doesn't have traits, so we approximate with interface *)
    | FileInfo.SI_Enum -> SymbolInformation.Enum
    (* TODO(T36697624): Add SymbolInformation.Record *)
    | FileInfo.SI_ClassMethod -> SymbolInformation.Method
    | FileInfo.SI_Function -> SymbolInformation.Function
    | FileInfo.SI_Typedef -> SymbolInformation.Class
    (* LSP doesn't have typedef, so we approximate with class *)
    | FileInfo.SI_GlobalConstant -> SymbolInformation.Constant
    | FileInfo.SI_Namespace -> SymbolInformation.Namespace
    | FileInfo.SI_Mixed -> SymbolInformation.Variable
    | FileInfo.SI_XHP -> SymbolInformation.Class
    | FileInfo.SI_Literal -> SymbolInformation.Variable
    | FileInfo.SI_ClassConstant -> SymbolInformation.Constant
    | FileInfo.SI_Property -> SymbolInformation.Property
    | FileInfo.SI_LocalVariable -> SymbolInformation.Variable
    | FileInfo.SI_Constructor -> SymbolInformation.Constructor
    (* Do these happen in practice? *)
    | FileInfo.SI_Keyword
    | FileInfo.SI_Unknown ->
      failwith "Unknown symbol kind"
  in
  {
    SymbolInformation.name = Utils.strip_ns symbol.SearchUtils.name;
    kind = hack_to_lsp_kind symbol.SearchUtils.result_type;
    location = hack_pos_to_lsp_location symbol.SearchUtils.pos ~default_path:"";
    containerName = None;
  }

let do_workspaceSymbol
    (ide_service : ClientIdeService.t ref)
    (tracking_id : string)
    (ref_unblocked_time : float ref)
    (params : WorkspaceSymbol.params) : WorkspaceSymbol.result Lwt.t =
  let query = params.WorkspaceSymbol.query in
  let request = ClientIdeMessage.Workspace_symbol query in
  let%lwt results =
    ide_rpc ide_service ~tracking_id ~ref_unblocked_time request
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
    | SymbolDefinition.ClassConst -> SymbolInformation.Constant
    | SymbolDefinition.GlobalConst -> SymbolInformation.Constant
    | SymbolDefinition.Enum -> SymbolInformation.Enum
    | SymbolDefinition.Interface -> SymbolInformation.Interface
    | SymbolDefinition.Trait -> SymbolInformation.Interface
    (* LSP doesn't have traits, so we approximate with interface *)
    | SymbolDefinition.LocalVar -> SymbolInformation.Variable
    | SymbolDefinition.TypeVar -> SymbolInformation.TypeParameter
    | SymbolDefinition.Typeconst -> SymbolInformation.Class
    (* e.g. "const type Ta = string;" -- absent from LSP *)
    | SymbolDefinition.Typedef -> SymbolInformation.Class
    (* e.g. top level type alias -- absent from LSP *)
    | SymbolDefinition.Param -> SymbolInformation.Variable
    (* We never return a param from a document-symbol-search *)
    | SymbolDefinition.Module -> SymbolInformation.Module
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
    (ide_service : ClientIdeService.t ref)
    (tracking_id : string)
    (ref_unblocked_time : float ref)
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : DocumentSymbol.params) : DocumentSymbol.result Lwt.t =
  let open DocumentSymbol in
  let open TextDocumentIdentifier in
  let filename = lsp_uri_to_path params.textDocument.uri in
  let text_document =
    get_text_document_item editor_open_files params.textDocument.uri
  in
  let document = lsp_document_to_ide text_document in
  let request = ClientIdeMessage.Document_symbol document in
  let%lwt outline =
    ide_rpc ide_service ~tracking_id ~ref_unblocked_time request
  in
  let converted =
    hack_symbol_tree_to_lsp ~filename ~accu:[] ~container_name:None outline
  in
  Lwt.return converted

let do_findReferences
    (state : state)
    (ide_service : ClientIdeService.t ref)
    (metadata : incoming_metadata)
    (ref_unblocked_time : float ref)
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : FindReferences.params)
    (id : lsp_id) : (state * result_telemetry) Lwt.t =
  let start_time_local_handle = Unix.gettimeofday () in
  let (document, location) =
    get_document_location editor_open_files params.FindReferences.loc
  in
  let all_open_documents = get_documents_from_open_files editor_open_files in
  let%lwt response =
    ide_rpc
      ide_service
      ~tracking_id:metadata.tracking_id
      ~ref_unblocked_time
      (ClientIdeMessage.Find_references (document, location, all_open_documents))
  in
  match response with
  | ClientIdeMessage.Invalid_symbol ->
    respond_jsonrpc ~powered_by:Serverless_ide id (FindReferencesResult []);
    let result_telemetry = make_result_telemetry (-1) in
    Lwt.return (state, result_telemetry)
  | ClientIdeMessage.Find_refs_success
      {
        full_name = _;
        action = None;
        hint_suffixes = _;
        open_file_results = ide_calculated_positions;
      } ->
    let positions =
      match UriMap.values ide_calculated_positions with
      (*
          UriMap.values returns [Pos.absolute list list] and if we're here,
          we should only have one element - the list of positions in the file the localvar is defined.
        *)
      | positions :: [] -> positions
      | _ -> assert false (* Explicitly handled in ClientIdeDaemon *)
    in
    let filename =
      Lsp_helpers.lsp_textDocumentIdentifier_to_filename
        params.FindReferences.loc.TextDocumentPositionParams.textDocument
    in
    let positions =
      List.map positions ~f:(fun (_name, pos) ->
          hack_pos_to_lsp_location ~default_path:filename pos)
    in
    respond_jsonrpc
      ~powered_by:Serverless_ide
      id
      (FindReferencesResult positions);
    let result_telemetry = make_result_telemetry (List.length positions) in
    Lwt.return (state, result_telemetry)
  | ClientIdeMessage.Find_refs_success
      { full_name; action = Some action; hint_suffixes; open_file_results } ->
    let stream_file =
      Option.map
        params.FindReferences.partialResultToken
        ~f:(fun partial_result_token ->
          let file =
            Stdlib.Filename.temp_file
              ~temp_dir:(ServerFiles.get_tmp ())
              (Printf.sprintf "find_refs_stream_%d_pid" (Unix.getpid ()))
              ".jsonl"
          in
          let file = Path.make file in
          let q = watch_refs_stream_file file ~open_file_results in
          Run_env.{ file; partial_result_token; q })
    in
    (* The [open_file_results] included the SymbolOccurrence text for each ref. We won't need that... *)
    let ide_calculated_positions =
      UriMap.map (List.map ~f:snd) open_file_results
    in
    let shellable_type =
      Run_env.FindRefs
        {
          Run_env.symbol = full_name;
          find_refs_action = action;
          stream_file;
          hint_suffixes;
          ide_calculated_positions;
        }
    in
    (* If we reach kickoff a shell-out to hh_server, processing that response
       also invokes respond_jsonrpc *)
    let%lwt (state, result_telemetry) =
      kickoff_shell_out_and_maybe_cancel
        state
        shellable_type
        ~triggering_request:
          {
            Run_env.id;
            metadata;
            start_time_local_handle;
            request = FindReferencesRequest params;
          }
    in
    Lwt.return (state, result_telemetry)

(** Helper for sending $/progress messages for find-refs items. *)
let notify_refs_file_items
    (shellout_standard_response : Run_env.shellout_standard_response)
    (refs : FindRefsWireFormat.half_open_one_based list) : unit =
  match shellout_standard_response.Run_env.stream_file with
  | Some Run_env.{ partial_result_token; _ } ->
    let notification =
      FindReferencesPartialResultNotification
        ( partial_result_token,
          List.map refs ~f:ide_shell_out_pos_to_lsp_location )
    in
    notification |> print_lsp_notification |> to_stdout;
    ()
  | None ->
    log "Refs-file: notify-refs called, but no stream_file";
    HackEventLogger.invariant_violation_bug
      "refs-file: notify-refs called, but no stream file";
    ()

(** This is the main handler for when the main loop detects new
items in the streaming-find-refs file. *)
let handle_refs_file_items
    ~(state : state)
    (items : FindRefsWireFormat.half_open_one_based list option) :
    result_telemetry option =
  match (items, state) with
  | ( Some refs,
      Running
        Run_env.
          {
            current_hh_shell =
              Some { shellable_type = FindRefs shellout_standard_response; _ };
            _;
          } ) ->
    notify_refs_file_items shellout_standard_response refs;
    None
  | ( Some _,
      Running Run_env.{ current_hh_shell = Some { shellable_type; _ }; _ } ) ->
    let data = Run_env.show_shellable_type shellable_type in
    log "Refs-file: wrong shellable type to send refs - %s" data;
    HackEventLogger.invariant_violation_bug
      ~data
      "refs-file wrong shellale type to send refs";
    None
  | (Some _, Running Run_env.{ current_hh_shell = None; _ }) ->
    log "Refs-file: got items when current_hh_shell is None";
    HackEventLogger.invariant_violation_bug
      "refs-file got items when current_hh_shell is None";
    None
  | (Some _, _) ->
    let data = state_to_string state in
    log "Refs-file: unexpected items in wrong state - %s" data;
    HackEventLogger.invariant_violation_bug
      ~data
      "refs-file unexpected items in wrong state";
    None
  | (None, _) ->
    (* This signals that the stream has been closed. The function [watch_refs_stream_file] only closes
       the stream when the stream-file has been unlinked. We only unlink the file in [handle_shell_out_complete],
       which also updates [state] to no longer be watching this file. Therefore, we should never encounter
       a closed stream. *)
    log "Refs-file: unexpected end of stream";
    HackEventLogger.invariant_violation_bug "refs-file unexpected end of stream";
    None

(** This is called when a shell-out to "hh --ide-find-refs-by-symbol" completes successfully *)
let do_findReferences2 ~shellout_standard_response ~hh_locations :
    (Lsp.lsp_result * int) Lwt.t =
  let Run_env.{ ide_calculated_positions; stream_file; _ } =
    shellout_standard_response
  in
  (* lsp_locations return a file for each position. To support unsaved, edited files
     let's discard locations for files that we previously fetched from ClientIDEDaemon.

     First: filter out locations for any where the DocumentUri.t matches a relative_path
     in the returned map
     Second: augment above list with values in `ide_calculated_positions`
  *)
  let hh_locations =
    List.filter
      ~f:(fun location ->
        let uri = location.Lsp.Location.uri in
        not @@ UriMap.mem uri ide_calculated_positions)
      hh_locations
  in
  let ide_locations =
    UriMap.bindings ide_calculated_positions
    |> List.map ~f:(fun (lsp_uri, pos_list) ->
           let filename = Lsp.string_of_uri lsp_uri in
           let lsp_locations =
             List.map
               ~f:(hack_pos_to_lsp_location ~default_path:filename)
               pos_list
           in
           lsp_locations)
    |> List.concat
  in
  let all_locations = List.rev_append ide_locations hh_locations in
  (* LSP spec explains: "If a server reports partial result via a corresponding $/progress, the whole result
     must be reported using n $/progress notifications. The final response has to be empty in terms of result values."
     Thus we'll give an empty response here.
     But first, flush whatever remaining $/progress. The protocol of [watch_refs_stream_file] is that it opens an fd
     on [file], polls the fd for changes and writes them to [q], but also polls and when [file] is unlinked then
     it closes [q]. Thus our protocol is: unlink [file], await all items until [q] is closed, publish them
     as $/progress, and then give an empty response.
     Proof of termination: this is guaranteed to terminate if no one creates another file with the same name as [file].
     Since the filename includes our PID, only we can create another file. But we're not
     going to create another file while we're here waiting! *)
  match stream_file with
  | Some { Run_env.file; q; partial_result_token = _ } ->
    Unix.unlink (Path.to_string file);
    log "Unlinked find-refs streaming file %s" (Path.to_string file);
    (* [Lwt_stream.to_list] awaits until the stream is closed, picking up all remaining items. *)
    let%lwt items_list = Lwt_stream.to_list q in
    List.iter items_list ~f:(fun items ->
        notify_refs_file_items shellout_standard_response items);
    Lwt.return (FindReferencesResult [], List.length all_locations)
  | None ->
    Lwt.return (FindReferencesResult all_locations, List.length all_locations)

let do_goToImplementation
    (state : state)
    (ide_service : ClientIdeService.t ref)
    (metadata : incoming_metadata)
    (tracking_id : string)
    (ref_unblocked_time : float ref)
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : Implementation.params)
    (id : lsp_id) : (state * result_telemetry) Lwt.t =
  let (document, location) = get_document_location editor_open_files params in
  let all_open_documents = get_documents_from_open_files editor_open_files in
  let%lwt response =
    ide_rpc
      ide_service
      ~tracking_id
      ~ref_unblocked_time
      (ClientIdeMessage.Go_to_implementation
         (document, location, all_open_documents))
  in
  match response with
  | ClientIdeMessage.Invalid_symbol_impl ->
    respond_jsonrpc ~powered_by:Serverless_ide id (ImplementationResult []);
    let result_telemetry = make_result_telemetry 0 in
    Lwt.return (state, result_telemetry)
  | ClientIdeMessage.Go_to_impl_success
      (symbol, find_refs_action, ide_calculated_positions) ->
    let open Run_env in
    let shellable_type =
      GoToImpl
        {
          symbol;
          find_refs_action;
          stream_file = None;
          hint_suffixes = [];
          ide_calculated_positions;
        }
    in
    let start_time_local_handle = Unix.gettimeofday () in
    let%lwt (state, result_telemetry) =
      kickoff_shell_out_and_maybe_cancel
        state
        shellable_type
        ~triggering_request:
          {
            id;
            metadata;
            start_time_local_handle;
            request = ImplementationRequest params;
          }
    in
    Lwt.return (state, result_telemetry)

(** This is called when a shellout to "hh --ide-go-to-impl" completes successfully *)
let do_goToImplementation2 ~ide_calculated_positions ~hh_locations :
    Lsp.lsp_result * int =
  (* reject all server-supplied locations for files that we calculated in ClientIdeDaemon, then join the lists *)
  let filtered_list =
    List.filter hh_locations ~f:(fun loc ->
        not @@ UriMap.mem loc.Lsp.Location.uri ide_calculated_positions)
  in
  let ide_supplied_positions =
    UriMap.elements ide_calculated_positions
    |> List.concat_map ~f:(fun (_uri, pos_list) ->
           List.map pos_list ~f:(fun pos ->
               hack_pos_to_lsp_location pos ~default_path:(Pos.filename pos)))
  in
  let locations = filtered_list @ ide_supplied_positions in
  (ImplementationResult locations, List.length locations)

(** Shared function for hack range conversion *)
let hack_range_to_lsp_highlight range =
  { DocumentHighlight.range = ide_range_to_lsp range; kind = None }

let do_highlight
    (ide_service : ClientIdeService.t ref)
    (tracking_id : string)
    (ref_unblocked_time : float ref)
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : DocumentHighlight.params) : DocumentHighlight.result Lwt.t =
  let (document, location) = get_document_location editor_open_files params in
  let%lwt ranges =
    ide_rpc
      ide_service
      ~tracking_id
      ~ref_unblocked_time
      (ClientIdeMessage.Document_highlight (document, location))
  in
  Lwt.return (List.map ranges ~f:hack_range_to_lsp_highlight)

let do_formatting_common
    (uri : Lsp.DocumentUri.t)
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

let do_willSaveWaitUntil
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : WillSaveWaitUntil.params) : WillSaveWaitUntil.result =
  let { WillSaveWaitUntil.textDocument; reason } = params in
  let is_autosave =
    match reason with
    | AfterDelay -> true
    | Manual
    | FocusOut ->
      false
  in
  let uri = textDocument.TextDocumentIdentifier.uri in
  let lsp_doc = UriMap.find uri editor_open_files in
  let content = lsp_doc.Lsp.TextDocumentItem.text in
  if (not is_autosave) && Formatting.is_formattable content then
    let open DocumentFormatting in
    do_documentFormatting
      editor_open_files
      {
        textDocument = params.WillSaveWaitUntil.textDocument;
        options = { tabSize = 2; insertSpaces = true };
      }
  else
    []

let should_use_snippet_edits (initialize_params : Initialize.params option) :
    bool =
  initialize_params
  |> Option.map
       ~f:
         Initialize.(
           fun { client_capabilities = caps; _ } ->
             caps.client_experimental
               .ClientExperimentalCapabilities.snippetTextEdit)
  |> Option.value ~default:false

let location_of_code_action_request
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : CodeActionRequest.params) :
    ClientIdeMessage.document * Ide_api_types.range * Path.t =
  let text_document =
    get_text_document_item
      editor_open_files
      params.CodeActionRequest.textDocument.TextDocumentIdentifier.uri
  in
  let document = lsp_document_to_ide text_document in
  let file_path = document.ClientIdeMessage.file_path in
  let range = lsp_range_to_ide params.CodeActionRequest.range in
  (document, range, file_path)

let do_codeAction
    (ide_service : ClientIdeService.t ref)
    (tracking_id : string)
    (ref_unblocked_time : float ref)
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : CodeActionRequest.params) :
    (CodeAction.result * Path.t * Errors.finalized_error list option) Lwt.t =
  let (document, range, file_path) =
    location_of_code_action_request editor_open_files params
  in
  let%lwt (actions, errors_opt) =
    ide_rpc
      ide_service
      ~tracking_id
      ~ref_unblocked_time
      (ClientIdeMessage.Code_action (document, range))
  in
  Lwt.return (actions, file_path, errors_opt)

let do_codeAction_resolve
    (ide_service : ClientIdeService.t ref)
    (tracking_id : string)
    (ref_unblocked_time : float ref)
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : CodeActionRequest.params)
    ~(resolve_title : string) : CodeActionResolve.result Lwt.t =
  let (document, range, _file_path) =
    location_of_code_action_request editor_open_files params
  in
  let use_snippet_edits = should_use_snippet_edits !initialize_params_ref in
  let%lwt result =
    ide_rpc
      ide_service
      ~tracking_id
      ~ref_unblocked_time
      (ClientIdeMessage.Code_action_resolve
         { document; range; resolve_title; use_snippet_edits })
  in
  Lwt.return result

let do_signatureHelp
    (ide_service : ClientIdeService.t ref)
    (tracking_id : string)
    (ref_unblocked_time : float ref)
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : SignatureHelp.params) : SignatureHelp.result Lwt.t =
  let (document, location) = get_document_location editor_open_files params in
  let%lwt signatures =
    ide_rpc
      ide_service
      ~tracking_id
      ~ref_unblocked_time
      (ClientIdeMessage.Signature_help (document, location))
  in
  Lwt.return signatures

let patch_to_workspace_edit_change (patch : ServerRenameTypes.patch) :
    Lsp.DocumentUri.t * TextEdit.t =
  let open ServerRenameTypes in
  let open Pos in
  let text_edit =
    match patch with
    | Insert insert_patch
    | Replace insert_patch ->
      {
        TextEdit.range =
          Lsp_helpers.hack_pos_to_lsp_range ~equal:String.equal insert_patch.pos;
        newText = insert_patch.text;
      }
    | Remove pos ->
      {
        TextEdit.range =
          Lsp_helpers.hack_pos_to_lsp_range ~equal:String.equal pos;
        newText = "";
      }
  in
  let uri_string =
    match patch with
    | Insert insert_patch
    | Replace insert_patch ->
      File_url.create (filename insert_patch.pos)
    | Remove pos -> File_url.create (filename pos)
  in
  (Lsp.DocumentUri.Uri uri_string, text_edit)

let patches_to_workspace_edit (patches : ServerRenameTypes.patch list) :
    WorkspaceEdit.t =
  let changes = List.map patches ~f:patch_to_workspace_edit_change in
  let changes =
    List.fold
      changes
      ~init:Lsp.DocumentUri.Map.empty
      ~f:(fun acc (uri, text_edit) ->
        let current_edits =
          Option.value ~default:[] (Lsp.DocumentUri.Map.find_opt uri acc)
        in
        let new_edits = text_edit :: current_edits in
        Lsp.DocumentUri.Map.add uri new_edits acc)
  in
  { WorkspaceEdit.changes }

let do_documentRename
    (state : state)
    (ide_service : ClientIdeService.t ref)
    (metadata : incoming_metadata)
    (ref_unblocked_time : float ref)
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : Rename.params)
    (id : lsp_id) : (state * result_telemetry) Lwt.t =
  let start_time_local_handle = Unix.gettimeofday () in
  let document_position = rename_params_to_document_position params in
  let (document, location) =
    get_document_location editor_open_files document_position
  in
  let new_name = params.Rename.newName in
  let all_open_documents = get_documents_from_open_files editor_open_files in
  let%lwt response =
    ide_rpc
      ide_service
      ~tracking_id:metadata.tracking_id
      ~ref_unblocked_time
      (ClientIdeMessage.Rename (document, location, new_name, all_open_documents))
  in
  match response with
  | ClientIdeMessage.Not_renameable_position ->
    let message = "Tried to rename a non-renameable symbol" in
    raise
      (Error.LspException
         { Error.code = Error.InvalidRequest; message; data = None })
  | ClientIdeMessage.Rename_success { shellout = None; local } ->
    let patches = patches_to_workspace_edit local in
    respond_jsonrpc ~powered_by:Hh_server id (RenameResult patches);
    let result_telemetry = make_result_telemetry (List.length local) in
    Lwt.return (state, result_telemetry)
  | ClientIdeMessage.Rename_success
      {
        shellout = Some (symbol_definition, find_refs_action);
        local = ide_calculated_patches;
      } ->
    let shellable_type =
      Run_env.Rename
        {
          symbol_definition;
          find_refs_action;
          new_name;
          ide_calculated_patches;
        }
    in
    let%lwt (state, result_telemetry) =
      kickoff_shell_out_and_maybe_cancel
        state
        shellable_type
        ~triggering_request:
          {
            Run_env.id;
            metadata;
            start_time_local_handle;
            request = RenameRequest params;
          }
    in
    Lwt.return (state, result_telemetry)

(** This is called when a shell-out to "hh --ide-rename-by-symbol" completes successfully *)
let do_documentRename2 ~ide_calculated_patches ~hh_edits : Lsp.lsp_result * int
    =
  (* The list of patches we receive from shelling out reflect the state
     of files on disk, which is incorrect for edited files. To fix,

     1) Filter out all changes for files that are open
     2) add the list of ide_calculated changes
  *)
  let hh_changes = hh_edits.WorkspaceEdit.changes in
  let ide_changes = patches_to_workspace_edit ide_calculated_patches in
  let changes =
    Lsp.DocumentUri.Map.fold
      (fun file ide combined -> Lsp.DocumentUri.Map.add file ide combined)
      ide_changes.WorkspaceEdit.changes
      hh_changes
  in
  let result_count =
    Lsp.DocumentUri.Map.fold
      (fun _file changes tot -> tot + List.length changes)
      changes
      0
  in
  (RenameResult { WorkspaceEdit.changes }, result_count)

let hack_type_hierarchy_to_lsp
    (filename : string) (type_hierarchy : ServerTypeHierarchyTypes.result) :
    Lsp.TypeHierarchy.result =
  let hack_member_kind_to_lsp = function
    | ServerTypeHierarchyTypes.Method -> Lsp.TypeHierarchy.Method
    | ServerTypeHierarchyTypes.SMethod -> Lsp.TypeHierarchy.SMethod
    | ServerTypeHierarchyTypes.Property -> Lsp.TypeHierarchy.Property
    | ServerTypeHierarchyTypes.SProperty -> Lsp.TypeHierarchy.SProperty
    | ServerTypeHierarchyTypes.Const -> Lsp.TypeHierarchy.Const
  in
  let hack_member_entry_to_Lsp (entry : ServerTypeHierarchyTypes.memberEntry) :
      Lsp.TypeHierarchy.memberEntry =
    let Lsp.Location.{ uri; range } =
      hack_pos_to_lsp_location
        entry.ServerTypeHierarchyTypes.pos
        ~default_path:filename
    in
    let open ServerTypeHierarchyTypes in
    Lsp.TypeHierarchy.
      {
        name = entry.name;
        snippet = entry.snippet;
        uri;
        range;
        kind = hack_member_kind_to_lsp entry.kind;
        origin = entry.origin;
      }
  in
  let hack_enty_kind_to_lsp = function
    | ServerTypeHierarchyTypes.Class -> Lsp.TypeHierarchy.Class
    | ServerTypeHierarchyTypes.Interface -> Lsp.TypeHierarchy.Interface
    | ServerTypeHierarchyTypes.Trait -> Lsp.TypeHierarchy.Trait
    | ServerTypeHierarchyTypes.Enum -> Lsp.TypeHierarchy.Enum
  in
  let hack_ancestor_entry_to_Lsp
      (entry : ServerTypeHierarchyTypes.ancestorEntry) :
      Lsp.TypeHierarchy.ancestorEntry =
    match entry with
    | ServerTypeHierarchyTypes.AncestorName name ->
      Lsp.TypeHierarchy.AncestorName name
    | ServerTypeHierarchyTypes.AncestorDetails entry ->
      let Lsp.Location.{ uri; range } =
        hack_pos_to_lsp_location entry.pos ~default_path:filename
      in
      Lsp.TypeHierarchy.AncestorDetails
        {
          name = entry.name;
          uri;
          range;
          kind = hack_enty_kind_to_lsp entry.kind;
        }
  in
  let hack_hierarchy_entry_to_Lsp
      (entry : ServerTypeHierarchyTypes.hierarchyEntry) :
      Lsp.TypeHierarchy.hierarchyEntry =
    let Lsp.Location.{ uri; range } =
      hack_pos_to_lsp_location
        entry.ServerTypeHierarchyTypes.pos
        ~default_path:filename
    in
    let open ServerTypeHierarchyTypes in
    Lsp.TypeHierarchy.
      {
        name = entry.name;
        uri;
        range;
        kind = hack_enty_kind_to_lsp entry.kind;
        ancestors = List.map entry.ancestors ~f:hack_ancestor_entry_to_Lsp;
        members = List.map entry.members ~f:hack_member_entry_to_Lsp;
      }
  in
  match type_hierarchy with
  | None -> None
  | Some h -> Some (hack_hierarchy_entry_to_Lsp h)

let do_autoclose
    (ide_service : ClientIdeService.t ref)
    (tracking_id : string)
    (ref_unblocked_time : float ref)
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : AutoCloseJsx.params) : AutoCloseJsx.result Lwt.t =
  let (document, location) = get_document_location editor_open_files params in
  let%lwt result =
    ide_rpc
      ide_service
      ~tracking_id
      ~ref_unblocked_time
      (ClientIdeMessage.AutoClose (document, location))
  in
  Lwt.return result

let do_typeHierarchy
    (ide_service : ClientIdeService.t ref)
    (tracking_id : string)
    (ref_unblocked_time : float ref)
    (editor_open_files : Lsp.TextDocumentItem.t UriMap.t)
    (params : TypeHierarchy.params) : TypeHierarchy.result Lwt.t =
  let (document, location) = get_document_location editor_open_files params in
  let filename =
    lsp_uri_to_path
      params.TextDocumentPositionParams.textDocument.TextDocumentIdentifier.uri
  in
  let%lwt result =
    ide_rpc
      ide_service
      ~tracking_id
      ~ref_unblocked_time
      (ClientIdeMessage.Type_Hierarchy (document, location))
  in
  let converted = hack_type_hierarchy_to_lsp filename result in
  Lwt.return converted

(** TEMPORARY VALIDATION FOR IDE_STANDALONE. TODO(ljw): delete this once ide_standalone ships T92870399 *)
let validate_error_TEMPORARY
    (uri : DocumentUri.t)
    (lenv : Run_env.t)
    (actual : float * errors_from)
    ~(expected : Errors.finalized_error list)
    ~(start_time : float) : (float * errors_from) * string list =
  (* helper to diff two sorted lists: "list_diff ([],[]) xs ys" will return a pair
     (only_xs, only_ys) with those elements that are only in xs, and those only in ys. *)
  let rec list_diff (only_xs, only_ys) xs ys ~compare =
    match (xs, ys) with
    | ([], ys) -> (only_xs, ys @ only_ys)
    | (xs, []) -> (xs @ only_xs, only_ys)
    | (x :: xs, y :: ys) ->
      let c = compare x y in
      if c < 0 then
        list_diff (x :: only_xs, ys) xs (y :: ys) ~compare
      else if c > 0 then
        list_diff (only_xs, y :: only_ys) (x :: xs) ys ~compare
      else
        list_diff (only_xs, only_ys) xs ys ~compare
  in

  (* helper to accumulate log output into [diff] *)
  let format_diff (disposition : string) (errors : Errors.finalized_error list)
      : string list =
    List.map errors ~f:(fun { User_error.claim = (pos, msg); code; _ } ->
        let (line, start, end_) = Pos.info_pos pos in
        Printf.sprintf
          "%s: [%d] %s(%d:%d-%d) %s"
          disposition
          code
          (Pos.filename pos)
          line
          start
          end_
          msg)
  in

  match actual with
  | (_timestamp, Errors_from_errors_file) ->
    (* What was most recently published for [uri] came from a previous errors-file *)
    (actual, [])
  | (timestamp, Errors_from_clientIdeDaemon _)
    when Float.(timestamp > start_time) ->
    (* What was most recently published for [uri] came from clientIdeDaemon, but
       came from something (e.g. a didChange) that happened after the start of the typecheck,
       so it might reflect something that hh_server wasn't aware of, so we can't make a useful check. *)
    (actual, [])
  | (_timestamp, Errors_from_clientIdeDaemon _)
    when UriSet.mem uri lenv.Run_env.uris_with_unsaved_changes ->
    (* What was most recently published for [uri] came from clientIdeDaemon, but it
       reflects unsaved changes, and hence reflects something that hh_server isn't aware of,
       so we can't make a useful check *)
    (actual, [])
  | (_timestamp, Errors_from_clientIdeDaemon _)
    when Option.is_none (UriMap.find_opt uri lenv.Run_env.editor_open_files) ->
    (* What was most recently published for [uri] came from clientIdeDaemon before the start of the
       typecheck, but it was closed prior to the start of the typecheck, so we don't know if hh_server
       is looking at file-changes to it after it had been closed and we can't make a useful check. *)
    (actual, [])
  | (_timestamp, Errors_from_clientIdeDaemon { validated = true; _ }) ->
    (* Here is an open file whose errors were published from clientIdeDaemon prior to the start of not only this
       current typecheck, but also prior to the start of the *previous* typecheck. We need no further validation. *)
    (actual, [])
  | (timestamp, Errors_from_clientIdeDaemon { errors; _ }) ->
    let (absent_from_clientIdeDaemon, extra_in_clientIdeDaemon) =
      list_diff ([], []) expected errors ~compare:Errors.compare_finalized
    in
    let diff =
      format_diff "absent_from_clientIdeDaemon" absent_from_clientIdeDaemon
      @ format_diff "extra_in_clientIdeDaemon" extra_in_clientIdeDaemon
    in
    ((timestamp, Errors_from_clientIdeDaemon { errors; validated = true }), diff)

(** TEMPORARY VALIDATION FOR IDE_STANDALONE. TODO(ljw): delete this once ide_standalone ships T92870399
Validates that errors reported by clientIdeDaemon are same as what was reported by hh_server.

This function is called by [handle_errors_file_item] when it receives a new report
of errors from the errors-file. It validates that, if there are any open unmodified files
which had last received errors from clientIdeDaemon prior the start of the typecheck, then
are those clientIdeDaemon errors identical to the ones reported in the errors file?
It also has the side effect of storing, in [lenv.uris_with_standalone_diagnostics],
that these particular clientIdeDaemon have been validated against the errors-file;
this fact is used in [validate_error_absence].

Why do we only look at unmodified files whose last received clientIdeDaemon errors were from
prior to the start of the typecheck? -- because for sure clientIdeDaemon and hh_server
both saw the same source text for them. Why can't we look at closed files? -- because
they might have been modified on disk after they were closed. *)
let validate_error_item_TEMPORARY
    (lenv : Run_env.t)
    (ide_service : ClientIdeService.t ref)
    (expected : Errors.finalized_error list Relative_path.Map.t)
    ~(start_time : float) : Run_env.t =
  (* helper to do logging *)
  let log_diff uri reason ~expected diff =
    HackEventLogger.live_squiggle_diff
      ~uri:(string_of_uri uri)
      ~reason
      ~expected_error_count:(List.length expected)
      diff;
    if not (List.is_empty diff) then
      Hh_logger.log
        "LIVE_SQUIGGLE_DIFF_ERROR[item] %s\n%s"
        (string_of_uri uri)
        (String.concat ~sep:"\n" diff)
  in

  let lenv =
    Relative_path.Map.fold expected ~init:lenv ~f:(fun path expected lenv ->
        let path = Relative_path.to_absolute path in
        let uri = path_string_to_lsp_uri path ~default_path:path in
        let actual_opt =
          UriMap.find_opt uri lenv.Run_env.uris_with_standalone_diagnostics
        in
        let status = ClientIdeService.get_status !ide_service in
        (* We use [Option.value_exn] because this function [validate_error_item_TEMPORARY]
           is only ever called under ide_standalone=true, which implies ide_serverless=true,
           so hence there must be an ide_service. *)
        match (actual_opt, status) with
        | (Some actual, _) ->
          (* If we got a report of errors in [uri] from the errors-file, and we've
             previously published diagnostics for [uri], we'll validate that what
             we previously published is correct... *)
          let (actual, diff) =
            validate_error_TEMPORARY uri lenv actual ~expected ~start_time
          in
          log_diff uri "item-reported" ~expected diff;
          let uris_with_standalone_diagnostics =
            UriMap.add uri actual lenv.Run_env.uris_with_standalone_diagnostics
          in
          { lenv with Run_env.uris_with_standalone_diagnostics }
        | (None, ClientIdeService.Status.Ready) ->
          (* We got a report of errors in [uri] from the errors-file, but we don't
             currently have any diagnostics published [uri]. I wonder why not? ...
             The following function only validates at files which are open and unmodified
             in the editor. If an open and unmodified file has no published diagnostics,
             we don't know when it claimed to have no diagnostics, whether that was
             before the typecheck started (in which case errors-file should agree that it has
             no diagnostics), or following a didSave after the typecheck started (in which case
             we can't tell). Let's pretend, for sake of "good-enough" telemetry, that it
             was before the typecheck started and see whether errors-file agrees. *)
          let pretend_actual =
            ( start_time,
              Errors_from_clientIdeDaemon { validated = false; errors = [] } )
          in
          let (_pretend_actual, diff) =
            validate_error_TEMPORARY
              uri
              lenv
              pretend_actual
              ~expected
              ~start_time
          in
          log_diff uri "item-unreported" ~expected diff;
          lenv
        | (None, ClientIdeService.Status.(Initializing | Rpc _ | Stopped _)) ->
          (* We got a report of errors in [uri] from the errors-file, but we don't
             currently have any diagnostics published for [uri] because clientIdeDaemon
             isn't even ready yet. Nothing worth checking. *)
          lenv)
  in
  lenv

(** TEMPORARY VALIDATION FOR IDE_STANDALONE. TODO(ljw): delete this once ide_standalone ships T92870399
Validates that clientIdeDaemon didn't report additional errors beyond what was reported
by hh_server.

This function is called when [handle_errors_file_item] is told that the errors-file is
completed. It validates that, if there are any unmodified files which had last received
errors from clientIdeDaemon prior to the start of the typecheck, then all of them
have [Errors_from.validated] flag true, meaning that they have been checked against
the errors-file by [validate_error_presence]. If a file hasn't, then it's a false
positive reported by clientIdeStandalone. *)
let validate_error_complete_TEMPORARY (lenv : Run_env.t) ~(start_time : float) :
    Run_env.t =
  let uris_with_standalone_diagnostics =
    UriMap.mapi
      (fun uri actual ->
        let (actual, diff) =
          validate_error_TEMPORARY uri lenv actual ~expected:[] ~start_time
        in
        HackEventLogger.live_squiggle_diff
          ~uri:(string_of_uri uri)
          ~reason:"complete"
          ~expected_error_count:0
          diff;
        if not (List.is_empty diff) then
          Hh_logger.log
            "LIVE_SQUIGGLE_DIFF_ERROR[complete] %s\n%s"
            (string_of_uri uri)
            (String.concat ~sep:"\n" diff);
        actual)
      lenv.Run_env.uris_with_standalone_diagnostics
  in
  { lenv with Run_env.uris_with_standalone_diagnostics }

(** This reports on every live-squiggle-recheck that we attempt.
An Ok [result] means the recheck succeeded and we published diagnostics;
Error means the recheck was cancelled. *)
let report_recheck_telemetry
    ~(trigger : errors_trigger)
    ~(ref_unblocked_time : float ref)
    (uri : DocumentUri.t)
    (result : (Errors.finalized_error list, unit) Result.t) : unit =
  let (result, result_count) =
    match result with
    | Ok errors when List.is_empty errors -> ("publishedEmpty", Some 0)
    | Ok errors -> ("publishedErrors", Some (List.length errors))
    | Error () -> ("cancelled", None)
  in
  let notification =
    TelemetryNotification
      ( {
          LogMessage.type_ = MessageType.InfoMessage;
          message = "timing info for file check";
        },
        [
          ("telemetryKind", Hh_json.string_ "timingForCheck");
          ("checkScope", Hh_json.string_ "file");
          ("uri", Hh_json.string_ (string_of_uri uri));
          ("checkResult", Hh_json.string_ result);
        ]
        @
        match trigger with
        | Initialize_trigger -> [("triggerKind", Hh_json.string_ "init")]
        | Message_trigger { message; metadata } ->
          [
            ("triggerKind", Hh_json.string_ "message");
            ( "durationMs",
              Hh_json.int_
                (int_of_float
                   Float.((Unix.gettimeofday () -. metadata.timestamp) * 1000.0))
            );
            ( "triggerMessage",
              Hh_json.JSON_Object
                [
                  ( "method",
                    Hh_json.string_ (Lsp_fmt.message_name_to_string message) );
                  ("activityId", Hh_json.string_ metadata.activity_id);
                ] );
          ] )
  in
  let (trigger_method, timestamp, activity_id) =
    match trigger with
    | Initialize_trigger -> ("[init]", None, None)
    | Message_trigger { metadata; message } ->
      ( Lsp_fmt.message_name_to_string message,
        Some metadata.timestamp,
        Some metadata.activity_id )
  in
  notify_jsonrpc ~powered_by:Serverless_ide notification;
  HackEventLogger.client_lsp_recheck
    ~root:!env.root
    ~path:(lsp_uri_to_path uri |> Relative_path.create_detect_prefix)
    ~trigger_method
    ~result_count
    ~activity_id
    ~start_queue_time:timestamp
    ~start_handle_time:!ref_unblocked_time;
  ()

(** This function shold be called anytime we've recomputed live squiggles
(e.g. upon didChange, codeAction, didClose). It (1) sends publishDiagnostics
as necessary, (2) sends an LSP telemetry/event to say what it has done. *)
let publish_and_report_after_recomputing_live_squiggles
    (state : state)
    (file_path : Path.t)
    (errors : Errors.finalized_error list)
    ~(trigger : errors_trigger)
    ~(ref_unblocked_time : float ref) : state =
  match state with
  | Pre_init -> failwith "how we got errors before initialize?"
  | Post_shutdown -> state (* no-op *)
  | Running lenv ->
    let file_path = Path.to_string file_path in
    let uri = path_string_to_lsp_uri file_path ~default_path:file_path in
    let uris = lenv.Run_env.uris_with_standalone_diagnostics in
    let uris_with_standalone_diagnostics =
      if List.is_empty errors then
        UriMap.remove uri uris
      else
        UriMap.add
          uri
          ( Unix.gettimeofday (),
            Errors_from_clientIdeDaemon { errors; validated = false } )
          uris
    in
    let params = hack_errors_to_lsp_diagnostic file_path errors in
    let notification = PublishDiagnosticsNotification params in
    notify_jsonrpc ~powered_by:Serverless_ide notification;
    report_recheck_telemetry ~trigger ~ref_unblocked_time uri (Ok errors);

    let new_state =
      Running { lenv with Run_env.uris_with_standalone_diagnostics }
    in
    new_state

(** When the errors-file gets new items or is closed, this function updates diagnostics as necessary,
and switches from TailingErrors to SeekingErrors if it's closed.
Principles: (1) don't touch open files, since they are governed solely by clientIdeDaemon;
(2) only update an existing diagnostic if the scrape's start_time is newer than it,
since these will have come recently from clientIdeDaemon, to which we grant primacy. *)
let handle_errors_file_item
    ~(state : state ref)
    ~(ide_service : ClientIdeService.t ref)
    (item : ServerProgress.ErrorsRead.read_result option) :
    result_telemetry option Lwt.t =
  (* a small helper, to send the actual lsp message *)
  let publish params =
    notify_jsonrpc ~powered_by:Hh_server (PublishDiagnosticsNotification params)
  in
  (* a small helper, to construct empty diagnostic params *)
  let empty_diagnostics uri =
    Lsp.PublishDiagnostics.{ uri; diagnostics = []; isStatusFB = false }
  in

  (* We must be in this state, else how could we have gotten an item from the errors-file q?
     See code in [get_next_event]. *)
  let (lenv, fd, start_time) =
    match (!state, !latest_hh_server_errors) with
    | (Running lenv, TailingErrors { fd; start_time; _ }) ->
      (lenv, fd, start_time)
    | _ -> failwith "unexpected state when processing handle_errors_file_item"
  in

  match item with
  | None ->
    (* We must have an item, because the errors-stream always ends with a sentinel, and we stop tailing upon the sentinel. *)
    log "Errors-file: unexpected end of stream";
    HackEventLogger.invariant_violation_bug
      "errors-file unexpected end of stream";
    latest_hh_server_errors :=
      SeekingErrors
        {
          prev_st_ino = None;
          seek_reason = (ServerProgress.NothingYet, "eof-error");
        };
    Lwt.return_none
  | Some (Error (end_sentinel, log_message)) ->
    (* Close the errors-file, and switch to "seeking mode" for the next errors-file to come along. *)
    let stat = Unix.fstat fd in
    Unix.close fd;
    latest_hh_server_errors :=
      SeekingErrors
        {
          prev_st_ino = Some stat.Unix.st_ino;
          seek_reason = (end_sentinel, log_message);
        };
    begin
      match end_sentinel with
      | ServerProgress.NothingYet
      | ServerProgress.Build_id_mismatch ->
        failwith
          ("not possible out of q: "
          ^ ServerProgress.show_errors_file_error end_sentinel
          ^ " "
          ^ log_message)
      | ServerProgress.Stopped
      | ServerProgress.Killed _ ->
        (* At this point we'd like to erase all the squiggles that came from hh_server.
           All we'll do is leave the errors for now, and then a subsequent [try_open_errors_file]
           will determine that we've transitioned from an ok state (like we leave it in right now)
           to an error state, and it takes that opportunity to erase all squiggles. *)
        ()
      | ServerProgress.Restarted _ ->
        (* If the typecheck restarted, we'll just leave all existing errors as they are.
           We have no evidence upon which to add or erase anything.
           It will all be fixed in the next typecheck to complete. *)
        ()
      | ServerProgress.Complete _telemetry ->
        let lenv = validate_error_complete_TEMPORARY lenv ~start_time in
        (* If the typecheck completed, then we can erase all diagnostics (from closed-files)
           that were reported prior to the start of the typecheck - regardless of whether that
           diagnostic had most recently been reported from errors-file or from clientIdeDaemon.
           Why only from closed-files? ... because diagnostics for open files are produced live by clientIdeDaemon,
           and our information from errors-file is necessarily more stale than that from clientIdeDaemon. *)
        let uris_with_standalone_diagnostics =
          lenv.Run_env.uris_with_standalone_diagnostics
          |> UriMap.filter_map (fun uri (existing_time, errors_from) ->
                 if
                   UriMap.mem uri lenv.Run_env.editor_open_files
                   || Float.(existing_time > start_time)
                 then begin
                   Some (existing_time, errors_from)
                 end else begin
                   publish (empty_diagnostics uri);
                   None
                 end)
        in
        state := Running { lenv with Run_env.uris_with_standalone_diagnostics };
        ()
    end;
    Lwt.return_none
  | Some (Ok (ServerProgress.Telemetry _)) -> Lwt.return_none
  | Some (Ok (ServerProgress.Errors { errors; timestamp })) ->
    let lenv =
      validate_error_item_TEMPORARY lenv ide_service errors ~start_time
    in
    (* If the php file is closed and has no diagnostics newer than start_time, replace or add.

       Why only for closed files? well, if the file is currently open in the IDE, then
       clientIdeDaemon-generated diagnostics (1) reflect unsaved changes which the errors-file
       doesn't, (2) is more up-to-date than the errors-file, or at least no older.

       Why only newer than start time? There are a lot of scenarios but all boil down to this
       simple rule. (1) Existing diagnostics might have come from a previous errors-file, and hence
       are older than the start of the current errors-file, and should be replaced. (2) Existing
       diagnostics might have come from a file still open in the editor, discussed above.
       (3) Existing diagnostics might have come from a file that was open in the editor but
       got closed before the current errors-file started, and hence the errors-file is more up-to-date.
       (4) Existing diagnostics might have come from a file that was open in the editor but
       got closed after the current errors-file started; hence the latest we received from
       clientIdeDaemon is more up to date. (5) Existing diagnostics might have come from
       the current errors-file, per the comment in ServerProgress.mli: 'Currently we make
       one report for all "duplicate name" errors across the project if any, followed by
       one report per batch that had errors. This means that a file might be mentioned
       twice, once in a "duplicate name" report, once later. This will change in future
       so that each file is reported only once.'. The behavior right now is that clientLsp
       will only show the "duplicate name" errors for a file, suppressing future errors
       that came from parsing+typechecking. It's not ideal, but it will change shortly,
       and is an acceptable compromise for sake of code cleanliness. *)
    let uris_with_standalone_diagnostics =
      Relative_path.Map.fold
        errors
        ~init:lenv.Run_env.uris_with_standalone_diagnostics
        ~f:(fun path file_errors acc ->
          let path = Relative_path.to_absolute path in
          let uri = path_string_to_lsp_uri path ~default_path:path in
          if UriMap.mem uri lenv.Run_env.editor_open_files then
            acc
          else
            match UriMap.find_opt uri acc with
            | Some (existing_timestamp, _errors_from)
              when Float.(existing_timestamp > start_time) ->
              acc
            | _ ->
              publish (hack_errors_to_lsp_diagnostic path file_errors);
              UriMap.add uri (timestamp, Errors_from_errors_file) acc)
    in
    state := Running { lenv with Run_env.uris_with_standalone_diagnostics };
    Lwt.return_none

(** If we're in [errors_conn = SeekingErrors], then this function will try to open the
current errors-file. If success then we'll set [latest_hh_server_errors] to [TailingErrors].
If failure then we'll set it to [SeekingErrors], which includes error explanation for why
the attempt failed. We'll also clear out all squiggles.

If the failure was due to a version mismatch, we also take the opportunity
to check: has the version changed under our feet since we ourselves started?
If it has, then we exit with an error code (so that VSCode will relaunch
the correct version of lsp afterwards). If it hasn't, then we continue,
and the version mismatch must necessarily indicate that hh_server is the
wrong version.

If we're in [errors_conn = TrailingErrors] already, this function is a no-op.

If we're called before even having received the initialize request from LSP client
(hence before we even know the project root), this is a no-op. *)
let try_open_errors_file ~(state : state ref) : unit Lwt.t =
  match !latest_hh_server_errors with
  | TailingErrors _ -> Lwt.return_unit
  | SeekingErrors { prev_st_ino; seek_reason } ->
    let errors_file_path = ServerFiles.errors_file_path !env.root in
    (* 1. can we open the file? *)
    let result =
      try
        Ok (Unix.openfile errors_file_path [Unix.O_RDONLY; Unix.O_NONBLOCK] 0)
      with
      | Unix.Unix_error (Unix.ENOENT, _, _) ->
        Error
          (SeekingErrors
             {
               prev_st_ino;
               seek_reason = (ServerProgress.NothingYet, "absent");
             })
      | exn ->
        Error
          (SeekingErrors
             {
               prev_st_ino;
               seek_reason = (ServerProgress.NothingYet, Exn.to_string exn);
             })
    in
    (* 2. is it different from the previous time we looked? *)
    let result =
      match (prev_st_ino, result) with
      | (Some st_ino, Ok fd) when st_ino = (Unix.fstat fd).Unix.st_ino ->
        Unix.close fd;
        Error (SeekingErrors { prev_st_ino; seek_reason })
      | _ -> result
    in
    (* 3. can we [ServerProgress.ErrorsRead.openfile] on it? *)
    let%lwt errors_conn =
      match result with
      | Error errors_conn -> Lwt.return errors_conn
      | Ok fd -> begin
        match ServerProgress.ErrorsRead.openfile fd with
        | Error (e, log_message) ->
          let prev_st_ino = Some (Unix.fstat fd).Unix.st_ino in
          Unix.close fd;
          log
            "Errors-file: failed to open. %s [%s]"
            (ServerProgress.show_errors_file_error e)
            log_message;
          let%lwt () = terminate_if_version_changed_since_start_of_lsp () in
          Lwt.return
            (SeekingErrors { prev_st_ino; seek_reason = (e, log_message) })
        | Ok { ServerProgress.ErrorsRead.timestamp; pid; _ } ->
          log
            "Errors-file: opened and tailing the check from %s"
            (Utils.timestring timestamp);
          let q = ServerProgressLwt.watch_errors_file ~pid fd in
          Lwt.return (TailingErrors { fd; start_time = timestamp; q })
      end
    in

    (* If we've transitioned to failure, then we'll have to erase all outstanding squiggles.
       It'd be nice to leave the ones produced by clientIdeDaemon.
       But that's too much book-keeping to be worth complicating the code for an edge case,
       so in the words of c3po "shut them all down. hurry!" *)
    let is_ok conn =
      match conn with
      | TailingErrors _ -> true
      | SeekingErrors
          { seek_reason = (ServerProgress.(Restarted _ | Complete _), _); _ } ->
        true
      | SeekingErrors
          {
            seek_reason =
              ( ServerProgress.(
                  NothingYet | Stopped | Killed _ | Build_id_mismatch),
                _ );
            _;
          } ->
        false
    in
    let has_transitioned_to_failure =
      is_ok !latest_hh_server_errors && not (is_ok errors_conn)
    in
    if has_transitioned_to_failure then state := dismiss_diagnostics !state;

    latest_hh_server_errors := errors_conn;
    Lwt.return_unit

(** This function handles the success/failure of a shell-out.
It is guaranteed to produce an LSP response for [shellable_type]. *)
let handle_shell_out_complete
    (result : (Lwt_utils.Process_success.t, Lwt_utils.Process_failure.t) result)
    (triggering_request : Run_env.triggering_request)
    (shellable_type : Run_env.shellable_type) : result_telemetry option Lwt.t =
  let start_postprocess_time = Unix.gettimeofday () in
  let make_duration_telemetry ~start_time ~end_time =
    let end_postprocess_time = Unix.gettimeofday () in
    let start_time_local_handle =
      triggering_request.Run_env.start_time_local_handle
    in
    Telemetry.create ()
    |> Telemetry.duration
         ~key:"local_duration"
         ~start_time:start_time_local_handle
         ~end_time:start_time
    |> Telemetry.duration ~key:"shellout_duration" ~start_time ~end_time
    |> Telemetry.duration
         ~key:"post_shellout_wait"
         ~start_time:end_time
         ~end_time:start_postprocess_time
    |> Telemetry.duration
         ~key:"postprocess_duration"
         ~start_time:start_postprocess_time
         ~end_time:end_postprocess_time
  in
  let%lwt (lsp_result, result_count, result_extra_data) =
    match result with
    | Error
        ({
           Lwt_utils.Process_failure.start_time;
           end_time;
           stderr;
           stdout;
           command_line;
           process_status;
           exn = _;
         } as failure) ->
      let code =
        match process_status with
        | Unix.WSIGNALED -7 -> Lsp.Error.RequestCancelled
        | _ -> Lsp.Error.InternalError
      in
      let process_status = Process.status_to_string process_status in
      log "shell-out failure: %s" (Lwt_utils.Process_failure.to_string failure);
      let message =
        [stderr; stdout; process_status]
        |> List.map ~f:String.strip
        |> List.filter ~f:(fun s -> not (String.is_empty s))
        |> String.concat ~sep:"\n"
      in
      let telemetry =
        make_duration_telemetry ~start_time ~end_time
        |> Telemetry.string_ ~key:"command_line" ~value:command_line
        |> Telemetry.string_ ~key:"status" ~value:process_status
        |> Telemetry.error ~e:message
      in
      let lsp_error =
        make_lsp_error
          ~code
          ~data:(Some (Telemetry.to_json telemetry))
          ~current_stack:false
          message
      in
      unlink_refs_stream_file_if_present shellable_type;
      Lwt.return (ErrorResult lsp_error, 0, None)
    | Ok { Lwt_utils.Process_success.stdout; start_time; end_time; _ } -> begin
      log
        "shell-out completed. %s"
        (String_utils.split_into_lines stdout
        |> List.hd
        |> Option.value ~default:""
        |> String_utils.truncate 80);
      try
        let%lwt (lsp_result, result_count) =
          let open Run_env in
          match shellable_type with
          | FindRefs shellout_standard_response ->
            let hh_locations = shellout_locations_to_lsp_locations_exn stdout in
            let%lwt (lsp_result, result_count) =
              do_findReferences2 ~shellout_standard_response ~hh_locations
            in
            Lwt.return (lsp_result, result_count)
          | GoToImpl { ide_calculated_positions; _ } ->
            let hh_locations = shellout_locations_to_lsp_locations_exn stdout in
            Lwt.return
              (do_goToImplementation2 ~ide_calculated_positions ~hh_locations)
          | Rename { ide_calculated_patches; _ } ->
            let hh_edits = shellout_patch_list_to_lsp_edits_exn stdout in
            Lwt.return (do_documentRename2 ~ide_calculated_patches ~hh_edits)
        in
        Lwt.return
          ( lsp_result,
            result_count,
            Some (make_duration_telemetry ~start_time ~end_time) )
      with
      | exn ->
        let e = Exception.wrap exn in
        let stack = Exception.get_backtrace_string e |> Exception.clean_stack in
        let message = Exception.get_ctor_string e in
        let telemetry = make_duration_telemetry ~start_time ~end_time in
        let lsp_error =
          make_lsp_error
            ~code:Lsp.Error.InternalError
            ~data:(Some (Telemetry.to_json telemetry))
            ~stack
            ~current_stack:false
            message
        in
        Lwt.return (ErrorResult lsp_error, 0, None)
    end
  in
  let { Run_env.id; metadata; start_time_local_handle; request } =
    triggering_request
  in
  (* The normal error-response-and-logging flow isn't easy for us to fit into,
     because it handles errors+exceptions thinking they come from the current event.
     So we'll do our json response and error-logging ourselves. *)
  respond_jsonrpc ~powered_by:Hh_server id lsp_result;
  match lsp_result with
  | ErrorResult lsp_error ->
    hack_log_error
      (Some (Client_message (metadata, RequestMessage (id, request))))
      lsp_error
      Error_from_daemon_recoverable
      start_time_local_handle;
    Lwt.return_some (make_result_telemetry 0 ~log_immediately:false)
  | _ -> Lwt.return_some (make_result_telemetry result_count ?result_extra_data)

let do_initialize ~initialize_params : Initialize.result =
  Initialize.
    {
      server_capabilities =
        {
          textDocumentSync =
            {
              want_openClose = true;
              want_change = IncrementalSync;
              want_willSave = false;
              want_willSaveWaitUntil = true;
              want_didSave = Some { includeText = false };
            };
          hoverProvider = true;
          completionProvider =
            Some
              CompletionOptions.
                {
                  resolveProvider = true;
                  completion_triggerCharacters =
                    ["$"; ">"; "\\"; ":"; "<"; "["; "'"; "\""; "{"; "#"];
                };
          signatureHelpProvider =
            Some { sighelp_triggerCharacters = ["("; ","] };
          definitionProvider = true;
          typeDefinitionProvider = true;
          referencesProvider = true;
          callHierarchyProvider = true;
          documentHighlightProvider = true;
          documentSymbolProvider = true;
          workspaceSymbolProvider = true;
          codeActionProvider = Some CodeActionOptions.{ resolveProvider = true };
          codeLensProvider = None;
          documentFormattingProvider = true;
          documentRangeFormattingProvider = true;
          documentOnTypeFormattingProvider =
            (* the Code@FB client always sets this to `true`, but non-Code@FB clients are likely to use `false` *)
            Option.some_if
              (not
                 initialize_params.initializationOptions
                   .skipLspServerOnTypeFormatting)
              { firstTriggerCharacter = ";"; moreTriggerCharacter = ["}"] };
          renameProvider = true;
          documentLinkProvider = None;
          executeCommandProvider = None;
          implementationProvider =
            !env.local_config.ServerLocalConfig.go_to_implementation;
          rageProviderFB = true;
          server_experimental =
            Some
              ServerExperimentalCapabilities.
                { snippetTextEdit = true; autoCloseJsx = true };
        };
    }

let do_didChangeWatchedFiles_registerCapability () : Lsp.lsp_request =
  (* We want a glob-pattern like "**/*.{php,phpt,hack,hackpartial,hck,hh,hhi,xhp}".
     I'm constructing it from FindUtils.extensions so our glob-pattern doesn't get out
     of sync with FindUtils.file_filter. *)
  let extensions =
    List.map FindUtils.extensions ~f:(fun s -> String_utils.lstrip s ".")
  in
  let globPattern =
    Printf.sprintf "**/*.{%s}" (extensions |> String.concat ~sep:",")
  in
  let registration_options =
    DidChangeWatchedFilesRegistrationOptions
      {
        DidChangeWatchedFiles.watchers = [{ DidChangeWatchedFiles.globPattern }];
      }
  in
  let registration =
    Lsp.RegisterCapability.make_registration registration_options
  in
  Lsp.RegisterCapabilityRequest
    { RegisterCapability.registrations = [registration] }

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
  match state with
  | Running lenv -> Running { lenv with Run_env.editor_open_files }
  | Pre_init
  | Post_shutdown ->
    state

let track_edits_if_necessary (state : state) (event : event) : state =
  (* We'll keep track of which files have unsaved edits. Note that not all
   * clients send didSave messages; for those we only rely on didClose. *)
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
  | Running lenv -> Running { lenv with Run_env.uris_with_unsaved_changes }
  | Pre_init
  | Post_shutdown ->
    state

let short_timeout = 2.5

let long_timeout = 15.0

(** If a message is stale, throw the necessary exception to cancel it. A message is
considered stale if it's sufficiently old and there are other messages in the queue
that are newer than it. *)
let cancel_if_stale (client : Jsonrpc.t) (timestamp : float) (timeout : float) :
    unit Lwt.t =
  let time_elapsed = Unix.gettimeofday () -. timestamp in
  if
    Float.(time_elapsed >= timeout)
    && Jsonrpc.has_message client
    && not (Sys_utils.deterministic_behavior_for_tests ())
  then
    let message =
      if Float.(timestamp < binary_start_time) then
        "binary took too long to launch"
      else
        "request timed out"
    in
    raise
      (Error.LspException
         { Error.code = Error.RequestCancelled; message; data = None })
  else
    Lwt.return_unit

let assert_workers_are_not_stopped () =
  if WorkerCancel.is_stop_requested () then begin
    HackEventLogger.invariant_violation_bug
      "ClientLsp workers are stopped but shouldn't be";
    WorkerCancel.resume_workers ()
  end;
  ()

(** This predicate will return true if and only if the second parameter
is a $/cancelRequest notification for LSP request [message]. *)
let is_cancellation_of_request ~message { Jsonrpc.json; _ } : bool =
  match message with
  | ResponseMessage _
  | NotificationMessage _ ->
    false
  | RequestMessage (id, _request) ->
    let module M = struct
      exception Is_response
    end in
    (try
       let peek = Lsp_fmt.parse_lsp json (fun _ -> raise M.Is_response) in
       match peek with
       | NotificationMessage
           (CancelRequestNotification { Lsp.CancelRequest.id = peek_id }) ->
         Lsp.IdKey.compare id peek_id = 0
       | _ -> false
     with
    | M.Is_response -> false)

(** LEGACY FUNCTION, to be deleted once we roll out lsp_cancellation *)
let cancel_if_has_pending_cancel_request
    (client : Jsonrpc.t) (message : lsp_message) : unit =
  match message with
  | ResponseMessage _ -> ()
  | NotificationMessage _ -> ()
  | RequestMessage _ ->
    (* Scan the queue for any pending (future) cancellation messages that are requesting
       cancellation of the same id as our current request *)
    let pending_cancel_request_opt =
      Jsonrpc.find_already_queued_message
        client
        ~f:(is_cancellation_of_request ~message)
    in
    (* If there is a future cancellation request, we won't even embark upon this message *)
    if Option.is_some pending_cancel_request_opt then
      raise
        (Error.LspException
           {
             Error.code = Error.RequestCancelled;
             message = "request cancelled";
             data = None;
           })
    else
      ()

(** This will run [f] but wrap it up in a way that respects cancellation:
1. If there's already a cancellation request for [message] in the queue, then we'll
just raise [RequestCancelled] Lsp exception immediately.
2. Otherwise, we'll await the callback [f], but if a cancellation request arrives
in the queue while awaiting then we'll use [WorkerCancel.stop_workers] to cooperatively
request it to finish.

INVARIANT: only one concurrent caller should invoke this function. (why? because
our cancellation mechanism [WorkerCancel.stop_workers] is a single bit, hence unable
to know which of multiple concurrent calls should be stopped). If this requirement
is violated, (1) we might end up cancelling the wrong thing, (2) we'll log
invariant_violation_bugs in [ide_rpc], here, and other places that call
[assert_workers_are_not_stopped]. It's easy to see how our caller upholds
the invariant -- we have only a single caller, the main ClientLsp message-loop,
which isn't concurrent with itself.

INVARIANT: This function assumes the invariant that workers are not stopped
prior to entering this function, and restores the invariant upon exiting.
(Again, this would go wrong in the presence of multiple concurrent callers). *)
let respect_cancellation
    (client : Jsonrpc.t) ~(predicate : Jsonrpc.timestamped_json -> bool) ~f =
  assert_workers_are_not_stopped ();
  (* Explanation: this statement embodies the invariant that workers are not
     stopped prior to entering. This function is the only place in the code which
     can stop workers, and it unstops workers in its [~finally] clause. Because
     of the invariant that no one will call this function concurrently with itself,
     we don't run into problems. *)
  let (cancellation_token, cancellation_source) = Lwt.wait () in
  (* Following promise will be fulfilled immediately if cancellation is already in the queue;
     otherwise will be fulfilled when it eventually arrives, or with [None] if [cancellation_token]
     is completed first or if there's a stream error *)
  let promise =
    Jsonrpc.await_until_found client ~predicate ~cancellation_token
  in
  (* If there was already a cancel request in the queue, we'll skip all processing right now! *)
  if not (Lwt.is_sleeping promise) then begin
    raise
      (Error.LspException
         {
           Error.code = Error.RequestCancelled;
           message = "request cancelled";
           data = None;
         })
  end;
  (* Otherwise, cancellation will cooperatively request clientIdeDaemon to stop *)
  let promise =
    Lwt.map
      (fun cancel_notification_opt ->
        match cancel_notification_opt with
        | None -> ()
        | Some { Jsonrpc.timestamp; _ } ->
          Hh_logger.log
            "acting upon cancellation at %s"
            (Utils.timestring timestamp);
          WorkerCancel.stop_workers ())
      promise
  in
  let%lwt result =
    Lwt_utils.try_finally ~f ~finally:(fun () ->
        ensure_cancelled cancellation_source;
        let%lwt () = promise in
        WorkerCancel.resume_workers ();
        Lwt.return_unit)
  in
  Lwt.return result

(** This function is called upon didOpen/didChange
to stick a uri into the [uris_that_need_check] queue. *)
let send_file_to_ide_and_defer_check
    ~ide_service
    ~state
    ~uri
    ~file_contents
    ~(trigger : errors_trigger)
    ~ref_unblocked_time : state Lwt.t =
  let file_path = uri |> lsp_uri_to_path |> Path.make in
  let tracking_id =
    match trigger with
    | Initialize_trigger -> Random_id.short_string ()
    | Message_trigger { metadata; _ } -> metadata.tracking_id
  in
  let%lwt _errors =
    ide_rpc
      ide_service
      ~tracking_id
      ~ref_unblocked_time
      ClientIdeMessage.(Did_open_or_change { file_path; file_contents })
  in
  match !state with
  | Running renv ->
    let uris_that_need_check = renv.Run_env.uris_that_need_check in
    Option.iter
      (UriMap.find_opt uri uris_that_need_check)
      ~f:(fun prev_trigger ->
        (* We had a previous trigger to typecheck the file and then a new trigger arrived.
           How to report on the previous attempt? ... as cancelled, just like the rest of LSP does it. *)
        report_recheck_telemetry
          ~trigger:prev_trigger
          ~ref_unblocked_time
          uri
          (Error ()));
    let uris_that_need_check = UriMap.add uri trigger uris_that_need_check in
    Lwt.return (Running Run_env.{ renv with uris_that_need_check })
  | _ -> Lwt.return !state

(************************************************************************)
(* Message handling                                                     *)
(************************************************************************)

(** send DidOpen/Close/Change/Save ide_service as needed *)
let handle_editor_buffer_message
    ~(state : state ref)
    ~(ide_service : ClientIdeService.t ref)
    ~(metadata : incoming_metadata)
    ~(ref_unblocked_time : float ref)
    ~(message : lsp_message) : unit Lwt.t =
  let uri_to_path uri = uri |> lsp_uri_to_path |> Path.make in
  match message with
  | NotificationMessage
      ( DidOpenNotification
          { DidOpen.textDocument = { TextDocumentItem.uri; _ }; _ }
      | DidChangeNotification
          {
            DidChange.textDocument = { VersionedTextDocumentIdentifier.uri; _ };
            _;
          } ) ->
    let editor_open_files =
      match get_editor_open_files !state with
      | Some files -> files
      | None -> UriMap.empty
    in
    let file_contents = get_document_contents editor_open_files uri in
    let%lwt new_state =
      send_file_to_ide_and_defer_check
        ~ide_service
        ~state
        ~uri
        ~file_contents
        ~trigger:(Message_trigger { metadata; message })
        ~ref_unblocked_time
    in
    state := new_state;
    Lwt.return_unit
  | NotificationMessage (DidCloseNotification params) ->
    let uri = params.DidClose.textDocument.TextDocumentIdentifier.uri in
    let file_path = uri_to_path uri in
    let%lwt errors =
      ide_rpc
        ide_service
        ~tracking_id:metadata.tracking_id
        ~ref_unblocked_time
        (ClientIdeMessage.Did_close file_path)
    in
    state :=
      publish_and_report_after_recomputing_live_squiggles
        !state
        file_path
        errors
        ~trigger:(Message_trigger { message; metadata })
        ~ref_unblocked_time;
    begin
      match !state with
      | Running renv ->
        let uris_that_need_check = renv.Run_env.uris_that_need_check in
        Option.iter
          (UriMap.find_opt uri uris_that_need_check)
          ~f:(fun prev_trigger ->
            report_recheck_telemetry
              ~trigger:prev_trigger
              ~ref_unblocked_time
              uri
              (Error ()));
        let uris_that_need_check = UriMap.remove uri uris_that_need_check in
        state := Running Run_env.{ renv with uris_that_need_check }
      | _ -> ()
    end;
    Lwt.return_unit
  | _ ->
    (* Don't handle other events for now. When we show typechecking errors for
       the open file, we'll start handling them. *)
    ref_unblocked_time := Unix.gettimeofday ();
    Lwt.return_unit

let set_verbose_to_file
    ~(ide_service : ClientIdeService.t ref)
    ~(tracking_id : string)
    (value : bool) : unit Lwt.t =
  verbose_to_file := value;
  if !verbose_to_file then
    Hh_logger.Level.set_min_level_file Hh_logger.Level.Debug
  else
    Hh_logger.Level.set_min_level_file Hh_logger.Level.Info;
  let ref_unblocked_time = ref 0. in

  let%lwt () =
    ide_rpc
      ide_service
      ~tracking_id
      ~ref_unblocked_time
      (ClientIdeMessage.Verbose_to_file !verbose_to_file)
  in
  Lwt.return_unit

(** Process and respond to a message from VSCode, and update [state] accordingly. *)
let handle_client_message
    ~(state : state ref)
    ~(client : Jsonrpc.t)
    ~(ide_service : ClientIdeService.t ref)
    ~(metadata : incoming_metadata)
    ~(message : lsp_message)
    ~(ref_unblocked_time : float ref) : result_telemetry option Lwt.t =
  let%lwt result_telemetry_opt =
    (* make sure to wrap any exceptions below in the promise *)
    let tracking_id = metadata.tracking_id in
    let timestamp = metadata.timestamp in
    let editor_open_files =
      match get_editor_open_files !state with
      | Some files -> files
      | None -> UriMap.empty
    in
    match (!state, message) with
    (* response *)
    | (_, ResponseMessage (id, response)) ->
      let (_, handler) = IdMap.find id !requests_outstanding in
      let%lwt new_state = handler response !state in
      state := new_state;
      Lwt.return_none
    (* shutdown request *)
    | (_, RequestMessage (id, ShutdownRequest)) ->
      let%lwt new_state = do_shutdown !state ide_service tracking_id in
      state := new_state;
      respond_jsonrpc ~powered_by:Language_server id ShutdownResult;
      Lwt.return_none
    (* cancel notification *)
    | (_, NotificationMessage (CancelRequestNotification { CancelRequest.id }))
      ->
      (* Most requests are handled in-order here in clientLsp:
         our loop gets around to pickup up request ID "x" off the queue,
         before doing anything else it does [cancel_if_has_pending_cancel_request]
         to see if there's a CancelRequestNotification ahead of it in the queue
         and if so just sends a cancellation response rather than handling it.
         Thus, we'll still get around to picking the CancelRequestNotification
         off the queue right here and now, which is fine!

         A few requests are handled asynchronously, though -- those that shell out
         to hh. In these cases, upon receipt of the CancelRequestNotification,
         we should actively SIGTERM the shell-out... *)
      cancel_shellout_if_applicable !state id;
      Lwt.return_none
    (* exit notification *)
    | (_, NotificationMessage ExitNotification) ->
      if is_post_shutdown !state then
        exit_ok ()
      else
        exit_fail ()
    (* setTrace notification *)
    | (_, NotificationMessage (SetTraceNotification params)) ->
      let value =
        match params with
        | SetTraceNotification.Verbose -> true
        | SetTraceNotification.Off -> false
      in

      let%lwt () = set_verbose_to_file ~ide_service ~tracking_id value in
      Lwt.return_none
    (* test entrypoint: shutdown client_ide_service *)
    | (_, RequestMessage (id, HackTestShutdownServerlessRequestFB)) ->
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
    | (_, RequestMessage (id, HackTestStopServerRequestFB)) ->
      ClientStop.kill_server !env.root !env.args.from;
      respond_jsonrpc ~powered_by:Serverless_ide id HackTestStopServerResultFB;
      Lwt.return_none
    (* test entrypoint: start hh_server *)
    | (_, RequestMessage (id, HackTestStartServerRequestFB)) ->
      start_server ();
      respond_jsonrpc ~powered_by:Serverless_ide id HackTestStartServerResultFB;
      Lwt.return_none
    (* initialize request *)
    | (Pre_init, RequestMessage (id, InitializeRequest initialize_params)) ->
      initialize_params_ref := Some initialize_params;

      let root_from_initialize_params =
        Wwwroot.interpret_command_line_root_parameter
          [Lsp_helpers.get_root initialize_params]
      in
      if not (Path.equal !env.args.root_from_cli root_from_initialize_params)
      then begin
        let data =
          Printf.sprintf
            "root_from_cli=%s initialize.root=%s"
            (Path.to_string !env.args.root_from_cli)
            (Path.to_string root_from_initialize_params)
        in
        failwith ("lsp initialize.root differs from launch arg - " ^ data)
      end;

      state :=
        Running
          {
            Run_env.editor_open_files = UriMap.empty;
            uris_that_need_check = UriMap.empty;
            uris_with_unsaved_changes = UriSet.empty;
            uris_with_standalone_diagnostics = UriMap.empty;
            current_hh_shell = None;
          };
      let result = do_initialize ~initialize_params in
      respond_jsonrpc ~powered_by:Language_server id (InitializeResult result);

      let (promise : unit Lwt.t) =
        run_ide_service !ide_service initialize_params None
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
        IdMap.add id (request, handler) !requests_outstanding;

      if not (Sys_utils.deterministic_behavior_for_tests ()) then
        Lsp_helpers.telemetry_log
          to_stdout
          ("Version in hhconfig and switch=" ^ !env.hhconfig_version_and_switch);

      (* If editor sent 'trace: on' then that will turn on verbose_to_file. But we won't turn off
         verbose here, since the command-line argument --verbose trumps initialization params. *)
      let%lwt () =
        match initialize_params.Initialize.trace with
        | Initialize.Off -> Lwt.return_unit
        | Initialize.Messages
        | Initialize.Verbose ->
          let%lwt () = set_verbose_to_file ~ide_service ~tracking_id true in
          Lwt.return_unit
      in

      Lwt.return_none
    (* any request/notification if we haven't yet initialized *)
    | (Pre_init, _) ->
      raise
        (Error.LspException
           {
             Error.code = Error.ServerNotInitialized;
             message = "Server not yet initialized";
             data = None;
           })
    | (Post_shutdown, _c) ->
      raise
        (Error.LspException
           {
             Error.code = Error.InvalidRequest;
             message = "already received shutdown request";
             data = None;
           })
    (* initialized notification *)
    | (_, NotificationMessage InitializedNotification) -> Lwt.return_none
    (* rage request *)
    | (_, RequestMessage (id, RageRequestFB)) ->
      let%lwt result = do_rageFB !state in
      respond_jsonrpc ~powered_by:Language_server id (RageResultFB result);
      Lwt.return_some (make_result_telemetry (List.length result))
    | (_, NotificationMessage (DidChangeWatchedFilesNotification notification))
      ->
      let changes =
        List.filter_map
          notification.DidChangeWatchedFiles.changes
          ~f:(fun change ->
            let path = lsp_uri_to_path change.DidChangeWatchedFiles.uri in
            (* This is just the file:///foo/bar uri turned into a string path /foo/bar.
               There's nothing in VSCode/LSP spec to stipulate that the uris are
               canonical paths file:///data/users/ljw/www-hg/foo.php or to
               symlinked paths file:///home/ljw/www/foo.php (where ~/www -> /data/users/ljw/www-hg)
               but experimentally the uris seem to be canonical paths. That's lucky
               because if we had to turn a symlink of a deleted file file:///home/ljw/www/foo.php
               into the actual canonical path /data/users/ljw/www-hg/foo.php then it'd be hard!
               Anyway, because they refer to canonical paths, we can safely use [FindUtils.file_filter]
               and [Relative_path.create_detect_prefix], both of which match string prefix on the
               canonical root. *)
            if FindUtils.file_filter path then
              Some (Relative_path.create_detect_prefix path)
            else
              None)
        |> Relative_path.Set.of_list
      in
      let%lwt () =
        ide_rpc
          ide_service
          ~tracking_id
          ~ref_unblocked_time
          (ClientIdeMessage.Did_change_watched_files changes)
      in
      Lwt.return_some
        (make_result_telemetry (Relative_path.Set.cardinal changes))
    (* Text document completion: "AutoComplete!" *)
    | (_, RequestMessage (id, CompletionRequest params)) ->
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let%lwt result =
        do_completion
          ide_service
          tracking_id
          ref_unblocked_time
          editor_open_files
          params
      in
      respond_jsonrpc ~powered_by:Serverless_ide id (CompletionResult result);
      Lwt.return_some
        (make_result_telemetry (List.length result.Completion.items))
    (* Resolve documentation for a symbol: "Autocomplete Docblock!" *)
    | (_, RequestMessage (id, CompletionItemResolveRequest params)) ->
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let%lwt result =
        do_resolve ide_service tracking_id ref_unblocked_time params
      in
      respond_jsonrpc
        ~powered_by:Serverless_ide
        id
        (CompletionItemResolveResult result);
      Lwt.return_none
    (* Document highlighting in serverless IDE *)
    | (_, RequestMessage (id, DocumentHighlightRequest params)) ->
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let%lwt result =
        do_highlight
          ide_service
          tracking_id
          ref_unblocked_time
          editor_open_files
          params
      in
      respond_jsonrpc
        ~powered_by:Serverless_ide
        id
        (DocumentHighlightResult result);
      Lwt.return_some (make_result_telemetry (List.length result))
    (* Hover docblocks in serverless IDE *)
    | (_, RequestMessage (id, HoverRequest params)) ->
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let%lwt result =
        do_hover
          ide_service
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
      Lwt.return_some (make_result_telemetry result_count)
    | (_, RequestMessage (id, DocumentSymbolRequest params)) ->
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let%lwt result =
        do_documentSymbol
          ide_service
          tracking_id
          ref_unblocked_time
          editor_open_files
          params
      in
      respond_jsonrpc
        ~powered_by:Serverless_ide
        id
        (DocumentSymbolResult result);
      Lwt.return_some (make_result_telemetry (List.length result))
    | (_, RequestMessage (id, WorkspaceSymbolRequest params)) ->
      let%lwt () = cancel_if_stale client timestamp long_timeout in
      let%lwt result =
        do_workspaceSymbol ide_service tracking_id ref_unblocked_time params
      in
      respond_jsonrpc
        ~powered_by:Serverless_ide
        id
        (WorkspaceSymbolResult result);
      Lwt.return_some (make_result_telemetry (List.length result))
    | (_, RequestMessage (id, DefinitionRequest params)) ->
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let%lwt (result, _has_xhp_attribute) =
        do_definition
          ide_service
          tracking_id
          ref_unblocked_time
          editor_open_files
          params
      in
      respond_jsonrpc ~powered_by:Serverless_ide id (DefinitionResult result);
      Lwt.return_some (make_result_telemetry (List.length result))
    | (_, RequestMessage (id, TypeDefinitionRequest params)) ->
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let%lwt result =
        do_typeDefinition
          ide_service
          tracking_id
          ref_unblocked_time
          editor_open_files
          params
      in
      respond_jsonrpc
        ~powered_by:Serverless_ide
        id
        (TypeDefinitionResult result);
      Lwt.return_some (make_result_telemetry (List.length result))
    (* textDocument/references request *)
    | (_, RequestMessage (id, FindReferencesRequest params)) ->
      let%lwt () = cancel_if_stale client timestamp long_timeout in
      let%lwt (new_state, result_telemetry) =
        do_findReferences
          !state
          ide_service
          metadata
          ref_unblocked_time
          editor_open_files
          params
          id
      in
      state := new_state;
      Lwt.return_some result_telemetry
    (* textDocument/implementation request *)
    | (_, RequestMessage (id, ImplementationRequest params)) ->
      let%lwt () = cancel_if_stale client timestamp long_timeout in
      let%lwt (new_state, result_telemetry) =
        do_goToImplementation
          !state
          ide_service
          metadata
          tracking_id
          ref_unblocked_time
          editor_open_files
          params
          id
      in
      state := new_state;
      Lwt.return_some result_telemetry
    (* textDocument/rename request *)
    | (_, RequestMessage (id, RenameRequest params)) ->
      let%lwt () = cancel_if_stale client timestamp long_timeout in
      let%lwt (new_state, result_telemetry) =
        do_documentRename
          !state
          ide_service
          metadata
          ref_unblocked_time
          editor_open_files
          params
          id
      in
      state := new_state;
      Lwt.return_some result_telemetry
    (* Resolve documentation for a symbol: "Autocomplete Docblock!" *)
    | (_, RequestMessage (id, SignatureHelpRequest params)) ->
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let%lwt result =
        do_signatureHelp
          ide_service
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
      Lwt.return_some (make_result_telemetry result_count)
    (* textDocument/codeAction request *)
    | (Running renv, RequestMessage (id, CodeActionRequest params)) ->
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let%lwt (result, _file_path, errors_opt) =
        do_codeAction
          ide_service
          tracking_id
          ref_unblocked_time
          editor_open_files
          params
      in
      respond_jsonrpc
        ~powered_by:Serverless_ide
        id
        (CodeActionResult (result, params));
      begin
        match errors_opt with
        | None -> ()
        | Some _errors ->
          let uri =
            params.CodeActionRequest.textDocument.TextDocumentIdentifier.uri
          in
          let uris_that_need_check = renv.Run_env.uris_that_need_check in
          let uris_that_need_check =
            UriMap.add
              uri
              (Message_trigger { message; metadata })
              uris_that_need_check
          in
          state := Running { renv with Run_env.uris_that_need_check }
      end;
      Lwt.return_some (make_result_telemetry (List.length result))
    (* codeAction/resolve request *)
    | (_, RequestMessage (id, CodeActionResolveRequest params)) ->
      let CodeActionResolveRequest.{ data = code_action_request_params; title }
          =
        params
      in
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let%lwt result =
        do_codeAction_resolve
          ide_service
          tracking_id
          ref_unblocked_time
          editor_open_files
          code_action_request_params
          ~resolve_title:title
      in
      respond_jsonrpc
        ~powered_by:Serverless_ide
        id
        (CodeActionResolveResult result);
      let result_extra_data =
        Telemetry.create () |> Telemetry.string_ ~key:"title" ~value:title
      in
      Lwt.return_some (make_result_telemetry 1 ~result_extra_data)
    (* textDocument/formatting *)
    | (_, RequestMessage (id, DocumentFormattingRequest params)) ->
      let result = do_documentFormatting editor_open_files params in
      respond_jsonrpc
        ~powered_by:Language_server
        id
        (DocumentFormattingResult result);
      Lwt.return_some (make_result_telemetry (List.length result))
    (* textDocument/rangeFormatting *)
    | (_, RequestMessage (id, DocumentRangeFormattingRequest params)) ->
      let result = do_documentRangeFormatting editor_open_files params in
      respond_jsonrpc
        ~powered_by:Language_server
        id
        (DocumentRangeFormattingResult result);
      Lwt.return_some (make_result_telemetry (List.length result))
    (* textDocument/onTypeFormatting. TODO(T155870670): remove this *)
    | (_, RequestMessage (id, DocumentOnTypeFormattingRequest params)) ->
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let result = do_documentOnTypeFormatting editor_open_files params in
      respond_jsonrpc
        ~powered_by:Language_server
        id
        (DocumentOnTypeFormattingResult result);
      Lwt.return_some (make_result_telemetry (List.length result))
    (* textDocument/willSaveWaitUntil request *)
    | (_, RequestMessage (id, WillSaveWaitUntilRequest params)) ->
      let result = do_willSaveWaitUntil editor_open_files params in
      respond_jsonrpc
        ~powered_by:Language_server
        id
        (WillSaveWaitUntilResult result);
      Lwt.return_some (make_result_telemetry (List.length result))
    (* editor buffer events *)
    | ( _,
        NotificationMessage
          ( DidOpenNotification _ | DidChangeNotification _
          | DidCloseNotification _ | DidSaveNotification _ ) ) ->
      let%lwt () =
        handle_editor_buffer_message
          ~state
          ~ide_service
          ~metadata
          ~ref_unblocked_time
          ~message
      in
      Lwt.return_none
    (* typeHierarchy request *)
    | (_, RequestMessage (id, TypeHierarchyRequest params)) ->
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let%lwt result =
        do_typeHierarchy
          ide_service
          tracking_id
          ref_unblocked_time
          editor_open_files
          params
      in
      respond_jsonrpc ~powered_by:Serverless_ide id (TypeHierarchyResult result);
      let result_count =
        match result with
        | None -> 0
        | Some _result -> 1
      in
      Lwt.return_some (make_result_telemetry result_count)
    (* autoclose request *)
    | (_, RequestMessage (id, AutoCloseRequest params)) ->
      let%lwt () = cancel_if_stale client timestamp short_timeout in
      let%lwt result =
        do_autoclose
          ide_service
          tracking_id
          ref_unblocked_time
          editor_open_files
          params
      in
      respond_jsonrpc ~powered_by:Serverless_ide id (AutoCloseResult result);
      let result_count = Option.length result in
      Lwt.return_some (make_result_telemetry result_count)
    (* unhandled *)
    | (Running _, _) ->
      raise
        (Error.LspException
           {
             Error.code = Error.MethodNotFound;
             message =
               "not implemented: " ^ Lsp_fmt.message_name_to_string message;
             data = None;
           })
  in
  Lwt.return result_telemetry_opt

(** Process and respond to a notification from clientIdeDaemon, and update [state] accordingly. *)
let handle_daemon_notification
    ~(state : state ref)
    ~(ide_service : ClientIdeService.t ref)
    ~(notification : ClientIdeMessage.notification)
    ~(ref_unblocked_time : float ref) : result_telemetry option Lwt.t =
  (* In response to ide_service notifications we have these goals:
     * in case of Done_init failure, we have to announce the failure to the user
     * in case of Done_init success, we need squiggles for open files
     * in a few other cases, we send telemetry events so that test harnesses
       get insight into the internal state of the ide_service
     * after every single event, including client_ide_notification events,
       our caller queries the ide_service for what status it wants to display to
       the user, so these notifications have the goal of triggering that refresh. *)
  match notification with
  | ClientIdeMessage.Done_init (Ok ()) ->
    Lsp_helpers.telemetry_log to_stdout "[client-ide] Finished init: ok";
    let editor_open_files =
      (match get_editor_open_files !state with
      | Some files -> files
      | None -> UriMap.empty)
      |> UriMap.elements
    in
    let%lwt () =
      Lwt_list.iter_s
        (fun (uri, { TextDocumentItem.text = file_contents; _ }) ->
          let%lwt new_state =
            send_file_to_ide_and_defer_check
              ~ide_service
              ~state
              ~uri
              ~file_contents
              ~trigger:Initialize_trigger
              ~ref_unblocked_time
          in
          state := new_state;
          Lwt.return_unit)
        editor_open_files
    in
    Lwt.return_none
  | ClientIdeMessage.Done_init (Error error_data) ->
    log_debug "<-- done_init";
    Lsp_helpers.telemetry_log to_stdout "[client-ide] Finished init: failure";
    let%lwt () = announce_ide_failure error_data in
    Lwt.return_none

let handle_deferred_check
    ~(state : state ref)
    ~(ide_service : ClientIdeService.t ref)
    ~(ref_unblocked_time : float ref)
    (uri : DocumentUri.t)
    (trigger : errors_trigger) : result_telemetry option Lwt.t =
  match !state with
  | Pre_init
  | Post_shutdown ->
    Lwt.return_none
  | Running renv ->
    state :=
      Running
        Run_env.
          {
            renv with
            uris_that_need_check =
              UriMap.remove uri renv.Run_env.uris_that_need_check;
          };
    let editor_open_files =
      match get_editor_open_files !state with
      | Some files -> files
      | None -> UriMap.empty
    in
    let file_contents = get_document_contents editor_open_files uri in
    let file_path = uri |> lsp_uri_to_path |> Path.make in
    let tracking_id =
      match trigger with
      | Initialize_trigger -> Random_id.short_string ()
      | Message_trigger { metadata; _ } -> metadata.tracking_id
    in
    let%lwt errors =
      ide_rpc
        ide_service
        ~tracking_id
        ~ref_unblocked_time
        ClientIdeMessage.(Diagnostics { file_path; file_contents })
    in
    let new_state =
      publish_and_report_after_recomputing_live_squiggles
        !state
        file_path
        errors
        ~trigger
        ~ref_unblocked_time
    in
    state := new_state;
    Lwt.return_none

(** Called once a second but only when there are no pending messages from client
or clientIdeDaemon *)
let handle_tick ~(state : state ref) : result_telemetry option Lwt.t =
  EventLogger.recheck_disk_files ();
  HackEventLogger.Memory.profile_if_needed ();
  let%lwt () = try_open_errors_file ~state in
  let (promise : unit Lwt.t) = EventLoggerLwt.flush () in
  ignore_promise_but_handle_failure
    promise
    ~desc:"tick-event-flush"
    ~terminate_on_failure:false;
  Lwt.return_none

let main
    (args : args)
    ~(init_id : string)
    ~(local_config : ServerLocalConfig.t)
    ~(init_proc_stack : string list option) : Exit_status.t Lwt.t =
  Printexc.record_backtrace true;

  let root = args.root_from_cli in

  (* Log to a file on disk. Note that calls to `Hh_logger` will always write to
     `stderr`; this is in addition to that. *)
  let client_lsp_log_fn = ServerFiles.client_lsp_log root in
  begin
    try Sys.rename client_lsp_log_fn (client_lsp_log_fn ^ ".old") with
    | _e -> ()
  end;
  Hh_logger.set_log client_lsp_log_fn;
  (* The --verbose flag in env.verbose is the only thing that controls verbosity
     to stderr. Meanwhile, verbosity-to-file can be altered dynamically by the user.
     Why are they different? because we should write to stderr under a test harness,
     but we should never write to stderr when invoked by VSCode - it's not even guaranteed
     to drain the stderr pipe. *)
  if args.verbose then begin
    Hh_logger.Level.set_min_level_stderr Hh_logger.Level.Debug;
    Hh_logger.Level.set_min_level_file Hh_logger.Level.Debug
  end else begin
    Hh_logger.Level.set_min_level_stderr Hh_logger.Level.Error;
    Hh_logger.Level.set_min_level_file Hh_logger.Level.Info
  end;
  log "Starting clientLsp at %s" client_lsp_log_fn;
  log "cmd: %s" (String.concat ~sep:" " (Array.to_list Sys.argv));
  log "LSP Init id: %s" init_id;

  ServerProgress.set_root root;
  Relative_path.set_path_prefix Relative_path.Root root;

  let%lwt version = read_hhconfig_version root in
  let%lwt hhconfig_version_and_switch = read_hhconfig_version_and_switch root in
  HackEventLogger.set_hhconfig_version (Some (String_utils.lstrip version "^"));
  HackEventLogger.client_lsp_start ~init_proc_stack ~hhconfig_version_and_switch;
  (* [SharedMem.empty_config] will give us shared globals like [hh_shared.c:workers_should_exit]
     but no key/value heaps. *)
  let shm_handle = SharedMem.init SharedMem.empty_config ~num_workers:1 in

  env := { args; hhconfig_version_and_switch; root; local_config };

  let ide_service =
    ref
      (ClientIdeService.make
         {
           ClientIdeMessage.init_id;
           verbose_to_stderr = args.verbose;
           verbose_to_file = args.verbose;
           shm_handle;
         })
  in
  background_status_refresher ide_service;

  let client = Jsonrpc.make_t () in
  let deferred_action : (unit -> unit Lwt.t) option ref = ref None in
  let state = ref Pre_init in
  let ref_event = ref None in
  let ref_unblocked_time = ref (Unix.gettimeofday ()) in
  (* ref_unblocked_time is the time at which we're no longer blocked on either
   * clientLsp message-loop or hh_server, and can start actually handling.
   * Everything that blocks will update this variable. *)
  let process_next_event () : unit Lwt.t =
    assert_workers_are_not_stopped ();
    (* This assertion is satisfied because the only place that stops workers is inside
       [respect_cancellation], but its ~finally clause guarantees that it unstops workers
       before it completes i.e. before we continue this [process_next_event] loop. *)
    try%lwt
      let%lwt () =
        match !deferred_action with
        | Some deferred_action ->
          let%lwt () = deferred_action () in
          Lwt.return_unit
        | None -> Lwt.return_unit
      in
      deferred_action := None;
      let%lwt event = get_next_event state client ide_service in
      if not (is_tick event) then
        log_debug "next event: %s" (event_to_string event);
      ref_event := Some event;
      ref_unblocked_time := Unix.gettimeofday ();

      (* we keep track of all open files and their contents *)
      state := track_open_and_recent_files !state event;

      (* we keep track of all files that have unsaved changes in them *)
      state := track_edits_if_necessary !state event;

      (* this is the main handler for each message*)
      let%lwt result_telemetry_opt =
        match event with
        | Client_message (metadata, message) ->
          if !env.local_config.ServerLocalConfig.lsp_cancellation then begin
            (* Note: the function [respect_cancellation] requires the invariant that it not be called
               concurrently with itself. That's satisfied because here in the ClientLsp main loop
               is the only place we call it, and the main loop isn't concurrent. *)
            respect_cancellation
              client
              ~predicate:(is_cancellation_of_request ~message)
              ~f:(fun () ->
                handle_client_message
                  ~state
                  ~client
                  ~ide_service
                  ~metadata
                  ~message
                  ~ref_unblocked_time)
          end else begin
            (* TODO(ljw): delete this branch once we roll out lsp_cancellation *)
            cancel_if_has_pending_cancel_request client message;
            handle_client_message
              ~state
              ~client
              ~ide_service
              ~metadata
              ~message
              ~ref_unblocked_time
          end
        | Daemon_notification notification ->
          handle_daemon_notification
            ~state
            ~ide_service
            ~notification
            ~ref_unblocked_time
        | Errors_file result ->
          handle_errors_file_item ~state ~ide_service result
        | Refs_file result ->
          Lwt.return (handle_refs_file_items ~state:!state result)
        | Shell_out_complete (result, triggering_request, shellable_type) ->
          (* this handler doesn't change state; the state was already changed earlier
             by [get_next_event], which removed [current_hh_shell] from [state]
             and stored it within the arguments here to [Shell_out_complete]. *)
          handle_shell_out_complete result triggering_request shellable_type
        | Tick -> handle_tick ~state
        | Deferred_check (uri, trigger) ->
          handle_deferred_check
            ~state
            ~ide_service
            ~ref_unblocked_time
            uri
            trigger
      in
      (* for LSP requests and notifications, we keep a log of what+when we responded.
         INVARIANT: every LSP request gets either a response logged here,
         or an error logged by one of the handlers below. *)
      log_response_if_necessary event result_telemetry_opt !ref_unblocked_time;

      Lwt.return_unit
    with
    | Client_fatal_connection_exception { Marshal_tools.stack; message } ->
      let e = make_lsp_error ~stack message in
      hack_log_error !ref_event e Error_from_client_fatal !ref_unblocked_time;
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
        !ref_unblocked_time;
      Lsp_helpers.telemetry_error to_stdout (message ^ ", from_client\n" ^ stack);
      Lwt.return_unit
    | (Daemon_nonfatal_exception e | Error.LspException e) as exn ->
      let exn = Exception.wrap exn in
      let error_source =
        match (e.Error.code, Exception.unwrap exn) with
        | (Error.RequestCancelled, _) -> Error_from_lsp_cancelled
        | (_, Daemon_nonfatal_exception _) -> Error_from_daemon_recoverable
        | (_, _) -> Error_from_lsp_misc
      in
      let e =
        make_lsp_error ~data:e.Error.data ~code:e.Error.code e.Error.message
      in
      respond_to_error !ref_event e;
      hack_log_error !ref_event e error_source !ref_unblocked_time;
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
      hack_log_error !ref_event e Error_from_lsp_misc !ref_unblocked_time;
      Lwt.return_unit
  in
  let rec main_loop () : unit Lwt.t =
    let%lwt () = process_next_event () in
    main_loop ()
  in
  let%lwt () = main_loop () in
  Lwt.return Exit_status.No_error
