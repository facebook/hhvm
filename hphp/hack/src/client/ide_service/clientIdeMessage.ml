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

type document_location = {
  file_path: Path.t;
  file_contents: string option;
  (* if absent, should read from file on disk *)
  line: int;
  column: int;
}

type document_and_path = {
  file_path: Path.t;
  file_contents: string;
}

(** Denotes a location of the cursor in a document at which an IDE request is
being executed (e.g. hover). *)

module Ide_file_opened = struct
  type request = document_and_path

  type result = Errors.t
end

module Ide_file_changed = struct
  type request = document_and_path

  type result = Errors.t
end

module Hover = struct
  type request = document_location

  type result = HoverService.result
end

module Definition = struct
  type request = document_location

  type result = ServerCommandTypes.Go_to_definition.result
end

(* Handles "textDocument/typeDefinition" LSP messages *)
module Type_definition = struct
  type request = document_location

  type result = ServerCommandTypes.Go_to_type_definition.result
end

(* Handles "textDocument/completion" LSP messages *)
module Completion = struct
  type request = {
    document_location: document_location;
    is_manually_invoked: bool;
  }

  type result = AutocompleteTypes.ide_result
end

(* "completionItem/resolve" LSP messages - if we have symbol name *)
module Completion_resolve = struct
  type request = {
    symbol: string;
    kind: SearchUtils.si_kind;
  }

  type result = DocblockService.result
end

(* "completionItem/resolve" LSP messages - if we have file/line/column *)
module Completion_resolve_location = struct
  type request = {
    kind: SearchUtils.si_kind;
    document_location: document_location;
  }

  type result = DocblockService.result
end

(* Handles "textDocument/documentHighlight" LSP messages *)
module Document_highlight = struct
  type request = document_location

  type result = Ide_api_types.range list
end

(* Handles "textDocument/references" LSP messages - for local variables only *)
module Find_references = struct
  open ServerCommandTypes

  type request = document_location

  type result = (Find_refs.ide_result, Find_refs.action) Hh_prelude.result
end

(* Handles "textDocument/signatureHelp" LSP messages *)
module Signature_help = struct
  type request = document_location

  type result = Lsp.SignatureHelp.result
end

(* Handles "textDocument/documentSymbol" LSP messages *)
module Document_symbol = struct
  type request = document_location

  type result = FileOutline.outline
end

(* Handles "textDocument/codeActions" LSP messages *)
module Code_action = struct
  type request = {
    file_path: Path.t;
    file_contents: string option;
    range: Ide_api_types.range;
  }

  type result = Lsp.CodeAction.command_or_action list
end

module Type_coverage = struct
  type request = document_and_path

  type result = Coverage_level_defs.result
end

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
  | Ide_file_opened : Ide_file_opened.request -> Ide_file_opened.result t
  | Ide_file_changed : Ide_file_changed.request -> Ide_file_changed.result t
  | Ide_file_closed : Path.t -> unit t
  | Verbose_to_file : bool -> unit t
  | Hover : Hover.request -> Hover.result t
  | Definition : Definition.request -> Definition.result t
  | Completion : Completion.request -> Completion.result t
  | Completion_resolve :
      Completion_resolve.request
      -> Completion_resolve.result t
  | Completion_resolve_location :
      Completion_resolve_location.request
      -> Completion_resolve_location.result t
  | Document_highlight :
      Document_highlight.request
      -> Document_highlight.result t
  | Document_symbol : Document_symbol.request -> Document_symbol.result t
  | Workspace_symbol : string -> SearchUtils.result t
  | Type_definition : Type_definition.request -> Type_definition.result t
  | Type_coverage : Type_coverage.request -> Type_coverage.result t
  | Signature_help : Signature_help.request -> Signature_help.result t
  | Code_action : Code_action.request -> Code_action.result t
  | Find_references : Find_references.request -> Find_references.result t

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
  | Hover { file_path; _ } ->
    Printf.sprintf "Hover(%s)" (Path.to_string file_path)
  | Definition { file_path; _ } ->
    Printf.sprintf "Definition(%s)" (Path.to_string file_path)
  | Completion { Completion.document_location = { file_path; _ }; _ } ->
    Printf.sprintf "Completion(%s)" (Path.to_string file_path)
  | Completion_resolve { Completion_resolve.symbol; _ } ->
    Printf.sprintf "Completion_resolve(%s)" symbol
  | Completion_resolve_location
      { Completion_resolve_location.document_location = { file_path; _ }; _ } ->
    Printf.sprintf "Completion_resolve_location(%s)" (Path.to_string file_path)
  | Document_highlight { file_path; _ } ->
    Printf.sprintf "Document_highlight(%s)" (Path.to_string file_path)
  | Document_symbol { file_path; _ } ->
    Printf.sprintf "Document_symbol(%s)" (Path.to_string file_path)
  | Workspace_symbol query -> Printf.sprintf "Workspace_symbol(%s)" query
  | Type_definition { file_path; _ } ->
    Printf.sprintf "Type_definition(%s)" (Path.to_string file_path)
  | Type_coverage { file_path; _ } ->
    Printf.sprintf "Type_coverage(%s)" (Path.to_string file_path)
  | Signature_help { file_path; _ } ->
    Printf.sprintf "Signature_help(%s)" (Path.to_string file_path)
  | Code_action { Code_action.file_path; _ } ->
    Printf.sprintf "Code_action(%s)" (Path.to_string file_path)
  | Find_references { file_path; _ } ->
    Printf.sprintf "Find_references(%s)" (Path.to_string file_path)

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
