(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type lsp_id =
  | NumberId of int
  | StringId of string

type partial_result_token = PartialResultToken of string

module DocumentUri = struct
  module M = struct
    type t = Uri of string [@@deriving eq, ord]
  end

  include M
  module Map = WrappedMap.Make (M)
end

let uri_of_string (s : string) : DocumentUri.t = DocumentUri.Uri s

let string_of_uri (DocumentUri.Uri s) : string = s

type position = {
  line: int;
  character: int;
}
[@@deriving eq]

type range = {
  start: position;
  end_: position;
}
[@@deriving eq]

type textDocumentSaveReason =
  | Manual [@value 1]
  | AfterDelay [@value 2]
  | FocusOut [@value 3]
[@@deriving enum]

module Location = struct
  type t = {
    uri: DocumentUri.t;
    range: range;
  }
  [@@deriving eq]
end

module DefinitionLocation = struct
  type t = {
    location: Location.t;
    title: string option;
  }
end

type markedString =
  | MarkedString of string
  | MarkedCode of string * string

module Command = struct
  type t = {
    title: string;
    command: string;
    arguments: Hh_json.json list;
  }
end

module TextEdit = struct
  type t = {
    range: range;
    newText: string;
  }
end

module TextDocumentIdentifier = struct
  type t = { uri: DocumentUri.t }
end

module VersionedTextDocumentIdentifier = struct
  type t = {
    uri: DocumentUri.t;
    version: int;
  }
end

module TextDocumentEdit = struct
  type t = {
    textDocument: VersionedTextDocumentIdentifier.t;
    edits: TextEdit.t list;
  }
end

module WorkspaceEdit = struct
  type t = { changes: TextEdit.t list SMap.t }
end

module TextDocumentItem = struct
  type t = {
    uri: DocumentUri.t;
    languageId: string;
    version: int;
    text: string;
  }
end

module CodeLens = struct
  type t = {
    range: range;
    command: Command.t;
    data: Hh_json.json option;
  }
end

module TextDocumentPositionParams = struct
  type t = {
    textDocument: TextDocumentIdentifier.t;
    position: position;
  }
end

module DocumentFilter = struct
  type t = {
    language: string option;
    scheme: string option;
    pattern: string option;
  }
end

module DocumentSelector = struct
  type t = DocumentFilter.t list
end

module SymbolInformation = struct
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
    location: Location.t;
    containerName: string option;
  }
end

module CallHierarchyItem = struct
  type t = {
    name: string;
    kind: SymbolInformation.symbolKind;
    detail: string option;
    uri: DocumentUri.t;
    range: range;
    selectionRange: range;
  }
end

module CallHierarchyCallsRequestParam = struct
  type t = { item: CallHierarchyItem.t }
end

module MessageType = struct
  type t =
    | ErrorMessage [@value 1]
    | WarningMessage [@value 2]
    | InfoMessage [@value 3]
    | LogMessage [@value 4]
  [@@deriving eq, enum]
end

module CodeActionKind = struct
  (** CodeActionKind.t uses a pair to represent a non-empty list
  and we provide utility functions for creation, membership, printing.*)
  type t = string * string list

  let is_kind : t -> t -> bool =
    let rec is_prefix_of ks xs =
      match (ks, xs) with
      | ([], _) -> true
      | (k :: ks, x :: xs) when String.equal k x -> is_prefix_of ks xs
      | (_, _) -> false
    in
    (fun (k, ks) (x, xs) -> String.equal k x && is_prefix_of ks xs)

  let contains_kind k ks = List.exists ~f:(is_kind k) ks

  let contains_kind_opt ~default k ks =
    match ks with
    | Some ks -> contains_kind k ks
    | None -> default

  let kind_of_string : string -> t =
   fun s ->
    match Caml.String.split_on_char '.' s with
    | [] -> failwith "split_on_char does not return an empty list"
    | k :: ks -> (k, ks)

  let string_of_kind : t -> string =
   (fun (k, ks) -> String.concat ~sep:"." (k :: ks))

  let sub_kind : t -> string -> t =
    let cons_to_end (ss : string list) (s : string) =
      Base.List.(fold_right ss ~f:cons ~init:[s])
    in
    (fun (k, ks) s -> (k, cons_to_end ks s))

  let quickfix = kind_of_string "quickfix"

  let refactor = kind_of_string "refactor"

  let source = kind_of_string "source"
end

module CancelRequest = struct
  type params = cancelParams

  and cancelParams = { id: lsp_id }
end

module SetTraceNotification = struct
  type params =
    | Verbose
    | Off
end

module Initialize = struct
  type textDocumentSyncKind =
    | NoSync [@value 0]
    | FullSync [@value 1]
    | IncrementalSync [@value 2]
  [@@deriving enum]

  module CodeActionOptions = struct
    type t = { resolveProvider: bool }
  end

  module CompletionOptions = struct
    type t = {
      resolveProvider: bool;
      completion_triggerCharacters: string list;
    }
  end

  module ServerExperimentalCapabilities = struct
    type t = { snippetTextEdit: bool }
  end

  module ClientExperimentalCapabilities = struct
    type t = { snippetTextEdit: bool }
  end

  type params = {
    processId: int option;
    rootPath: string option;
    rootUri: DocumentUri.t option;
    initializationOptions: initializationOptions;
    client_capabilities: client_capabilities;
    trace: trace;
  }

  and result = { server_capabilities: server_capabilities }

  and errorData = { retry: bool }

  and trace =
    | Off
    | Messages
    | Verbose

  and initializationOptions = {
    namingTableSavedStatePath: string option;
    namingTableSavedStateTestDelay: float;
    delayUntilDoneInit: bool;
    skipLspServerOnTypeFormatting: bool;
  }

  and client_capabilities = {
    workspace: workspaceClientCapabilities;
    textDocument: textDocumentClientCapabilities;
    window: windowClientCapabilities;
    telemetry: telemetryClientCapabilities;
    client_experimental: ClientExperimentalCapabilities.t;
  }

  and workspaceClientCapabilities = {
    applyEdit: bool;
    workspaceEdit: workspaceEdit;
    didChangeWatchedFiles: dynamicRegistration;
  }

  and dynamicRegistration = { dynamicRegistration: bool }

  and workspaceEdit = { documentChanges: bool }

  and textDocumentClientCapabilities = {
    synchronization: synchronization;
    completion: completion;
    codeAction: codeAction;
    definition: definition;
    typeDefinition: typeDefinition;
    declaration: declaration;
    implementation: implementation;
  }

  and synchronization = {
    can_willSave: bool;
    can_willSaveWaitUntil: bool;
    can_didSave: bool;
  }

  and completion = { completionItem: completionItem }

  and completionItem = { snippetSupport: bool }

  and codeAction = {
    codeAction_dynamicRegistration: bool;
    codeActionLiteralSupport: codeActionliteralSupport option;
  }

  and definition = { definitionLinkSupport: bool }

  and typeDefinition = { typeDefinitionLinkSupport: bool }

  and declaration = { declarationLinkSupport: bool }

  and implementation = { implementationLinkSupport: bool }

  and codeActionliteralSupport = { codeAction_valueSet: CodeActionKind.t list }

  and windowClientCapabilities = { status: bool }

  and telemetryClientCapabilities = { connectionStatus: bool }

  and server_capabilities = {
    textDocumentSync: textDocumentSyncOptions;
    hoverProvider: bool;
    completionProvider: CompletionOptions.t option;
    signatureHelpProvider: signatureHelpOptions option;
    definitionProvider: bool;
    typeDefinitionProvider: bool;
    referencesProvider: bool;
    callHierarchyProvider: bool;
    documentHighlightProvider: bool;
    documentSymbolProvider: bool;
    workspaceSymbolProvider: bool;
    codeActionProvider: CodeActionOptions.t option;
    codeLensProvider: codeLensOptions option;
    documentFormattingProvider: bool;
    documentRangeFormattingProvider: bool;
    documentOnTypeFormattingProvider: documentOnTypeFormattingOptions option;
    renameProvider: bool;
    documentLinkProvider: documentLinkOptions option;
    executeCommandProvider: executeCommandOptions option;
    implementationProvider: bool;
    rageProviderFB: bool;  (** Nuclide-specific feature *)
    server_experimental: ServerExperimentalCapabilities.t option;
  }

  and signatureHelpOptions = { sighelp_triggerCharacters: string list }

  and codeLensOptions = { codelens_resolveProvider: bool }

  and documentOnTypeFormattingOptions = {
    firstTriggerCharacter: string;
    moreTriggerCharacter: string list;
  }

  and documentLinkOptions = { doclink_resolveProvider: bool }

  and executeCommandOptions = { commands: string list }

  and textDocumentSyncOptions = {
    want_openClose: bool;
    want_change: textDocumentSyncKind;
    want_willSave: bool;
    want_willSaveWaitUntil: bool;
    want_didSave: saveOptions option;
  }

  and saveOptions = { includeText: bool }
end

module Error = struct
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

  exception LspException of t
end

module RageFB = struct
  type result = rageItem list

  and rageItem = {
    title: string option;
    data: string;
  }
end

module CodeLensResolve = struct
  type params = CodeLens.t

  and result = CodeLens.t
end

module Hover = struct
  type params = TextDocumentPositionParams.t

  and result = hoverResult option

  and hoverResult = {
    contents: markedString list;
    range: range option;
  }
end

module PublishDiagnostics = struct
  type diagnosticSeverity =
    | Error [@value 1]
    | Warning [@value 2]
    | Information [@value 3]
    | Hint [@value 4]
  [@@deriving eq, enum]

  type params = publishDiagnosticsParams

  and publishDiagnosticsParams = {
    uri: DocumentUri.t;
    diagnostics: diagnostic list;
    isStatusFB: bool;
  }

  and diagnostic = {
    range: range;
    severity: diagnosticSeverity option;
    code: diagnosticCode;
    source: string option;
    message: string;
    relatedInformation: diagnosticRelatedInformation list;
    relatedLocations: relatedLocation list;
  }
  [@@deriving eq]

  and diagnosticCode =
    | IntCode of int
    | StringCode of string
    | NoCode
  [@@deriving eq]

  and diagnosticRelatedInformation = {
    relatedLocation: Location.t;
    relatedMessage: string;
  }
  [@@deriving eq]

  and relatedLocation = diagnosticRelatedInformation
end

module DidOpen = struct
  type params = didOpenTextDocumentParams

  and didOpenTextDocumentParams = { textDocument: TextDocumentItem.t }
end

module DidClose = struct
  type params = didCloseTextDocumentParams

  and didCloseTextDocumentParams = { textDocument: TextDocumentIdentifier.t }
end

module DidSave = struct
  type params = didSaveTextDocumentParams

  and didSaveTextDocumentParams = {
    textDocument: TextDocumentIdentifier.t;
    text: string option;
  }
end

module DidChange = struct
  type params = didChangeTextDocumentParams

  and didChangeTextDocumentParams = {
    textDocument: VersionedTextDocumentIdentifier.t;
    contentChanges: textDocumentContentChangeEvent list;
  }

  and textDocumentContentChangeEvent = {
    range: range option;
    rangeLength: int option;
    text: string;
  }
end

module WillSaveWaitUntil = struct
  type params = willSaveWaitUntilTextDocumentParams

  and willSaveWaitUntilTextDocumentParams = {
    textDocument: TextDocumentIdentifier.t;
    reason: textDocumentSaveReason;
  }

  and result = TextEdit.t list
end

module DidChangeWatchedFiles = struct
  type registerOptions = { watchers: fileSystemWatcher list }

  and fileSystemWatcher = { globPattern: string }

  type fileChangeType =
    | Created [@value 1]
    | Updated [@value 2]
    | Deleted [@value 3]
  [@@deriving enum]

  type params = { changes: fileEvent list }

  and fileEvent = {
    uri: DocumentUri.t;
    type_: fileChangeType;
  }
end

module Definition = struct
  type params = TextDocumentPositionParams.t

  and result = DefinitionLocation.t list
end

module TypeDefinition = struct
  type params = TextDocumentPositionParams.t

  and result = DefinitionLocation.t list
end

module Implementation = struct
  type params = TextDocumentPositionParams.t

  and result = Location.t list
end

(* textDocument/codeAction and codeAction/resolve *)
module CodeAction = struct
  type 'a t = {
    title: string;
    kind: CodeActionKind.t;
    diagnostics: PublishDiagnostics.diagnostic list;
    action: 'a edit_and_or_command;
  }

  and 'a edit_and_or_command =
    | EditOnly of WorkspaceEdit.t
    | CommandOnly of Command.t
    | BothEditThenCommand of (WorkspaceEdit.t * Command.t)
    | UnresolvedEdit of 'a
        (** UnresolvedEdit is for this flow:
      client --textDocument/codeAction --> server --response_with_unresolved_fields-->
      client --codeAction/resolve --> server --response_with_all_fields--> client
    *)

  type 'a command_or_action_ =
    | Command of Command.t
    | Action of 'a t

  type resolved_marker = |

  type resolved_command_or_action = resolved_marker command_or_action_

  type command_or_action = unit command_or_action_

  type result = command_or_action list
end

module CodeActionRequest = struct
  type params = {
    textDocument: TextDocumentIdentifier.t;
    range: range;
    context: codeActionContext;
  }

  and codeActionContext = {
    diagnostics: PublishDiagnostics.diagnostic list;
    only: CodeActionKind.t list option;
  }
end

module CodeActionResolve = struct
  type result = (CodeAction.resolved_command_or_action, Error.t) Result.t
end

(** method="codeAction/resolve" *)
module CodeActionResolveRequest = struct
  type params = {
    data: CodeActionRequest.params;
    title: string;
  }
end

(* Completion request, method="textDocument/completion" *)
module Completion = struct
  type completionItemKind =
    | Text [@value 1]
    | Method [@value 2]
    | Function [@value 3]
    | Constructor [@value 4]
    | Field [@value 5]
    | Variable [@value 6]
    | Class [@value 7]
    | Interface [@value 8]
    | Module [@value 9]
    | Property [@value 10]
    | Unit [@value 11]
    | Value [@value 12]
    | Enum [@value 13]
    | Keyword [@value 14]
    | Snippet [@value 15]
    | Color [@value 16]
    | File [@value 17]
    | Reference [@value 18]
    | Folder [@value 19]
    | MemberOf [@value 20]
    | Constant [@value 21]
    | Struct [@value 22]
    | Event [@value 23]
    | Operator [@value 24]
    | TypeParameter [@value 25]
  [@@deriving enum]

  type insertTextFormat =
    | PlainText [@value 1]
    | SnippetFormat [@value 2]
  [@@deriving enum]

  type completionTriggerKind =
    | Invoked [@value 1]
    | TriggerCharacter [@value 2]
    | TriggerForIncompleteCompletions [@value 3]
  [@@deriving enum]

  let is_invoked = function
    | Invoked -> true
    | TriggerCharacter
    | TriggerForIncompleteCompletions ->
      false

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

  and completionList = {
    isIncomplete: bool;
    items: completionItem list;
  }

  and completionDocumentation =
    | MarkedStringsDocumentation of markedString list
    | UnparsedDocumentation of Hh_json.json

  and completionItem = {
    label: string;
    kind: completionItemKind option;
    detail: string option;
    documentation: completionDocumentation option;
    sortText: string option;
    filterText: string option;
    insertText: string option;
    insertTextFormat: insertTextFormat option;
    textEdit: TextEdit.t option;
    additionalTextEdits: TextEdit.t list;
    command: Command.t option;
    data: Hh_json.json option;
  }
end

module CompletionItemResolve = struct
  type params = Completion.completionItem

  and result = Completion.completionItem
end

module WorkspaceSymbol = struct
  type params = workspaceSymbolParams

  and result = SymbolInformation.t list

  and workspaceSymbolParams = { query: string  (** a non-empty query string *) }
end

module DocumentSymbol = struct
  type params = documentSymbolParams

  and result = SymbolInformation.t list

  and documentSymbolParams = { textDocument: TextDocumentIdentifier.t }
end

module FindReferences = struct
  type params = referenceParams

  and result = Location.t list

  and referenceParams = {
    loc: TextDocumentPositionParams.t;
    context: referenceContext;
    partialResultToken: partial_result_token option;
  }

  and referenceContext = {
    includeDeclaration: bool;
    includeIndirectReferences: bool;
  }
end

module PrepareCallHierarchy = struct
  type params = TextDocumentPositionParams.t

  type result = CallHierarchyItem.t list option
end

module CallHierarchyIncomingCalls = struct
  type params = CallHierarchyCallsRequestParam.t

  type result = callHierarchyIncomingCall list option

  and callHierarchyIncomingCall = {
    from: CallHierarchyItem.t;
    fromRanges: range list;
  }
end

module CallHierarchyOutgoingCalls = struct
  type params = CallHierarchyCallsRequestParam.t

  type result = callHierarchyOutgoingCall list option

  and callHierarchyOutgoingCall = {
    call_to: CallHierarchyItem.t;
    fromRanges: range list;
  }
end

module DocumentHighlight = struct
  type params = TextDocumentPositionParams.t

  type documentHighlightKind =
    | Text [@value 1]
    | Read [@value 2]
    | Write [@value 3]
  [@@deriving enum]

  type result = documentHighlight list

  and documentHighlight = {
    range: range;
    kind: documentHighlightKind option;
  }
end

module DocumentFormatting = struct
  type params = documentFormattingParams

  and result = TextEdit.t list

  and documentFormattingParams = {
    textDocument: TextDocumentIdentifier.t;
    options: formattingOptions;
  }

  and formattingOptions = {
    tabSize: int;
    insertSpaces: bool;
  }
end

module DocumentRangeFormatting = struct
  type params = documentRangeFormattingParams

  and result = TextEdit.t list

  and documentRangeFormattingParams = {
    textDocument: TextDocumentIdentifier.t;
    range: range;
    options: DocumentFormatting.formattingOptions;
  }
end

(** Document On Type Formatting req., method="textDocument/onTypeFormatting" *)
module DocumentOnTypeFormatting = struct
  type params = documentOnTypeFormattingParams

  and result = TextEdit.t list

  and documentOnTypeFormattingParams = {
    textDocument: TextDocumentIdentifier.t;
    position: position;
    ch: string;
    options: DocumentFormatting.formattingOptions;
  }
end

module SignatureHelp = struct
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

(* Document Type Hierarchy request, method="textDocument/typeHierarchy" *)
module TypeHierarchy = struct
  type params = TextDocumentPositionParams.t

  type memberKind =
    | Method [@value 1]
    | SMethod [@value 2]
    | Property [@value 3]
    | SProperty [@value 4]
    | Const [@value 5]
  [@@deriving enum]

  type memberEntry = {
    name: string;
    snippet: string;
    kind: memberKind;
    uri: DocumentUri.t;
    range: range;
    origin: string;
  }

  type entryKind =
    | Class [@value 1]
    | Interface [@value 2]
    | Enum [@value 3]
    | Trait [@value 4]
  [@@deriving enum]

  type ancestorEntry =
    | AncestorName of string
    | AncestorDetails of {
        name: string;
        kind: entryKind;
        uri: DocumentUri.t;
        range: range;
      }

  type hierarchyEntry = {
    name: string;
    uri: DocumentUri.t;
    range: range;
    kind: entryKind;
    ancestors: ancestorEntry list;
    members: memberEntry list;
  }

  type result = hierarchyEntry option
end

(* Workspace Rename request, method="textDocument/rename" *)
module Rename = struct
  type params = renameParams

  and result = WorkspaceEdit.t

  and renameParams = {
    textDocument: TextDocumentIdentifier.t;
    position: position;
    newName: string;
  }
end

(** Code Lens request, method="textDocument/codeLens" *)
module DocumentCodeLens = struct
  type params = codelensParams

  and result = CodeLens.t list

  and codelensParams = { textDocument: TextDocumentIdentifier.t }
end

module LogMessage = struct
  type params = logMessageParams

  and logMessageParams = {
    type_: MessageType.t;
    message: string;
  }
end

module ShowMessage = struct
  type params = showMessageParams

  and showMessageParams = {
    type_: MessageType.t;
    message: string;
  }
end

module ShowMessageRequest = struct
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

module ShowStatusFB = struct
  type params = showStatusParams

  and result = unit

  (** the showStatus LSP request will be handled by our VSCode extension.
  It's a facebook-specific extension to the LSP spec. How it's rendered
  is currently [shortMessage] in the status-bar, and "[progress]/[total] [message]"
  in the tooltip. The [telemetry] field isn't displayed to the user, but might
  be useful to someone debugging an LSP transcript. *)
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

module ConnectionStatusFB = struct
  type params = connectionStatusParams

  and connectionStatusParams = { isConnected: bool }
end

type lsp_registration_options =
  | DidChangeWatchedFilesRegistrationOptions of
      DidChangeWatchedFiles.registerOptions

module RegisterCapability = struct
  type params = { registrations: registration list }

  and registration = {
    id: string;
    method_: string;
    registerOptions: lsp_registration_options;
  }

  let make_registration (registerOptions : lsp_registration_options) :
      registration =
    let (id, method_) =
      match registerOptions with
      | DidChangeWatchedFilesRegistrationOptions _ ->
        ("did-change-watched-files", "workspace/didChangeWatchedFiles")
    in
    { id; method_; registerOptions }
end

(**
 * Here are gathered-up ADTs for all the messages we handle
*)

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
  | TypeHierarchyRequest of TypeHierarchy.params
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
  | TypeHierarchyResult of TypeHierarchy.result
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
  | ShowMessageNotification of ShowMessage.params
  | ConnectionStatusNotificationFB of ConnectionStatusFB.params
  | InitializedNotification
  | FindReferencesPartialResultNotification of
      partial_result_token * FindReferences.result
  | SetTraceNotification of SetTraceNotification.params
  | LogTraceNotification
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

module IdKey = struct
  type t = lsp_id

  let compare (x : t) (y : t) =
    match (x, y) with
    | (NumberId x, NumberId y) -> x - y
    | (NumberId _, StringId _) -> -1
    | (StringId x, StringId y) -> String.compare x y
    | (StringId _, NumberId _) -> 1
end

module IdSet = Caml.Set.Make (IdKey)
module IdMap = WrappedMap.Make (IdKey)

module UriKey = struct
  type t = DocumentUri.t

  let compare (DocumentUri.Uri x) (DocumentUri.Uri y) = String.compare x y
end

module UriSet = Caml.Set.Make (UriKey)
module UriMap = WrappedMap.Make (UriKey)

let lsp_result_to_log_string = function
  | InitializeResult _ -> "InitializeResult"
  | ShutdownResult -> "ShutdownResult"
  | CodeLensResolveResult _ -> "CodeLensResolveResult"
  | HoverResult _ -> "HoverResult"
  | DefinitionResult _ -> "DefinitionResult"
  | TypeDefinitionResult _ -> "TypeDefinitionResult"
  | ImplementationResult _ -> "ImplementationResult"
  | CodeActionResult _ -> "CodeActionResult"
  | CodeActionResolveResult _ -> "CodeActionResolveResult"
  | CompletionResult _ -> "CompletionResult"
  | CompletionItemResolveResult _ -> "CompletionItemResolveResult"
  | WorkspaceSymbolResult _ -> "WorkspaceSymbolResult"
  | DocumentSymbolResult _ -> "DocumentSymbolResult"
  | FindReferencesResult _ -> "FindReferencesResult"
  | PrepareCallHierarchyResult _ -> "PrepareCallHierarchyResult"
  | CallHierarchyIncomingCallsResult _ -> "CallHierarchyIncomingCallsResult"
  | CallHierarchyOutgoingCallsResult _ -> "CallHierarchyOutgoingCallsResult"
  | DocumentHighlightResult _ -> "DocumentHighlightResult"
  | DocumentFormattingResult _ -> "DocumentFormattingResult"
  | DocumentRangeFormattingResult _ -> "DocumentRangeFormattingResult"
  | DocumentOnTypeFormattingResult _ -> "DocumentOnTypeFormattingResult"
  | ShowMessageRequestResult _ -> "ShowMessageRequestResult"
  | ShowStatusResultFB _ -> "ShowStatusResultFB"
  | RageResultFB _ -> "RageResultFB"
  | RenameResult _ -> "RenameResult"
  | DocumentCodeLensResult _ -> "DocumentCodeLensResult"
  | SignatureHelpResult _ -> "SignatureHelpResult"
  | HackTestStartServerResultFB -> "HackTestStartServerResultFB"
  | HackTestStopServerResultFB -> "HackTestStopServerResultFB"
  | HackTestShutdownServerlessResultFB -> "HackTestShutdownServerlessResultFB"
  | RegisterCapabilityRequestResult -> "RegisterCapabilityRequestResult"
  | WillSaveWaitUntilResult _ -> "WillSaveWaitUntilResult"
  | ErrorResult _ -> "ErrorResult"
  | TypeHierarchyResult _ -> "TypeHierarchyResult"
