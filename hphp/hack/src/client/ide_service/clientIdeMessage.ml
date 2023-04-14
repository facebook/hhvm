(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

module Initialize_from_saved_state = struct
  type t = {
    root: Path.t;
    naming_table_load_info: naming_table_load_info option;
    config: (string * string) list;
    ignore_hh_version: bool;
    open_files: Path.t list;
  }

  and naming_table_load_info = {
    path: Path.t;
    test_delay: float;  (** artificial delay in seconds, for test purposes *)
  }
end

type document = {
  file_path: Path.t;
  file_contents: string;
}

type location = {
  line: int;
  column: int;
}

type completion_request = { is_manually_invoked: bool }

(** Represents a path corresponding to a file which has changed on disk. We
don't use `Path.t`:

  * It invokes `realpath`. However, for files that don't exist (i.e. deleted
  files), it returns the path unchanged. This can cause bugs. For example, if
  the repo root is a symlink, it will be resolved for files which do exist,
  but left unchanged for files that don't exist.
  * It's an unnecessary syscall per file.
  * It can cause accidental file fetches on a virtual filesystem. We typically
  end up filtering the list of changed files, so we may never use the fetched
  file content.
*)
type changed_file = Changed_file of string

(* GADT for request/response types. See [ServerCommandTypes] for a discussion on
   using GADTs in this way. *)
type _ t =
  | Initialize_from_saved_state : Initialize_from_saved_state.t -> unit t
      (** Invariant: the client sends no messages to the daemon before
      Initialize_from_state. And the daemon sends no messages before
      it has responded. *)
  | Shutdown : unit -> unit t
  | Disk_files_changed : changed_file list -> unit t
  | Ide_file_opened : document -> Errors.t t
  | Ide_file_changed : document -> Errors.t t
  | Ide_file_closed : Path.t -> unit t
  | Verbose_to_file : bool -> unit t
  | Hover : document * location -> HoverService.result t
  | Definition :
      document * location
      -> ServerCommandTypes.Go_to_definition.result t
  | Completion :
      document * location * completion_request
      -> AutocompleteTypes.ide_result t
      (** Handles "textDocument/completion" LSP messages *)
  | Completion_resolve_location :
      document * location * SearchUtils.si_kind
      -> DocblockService.result t
      (** "completionItem/resolve" LSP messages - if we have file/line/column *)
  | Completion_resolve :
      string * SearchUtils.si_kind
      -> DocblockService.result t
      (** "completionItem/resolve" LSP messages - if we have symbol name, and [Completion_resolve_location] failed *)
  | Document_highlight : document * location -> Ide_api_types.range list t
      (** Handles "textDocument/documentHighlight" LSP messages *)
  | Document_symbol : document * location -> FileOutline.outline t
      (** Handles "textDocument/documentSymbol" LSP messages *)
  | Workspace_symbol : string -> SearchUtils.result t
  | Find_references :
      document * location
      -> ( ServerCommandTypes.Find_refs.ide_result,
           string * ServerCommandTypes.Find_refs.action )
         result
         t
      (** The result of Find_references is either:
       - In the success case, a [Find_refs.ide_result], which is an optional tuple of
         symbol name from [SymbolDefinition.full_name] and positions for that symbol.
       - In the failure case, we return both a symbol's name from [SymbolDefinition.full_name],
         and the [Find_refs.action] which describes what the symbol refers to,
         e.g. Class of class_name, Member of member_name with a Method of method_name. *)
  | Rename :
      document * location * string
      -> ( ServerRenameTypes.patch list option,
           ServerCommandTypes.Find_refs.action )
         result
         t
  | Type_definition :
      document * location
      -> ServerCommandTypes.Go_to_type_definition.result t
      (** Handles "textDocument/typeDefinition" LSP messages *)
  | Type_coverage : document -> Coverage_level_defs.result t
  | Signature_help : document * location -> Lsp.SignatureHelp.result t
      (** Handles "textDocument/signatureHelp" LSP messages *)
  | Code_action :
      document * Ide_api_types.range
      -> Lsp.CodeAction.command_or_action list t
      (** Handles "textDocument/codeActions" LSP messages *)

let t_to_string : type a. a t -> string = function
  | Initialize_from_saved_state _ -> "Initialize_from_saved_state"
  | Shutdown () -> "Shutdown"
  | Disk_files_changed files ->
    let files = List.map files ~f:(fun (Changed_file path) -> path) in
    let (files, remainder) = List.split_n files 10 in
    let remainder =
      if List.is_empty remainder then
        ""
      else
        ",..."
    in
    Printf.sprintf "Disk_file_changed(%s%s)" (String.concat files) remainder
  | Ide_file_opened { file_path; _ } ->
    Printf.sprintf "Ide_file_opened(%s)" (Path.to_string file_path)
  | Ide_file_changed { file_path; _ } ->
    Printf.sprintf "Ide_file_changed(%s)" (Path.to_string file_path)
  | Ide_file_closed file_path ->
    Printf.sprintf "Ide_file_closed(%s)" (Path.to_string file_path)
  | Verbose_to_file verbose -> Printf.sprintf "Verbose_to_file(%b)" verbose
  | Hover ({ file_path; _ }, _) ->
    Printf.sprintf "Hover(%s)" (Path.to_string file_path)
  | Definition ({ file_path; _ }, _) ->
    Printf.sprintf "Definition(%s)" (Path.to_string file_path)
  | Completion ({ file_path; _ }, _, _) ->
    Printf.sprintf "Completion(%s)" (Path.to_string file_path)
  | Completion_resolve (symbol, _) ->
    Printf.sprintf "Completion_resolve(%s)" symbol
  | Completion_resolve_location ({ file_path; _ }, _, _) ->
    Printf.sprintf "Completion_resolve_location(%s)" (Path.to_string file_path)
  | Document_highlight ({ file_path; _ }, _) ->
    Printf.sprintf "Document_highlight(%s)" (Path.to_string file_path)
  | Document_symbol ({ file_path; _ }, _) ->
    Printf.sprintf "Document_symbol(%s)" (Path.to_string file_path)
  | Workspace_symbol query -> Printf.sprintf "Workspace_symbol(%s)" query
  | Type_definition ({ file_path; _ }, _) ->
    Printf.sprintf "Type_definition(%s)" (Path.to_string file_path)
  | Type_coverage { file_path; _ } ->
    Printf.sprintf "Type_coverage(%s)" (Path.to_string file_path)
  | Signature_help ({ file_path; _ }, _) ->
    Printf.sprintf "Signature_help(%s)" (Path.to_string file_path)
  | Code_action ({ file_path; _ }, _) ->
    Printf.sprintf "Code_action(%s)" (Path.to_string file_path)
  | Find_references ({ file_path; _ }, _) ->
    Printf.sprintf "Find_references(%s)" (Path.to_string file_path)
  | Rename ({ file_path; _ }, _, _) ->
    Printf.sprintf "Rename(%s)" (Path.to_string file_path)

type 'a tracked_t = {
  tracking_id: string;
  message: 'a t;
}

let tracked_t_to_string : type a. a tracked_t -> string =
 fun { tracking_id; message } ->
  Printf.sprintf "#%s: %s" tracking_id (t_to_string message)

module Processing_files = struct
  type t = {
    processed: int;
    total: int;
  }

  let to_string (t : t) : string = Printf.sprintf "%d/%d" t.processed t.total
end

(** This is a user-facing structure to explain why ClientIde isn't working. *)
type stopped_reason = {
  (* max 20 chars, for status bar. Will be prepended by "Hack:" *)
  short_user_message: string;
  (* max 10 words, for tooltip and alert. Will be postpended by " See <log>" *)
  medium_user_message: string;
  (* should we have window/showMessage, i.e. an alert? *)
  is_actionable: bool;
  (* max 5 lines, for window/logMessage, i.e. Output>Hack window. Will be postpended by "\nDetails: <url>" *)
  long_user_message: string;
  (* for experts. Will go in Hh_logger.log, and will be uploaded *)
  debug_details: string;
}

type notification =
  | Full_index_fallback
      (** The daemon is falling back to performing a full index of the repo to build a naming table.*)
  | Done_init of (Processing_files.t, stopped_reason) result
  | Processing_files of Processing_files.t
  | Done_processing

let notification_to_string (n : notification) : string =
  match n with
  | Full_index_fallback -> "Full_index_fallback"
  | Done_init (Ok p) ->
    Printf.sprintf "Done_init(%s)" (Processing_files.to_string p)
  | Done_init (Error edata) ->
    Printf.sprintf "Done_init(%s)" edata.medium_user_message
  | Processing_files p ->
    Printf.sprintf "Processing_file(%s)" (Processing_files.to_string p)
  | Done_processing -> "Done_processing"

type 'a timed_response = {
  unblocked_time: float;
  tracking_id: string;
  response: ('a, Lsp.Error.t) result;
}

type message_from_daemon =
  | Notification of notification
  | Response : 'a timed_response -> message_from_daemon

let message_from_daemon_to_string (m : message_from_daemon) : string =
  match m with
  | Notification n -> notification_to_string n
  | Response { response = Error { Lsp.Error.message; _ }; tracking_id; _ } ->
    Printf.sprintf "#%s: Response_error(%s)" tracking_id message
  | Response { response = Ok _; tracking_id; _ } ->
    Printf.sprintf "#%s: Response_ok" tracking_id

type daemon_args = {
  init_id: string;
  verbose_to_stderr: bool;
  verbose_to_file: bool;
}
