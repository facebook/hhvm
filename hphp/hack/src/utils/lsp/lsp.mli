(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
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
 *   the really primitive types like location and DocumentUri.t.
 *   The few places where we still had to rename fields to avoid OCaml name
 *   clashes I've noted in the comments with the word "wire" to indicate the
 *   over-the-wire form of the name.
 * - Names have been translated from jsonConvention into ocaml convention
 *   only where necessary, e.g. because ocaml uses lowercase for types.
 * - The spec has space for extra fields like "experimental". It obviously
 *   doesn't make sense to encode them in a type system. I've omitted them
 *   entirely.
*)
open Hh_prelude

type lsp_id =
  | NumberId of int
  | StringId of string

type partial_result_token = PartialResultToken of string

(** Note: this datatype provides no invariants that the string is well-formed. *)
module DocumentUri : sig
  type t = Uri of string [@@deriving eq, ord]

  module Map : WrappedMap.S with type key := t
end

val uri_of_string : string -> DocumentUri.t

val string_of_uri : DocumentUri.t -> string

(** A position is between two characters like an 'insert' cursor in a editor *)
type position = {
  line: int;  (** line position in a document [zero-based] *)
  character: int;  (** character offset on a line in a document [zero-based] *)
}
[@@deriving eq]

(** A range is comparable to a selection in an editor *)
type range = {
  start: position;  (** the range's start position *)
  end_: position;  (** the range's end position [exclusive] *)
}
[@@deriving eq]

type textDocumentSaveReason =
  | Manual [@value 1]
  | AfterDelay [@value 2]
  | FocusOut [@value 3]
[@@deriving enum]

(** Represents a location inside a resource, such as a line inside a text file *)
module Location : sig
  type t = {
    uri: DocumentUri.t;
    range: range;
  }
  [@@deriving eq]
end

(** Represents a location inside a resource which also wants to display a
   friendly name to the user. *)
module DefinitionLocation : sig
  type t = {
    location: Location.t;
    title: string option;
  }
end

(** markedString can be used to render human readable text. It is either a
 * markdown string or a code-block that provides a language and a code snippet.
 * Note that markdown strings will be sanitized by the client - including
 * escaping html *)
type markedString =
  | MarkedString of string
  | MarkedCode of string * string  (** lang, value *)

(* Represents a reference to a command. Provides a title which will be used to
 * represent a command in the UI. Commands are identitifed using a string
 * identifier and the protocol currently doesn't specify a set of well known
 * commands. So executing a command requires some tool extension code. *)
module Command : sig
  type t = {
    title: string;  (** title of the command, like `save` *)
    command: string;  (** the identifier of the actual command handler *)
    arguments: Hh_json.json list;  (** wire: it can be omitted *)
  }
end

(** A textual edit applicable to a text document. If n textEdits are applied
   to a text document all text edits describe changes to the initial document
   version. Execution wise text edits should applied from the bottom to the
   top of the text document. Overlapping text edits are not supported. *)
module TextEdit : sig
  type t = {
    range: range;  (** to insert text, use a range where start = end *)
    newText: string;  (** for delete operations, use an empty string *)
  }
end

(** Text documents are identified using a URI. *)
module TextDocumentIdentifier : sig
  type t = { uri: DocumentUri.t }
end

(** An identifier to denote a specific version of a text document. *)
module VersionedTextDocumentIdentifier : sig
  type t = {
    uri: DocumentUri.t;
    version: int;
  }
end

(** Describes textual changes on a single text document. The text document is
   referred to as a VersionedTextDocumentIdentifier to allow clients to check
   the text document version before an edit is applied. *)
module TextDocumentEdit : sig
  type t = {
    textDocument: VersionedTextDocumentIdentifier.t;
    edits: TextEdit.t list;
  }
end

(** A workspace edit represents changes to many resources managed in the
   workspace. A workspace edit consists of a mapping from a URI to an
   array of TextEdits to be applied to the document with that URI. *)
module WorkspaceEdit : sig
  type t = {
    changes: TextEdit.t list DocumentUri.Map.t;
        (* holds changes to existing docs *)
  }
end

(** An item to transfer a text document from the client to the server. The
   version number strictly increases after each change, including undo/redo. *)
module TextDocumentItem : sig
  type t = {
    uri: DocumentUri.t;
    languageId: string;
    version: int;
    text: string;
  }
end

(**
 * A code lens represents a command that should be shown along with
 * source text, like the number of references, a way to run tests, etc.
 *
 * A code lens is _unresolved_ when no command is associated to it. For performance
 * reasons the creation of a code lens and resolving should be done in two stages.
 *)
module CodeLens : sig
  type t = {
    range: range;
    command: Command.t;
    data: Hh_json.json option;
  }
end

(** A parameter literal used in requests to pass a text document and a position
   inside that document. *)
module TextDocumentPositionParams : sig
  type t = {
    textDocument: TextDocumentIdentifier.t;
    position: position;
  }
end

(** A document filter denotes a document through properties like language,
   schema or pattern. E.g. language:"typescript",scheme:"file"
   or language:"json",pattern:"**/package.json" *)
module DocumentFilter : sig
  type t = {
    language: string option;  (** a language id, like "typescript" *)
    scheme: string option;  (** a uri scheme, like "file" or "untitled" *)
    pattern: string option;  (** a glob pattern, like "*.{ts,js}" *)
  }
end

(** A document selector is the combination of one or many document filters. *)
module DocumentSelector : sig
  type t = DocumentFilter.t list
end

(** Represents information about programming constructs like variables etc. *)
module SymbolInformation : sig
  (** These numbers should match
   * https://microsoft.github.io/language-server-protocol/specifications/specification-3-17/#symbolKind
   *)
  type symbolKind =
    | File [@value 1]
    | Module [@value 2]
    | Namespace [@value 3]
    | Package [@value 4]
    | Class [@value 5]
    | Method [@value 6]
    | Property [@value 7]
    | Field [@value 8]
    | Constructor [@value 9]
    | Enum [@value 10]
    | Interface [@value 11]
    | Function [@value 12]
    | Variable [@value 13]
    | Constant [@value 14]
    | String [@value 15]
    | Number [@value 16]
    | Boolean [@value 17]
    | Array [@value 18]
    | Object [@value 19]
    | Key [@value 20]
    | Null [@value 21]
    | EnumMember [@value 22]
    | Struct [@value 23]
    | Event [@value 24]
    | Operator [@value 25]
    | TypeParameter [@value 26]
  [@@deriving enum]

  type t = {
    name: string;
    kind: symbolKind;
    location: Location.t;  (** the span of the symbol including its contents *)
    containerName: string option;  (** the symbol containing this symbol *)
  }
end

(** Represents an item in the Call Hieararchy *)
module CallHierarchyItem : sig
  type t = {
    name: string;
    kind: SymbolInformation.symbolKind;
    detail: string option;
    uri: DocumentUri.t;
    range: range;
    selectionRange: range;
  }
end

(** Represents a parameter for a CallHierarchyIncomingCallsRequest or CallHierarchyOutgoingCallsRequest *)
module CallHierarchyCallsRequestParam : sig
  type t = { item: CallHierarchyItem.t }
end

(** For showing messages (not diagnostics) in the user interface. *)
module MessageType : sig
  type t =
    | ErrorMessage [@value 1]
    | WarningMessage [@value 2]
    | InfoMessage [@value 3]
    | LogMessage [@value 4]
  [@@deriving eq, enum]
end

(** Cancellation notification, method="$/cancelRequest" *)
module CancelRequest : sig
  type params = cancelParams

  and cancelParams = { id: lsp_id  (** the request id to cancel *) }
end

(** SetTraceNotification, method="$/setTraceNotification" *)
module SetTraceNotification : sig
  type params =
    | Verbose
    | Off
end

(**  The kind of a code action.
  *  Kinds are a hierarchical list of identifiers separated by `.`, e.g.
  * `"refactor.extract.function"`.
  * The set of kinds is open and client needs to announce the kinds it supports
  * to the server during initialization.
  * Module CodeAction below also references this module as Kind.
  *)
module CodeActionKind : sig
  type t = string * string list

  (** is x of kind k? *)
  val is_kind : t -> t -> bool

  (** does `ks` contain kind `k` *)
  val contains_kind : t -> t list -> bool

  (** does an optional list of kinds `ks` contain kind `k` *)
  val contains_kind_opt : default:bool -> t -> t list option -> bool

  (** Create a kind from a string that follows the spec *)
  val kind_of_string : string -> t

  (** Create the equivalent string that the spec would have required *)
  val string_of_kind : t -> string

  (** Create a new sub-kind of an existing kind *)
  val sub_kind : t -> string -> t

  (** A constant defined by the spec *)
  val quickfix : t

  (** A constant defined by the spec *)
  val refactor : t

  (** Document-wide code actions *)
  val source : t
end

(** Initialize request, method="initialize" *)
module Initialize : sig
  type textDocumentSyncKind =
    | NoSync [@value 0]  (** docs should not be synced at all. Wire "None" *)
    | FullSync [@value 1]
        (** synced by always sending full content. Wire "Full" *)
    | IncrementalSync [@value 2]
  [@@deriving enum]

  module CodeActionOptions : sig
    type t = { resolveProvider: bool }
  end

  module CompletionOptions : sig
    type t = {
      resolveProvider: bool;  (** server resolves extra info on demand *)
      completion_triggerCharacters: string list;  (** wire "triggerCharacters" *)
    }
  end

  module ServerExperimentalCapabilities : sig
    type t = {
      snippetTextEdit: bool;
          (** see ClientExperimentalCapabilities.snippetTextEdit *)
      autoCloseJsx: bool;
    }
  end

  module ClientExperimentalCapabilities : sig
    type t = {
      snippetTextEdit: bool;
          (** A client that supports this capability accepts snippet text edits like `${0:foo}`.
       * https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#snippet_syntax
       **)
      autoCloseJsx: bool;
    }
  end

  type params = {
    processId: int option;  (** pid of parent process *)
    rootPath: string option;  (** deprecated *)
    rootUri: DocumentUri.t option;  (** the root URI of the workspace *)
    initializationOptions: initializationOptions;
    client_capabilities: client_capabilities;  (** "capabilities" over wire *)
    trace: trace;  (** the initial trace setting, default="off" *)
  }

  and result = {
    server_capabilities: server_capabilities;  (** "capabilities" over wire *)
  }

  and errorData = {
    retry: bool;  (** should client retry the initialize request *)
  }

  and trace =
    | Off
    | Messages
    | Verbose

  (** These are hack-specific options. They're all optional in initialize request,
  and we pick a default if necessary while parsing. *)
  and initializationOptions = {
    namingTableSavedStatePath: string option;
        (** used for test scenarios where we pass a naming-table sqlite file,
    rather than leaving clientIdeDaemon to find and download one itself. *)
    namingTableSavedStateTestDelay: float;
        (** used for test scenarios where we've passed
    a naming-sqlite file and so clientIdeDaemon completes instantly, but we still want
    a little bit of a delay before it reports readiness so as to exercise race conditions.
    This is the delay in seconds. *)
    delayUntilDoneInit: bool;
        (** used for test scenarios where we want clientLsp to delay receiving any
    further LSP requests until clientIdeDaemon has done its init. *)
    skipLspServerOnTypeFormatting: bool;
        (** `true` iff formatting is to be done on the LSP client,
          * rather than provided by the LSP server.
        *)
  }

  and client_capabilities = {
    workspace: workspaceClientCapabilities;
    textDocument: textDocumentClientCapabilities;
    window: windowClientCapabilities;
    telemetry: telemetryClientCapabilities;  (** omitted: experimental *)
    client_experimental: ClientExperimentalCapabilities.t;
  }

  and workspaceClientCapabilities = {
    applyEdit: bool;  (** client supports appling batch edits *)
    workspaceEdit: workspaceEdit;
    didChangeWatchedFiles: dynamicRegistration;
        (** omitted: other dynamic-registration fields *)
  }

  and dynamicRegistration = {
    dynamicRegistration: bool;
        (** client supports dynamic registration for this capability *)
  }

  and workspaceEdit = {
    documentChanges: bool;  (** client supports versioned doc changes *)
  }

  and textDocumentClientCapabilities = {
    synchronization: synchronization;
    completion: completion;  (** textDocument/completion *)
    codeAction: codeAction;
    definition: definition;
    typeDefinition: typeDefinition;
    declaration: declaration;
    implementation: implementation;
  }

  (** synchronization capabilities say what messages the client is capable
   * of sending, should be be so asked by the server.
   * We use the "can_" prefix for OCaml naming reasons; it's absent in LSP *)
  and synchronization = {
    can_willSave: bool;  (** client can send textDocument/willSave *)
    can_willSaveWaitUntil: bool;  (** textDoc.../willSaveWaitUntil *)
    can_didSave: bool;  (** textDocument/didSave *)
  }

  and completion = { completionItem: completionItem }

  and completionItem = {
    snippetSupport: bool;  (** client can do snippets as insert text *)
  }

  and codeAction = {
    codeAction_dynamicRegistration: bool;
        (** wire: dynamicRegistraction
     Whether code action supports dynamic registration. *)
    codeActionLiteralSupport: codeActionliteralSupport option;
        (** The client support code action literals as a valid
     * response of the `textDocument/codeAction` request. *)
  }

  (** The code action kind values the client supports. When this
     * property exists the client also guarantees that it will
     * handle values outside its set gracefully and falls back
     * to a default value when unknown. *)
  and codeActionliteralSupport = {
    codeAction_valueSet: CodeActionKind.t list;  (** wire: valueSet *)
  }

  and definition = { definitionLinkSupport: bool }

  and typeDefinition = { typeDefinitionLinkSupport: bool }

  and declaration = { declarationLinkSupport: bool }

  and implementation = { implementationLinkSupport: bool }

  and windowClientCapabilities = {
    status: bool;
        (** Nuclide-specific: client supports window/showStatusRequest *)
  }

  and telemetryClientCapabilities = {
    connectionStatus: bool;
        (** Nuclide-specific: client supports telemetry/connectionStatus *)
  }

  (** What capabilities the server provides *)
  and server_capabilities = {
    textDocumentSync: textDocumentSyncOptions;  (** how to sync *)
    hoverProvider: bool;
    completionProvider: CompletionOptions.t option;
    signatureHelpProvider: signatureHelpOptions option;
    definitionProvider: bool;
    typeDefinitionProvider: bool;
    referencesProvider: bool;
    callHierarchyProvider: bool;
    documentHighlightProvider: bool;
    documentSymbolProvider: bool;  (** ie. document outline *)
    workspaceSymbolProvider: bool;  (** ie. find-symbol-in-project *)
    codeActionProvider: CodeActionOptions.t option;
    codeLensProvider: codeLensOptions option;
    documentFormattingProvider: bool;
    documentRangeFormattingProvider: bool;
    documentOnTypeFormattingProvider: documentOnTypeFormattingOptions option;
    renameProvider: bool;
    documentLinkProvider: documentLinkOptions option;
    executeCommandProvider: executeCommandOptions option;
    implementationProvider: bool;
    rageProviderFB: bool;
    server_experimental: ServerExperimentalCapabilities.t option;
  }

  and signatureHelpOptions = { sighelp_triggerCharacters: string list }

  and codeLensOptions = {
    codelens_resolveProvider: bool;  (** wire "resolveProvider" *)
  }

  and documentOnTypeFormattingOptions = {
    firstTriggerCharacter: string;
    moreTriggerCharacter: string list;  (** e.g. "}" *)
  }

  and documentLinkOptions = {
    doclink_resolveProvider: bool;  (** wire "resolveProvider" *)
  }

  and executeCommandOptions = {
    commands: string list;  (** the commands to be executed on the server *)
  }

  (** text document sync options say what messages the server requests the
   * client to send. We use the "want_" prefix for OCaml naming reasons;
   * this prefix is absent in LSP. *)
  and textDocumentSyncOptions = {
    want_openClose: bool;  (** textDocument/didOpen+didClose *)
    want_change: textDocumentSyncKind;
    want_willSave: bool;  (** textDocument/willSave *)
    want_willSaveWaitUntil: bool;  (** textDoc.../willSaveWaitUntil *)
    want_didSave: saveOptions option;  (** textDocument/didSave *)
  }

  (** full only on open. Wire "Incremental" *)
  and saveOptions = {
    includeText: bool;  (** the client should include content on save *)
  }
end

(** ErrorResponse *)
module Error : sig
  type code =
    | ParseError [@value -32700]
    | InvalidRequest [@value -32600]
    | MethodNotFound [@value -32601]
    | InvalidParams [@value -32602]
    | InternalError [@value -32603]
    | ServerErrorStart [@value -32099]
    | ServerErrorEnd [@value -32000]
    | ServerNotInitialized [@value -32002]
    | UnknownErrorCode [@value -32001]
    | RequestCancelled [@value -32800]
    | ContentModified [@value -32801]
  [@@deriving show, enum]

  type t = {
    code: code;
    message: string;
    data: Hh_json.json option;
  }

  (** For methods which want to return exceptions, and they also want to decide
  how the exception gets serialized over LSP, they should throw this one. *)
  exception LspException of t
end

(** Rage request, method="telemetry/rage" *)
module RageFB : sig
  type result = rageItem list

  and rageItem = {
    title: string option;
    data: string;
  }
end

(** Code Lens resolve request, method="codeLens/resolve" *)
module CodeLensResolve : sig
  type params = CodeLens.t

  and result = CodeLens.t
end

(** Hover request, method="textDocument/hover" *)
module Hover : sig
  type params = TextDocumentPositionParams.t

  and result = hoverResult option

  and hoverResult = {
    contents: markedString list;  (** wire: either a single one or an array *)
    range: range option;
  }
end

(** PublishDiagnostics notification, method="textDocument/PublishDiagnostics" *)
module PublishDiagnostics : sig
  type diagnosticCode =
    | IntCode of int
    | StringCode of string
    | NoCode
  [@@deriving eq]

  type diagnosticSeverity =
    | Error
    | Warning
    | Information
    | Hint
  [@@deriving enum, eq]

  type params = publishDiagnosticsParams

  and publishDiagnosticsParams = {
    uri: DocumentUri.t;
    diagnostics: diagnostic list;
    isStatusFB: bool;
        (** FB-specific extension, for diagnostics used only to show status *)
  }

  and diagnostic = {
    range: range;  (** the range at which the message applies *)
    severity: diagnosticSeverity option;  (** if omitted, client decides *)
    code: diagnosticCode;
    source: string option;  (** human-readable string, eg. typescript/lint *)
    message: string;  (** the diagnostic's message *)
    relatedInformation: diagnosticRelatedInformation list;
    relatedLocations: relatedLocation list;  (** legacy FB extension *)
  }
  [@@deriving eq]

  and diagnosticRelatedInformation = {
    relatedLocation: Location.t;  (** wire: just "location" *)
    relatedMessage: string;  (** wire: just "message" *)
  }
  [@@deriving eq]

  (** legacy FB extension *)
  and relatedLocation = diagnosticRelatedInformation
end

(** DidOpenTextDocument notification, method="textDocument/didOpen" *)
module DidOpen : sig
  type params = didOpenTextDocumentParams

  and didOpenTextDocumentParams = {
    textDocument: TextDocumentItem.t;  (** the document that was opened *)
  }
end

(** DidCloseTextDocument notification, method="textDocument/didClose" *)
module DidClose : sig
  type params = didCloseTextDocumentParams

  and didCloseTextDocumentParams = {
    textDocument: TextDocumentIdentifier.t;  (** the doc that was closed *)
  }
end

(** DidSaveTextDocument notification, method="textDocument/didSave" *)
module DidSave : sig
  type params = didSaveTextDocumentParams

  and didSaveTextDocumentParams = {
    textDocument: TextDocumentIdentifier.t;  (** the doc that was saved *)
    text: string option;  (** content when saved; depends on includeText *)
  }
end

(** DidChangeTextDocument notification, method="textDocument/didChange" *)
module DidChange : sig
  type params = didChangeTextDocumentParams

  and didChangeTextDocumentParams = {
    textDocument: VersionedTextDocumentIdentifier.t;
    contentChanges: textDocumentContentChangeEvent list;
  }

  and textDocumentContentChangeEvent = {
    range: range option;  (** the range of the document that changed *)
    rangeLength: int option;  (** the length that got replaced *)
    text: string;  (** the new text of the range/document *)
  }
end

(** WillSaveWaitUntilTextDocument request, method="textDocument/willSaveWaitUntil" *)
module WillSaveWaitUntil : sig
  type params = willSaveWaitUntilTextDocumentParams

  and willSaveWaitUntilTextDocumentParams = {
    textDocument: TextDocumentIdentifier.t;
    reason: textDocumentSaveReason;
  }

  and result = TextEdit.t list
end

(** Watched files changed notification, method="workspace/didChangeWatchedFiles" *)
module DidChangeWatchedFiles : sig
  type registerOptions = { watchers: fileSystemWatcher list }

  and fileSystemWatcher = { globPattern: string }

  type fileChangeType =
    | Created
    | Updated
    | Deleted
  [@@deriving enum]

  type params = { changes: fileEvent list }

  and fileEvent = {
    uri: DocumentUri.t;
    type_: fileChangeType;
  }
end

(** Goto Definition request, method="textDocument/definition" *)
module Definition : sig
  type params = TextDocumentPositionParams.t

  (** wire: either a single one or an array *)
  and result = DefinitionLocation.t list
end

(** Goto TypeDefinition request, method="textDocument/typeDefinition" *)
module TypeDefinition : sig
  type params = TextDocumentPositionParams.t

  and result = DefinitionLocation.t list
end

(** Go To Implementation request, method="textDocument/implementation" *)
module Implementation : sig
  type params = TextDocumentPositionParams.t

  and result = Location.t list
end

(** A code action represents a change that can be performed in code, e.g. to fix a problem or
     to refactor code. *)
module CodeAction : sig
  (**
  Note: For "textDocument/codeAction" requests we return a `data` field containing
  the original request params, then when the client sends "codeAction/resolve"
  we read the `data` param to re-calculate the requested code action. This
  adding of the "data" field is done in our serialization step, to avoid passing
  extra state around and enforce that `data` is all+only the original request params.
  See [edit_or_command] for more information on the resolution flow.
  *)
  type 'resolution_phase t = {
    title: string;  (** A short, human-readable, title for this code action. *)
    kind: CodeActionKind.t;
        (** The kind of the code action. Used to filter code actions. *)
    diagnostics: PublishDiagnostics.diagnostic list;
        (** The diagnostics that this code action resolves. *)
    action: 'resolution_phase edit_and_or_command;
        (* A CodeAction must set either `edit`, a `command` (or neither iff only resolved lazily)
           If both are supplied, the `edit` is applied first, then the `command` is executed.
           If neither is supplied, the client requests 'edit' be resolved using "codeAction/resolve"
        *)
  }

  (**
  'resolution_phase is used to follow the protocol in a type-safe and prescribed manner:
  LSP protocol:
  1. The client sends server "textDocument/codeAction"
  2. The server can send back an unresolved code action (neither "edit" nor "command" fields)
  3. If the code action is unresolved, the client sends "codeAction/resolve"

  Our implementation flow:
    - create a representation of a code action which includes a lazily-computed edit
      - if the request is "textDocument/codeAction", we do not compute an edit
      - if the request is "codeAction/resolve", we have access to the original
      request params via the `data` field (see [t] comments above) and perform
      the same calculation as for "textDocument/codeAction" and then compute the edit.
  *)
  and 'resolution_phase edit_and_or_command =
    | EditOnly of WorkspaceEdit.t
    | CommandOnly of Command.t
    | BothEditThenCommand of (WorkspaceEdit.t * Command.t)
    | UnresolvedEdit of 'resolution_phase

  type 'resolution_phase command_or_action_ =
    | Command of Command.t
    | Action of 'resolution_phase t

  type resolved_marker = |

  type resolved_command_or_action = resolved_marker command_or_action_

  type command_or_action = unit command_or_action_

  type result = command_or_action list
end

(** Code Action Request, method="textDocument/codeAction" *)
module CodeActionRequest : sig
  type params = {
    textDocument: TextDocumentIdentifier.t;
        (** The document in which the command was invoked. *)
    range: range;  (** The range for which the command was invoked. *)
    context: codeActionContext;  (** Context carrying additional information. *)
  }

  (** Contains additional diagnostic information about the context in which
     a code action is run. *)
  and codeActionContext = {
    diagnostics: PublishDiagnostics.diagnostic list;
    only: CodeActionKind.t list option;
  }
end

(** Completion request, method="textDocument/completion" *)
module CodeActionResolve : sig
  type result = (CodeAction.resolved_command_or_action, Error.t) Result.t
end

(** method="codeAction/resolve" *)
module CodeActionResolveRequest : sig
  (**
    The client sends a partially-resolved [CodeAction] with an additional [data] field.
    We don't bother parsing all the fields from the partially-resolved [CodeAction] because
    [data] and [title] are all we need and so we don't have to duplicate the entire
    [CodeAction.command_or_action] shape here *)
  type params = {
    data: CodeActionRequest.params;
    title: string;
        (** From LSP spec: "A data entry field that is preserved on a code action between
          a `textDocument/codeAction` and a `codeAction/resolve` request"
         We commit to a single representation for simplicity and type-safety *)
  }
end

module Completion : sig
  (** These numbers should match
   * https://microsoft.github.io/language-server-protocol/specification#textDocument_completion
   *)
  type completionItemKind =
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
    | Folder (* 19 *)
    | MemberOf (* 20 *)
    | Constant (* 21 *)
    | Struct (* 22 *)
    | Event (* 23 *)
    | Operator (* 24 *)
    | TypeParameter (* 25 *)
  [@@deriving enum]

  (** These numbers should match
   * https://microsoft.github.io/language-server-protocol/specification#textDocument_completion
   *)
  type insertTextFormat =
    | PlainText (* 1 *)  (** the insertText/textEdits are just plain strings *)
    | SnippetFormat (* 2 *)  (** wire: just "Snippet" *)
  [@@deriving enum]

  type completionTriggerKind =
    | Invoked [@value 1]
    | TriggerCharacter [@value 2]
    | TriggerForIncompleteCompletions [@value 3]
  [@@deriving enum]

  val is_invoked : completionTriggerKind -> bool

  type params = completionParams

  and completionParams = {
    loc: TextDocumentPositionParams.t;
    context: completionContext option;
  }

  and completionContext = {
    triggerKind: completionTriggerKind;
    triggerCharacter: string option;
  }

  and result = completionList

  (** wire: can also be 'completionItem list' *)
  and completionList = {
    isIncomplete: bool;  (** further typing should result in recomputing *)
    items: completionItem list;
  }

  and completionDocumentation =
    | MarkedStringsDocumentation of markedString list
    | UnparsedDocumentation of Hh_json.json

  and completionItem = {
    label: string;  (** the label in the UI *)
    kind: completionItemKind option;  (** tells editor which icon to use *)
    detail: string option;  (** human-readable string like type/symbol info *)
    documentation: completionDocumentation option;
        (** human-readable doc-comment *)
    sortText: string option;  (** used for sorting; if absent, uses label *)
    filterText: string option;  (** used for filtering; if absent, uses label *)
    insertText: string option;  (** used for inserting; if absent, uses label *)
    insertTextFormat: insertTextFormat option;
    textEdit: TextEdit.t option;
    additionalTextEdits: TextEdit.t list;  (** wire: split into hd and tl *)
    command: Command.t option;  (** if present, is executed after completion *)
    data: Hh_json.json option;
  }
end

(** Completion Item Resolve request, method="completionItem/resolve" *)
module CompletionItemResolve : sig
  type params = Completion.completionItem

  and result = Completion.completionItem
end

(** Workspace Symbols request, method="workspace/symbol" *)
module WorkspaceSymbol : sig
  type params = workspaceSymbolParams

  and result = SymbolInformation.t list

  and workspaceSymbolParams = { query: string }
end

(** Document Symbols request, method="textDocument/documentSymbol" *)
module DocumentSymbol : sig
  type params = documentSymbolParams

  and result = SymbolInformation.t list

  and documentSymbolParams = { textDocument: TextDocumentIdentifier.t }
end

(** Find References request, method="textDocument/references" *)
module FindReferences : sig
  type params = referenceParams

  and result = Location.t list

  and referenceParams = {
    loc: TextDocumentPositionParams.t;
        (** wire: loc's members are part of referenceParams *)
    context: referenceContext;
    partialResultToken: partial_result_token option;
  }

  and referenceContext = {
    includeDeclaration: bool;  (** include declaration of current symbol *)
    includeIndirectReferences: bool;
  }
end

module PrepareCallHierarchy : sig
  type params = TextDocumentPositionParams.t

  type result = CallHierarchyItem.t list option
end

module CallHierarchyIncomingCalls : sig
  type params = CallHierarchyCallsRequestParam.t

  type result = callHierarchyIncomingCall list option

  and callHierarchyIncomingCall = {
    from: CallHierarchyItem.t;
    fromRanges: range list;
  }
end

module CallHierarchyOutgoingCalls : sig
  type params = CallHierarchyCallsRequestParam.t

  type result = callHierarchyOutgoingCall list option

  and callHierarchyOutgoingCall = {
    call_to: CallHierarchyItem.t;
        (** The name should just be "to", but "to" is a reserved keyword in OCaml*)
    fromRanges: range list;
  }
end

(** Document Highlights request, method="textDocument/documentHighlight" *)
module DocumentHighlight : sig
  type params = TextDocumentPositionParams.t

  type documentHighlightKind =
    | Text [@value 1]  (** a textual occurrence *)
    | Read [@value 2]  (** read-access of a symbol, like reading a variable *)
    | Write [@value 3]  (** write-access of a symbol, like writing a variable *)
  [@@deriving enum]

  type result = documentHighlight list

  and documentHighlight = {
    range: range;  (** the range this highlight applies to *)
    kind: documentHighlightKind option;
  }
end

(** Document Formatting request, method="textDocument/formatting" *)
module DocumentFormatting : sig
  type params = documentFormattingParams

  and result = TextEdit.t list

  and documentFormattingParams = {
    textDocument: TextDocumentIdentifier.t;
    options: formattingOptions;
  }

  and formattingOptions = {
    tabSize: int;  (** size of a tab in spaces *)
    insertSpaces: bool;
        (** prefer spaces over tabs
    omitted: signature for further properties *)
  }
end

(** Document Range Formatting request, method="textDocument/rangeFormatting" *)
module DocumentRangeFormatting : sig
  type params = documentRangeFormattingParams

  and result = TextEdit.t list

  and documentRangeFormattingParams = {
    textDocument: TextDocumentIdentifier.t;
    range: range;
    options: DocumentFormatting.formattingOptions;
  }
end

module DocumentOnTypeFormatting : sig
  type params = documentOnTypeFormattingParams

  and result = TextEdit.t list

  and documentOnTypeFormattingParams = {
    textDocument: TextDocumentIdentifier.t;
    position: position;  (** the position at which this request was sent *)
    ch: string;  (** the character that has been typed *)
    options: DocumentFormatting.formattingOptions;
  }
end

(** Document Signature Help request, method="textDocument/signatureHelp" *)
module SignatureHelp : sig
  type params = TextDocumentPositionParams.t

  and result = t option

  and t = {
    signatures: signature_information list;
    activeSignature: int;
    activeParameter: int;
  }

  and signature_information = {
    siginfo_label: string;
    siginfo_documentation: string option;
    parameters: parameter_information list;
  }

  and parameter_information = {
    parinfo_label: string;
    parinfo_documentation: string option;
  }
end

module AutoCloseJsx : sig
  type params = TextDocumentPositionParams.t

  and result = string option
end

(** Workspace Rename request, method="textDocument/rename" *)
module Rename : sig
  type params = renameParams

  and result = WorkspaceEdit.t

  and renameParams = {
    textDocument: TextDocumentIdentifier.t;
    position: position;
    newName: string;
  }
end

module DocumentCodeLens : sig
  type params = codelensParams

  and result = CodeLens.t list

  and codelensParams = { textDocument: TextDocumentIdentifier.t }
end

(** LogMessage notification, method="window/logMessage" *)
module LogMessage : sig
  type params = logMessageParams

  and logMessageParams = {
    type_: MessageType.t;
    message: string;
  }
end

(** ShowMessage notification, method="window/showMessage" *)
module ShowMessage : sig
  type params = showMessageParams

  and showMessageParams = {
    type_: MessageType.t;
    message: string;
  }
end

(** ShowMessage request, method="window/showMessageRequest" *)
module ShowMessageRequest : sig
  type t =
    | Present of { id: lsp_id }
    | Absent

  and params = showMessageRequestParams

  and result = messageActionItem option

  and showMessageRequestParams = {
    type_: MessageType.t;
    message: string;
    actions: messageActionItem list;
  }

  and messageActionItem = { title: string }
end

(** ShowStatus request, method="window/showStatus" *)
module ShowStatusFB : sig
  type params = showStatusParams

  and result = unit

  and showStatusParams = {
    request: showStatusRequestParams;
    progress: int option;
    total: int option;
    shortMessage: string option;
    telemetry: Hh_json.json option;
  }

  and showStatusRequestParams = {
    type_: MessageType.t;
    message: string;
  }
end

(** ConnectionStatus notification, method="telemetry/connectionStatus" *)
module ConnectionStatusFB : sig
  type params = connectionStatusParams

  and connectionStatusParams = { isConnected: bool }
end

type lsp_registration_options =
  | DidChangeWatchedFilesRegistrationOptions of
      DidChangeWatchedFiles.registerOptions

(** Register capability request, method="client/registerCapability" *)
module RegisterCapability : sig
  type params = { registrations: registration list }

  and registration = {
    id: string;
        (** The ID field is arbitrary but unique per type of capability (for future
       deregistering, which we don't do). *)
    method_: string;
    registerOptions: lsp_registration_options;
  }

  val make_registration : lsp_registration_options -> registration
end

type lsp_request =
  | InitializeRequest of Initialize.params
  | RegisterCapabilityRequest of RegisterCapability.params
  | ShutdownRequest
  | CodeLensResolveRequest of CodeLensResolve.params
  | HoverRequest of Hover.params
  | DefinitionRequest of Definition.params
  | TypeDefinitionRequest of TypeDefinition.params
  | ImplementationRequest of Implementation.params
  | CodeActionRequest of CodeActionRequest.params
  | CodeActionResolveRequest of CodeActionResolveRequest.params
  | CompletionRequest of Completion.params
  | CompletionItemResolveRequest of CompletionItemResolve.params
  | WorkspaceSymbolRequest of WorkspaceSymbol.params
  | DocumentSymbolRequest of DocumentSymbol.params
  | FindReferencesRequest of FindReferences.params
  | PrepareCallHierarchyRequest of PrepareCallHierarchy.params
  | CallHierarchyIncomingCallsRequest of CallHierarchyIncomingCalls.params
  | CallHierarchyOutgoingCallsRequest of CallHierarchyOutgoingCalls.params
  | DocumentHighlightRequest of DocumentHighlight.params
  | DocumentFormattingRequest of DocumentFormatting.params
  | DocumentRangeFormattingRequest of DocumentRangeFormatting.params
  | DocumentOnTypeFormattingRequest of DocumentOnTypeFormatting.params
  | ShowMessageRequestRequest of ShowMessageRequest.params
  | ShowStatusRequestFB of ShowStatusFB.params
  | RageRequestFB
  | RenameRequest of Rename.params
  | DocumentCodeLensRequest of DocumentCodeLens.params
  | SignatureHelpRequest of SignatureHelp.params
  | AutoCloseRequest of AutoCloseJsx.params
  | HackTestStartServerRequestFB
  | HackTestStopServerRequestFB
  | HackTestShutdownServerlessRequestFB
  | WillSaveWaitUntilRequest of WillSaveWaitUntil.params
  | UnknownRequest of string * Hh_json.json option

type lsp_result =
  | InitializeResult of Initialize.result
  | ShutdownResult
  | CodeLensResolveResult of CodeLensResolve.result
  | HoverResult of Hover.result
  | DefinitionResult of Definition.result
  | TypeDefinitionResult of TypeDefinition.result
  | ImplementationResult of Implementation.result
  | CodeActionResult of CodeAction.result * CodeActionRequest.params
  | CodeActionResolveResult of CodeActionResolve.result
  | CompletionResult of Completion.result
  | CompletionItemResolveResult of CompletionItemResolve.result
  | WorkspaceSymbolResult of WorkspaceSymbol.result
  | DocumentSymbolResult of DocumentSymbol.result
  | FindReferencesResult of FindReferences.result
  | PrepareCallHierarchyResult of PrepareCallHierarchy.result
  | CallHierarchyIncomingCallsResult of CallHierarchyIncomingCalls.result
  | CallHierarchyOutgoingCallsResult of CallHierarchyOutgoingCalls.result
  | DocumentHighlightResult of DocumentHighlight.result
  | DocumentFormattingResult of DocumentFormatting.result
  | DocumentRangeFormattingResult of DocumentRangeFormatting.result
  | DocumentOnTypeFormattingResult of DocumentOnTypeFormatting.result
  | ShowMessageRequestResult of ShowMessageRequest.result
  | ShowStatusResultFB of ShowStatusFB.result
  | RageResultFB of RageFB.result
  | RenameResult of Rename.result
  | DocumentCodeLensResult of DocumentCodeLens.result
  | SignatureHelpResult of SignatureHelp.result
  | AutoCloseResult of AutoCloseJsx.result
  | HackTestStartServerResultFB
  | HackTestStopServerResultFB
  | HackTestShutdownServerlessResultFB
  | RegisterCapabilityRequestResult
  | WillSaveWaitUntilResult of WillSaveWaitUntil.result
  | ErrorResult of Error.t

type lsp_notification =
  | ExitNotification
  | CancelRequestNotification of CancelRequest.params
  | PublishDiagnosticsNotification of PublishDiagnostics.params
  | DidOpenNotification of DidOpen.params
  | DidCloseNotification of DidClose.params
  | DidSaveNotification of DidSave.params
  | DidChangeNotification of DidChange.params
  | DidChangeWatchedFilesNotification of DidChangeWatchedFiles.params
  | LogMessageNotification of LogMessage.params
  | TelemetryNotification of LogMessage.params * (string * Hh_json.json) list
      (** For telemetry, LSP allows 'any', but we're going to send these *)
  | ShowMessageNotification of ShowMessage.params
  | ConnectionStatusNotificationFB of ConnectionStatusFB.params
  | InitializedNotification
  | FindReferencesPartialResultNotification of
      partial_result_token * FindReferences.result
  | SetTraceNotification of SetTraceNotification.params
  | LogTraceNotification (* $/logTraceNotification *)
  | UnknownNotification of string * Hh_json.json option

type lsp_message =
  | RequestMessage of lsp_id * lsp_request
  | ResponseMessage of lsp_id * lsp_result
  | NotificationMessage of lsp_notification

type 'a lsp_handler = 'a lsp_result_handler * 'a lsp_error_handler

and 'a lsp_error_handler = Error.t * string -> 'a -> 'a

and 'a lsp_result_handler =
  | ShowMessageHandler of (ShowMessageRequest.result -> 'a -> 'a)
  | ShowStatusHandler of (ShowStatusFB.result -> 'a -> 'a)

module IdKey : sig
  type t = lsp_id

  val compare : t -> t -> int
end

module IdSet : sig
  include module type of Stdlib.Set.Make (IdKey)
end

module IdMap : sig
  include module type of WrappedMap.Make (IdKey)
end

module UriKey : sig
  type t = DocumentUri.t

  val compare : t -> t -> int
end

module UriSet : sig
  include module type of Stdlib.Set.Make (UriKey)
end

module UriMap : sig
  include module type of WrappedMap.Make (UriKey)
end

val lsp_result_to_log_string : lsp_result -> string
