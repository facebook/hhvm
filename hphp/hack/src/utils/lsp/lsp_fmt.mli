(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val parse_id : Hh_json.json -> Lsp.lsp_id

val parse_id_opt : Hh_json.json option -> Lsp.lsp_id option

val print_id : Lsp.lsp_id -> Hh_json.json

val id_to_string : Lsp.lsp_id -> string

val parse_position : Hh_json.json option -> Lsp.position

val print_position : Lsp.position -> Hh_json.json

val print_range : Lsp.range -> Hh_json.json

val print_location : Lsp.Location.t -> Hh_json.json

val print_locations : Lsp.Location.t list -> Hh_json.json

val print_definition_location : Lsp.DefinitionLocation.t -> Hh_json.json

val print_definition_locations : Lsp.DefinitionLocation.t list -> Hh_json.json

val parse_range_exn : Hh_json.json option -> Lsp.range

val parse_range_opt : Hh_json.json option -> Lsp.range option

val parse_textDocumentIdentifier :
  Hh_json.json option -> Lsp.TextDocumentIdentifier.t

val parse_versionedTextDocumentIdentifier :
  Hh_json.json option -> Lsp.VersionedTextDocumentIdentifier.t

val parse_textDocumentItem : Hh_json.json option -> Lsp.TextDocumentItem.t

val print_textDocumentItem : Lsp.TextDocumentItem.t -> Hh_json.json

val print_markedItem : Lsp.markedString -> Hh_json.json

val parse_textDocumentPositionParams :
  Hh_json.json option -> Lsp.TextDocumentPositionParams.t

val parse_textEdit : Hh_json.json option -> Lsp.TextEdit.t option

val print_textEdit : Lsp.TextEdit.t -> Hh_json.json

val print_command : Lsp.Command.t -> Hh_json.json

val parse_command : Hh_json.json option -> Lsp.Command.t

val parse_formattingOptions :
  Hh_json.json option -> Lsp.DocumentFormatting.formattingOptions

val print_symbolInformation : Lsp.SymbolInformation.t -> Hh_json.json

val print_shutdown : unit -> Hh_json.json

val parse_cancelRequest : Hh_json.json option -> Lsp.CancelRequest.params

val print_cancelRequest : Lsp.CancelRequest.params -> Hh_json.json

val print_rage : Lsp.RageFB.result -> Hh_json.json

val parse_didOpen : Hh_json.json option -> Lsp.DidOpen.params

val print_didOpen : Lsp.DidOpen.params -> Hh_json.json

val parse_didClose : Hh_json.json option -> Lsp.DidClose.params

val parse_didSave : Hh_json.json option -> Lsp.DidSave.params

val parse_didChange : Hh_json.json option -> Lsp.DidChange.params

val parse_signatureHelp : Hh_json.json option -> Lsp.SignatureHelp.params

val print_signatureHelp : Lsp.SignatureHelp.result -> Hh_json.json

val parse_documentRename : Hh_json.json option -> Lsp.Rename.params

val print_documentRename : Lsp.Rename.result -> Hh_json.json

val print_diagnostics : Lsp.PublishDiagnostics.params -> Hh_json.json

val print_logMessage : Lsp.MessageType.t -> string -> Hh_json.json

val print_showMessage : Lsp.MessageType.t -> string -> Hh_json.json

val print_showMessageRequest :
  Lsp.ShowMessageRequest.showMessageRequestParams -> Hh_json.json

val parse_result_showMessageRequest :
  Hh_json.json option -> Lsp.ShowMessageRequest.result

val print_showStatus : Lsp.ShowStatusFB.showStatusParams -> Hh_json.json

val print_connectionStatus : Lsp.ConnectionStatusFB.params -> Hh_json.json

val parse_hover : Hh_json.json option -> Lsp.Hover.params

val print_hover : Lsp.Hover.result -> Hh_json.json

val parse_completionItem :
  Hh_json.json option -> Lsp.CompletionItemResolve.params

val print_completionItem : Lsp.Completion.completionItem -> Hh_json.json

val parse_completion : Hh_json.json option -> Lsp.Completion.params

val print_completion : Lsp.Completion.result -> Hh_json.json

val parse_workspaceSymbol : Hh_json.json option -> Lsp.WorkspaceSymbol.params

val print_workspaceSymbol : Lsp.WorkspaceSymbol.result -> Hh_json.json

val parse_documentSymbol : Hh_json.json option -> Lsp.DocumentSymbol.params

val print_documentSymbol : Lsp.DocumentSymbol.result -> Hh_json.json

val parse_findReferences : Hh_json.json option -> Lsp.FindReferences.params

val parse_documentHighlight :
  Hh_json.json option -> Lsp.DocumentHighlight.params

val print_documentHighlight : Lsp.DocumentHighlight.result -> Hh_json.json

val parse_typeCoverage : Hh_json.json option -> Lsp.TypeCoverageFB.params

val print_typeCoverage : Lsp.TypeCoverageFB.result -> Hh_json.json

val parse_toggleTypeCoverage :
  Hh_json.json option -> Lsp.ToggleTypeCoverageFB.params

val parse_documentFormatting :
  Hh_json.json option -> Lsp.DocumentFormatting.params

val print_documentFormatting : Lsp.DocumentFormatting.result -> Hh_json.json

val parse_documentRangeFormatting :
  Hh_json.json option -> Lsp.DocumentRangeFormatting.params

val print_documentRangeFormatting :
  Lsp.DocumentRangeFormatting.result -> Hh_json.json

val parse_documentOnTypeFormatting :
  Hh_json.json option -> Lsp.DocumentOnTypeFormatting.params

val print_documentOnTypeFormatting :
  Lsp.DocumentOnTypeFormatting.result -> Hh_json.json

val parse_initialize : Hh_json.json option -> Lsp.Initialize.params

val print_initializeError : Lsp.Initialize.errorData -> Hh_json.json

val print_initialize : Lsp.Initialize.result -> Hh_json.json

val print_registerCapability : Lsp.RegisterCapability.params -> Hh_json.json

val parse_didChangeWatchedFiles :
  Hh_json.json option -> Lsp.DidChangeWatchedFiles.params

val print_error : Lsp.Error.t -> Hh_json.json

val error_to_log_string : Lsp.Error.t -> string

val parse_error : Hh_json.json -> Lsp.Error.t

val request_name_to_string : Lsp.lsp_request -> string

val result_name_to_string : Lsp.lsp_result -> string

val notification_name_to_string : Lsp.lsp_notification -> string

val message_name_to_string : Lsp.lsp_message -> string

val denorm_message_to_string : Lsp.lsp_message -> string

val parse_lsp_request : string -> Hh_json.json option -> Lsp.lsp_request

val parse_lsp_notification :
  string -> Hh_json.json option -> Lsp.lsp_notification

val parse_lsp_result : Lsp.lsp_request -> Hh_json.json -> Lsp.lsp_result

val parse_lsp :
  Hh_json.json -> (Lsp.lsp_id -> Lsp.lsp_request) -> Lsp.lsp_message

val print_lsp_request : Lsp.lsp_id -> Lsp.lsp_request -> Hh_json.json

val print_lsp_response : Lsp.lsp_id -> Lsp.lsp_result -> Hh_json.json

val print_lsp_notification : Lsp.lsp_notification -> Hh_json.json

val print_lsp : Lsp.lsp_message -> Hh_json.json

val get_uri_opt : Lsp.lsp_message -> Lsp.documentUri option
