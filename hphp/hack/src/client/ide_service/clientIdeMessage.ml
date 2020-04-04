(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Initialize_from_saved_state = struct
  type t = {
    root: Path.t;
    naming_table_saved_state_path: Path.t option;
    use_ranked_autocomplete: bool;
    config: (string * string) list;
    open_files: Path.t list;
  }

  type result = { num_changed_files_to_process: int }
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

module File_opened = struct
  type request = document_and_path
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

module Type_coverage = struct
  type request = document_and_path

  type result = Coverage_level_defs.result
end

(* GADT for request/response types. See [ServerCommandTypes] for a discussion on
   using GADTs in this way. *)
type _ t =
  | Initialize_from_saved_state :
      Initialize_from_saved_state.t
      -> Initialize_from_saved_state.result t
  | Shutdown : unit -> unit t
  | File_changed : Path.t -> unit t
  | File_opened : File_opened.request -> unit t
  | File_closed : Path.t -> unit t
  | Verbose : bool -> unit t
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
  | Type_definition : Type_definition.request -> Type_definition.result t
  | Type_coverage : Type_coverage.request -> Type_coverage.result t
  | Signature_help : Signature_help.request -> Signature_help.result t

let t_to_string : type a. a t -> string = function
  | Initialize_from_saved_state _ -> "Initialize_from_saved_state"
  | Shutdown () -> "Shutdown"
  | File_changed file_path ->
    Printf.sprintf "File_changed(%s)" (Path.to_string file_path)
  | File_opened { file_path; _ } ->
    Printf.sprintf "File_opened(%s)" (Path.to_string file_path)
  | File_closed file_path ->
    Printf.sprintf "File_closed(%s)" (Path.to_string file_path)
  | Verbose verbose -> Printf.sprintf "Verbose(%b)" verbose
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
  | Type_definition { file_path; _ } ->
    Printf.sprintf "Type_definition(%s)" (Path.to_string file_path)
  | Type_coverage { file_path; _ } ->
    Printf.sprintf "Type_coverage(%s)" (Path.to_string file_path)
  | Signature_help { file_path; _ } ->
    Printf.sprintf "Signature_help(%s)" (Path.to_string file_path)

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

type notification =
  | Initializing
  | Processing_files of Processing_files.t
  | Done_processing

let notification_to_string (n : notification) : string =
  match n with
  | Initializing -> "Initializing"
  | Processing_files p ->
    Printf.sprintf "Processing_file(%s)" (Processing_files.to_string p)
  | Done_processing -> "Done_processing"

type error_data = {
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

(** make_error_data is for reporting internal bugs, invariant
violations, unexpected exceptions, ... *)
let make_error_data (debug_details : string) ~(stack : string) : error_data =
  {
    short_user_message = "failed";
    medium_user_message = "Hack IDE has failed.";
    long_user_message =
      "Hack IDE has failed.\nThis is unexpected.\nPlease file a bug within your IDE.";
    debug_details = debug_details ^ "\nSTACK:\n" ^ stack;
    is_actionable = false;
  }

type 'a timed_response = {
  unblocked_time: float;
  response: ('a, error_data) result;
}

type message_from_daemon =
  | Notification of notification
  | Response : 'a timed_response -> message_from_daemon

let message_from_daemon_to_string (m : message_from_daemon) : string =
  match m with
  | Notification n -> notification_to_string n
  | Response { response = Error { short_user_message; _ }; _ } ->
    Printf.sprintf "Response_error(%s)" short_user_message
  | Response { response = Ok _; _ } -> Printf.sprintf "Response_ok"

type daemon_args = {
  init_id: string;
  verbose: bool;
}
