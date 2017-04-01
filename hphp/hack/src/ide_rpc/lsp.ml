(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(**
 * This file is an OCaml representation of the Language Server Protocol
 * https://github.com/Microsoft/language-server-protocol/blob/master/protocol.md
 * based on the current v3.
 *
 * Changes to make it more natural in OCaml:
 * - We don't represent the common base types of Requests/Errors/Notifications
 *   because base types don't naturally mix with abstract data types, and
 *   because code for these things is done more naturally at the JSON layer
 * - We avoid option types where we can. The idea is to follow the internet
 *   "robustness" rule of being liberal in what we accept, conservative in
 *   what we emit: if we're parsing a message and it lacks a field, and if
 *   the spec tells us how to interpret absence, then we do that interpretation
 *   at the JSON->LSP parsing level (so long as the interpretation is lossless).
 *   On the emitting side, we might as well emit all fields.
 * - For every request, like Initialize or workspace/Symbol, we've invented
 *   "Initialize.response = (Initialize.result, Initialize.error) Result"
 *   or "Symbol.response = (Symbol.result, Error.error) Result" to show
 *   the two possible return types from this request. Note that each different
 *   request can have its own custom error type, although most don't.
 * - Most datatypes go in modules since there are so many name-clashes in
 *   the protocol and OCaml doesn't like name-clashes. Only exceptions are
 *   the really primitive types like location and document_uri.
 *   The few places where we still had to rename fields to avoid OCaml name
 *   clashes I've noted in the comments with the word "wire" to indicate the
 *   over-the-wire form of the name.
 * - Names have been translated from jsonConvention into ocaml_convention.
 * - The spec has space for extra fields like "experimental". It obviously
 *   doesn't make sense to encode them in a type system. I've omitted them
 *   entirely.
*)


type document_uri = string

(* A position is between two characters like an 'insert' cursor in a editor *)
type position = {
  line: int;  (* line position in a document [zero-based] *)
  character: int;  (* character offset on a line in a document [zero-based] *)
}

(* A range is comparable to a selection in an editor *)
type range = {
  start: position;  (* the range's start position *)
  end_: position;  (* the range's end position [exclusive] *)
}

(* Represents a location inside a resource, such as a line inside a text file *)
module Location = struct
  type t = {
    uri: document_uri;
    range: range;
  }
end

(* Marked_string can be used to render human readable text. It is either a
 * markdown string or a code-block that provides a language and a code snippet,
 * equivalent to ```${language}\n${value}\n```. Note that markdown strings
 * will be sanitized by the client - including escaping html *)
type marked_string =
  | Marked_string of string
  | Marked_code of string * string (* lang, value *)

(* Represents a reference to a command. Provides a title which will be used to
 * represent a command in the UI. Commands are identitifed using a string
 * identifier and the protocol currently doesn't specify a set of well known
 * commands. So executing a command requires some tool extension code. *)
module Command = struct
  type t = {
    title: string;  (* title of the command, like `save` *)
    command: string;  (* the identifier of the actual command handler *)
    arguments: Hh_json.json list;  (* wire: it can be omitted *)
  }
end

(* A textual edit applicable to a text document. If n text_edits are applied
   to a text document all text edits describe changes to the initial document
   version. Execution wise text edits should applied from the bottom to the
   top of the text document. Overlapping text edits are not supported. *)
module Text_edit = struct
  type t = {
    range: range;  (* to insert text, use a range where start = end *)
    new_text: string;  (* for delete operations, use an empty string *)
  }
end

(* Text documents are identified using a URI. *)
module Text_document_identifier = struct
  type t = {
    uri: document_uri;  (* the text document's URI *)
  }
end

(* An identifier to denote a specific version of a text document. *)
module Versioned_text_document_identifier = struct
  type t = {
    uri: document_uri;  (* the text document's URI *)
    version: int;  (* the version number of this document *)
  }
end

(* Describes textual changes on a single text document. The text document is
  referred to as a versioned_text_document_identifier to allow clients to check
  the text document version before an edit is applied. *)
module Text_document_edit = struct
  type t = {
    text_document: Versioned_text_document_identifier.t;
    edits: Text_edit.t list;
  }
end

(* A workspace edit represents changes to many resources managed in the
   workspace. A workspace edit now consists of an array of text_document_edits
   each describing a change to a single text document. *)
module Workspace_edit = struct
  type t = {
    changes: Text_document_edit.t list;  (* holds changes to existing docs *)
  }
end

(* An item to transfer a text document from the client to the server. The
   version number strictly increases after each change, including undo/redo. *)
module Text_document_item = struct
  type t = {
    uri: document_uri;  (* the text document's URI *)
    language_id: string;  (* the text document's language identifier *)
    version: int;  (* the version of the document *)
    text: string;  (* the content of the opened text document *)
  }
end

(* A parameter literal used in requests to pass a text document and a position
   inside that document. *)
module Text_document_position_params = struct
  type t = {
    text_document: Text_document_identifier.t;  (* the text document *)
    position: position;  (* the position inside the text document *)
  }
end

(* A document filter denotes a document through properties like language,
   schema or pattern. E.g. language:"typescript",scheme:"file"
   or langauge:"json",pattern:"**/package.json" *)
module Document_filter = struct
  type t = {
    language: string option;  (* a language id, like "typescript" *)
    scheme: string option;  (* a uri scheme, like "file" or "untitled" *)
    pattern: string option;  (* a glob pattern, like "*.{ts,js}" *)
  }
end

(* A document selector is the combination of one or many document filters. *)
module Document_selector = struct
  type t = Document_filter.t list
end


(* Represents information about programming constructs like variables etc. *)
module Symbol_information = struct
  type t = {
    name: string;
    kind: symbol_kind;
    location: Location.t;  (* the location of the symbol token itself *)
    container_name: string option;  (* the symbol containing this symbol *)
  }

  and symbol_kind =
    | File  (* 1 *)
    | Module  (* 2 *)
    | Namespace  (* 3 *)
    | Package  (* 4 *)
    | Class  (* 5 *)
    | Method  (* 6 *)
    | Property  (* 7 *)
    | Field  (* 8 *)
    | Constructor  (* 9 *)
    | Enum  (* 10 *)
    | Interface  (* 11 *)
    | Function  (* 12 *)
    | Variable  (* 13 *)
    | Constant  (* 14 *)
    | String  (* 15 *)
    | Number  (* 16 *)
    | Boolean  (* 17 *)
    | Array  (* 18 *)
end


(* Cancellation notification, method="$/cancelRequest" *)
module CancelRequest = struct
  type params = cancel_params

  and cancel_params = {
    id: string;  (* the request id to cancel *)
  }
end

(* Initialize request, method="initialize" *)
module Initialize = struct
  type params = {
    process_id: int option;  (* pid of parent process *)
    root_path: string option;  (* deprecated *)
    root_uri: document_uri option;  (* the root URI of the workspace *)
    (* omitted: initiaization_options *)
    client_capabilities: client_capabilities;  (* "capabilities" over wire *)
    trace: trace;  (* the initial trace setting, default="off" *)
  }

  and result = {
    server_capabilities: server_capabilities; (* "capabilities" over wire *)
  }

  and error_data = {
    retry: bool;  (* should client retry the initialize request *)
  }

  and trace =
    | Off
    | Messages
    | Verbose

  and client_capabilities = {
    workspace: workspace_client_capabilities;
    text_document: text_document_client_capabilities;
    (* omitted: experimental *)
  }

  and workspace_client_capabilities = {
    apply_edit: bool;  (* the client supports appling batch edits *)
    workspace_edit: workspace_edit;
    (* omitted: dynamic-registration fields *)
  }

  and workspace_edit = {
    document_changes: bool;  (* client supports versioned doc changes *)
  }

  and text_document_client_capabilities = {
    synchronization: synchronization;
    completion: completion;  (* textDocument/completion *)
    (* omitted: dynamic-registration fields *)
  }

  (* synchronization capabilities say what messages the client is capable
   * of sending, should be be so asked by the server.
   * We use the "can_" prefix for OCaml naming reasons; it's absent in LSP *)
  and synchronization = {
    can_will_save: bool;  (* client can send textDocument/willSave *)
    can_will_save_wait_until: bool;  (* textDoc.../willSaveWaitUntil *)
    can_did_save: bool;  (* textDocument/didSave *)
  }

  and completion = {
    completion_item: completion_item;
  }

  and completion_item = {
    snippet_support: bool;  (* client can do snippets as insert text *)
  }

  (* What capabilities the server provides *)
  and server_capabilities = {
    text_document_sync: text_document_sync_options; (* how to sync *)
    hover_provider: bool;
    completion_provider: completion_options option;
    signature_help_provider: signature_help_options option;
    definition_provider: bool;
    references_provider: bool;
    document_highlight_provider: bool;
    document_symbol_provider: bool;  (* ie. document outline *)
    workspace_symbol_provider: bool;  (* ie. find-symbol-in-project *)
    code_action_provider: bool;
    code_lens_provider: code_lens_options option;
    document_formatting_provider: bool;
    document_range_formatting_provider: bool;
    document_on_type_formatting_provider :
      document_on_type_formatting_options option;
    rename_provider: bool;
    document_link_provider: document_link_options option;
    execute_command_provider: execute_command_options option;
    (* omitted: experimental *)
  }

  and completion_options = {
    resolve_provider: bool;  (* server resolves extra info on demand *)
    completion_trigger_characters: string list; (* wire "trigger_characters" *)
  }

  and signature_help_options = {
    sighelp_trigger_characters: string list; (* wire "trigger_characters" *)
  }

  and code_lens_options = {
    code_lens_resolve_provider: bool;  (* wire "resolve_provider" *)
  }

  and document_on_type_formatting_options = {
    first_trigger_character: string;  (* e.g. "}" *)
    more_trigger_characters: string list;
  }

  and document_link_options = {
    document_link_resolve_provider: bool;  (* wire "resolve_provider" *)
  }

  and execute_command_options = {
    commands: string list;  (* the commands to be executed on the server *)
  }

  (* text document sync options say what messages the server requests the
   * client to send. We use the "want_" prefix for OCaml naming reasons;
   * this prefix is absent in LSP. *)
  and text_document_sync_options = {
    want_open_close: bool;  (* textDocument/didOpen+didClose *)
    want_change: text_document_sync_kind;
    want_will_save: bool;  (* textDocument/willSave *)
    want_will_save_wait_until: bool;  (* textDoc.../willSaveWaitUntil *)
    want_did_save: save_options;  (* textDocument/didSave *)
  }

  and text_document_sync_kind =
    | NoSync (* 0 *)  (* docs should not be synced at all. Wire "None" *)
    | FullSync (* 1 *)  (* synced by always sending full content. Wire "Full" *)
    | IncrementalSync (* 2 *)  (* full only on open. Wire "Incremental" *)

  and save_options = {
    include_text: bool;  (* the client should include content on save *)
  }
end

(* Shutdown request, method="shutdown" *)
module Shutdown = struct
  type result = unit
end

(* Exit notification, method="exit" *)
module Exit = struct
end

(* Hover request, method="textDocument/hover" *)
module Hover = struct
  type params = Text_document_position_params.t

  and result = {
    contents: marked_string list; (* wire: either a single one or an array *)
    range: range option;
  }
end

(* PublishDiagnostics notification, method="textDocument/PublishDiagnostics" *)
module Publish_diagnostics = struct
  type params = publish_diagnostics_params

  and publish_diagnostics_params = {
    uri: document_uri;
    diagnostics: diagnostic list;
  }

  and diagnostic = {
    range: range;  (* the range at which the message applies *)
    severity: diagnostic_severity option;  (* if omitted, client decides *)
    code: int option;  (* the diagnostic's code. Wire: can be string too *)
    source: string option;  (* human-readable string, eg. typescript/lint *)
    message: string;  (* the diagnostic's message *)
  }

  and diagnostic_severity =
  | Error (* 1 *)
  | Warning (* 2 *)
  | Information (* 3 *)
  | Hint (* 4 *)
end

(* DidOpenTextDocument notification, method="textDocument/didOpen" *)
module Did_open = struct
  type params = did_open_text_document_params

  and did_open_text_document_params = {
    text_document: Text_document_item.t;  (* the document that was opened *)
  }
end

(* DidCloseTextDocument notification, method="textDocument/didClose" *)
module Did_close = struct
  type params = did_close_text_document_params

  and did_close_text_document_params = {
    text_document: Text_document_identifier.t; (* the doc that was closed *)
  }
end

(* DidChangeTextDocument notification, method="textDocument/didChange" *)
module Did_change = struct
  type params = did_change_text_document_params

  and did_change_text_document_params = {
    text_document: Versioned_text_document_identifier.t;
    content_changes: text_document_content_change_event list;
  }

  and text_document_content_change_event = {
    range: range option; (* the range of the document that changed *)
    range_length: int option; (* the length that got replaced *)
    text: string; (* the new text of the range/document *)
  }
end

(* Goto Definition request, method="textDocument/definition" *)
module Definition = struct
  type params = Text_document_position_params.t

  and result = Location.t list  (* wire: either a single one or an array *)
end

(* Completion request, method="textDocument/completion" *)
module Completion = struct
  type params = Text_document_position_params.t

  and result = completion_list  (* wire: can also be 'completion_item list' *)

  and completion_list = {
    is_incomplete: bool; (* further typing should result in recomputing *)
    items: completion_item list;
  }

  and completion_item = {
    label: string;  (* the label in the UI *)
    kind: completion_item_kind option;  (* tells editor which icon to use *)
    detail: string option;  (* human-readable string like type/symbol info *)
    documentation: string option;  (* human-readable doc-comment *)
    sort_text: string option;  (* used for sorting; if absent, uses label *)
    filter_text: string option;  (* used for filtering; if absent, uses label *)
    insert_text: string option;  (* used for inserting; if absent, uses label *)
    insert_text_format: insert_text_format;
    text_edits: Text_edit.t list;  (* wire: split into hd and tl *)
    command: Command.t option;  (* if present, is executed after completion *)
    data: Hh_json.json option;
  }

  and completion_item_kind =
    | Text (* 1 *)
    | Method (* 2 *)
    | Function (* 3 *)
    | Constructor (* 4 *)
    | Field (* 5 *)
    | Variable (* 6 *)
    | Class (* 7 *)
    | Interface (* 8 *)
    | Module (* 9 *)
    | Property (* 10 *)
    | Unit (* 11 *)
    | Value (* 12 *)
    | Enum (* 13 *)
    | Keyword (* 14 *)
    | Snippet (* 15 *)
    | Color (* 16 *)
    | File (* 17 *)
    | Reference (* 18 *)

  and insert_text_format =
    | PlainText (* 1 *)  (* the insert_text/text_edits are just plain strings *)
    | SnippetFormat (* 2 *)  (* wire: just "Snippet" *)
end


(* Completion Item Resolve request, method="completionItem/resolve" *)
module Completion_item_resolve = struct
  type params = Completion.completion_item

  and result = Completion.completion_item
end


(* Workspace Symbols request, method="workspace/symbol" *)
module Workspace_symbol = struct
  type params = workspace_symbol_params

  and result = Symbol_information.t list

  and workspace_symbol_params = {
    query: string;  (* a non-empty query string *)
  }
end


(* ErrorResponse *)
module Error = struct
  exception Parse of string (* -32700 *)
  exception Invalid_request of string (* -32600 *)
  exception Method_not_found of string (* -32601 *)
  exception Invalid_params of string (* -32602 *)
  exception Internal_error of string (* -32603 *)
  exception Server_error_start of string * Initialize.error_data (* -32099 *)
  exception Server_error_end of string (* -32000 *)
  exception Server_not_initialized of string (* -32002*)
  exception Unknown of string (* -32001 *)
end
