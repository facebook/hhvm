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
open Hh_json_helpers

(************************************************************************)
(* Miscellaneous LSP structures                                         *)
(************************************************************************)

let parse_id (json : Yojson.Safe.t) : lsp_id =
  match json with
  | `Int n -> NumberId n
  | `Intlit s -> begin
    try NumberId (int_of_string s) with
    | Failure _ ->
      raise
        (Error.LspException
           {
             Error.code = Error.ParseError;
             message = "float ids not allowed: " ^ s;
             data = None;
           })
  end
  | `String s -> StringId s
  | `Float f ->
    raise
      (Error.LspException
         {
           Error.code = Error.ParseError;
           message = "float ids not allowed: " ^ string_of_float f;
           data = None;
         })
  | _ ->
    raise
      (Error.LspException
         {
           Error.code = Error.ParseError;
           message = "not an id: " ^ Yojson.Safe.to_string json;
           data = None;
         })

let parse_id_opt (json : Yojson.Safe.t option) : lsp_id option =
  Option.map json ~f:parse_id

let print_id (id : lsp_id) : Yojson.Safe.t =
  match id with
  | NumberId n -> `Intlit (string_of_int n)
  | StringId s -> `String s

let id_to_string (id : lsp_id) : string =
  match id with
  | NumberId n -> string_of_int n
  | StringId s -> Printf.sprintf "\"%s\"" s

let parse_position (json : Yojson.Safe.t option) : position =
  { line = Jget.int_exn json "line"; character = Jget.int_exn json "character" }

let print_position (position : position) : Yojson.Safe.t =
  `Assoc [("line", `Int position.line); ("character", `Int position.character)]

let print_range (range : range) : Yojson.Safe.t =
  `Assoc
    [("start", print_position range.start); ("end", print_position range.end_)]

let print_location (location : Location.t) : Yojson.Safe.t =
  Location.(
    `Assoc
      [
        ("uri", `String (string_of_uri location.uri));
        ("range", print_range location.range);
      ])

let print_locations (r : Location.t list) : Yojson.Safe.t =
  `List (List.map r ~f:print_location)

let print_definition_location (definition_location : DefinitionLocation.t) :
    Yojson.Safe.t =
  DefinitionLocation.(
    let location = definition_location.location in
    let fields =
      List.filter_opt
        [
          Some ("uri", `String (string_of_uri location.Location.uri));
          Some ("range", print_range location.Location.range);
          Option.map definition_location.title ~f:(fun t ->
              ("title", `String t));
        ]
    in
    `Assoc fields)

let print_definition_locations (r : DefinitionLocation.t list) : Yojson.Safe.t =
  `List (List.map r ~f:print_definition_location)

let parse_range_exn (json : Yojson.Safe.t option) : range =
  {
    start = Jget.obj_exn json "start" |> parse_position;
    end_ = Jget.obj_exn json "end" |> parse_position;
  }

let parse_location (j : Yojson.Safe.t option) : Location.t =
  Location.
    {
      uri = Jget.string_exn j "uri" |> uri_of_string;
      range = Jget.obj_exn j "range" |> parse_range_exn;
    }

let parse_range_opt (json : Yojson.Safe.t option) : range option =
  if Option.is_none json then
    None
  else
    Some (parse_range_exn json)

(************************************************************************)

let print_error (e : Error.t) : Yojson.Safe.t =
  let data =
    match e.Error.data with
    | None -> []
    | Some data -> [("data", data)]
  in
  let entries =
    ("code", `Int (Error.code_to_enum e.Error.code))
    :: ("message", `String e.Error.message)
    :: data
  in
  `Assoc entries

let error_to_log_string (e : Error.t) : string =
  let data =
    Option.value_map
      e.Error.data
      ~f:(fun d -> Yojson.Safe.pretty_to_string d)
      ~default:""
  in
  Printf.sprintf
    "%s [%s]\n%s"
    e.Error.message
    (Error.show_code e.Error.code)
    data

let parse_error (error : Yojson.Safe.t) : Error.t =
  let json = Some error in
  let code =
    Jget.int_exn json "code"
    |> Error.code_of_enum
    |> Option.value ~default:Error.UnknownErrorCode
  in
  let message = Jget.string_exn json "message" in
  let data = Jget.val_opt json "data" in
  { Error.code; message; data }

let parse_textDocumentIdentifier (json : Yojson.Safe.t option) :
    TextDocumentIdentifier.t =
  TextDocumentIdentifier.{ uri = Jget.string_exn json "uri" |> uri_of_string }

let parse_versionedTextDocumentIdentifier (json : Yojson.Safe.t option) :
    VersionedTextDocumentIdentifier.t =
  VersionedTextDocumentIdentifier.
    {
      uri = Jget.string_exn json "uri" |> uri_of_string;
      version = Jget.int_d json "version" ~default:0;
    }

let parse_textDocumentItem (json : Yojson.Safe.t option) : TextDocumentItem.t =
  TextDocumentItem.
    {
      uri = Jget.string_exn json "uri" |> uri_of_string;
      languageId = Jget.string_d json "languageId" ~default:"";
      version = Jget.int_d json "version" ~default:0;
      text = Jget.string_exn json "text";
    }

let print_textDocumentItem (item : TextDocumentItem.t) : Yojson.Safe.t =
  TextDocumentItem.(
    `Assoc
      [
        ("uri", `String (string_of_uri item.uri));
        ("languageId", `String item.languageId);
        ("version", `Intlit (string_of_int item.version));
        ("text", `String item.text);
      ])

let print_markedItem (item : markedString) : Yojson.Safe.t =
  match item with
  | MarkedString s -> `String s
  | MarkedCode (language, value) ->
    `Assoc [("language", `String language); ("value", `String value)]

let parse_textDocumentPositionParams (params : Yojson.Safe.t option) :
    TextDocumentPositionParams.t =
  TextDocumentPositionParams.
    {
      textDocument =
        Jget.obj_exn params "textDocument" |> parse_textDocumentIdentifier;
      position = Jget.obj_exn params "position" |> parse_position;
    }

let parse_textEdit (params : Yojson.Safe.t option) : TextEdit.t option =
  match params with
  | None -> None
  | _ ->
    TextEdit.(
      Some
        {
          range = Jget.obj_exn params "range" |> parse_range_exn;
          newText = Jget.string_exn params "newText";
        })

let print_textEdit (edit : TextEdit.t) : Yojson.Safe.t =
  TextEdit.(
    `Assoc
      [("range", print_range edit.range); ("newText", `String edit.newText)])

let print_textEdits (r : TextEdit.t list) : Yojson.Safe.t =
  `List (List.map r ~f:print_textEdit)

let print_workspaceEdit (r : WorkspaceEdit.t) : Yojson.Safe.t =
  WorkspaceEdit.(
    let print_workspace_edit_changes (Lsp.DocumentUri.Uri uri, text_edits) =
      (uri, print_textEdits text_edits)
    in
    `Assoc
      [
        ( "changes",
          `Assoc
            (List.map
               (Lsp.DocumentUri.Map.elements r.changes)
               ~f:print_workspace_edit_changes) );
      ])

let print_command (command : Command.t) : Yojson.Safe.t =
  Command.(
    `Assoc
      [
        ("title", `String command.title);
        ("command", `String command.command);
        ("arguments", `List command.arguments);
      ])

let parse_command (json : Yojson.Safe.t option) : Command.t =
  Command.
    {
      title = Jget.string_d json "title" ~default:"";
      command = Jget.string_d json "command" ~default:"";
      arguments = Jget.array_d json "arguments" ~default:[] |> List.filter_opt;
    }

let parse_formattingOptions (json : Yojson.Safe.t option) :
    DocumentFormatting.formattingOptions =
  {
    DocumentFormatting.tabSize = Jget.int_d json "tabSize" ~default:2;
    insertSpaces = Jget.bool_d json "insertSpaces" ~default:true;
  }

let print_symbolInformation (info : SymbolInformation.t) : Yojson.Safe.t =
  SymbolInformation.(
    Jprint.object_opt
      [
        ("name", Some (`String info.name));
        ("kind", Some (`Int (SymbolInformation.symbolKind_to_enum info.kind)));
        ("location", Some (print_location info.location));
        ("detail", Option.map info.detail ~f:(fun d -> `String d));
        ("containerName", Option.map info.containerName ~f:(fun d -> `String d));
      ])

let parse_codeLens (json : Yojson.Safe.t option) : CodeLens.t =
  CodeLens.
    {
      range = Jget.obj_exn json "range" |> parse_range_exn;
      command = Jget.obj_exn json "command" |> parse_command;
      data = Jget.obj_exn json "data";
    }

let print_codeLens (codeLens : CodeLens.t) : Yojson.Safe.t =
  CodeLens.(
    `Assoc
      [
        ("range", print_range codeLens.range);
        ("command", print_command codeLens.command);
        ( "data",
          match codeLens.data with
          | None -> `Null
          | Some json -> json );
      ])

(************************************************************************)

let print_shutdown () : Yojson.Safe.t = `Null

(************************************************************************)

let parse_cancelRequest (params : Yojson.Safe.t option) : CancelRequest.params =
  CancelRequest.{ id = Jget.val_exn params "id" |> parse_id }

let print_cancelRequest (p : CancelRequest.params) : Yojson.Safe.t =
  CancelRequest.(`Assoc [("id", print_id p.id)])

(************************************************************************)

let parse_setTraceNotification (params : Yojson.Safe.t option) :
    SetTraceNotification.params =
  match Jget.string_opt params "value" with
  | Some "verbose" -> SetTraceNotification.Verbose
  | _ -> SetTraceNotification.Off

let parse_setTrace = parse_setTraceNotification

let print_setTraceNotification (p : SetTraceNotification.params) : Yojson.Safe.t
    =
  let s =
    match p with
    | SetTraceNotification.Verbose -> "verbose"
    | SetTraceNotification.Off -> "off"
  in
  `Assoc [("value", `String s)]

let print_setTrace = print_setTraceNotification

(************************************************************************)
let print_rage (r : RageFB.result) : Yojson.Safe.t =
  RageFB.(
    let print_item (item : rageItem) : Yojson.Safe.t =
      `Assoc
        [
          ("data", `String item.data);
          ( "title",
            match item.title with
            | None -> `Null
            | Some s -> `String s );
        ]
    in
    `List (List.map r ~f:print_item))

(************************************************************************)

let parse_didOpen (params : Yojson.Safe.t option) : DidOpen.params =
  DidOpen.
    {
      textDocument =
        Jget.obj_exn params "textDocument" |> parse_textDocumentItem;
    }

let print_didOpen (params : DidOpen.params) : Yojson.Safe.t =
  DidOpen.(
    `Assoc [("textDocument", params.textDocument |> print_textDocumentItem)])

(************************************************************************)

let parse_didClose (params : Yojson.Safe.t option) : DidClose.params =
  DidClose.
    {
      textDocument =
        Jget.obj_exn params "textDocument" |> parse_textDocumentIdentifier;
    }

(************************************************************************)

let parse_didSave (params : Yojson.Safe.t option) : DidSave.params =
  DidSave.
    {
      textDocument =
        Jget.obj_exn params "textDocument" |> parse_textDocumentIdentifier;
      text = Jget.string_opt params "text";
    }

(************************************************************************)

let parse_didChange (params : Yojson.Safe.t option) : DidChange.params =
  DidChange.(
    let parse_textDocumentContentChangeEvent json =
      {
        range = Jget.obj_opt json "range" |> parse_range_opt;
        rangeLength = Jget.int_opt json "rangeLength";
        text = Jget.string_exn json "text";
      }
    in
    {
      textDocument =
        Jget.obj_exn params "textDocument"
        |> parse_versionedTextDocumentIdentifier;
      contentChanges =
        Jget.array_d params "contentChanges" ~default:[]
        |> List.map ~f:parse_textDocumentContentChangeEvent;
    })

(************************************************************************)

let parse_signatureHelp (params : Yojson.Safe.t option) : SignatureHelp.params =
  parse_textDocumentPositionParams params

let print_signatureHelp (r : SignatureHelp.result) : Yojson.Safe.t =
  SignatureHelp.(
    let print_parInfo parInfo =
      `Assoc
        (List.filter_opt
           [
             Some ("label", `String parInfo.parinfo_label);
             Option.map parInfo.parinfo_documentation ~f:(fun d ->
                 ("documentation", `String d));
           ])
    in
    let print_sigInfo sigInfo =
      `Assoc
        (List.filter_opt
           [
             Some ("label", `String sigInfo.siginfo_label);
             Option.map sigInfo.siginfo_documentation ~f:(fun d ->
                 ("documentation", `String d));
             Some
               ( "parameters",
                 `List (List.map ~f:print_parInfo sigInfo.parameters) );
           ])
    in
    match r with
    | None -> `Null
    | Some r ->
      `Assoc
        [
          ("signatures", `List (List.map ~f:print_sigInfo r.signatures));
          ("activeSignature", `Int r.activeSignature);
          ("activeParameter", `Int r.activeParameter);
        ])

(************************************************************************)

let parse_AutoClose (params : Yojson.Safe.t option) : AutoCloseJsx.params =
  parse_textDocumentPositionParams params

let print_AutoClose (r : AutoCloseJsx.result) : Yojson.Safe.t =
  match r with
  | None -> `Null
  | Some r -> `String r

(************************************************************************)

let parse_codeLensResolve (params : Yojson.Safe.t option) :
    CodeLensResolve.params =
  parse_codeLens params

let print_codeLensResolve (r : CodeLensResolve.result) : Yojson.Safe.t =
  print_codeLens r

(************************************************************************)

let parse_documentRename (params : Yojson.Safe.t option) : Rename.params =
  Rename.
    {
      textDocument =
        Jget.obj_exn params "textDocument" |> parse_textDocumentIdentifier;
      position = Jget.obj_exn params "position" |> parse_position;
      newName = Jget.string_exn params "newName";
    }

(************************************************************************)

let parse_documentCodeLens (params : Yojson.Safe.t option) :
    DocumentCodeLens.params =
  DocumentCodeLens.
    {
      textDocument =
        Jget.obj_exn params "textDocument" |> parse_textDocumentIdentifier;
    }

let print_documentCodeLens (r : DocumentCodeLens.result) : Yojson.Safe.t =
  `List (List.map r ~f:print_codeLens)

(************************************************************************)

let print_diagnostic (diagnostic : PublishDiagnostics.diagnostic) :
    Yojson.Safe.t =
  PublishDiagnostics.(
    let print_diagnosticSeverity s = `Int (diagnosticSeverity_to_enum s) in
    let print_diagnosticCode = function
      | IntCode i -> Some (`Int i)
      | StringCode s -> Some (`String s)
      | NoCode -> None
    in
    let print_related (related : relatedLocation) : Yojson.Safe.t =
      `Assoc
        [
          ("location", print_location related.relatedLocation);
          ("message", `String related.relatedMessage);
        ]
    in
    let fields =
      List.filter_opt
        [
          Some ("range", print_range diagnostic.range);
          Option.map diagnostic.severity ~f:(fun s ->
              ("severity", print_diagnosticSeverity s));
          (match print_diagnosticCode diagnostic.code with
          | Some v -> Some ("code", v)
          | None -> None);
          Option.map diagnostic.source ~f:(fun s -> ("source", `String s));
          Some ("message", `String diagnostic.message);
          Some
            ( "relatedInformation",
              `List (List.map diagnostic.relatedInformation ~f:print_related) );
          Some
            ( "relatedLocations",
              `List (List.map diagnostic.relatedLocations ~f:print_related) );
          Option.map diagnostic.data ~f:(fun d -> ("data", d));
        ]
    in
    `Assoc fields)

let print_diagnostic_list (ds : PublishDiagnostics.diagnostic list) :
    Yojson.Safe.t =
  `List (List.map ds ~f:print_diagnostic)

let print_diagnostics (r : PublishDiagnostics.params) : Yojson.Safe.t =
  PublishDiagnostics.(
    let fields =
      List.filter_opt
        [
          Some ("uri", `String (string_of_uri r.uri));
          Some ("diagnostics", print_diagnostic_list r.diagnostics);
          (if r.isStatusFB then
            Some ("isStatusFB", `Bool true)
          else
            None);
        ]
    in
    `Assoc fields)

let parse_diagnostic (j : Yojson.Safe.t option) : PublishDiagnostics.diagnostic
    =
  PublishDiagnostics.(
    let parse_code = function
      | None -> NoCode
      | Some (`String s) -> StringCode s
      | Some (`Int i) -> IntCode i
      | Some (`Intlit s) -> begin
        try IntCode (int_of_string s) with
        | Failure _ ->
          raise
            (Error.LspException
               {
                 Error.code = Error.ParseError;
                 message = "Diagnostic code expected to be an int: " ^ s;
                 data = None;
               })
      end
      | _ ->
        raise
          (Error.LspException
             {
               Error.code = Error.ParseError;
               message = "Diagnostic code expected to be an int or string";
               data = None;
             })
    in
    let parse_info j =
      {
        relatedLocation = Jget.obj_exn j "location" |> parse_location;
        relatedMessage = Jget.string_exn j "message";
      }
    in
    {
      range = Jget.obj_exn j "range" |> parse_range_exn;
      severity =
        Jget.int_opt j "severity"
        |> Option.map ~f:diagnosticSeverity_of_enum
        |> Option.join;
      code = Jget.val_opt j "code" |> parse_code;
      source = Jget.string_opt j "source";
      message = Jget.string_exn j "message";
      relatedInformation =
        Jget.array_d j "relatedInformation" ~default:[]
        |> List.map ~f:parse_info;
      relatedLocations =
        Jget.array_d j "relatedLocations" ~default:[] |> List.map ~f:parse_info;
      data = Jget.val_opt j "data";
    })

let parse_kind json : CodeActionKind.t option =
  CodeActionKind.(
    match json with
    | Some (`String s) -> Some (kind_of_string s)
    | _ -> None)

let parse_kinds jsons : CodeActionKind.t list =
  List.map ~f:parse_kind jsons |> List.filter_opt

let parse_codeActionRequest (j : Yojson.Safe.t option) :
    CodeActionRequest.params =
  let parse_context c : CodeActionRequest.codeActionContext =
    CodeActionRequest.
      {
        diagnostics =
          Jget.array_exn c "diagnostics" |> List.map ~f:parse_diagnostic;
        only = Jget.array_opt c "only" |> Option.map ~f:parse_kinds;
      }
  in
  CodeActionRequest.
    {
      textDocument =
        Jget.obj_exn j "textDocument" |> parse_textDocumentIdentifier;
      range = Jget.obj_exn j "range" |> parse_range_exn;
      context = Jget.obj_exn j "context" |> parse_context;
    }

let parse_codeActionResolveRequest (j : Yojson.Safe.t option) :
    CodeActionResolveRequest.params =
  let data =
    let all = Jget.obj_exn j "data" in
    let request = Jget.obj_opt all "originalRequest" in
    parse_codeActionRequest request
  in
  let title = Jget.string_exn j "title" in
  CodeActionResolveRequest.{ data; title }

(************************************************************************)

let print_codeAction
    (CodeAction.{ title; kind; diagnostics; action; isAI } : 'a CodeAction.t)
    ~(unresolved_to_code_action_request : 'a -> CodeActionRequest.params) :
    Yojson.Safe.t =
  CodeAction.(
    let (edit, command, params_opt) =
      match action with
      | EditOnly e -> (Some e, None, None)
      | CommandOnly c -> (None, Some c, None)
      | BothEditThenCommand (e, c) -> (Some e, Some c, None)
      | UnresolvedEdit e ->
        (None, None, Some (unresolved_to_code_action_request e))
    in
    let print_params CodeActionRequest.{ textDocument; range; context } =
      `Assoc
        [
          ( "textDocument",
            `Assoc
              [
                ( "uri",
                  `String
                    (string_of_uri textDocument.TextDocumentIdentifier.uri) );
              ] );
          ("range", print_range range);
          ( "context",
            `Assoc
              [
                ( "diagnostics",
                  print_diagnostic_list context.CodeActionRequest.diagnostics );
              ] );
        ]
    in
    let data_opt =
      let fields =
        List.filter_opt
        @@ [
             Option.map params_opt ~f:(fun params ->
                 ("originalRequest", print_params params));
             Option.map isAI ~f:(fun isAI -> ("isAI", `Bool isAI));
           ]
      in
      match fields with
      | [] -> None
      | _ -> Some (`Assoc fields)
    in
    let result_fields =
      List.filter_opt
        [
          Some ("title", `String title);
          Some ("kind", `String (CodeActionKind.string_of_kind kind));
          Some ("diagnostics", print_diagnostic_list diagnostics);
          Option.map edit ~f:(fun e -> ("edit", print_workspaceEdit e));
          Option.map command ~f:(fun c -> ("command", print_command c));
          Option.map data_opt ~f:(fun d -> ("data", d));
        ]
    in
    `Assoc result_fields)

let print_codeActionResult
    (c : CodeAction.result) (p : CodeActionRequest.params) : Yojson.Safe.t =
  CodeAction.(
    let print_command_or_action = function
      | Command c -> print_command c
      | Action c ->
        print_codeAction c ~unresolved_to_code_action_request:(Fn.const p)
    in
    `List (List.map c ~f:print_command_or_action))

let print_codeActionResolveResult (c : CodeActionResolve.result) : Yojson.Safe.t
    =
  let open CodeAction in
  let print_command_or_action = function
    | Command c -> print_command c
    | Action c ->
      let unresolved_to_code_action_request :
          CodeAction.resolved_marker -> CodeActionRequest.params = function
        | _ -> .
      in
      print_codeAction c ~unresolved_to_code_action_request
  in
  match c with
  | Ok command_or_action -> print_command_or_action command_or_action
  | Error err -> print_error err

(************************************************************************)

let print_logMessage (type_ : MessageType.t) (message : string) : Yojson.Safe.t
    =
  `Assoc
    [("type", `Int (MessageType.to_enum type_)); ("message", `String message)]

(************************************************************************)

let print_telemetryNotification
    (r : LogMessage.params) (extras : (string * Yojson.Safe.t) list) :
    Yojson.Safe.t =
  (* LSP allows "any" for the format of telemetry notifications. It's up to us! *)
  `Assoc
    (("type", `Int (MessageType.to_enum r.LogMessage.type_))
    :: ("message", `String r.LogMessage.message)
    :: extras)

(************************************************************************)

let print_showMessage (type_ : MessageType.t) (message : string) : Yojson.Safe.t
    =
  ShowMessage.(
    let r = { type_; message } in
    `Assoc
      [
        ("type", `Int (MessageType.to_enum r.type_));
        ("message", `String r.message);
      ])

(************************************************************************)

let print_showMessageRequest (r : ShowMessageRequest.showMessageRequestParams) :
    Yojson.Safe.t =
  let print_action (action : ShowMessageRequest.messageActionItem) :
      Yojson.Safe.t =
    `Assoc [("title", `String action.ShowMessageRequest.title)]
  in
  let fields =
    List.filter_opt
      [
        Some ("type", `Int (MessageType.to_enum r.ShowMessageRequest.type_));
        Some ("message", `String r.ShowMessageRequest.message);
        Some
          ( "actions",
            `List (List.map r.ShowMessageRequest.actions ~f:print_action) );
      ]
  in
  `Assoc fields

let parse_result_showMessageRequest (result : Yojson.Safe.t option) :
    ShowMessageRequest.result =
  ShowMessageRequest.(
    let title = Jget.string_opt result "title" in
    Option.map title ~f:(fun title -> { title }))

(************************************************************************)

let print_showStatus (r : ShowStatusFB.showStatusParams) : Yojson.Safe.t =
  let rr = r.ShowStatusFB.request in
  let fields =
    List.filter_opt
      [
        Some ("type", `Int (MessageType.to_enum rr.ShowStatusFB.type_));
        Some ("message", `String rr.ShowStatusFB.message);
        Option.map r.ShowStatusFB.shortMessage ~f:(fun s ->
            ("shortMessage", `String s));
        Option.map r.ShowStatusFB.telemetry ~f:(fun t -> ("telemetry", t));
        Option.map r.ShowStatusFB.progress ~f:(fun progress ->
            let progress_fields =
              List.filter_opt
                [
                  Some ("numerator", `Int progress);
                  Option.map r.ShowStatusFB.total ~f:(fun t ->
                      ("denominator", `Int t));
                ]
            in
            ("progress", `Assoc progress_fields));
      ]
  in
  `Assoc fields

(************************************************************************)

let print_connectionStatus (p : ConnectionStatusFB.params) : Yojson.Safe.t =
  ConnectionStatusFB.(`Assoc [("isConnected", `Bool p.isConnected)])

(************************************************************************)

let parse_hover (params : Yojson.Safe.t option) : Hover.params =
  parse_textDocumentPositionParams params

let print_hover (r : Hover.result) : Yojson.Safe.t =
  Hover.(
    match r with
    | None -> `Null
    | Some r ->
      let fields =
        List.filter_opt
          [
            Some
              ("contents", `List (List.map r.Hover.contents ~f:print_markedItem));
            Option.map r.range ~f:(fun rng -> ("range", print_range rng));
          ]
      in
      `Assoc fields)

(************************************************************************)

let parse_completionItem (params : Yojson.Safe.t option) :
    CompletionItemResolve.params =
  Completion.(
    let textEdit = Jget.obj_opt params "textEdit" |> parse_textEdit in
    let additionalTextEdits =
      Jget.array_d params "additionalTextEdits" ~default:[]
      |> List.filter_map ~f:(fun e -> parse_textEdit e)
    in
    let command =
      match Jget.obj_opt params "command" with
      | None -> None
      | c -> Some (parse_command c)
    in
    let documentation =
      match Jget.obj_opt params "documentation" with
      | None -> None
      | Some json -> Some (UnparsedDocumentation json)
    in
    {
      label = Jget.string_exn params "label";
      kind =
        Option.bind (Jget.int_opt params "kind") ~f:completionItemKind_of_enum;
      detail = Jget.string_opt params "detail";
      documentation;
      sortText = Jget.string_opt params "sortText";
      filterText = Jget.string_opt params "filterText";
      insertText = Jget.string_opt params "insertText";
      insertTextFormat =
        Option.bind
          (Jget.int_opt params "insertTextFormat")
          ~f:insertTextFormat_of_enum;
      textEdit;
      additionalTextEdits;
      command;
      data = Jget.obj_opt params "data";
    })

let string_of_markedString (acc : string) (marked : markedString) : string =
  match marked with
  | MarkedCode (lang, code) -> acc ^ "```" ^ lang ^ "\n" ^ code ^ "\n" ^ "```\n"
  | MarkedString str -> acc ^ str ^ "\n"

let print_completionItem (item : Completion.completionItem) : Yojson.Safe.t =
  Completion.(
    let fields =
      List.filter_opt
        [
          Some ("label", `String item.label);
          Option.map item.kind ~f:(fun x ->
              ("kind", `Int (completionItemKind_to_enum x)));
          Option.map item.detail ~f:(fun d -> ("detail", `String d));
          (match item.documentation with
          | None -> None
          | Some (UnparsedDocumentation json) -> Some ("documentation", json)
          | Some (MarkedStringsDocumentation doc) ->
            Some
              ( "documentation",
                `Assoc
                  [
                    ("kind", `String "markdown");
                    ( "value",
                      `String
                        (String.strip
                           (List.fold doc ~init:"" ~f:string_of_markedString))
                    );
                  ] ));
          Option.map item.sortText ~f:(fun s -> ("sortText", `String s));
          Option.map item.filterText ~f:(fun s -> ("filterText", `String s));
          Option.map item.insertText ~f:(fun s -> ("insertText", `String s));
          Option.map item.insertTextFormat ~f:(fun x ->
              ("insertTextFormat", `Int (insertTextFormat_to_enum x)));
          Option.map item.textEdit ~f:(fun te ->
              ("textEdit", print_textEdit te));
          (match item.additionalTextEdits with
          | [] -> None
          | text_edits ->
            Some ("additionalTextEdits", print_textEdits text_edits));
          Option.map item.command ~f:(fun c -> ("command", print_command c));
          Option.map item.data ~f:(fun d -> ("data", d));
        ]
    in
    `Assoc fields)

let parse_completion (params : Yojson.Safe.t option) : Completion.params =
  Lsp.Completion.(
    let context = Jget.obj_opt params "context" in
    {
      loc = parse_textDocumentPositionParams params;
      context =
        (match context with
        | Some _ ->
          let tk = Jget.int_exn context "triggerKind" in
          Some
            {
              triggerKind =
                Option.value_exn
                  ~message:(Printf.sprintf "Unsupported trigger kind: %d" tk)
                  (Lsp.Completion.completionTriggerKind_of_enum tk);
              triggerCharacter = Jget.string_opt context "triggerCharacter";
            }
        | None -> None);
    })

let print_completion (r : Completion.result) : Yojson.Safe.t =
  Completion.(
    `Assoc
      [
        ("isIncomplete", `Bool r.isIncomplete);
        ("items", `List (List.map r.items ~f:print_completionItem));
      ])

(************************************************************************)

let parse_workspaceSymbol (params : Yojson.Safe.t option) :
    WorkspaceSymbol.params =
  WorkspaceSymbol.{ query = Jget.string_exn params "query" }

let print_workspaceSymbol (r : WorkspaceSymbol.result) : Yojson.Safe.t =
  `List (List.map r ~f:print_symbolInformation)

(************************************************************************)

let parse_documentSymbol (params : Yojson.Safe.t option) : DocumentSymbol.params
    =
  DocumentSymbol.
    {
      textDocument =
        Jget.obj_exn params "textDocument" |> parse_textDocumentIdentifier;
    }

let print_documentSymbol (r : DocumentSymbol.result) : Yojson.Safe.t =
  `List (List.map r ~f:print_symbolInformation)

(************************************************************************)

let parse_findReferences (params : Yojson.Safe.t option) : FindReferences.params
    =
  let partialResultToken =
    Jget.string_opt params "partialResultToken"
    |> Option.map ~f:(fun t -> PartialResultToken t)
  in
  let context = Jget.obj_opt params "context" in
  {
    FindReferences.loc = parse_textDocumentPositionParams params;
    partialResultToken;
    context =
      {
        FindReferences.includeDeclaration =
          Jget.bool_d context "includeDeclaration" ~default:true;
        includeIndirectReferences =
          Jget.bool_d context "includeIndirectReferences" ~default:false;
      };
  }

let print_findReferencesPartialResult (Lsp.PartialResultToken token) refs =
  `Assoc [("token", `String token); ("value", print_locations refs)]

(************************************************************************)

let parse_callItem (params : Yojson.Safe.t option) : CallHierarchyItem.t =
  let rangeObj = Jget.obj_exn params "range" in
  let selectionRangeObj = Jget.obj_exn params "selectionRange" in
  let open CallHierarchyItem in
  {
    name = Jget.string_exn params "name";
    kind =
      (match
         Jget.int_exn params "kind" |> SymbolInformation.symbolKind_of_enum
       with
      | None -> raise (Jget.Parse "invalid symbol kind")
      | Some s -> s);
    detail = Jget.string_opt params "detail";
    uri = Jget.string_exn params "uri" |> uri_of_string;
    range = parse_range_exn rangeObj;
    selectionRange = parse_range_exn selectionRangeObj;
  }

(************************************************************************)
let print_callItem (item : CallHierarchyItem.t) : Yojson.Safe.t =
  let open CallHierarchyItem in
  let fields =
    List.filter_opt
      [
        Some ("name", `String item.name);
        Some ("kind", `Int (SymbolInformation.symbolKind_to_enum item.kind));
        Option.map item.detail ~f:(fun d -> ("detail", `String d));
        Some ("uri", `String (string_of_uri item.uri));
        Some ("range", print_range item.range);
        Some ("selectionRange", print_range item.selectionRange);
      ]
  in
  `Assoc fields

(************************************************************************)

let parse_callHierarchyCalls (params : Yojson.Safe.t option) :
    CallHierarchyCallsRequestParam.t =
  let json_item = Jget.obj_opt params "item" in
  let parsed_item = parse_callItem json_item in
  let open CallHierarchyCallsRequestParam in
  { item = parsed_item }

(************************************************************************)

let print_PrepareCallHierarchyResult (r : PrepareCallHierarchy.result) :
    Yojson.Safe.t =
  match r with
  | None -> `Null
  | Some list -> `List (List.map list ~f:print_callItem)

(************************************************************************)
let print_CallHierarchyIncomingCallsResult
    (r : CallHierarchyIncomingCalls.result) : Yojson.Safe.t =
  let open CallHierarchyIncomingCalls in
  let print_CallHierarchyIncomingCall
      (call : CallHierarchyIncomingCalls.callHierarchyIncomingCall) :
      Yojson.Safe.t =
    `Assoc
      [
        ("from", print_callItem call.from);
        ("fromRanges", `List (List.map call.fromRanges ~f:print_range));
      ]
  in
  match r with
  | None -> `Null
  | Some list -> `List (List.map list ~f:print_CallHierarchyIncomingCall)

(************************************************************************)

let print_CallHierarchyOutgoingCallsResult
    (r : CallHierarchyOutgoingCalls.result) : Yojson.Safe.t =
  let open CallHierarchyOutgoingCalls in
  let print_CallHierarchyOutgoingCall
      (call : CallHierarchyOutgoingCalls.callHierarchyOutgoingCall) :
      Yojson.Safe.t =
    `Assoc
      [
        ("to", print_callItem call.call_to);
        ("fromRanges", `List (List.map call.fromRanges ~f:print_range));
      ]
  in
  match r with
  | None -> `Null
  | Some list -> `List (List.map list ~f:print_CallHierarchyOutgoingCall)

(************************************************************************)

let parse_documentHighlight (params : Yojson.Safe.t option) :
    DocumentHighlight.params =
  parse_textDocumentPositionParams params

let print_documentHighlight (r : DocumentHighlight.result) : Yojson.Safe.t =
  DocumentHighlight.(
    let print_highlight highlight =
      let fields =
        List.filter_opt
          [
            Some ("range", print_range highlight.range);
            Option.map highlight.kind ~f:(fun k ->
                ("kind", `Int (documentHighlightKind_to_enum k)));
          ]
      in
      `Assoc fields
    in
    `List (List.map r ~f:print_highlight))

(************************************************************************)

let parse_documentFormatting (params : Yojson.Safe.t option) :
    DocumentFormatting.params =
  {
    DocumentFormatting.textDocument =
      Jget.obj_exn params "textDocument" |> parse_textDocumentIdentifier;
    options = Jget.obj_opt params "options" |> parse_formattingOptions;
  }

let print_documentFormatting (r : DocumentFormatting.result) : Yojson.Safe.t =
  print_textEdits r

let parse_documentRangeFormatting (params : Yojson.Safe.t option) :
    DocumentRangeFormatting.params =
  {
    DocumentRangeFormatting.textDocument =
      Jget.obj_exn params "textDocument" |> parse_textDocumentIdentifier;
    range = Jget.obj_exn params "range" |> parse_range_exn;
    options = Jget.obj_opt params "options" |> parse_formattingOptions;
  }

let print_documentRangeFormatting (r : DocumentRangeFormatting.result) :
    Yojson.Safe.t =
  print_textEdits r

let parse_documentOnTypeFormatting (params : Yojson.Safe.t option) :
    DocumentOnTypeFormatting.params =
  {
    DocumentOnTypeFormatting.textDocument =
      Jget.obj_exn params "textDocument" |> parse_textDocumentIdentifier;
    position = Jget.obj_exn params "position" |> parse_position;
    ch = Jget.string_exn params "ch";
    options = Jget.obj_opt params "options" |> parse_formattingOptions;
  }

let print_documentOnTypeFormatting (r : DocumentOnTypeFormatting.result) :
    Yojson.Safe.t =
  print_textEdits r

let parse_willSaveWaitUntil (params : Yojson.Safe.t option) :
    WillSaveWaitUntil.params =
  let open WillSaveWaitUntil in
  {
    textDocument =
      Jget.obj_exn params "textDocument" |> parse_textDocumentIdentifier;
    reason =
      Jget.int_exn params "reason"
      |> textDocumentSaveReason_of_enum
      |> Option.value ~default:Manual;
  }

(************************************************************************)

let parse_initialize (params : Yojson.Safe.t option) : Initialize.params =
  Initialize.(
    let rec parse_initialize json =
      {
        processId = Jget.int_opt json "processId";
        rootPath = Jget.string_opt json "rootPath";
        rootUri = Option.map ~f:uri_of_string (Jget.string_opt json "rootUri");
        initializationOptions =
          Jget.obj_opt json "initializationOptions"
          |> parse_initializationOptions;
        client_capabilities =
          Jget.obj_opt json "capabilities" |> parse_capabilities;
        trace = Jget.string_opt json "trace" |> parse_trace;
      }
    and parse_trace (s : string option) : trace =
      match s with
      | Some "messages" -> Messages
      | Some "verbose" -> Verbose
      | _ -> Off
    and parse_initializationOptions json =
      {
        namingTableSavedStatePath =
          Jget.string_opt json "namingTableSavedStatePath";
        namingTableSavedStateTestDelay =
          Jget.float_d json "namingTableSavedStateTestDelay" ~default:0.0;
        delayUntilDoneInit =
          Jget.bool_d json "delayUntilDoneInit" ~default:false;
        skipLspServerOnTypeFormatting =
          Jget.bool_d json "skipLspServerOnTypeFormatting" ~default:false;
      }
    and parse_capabilities json =
      {
        workspace = Jget.obj_opt json "workspace" |> parse_workspace;
        textDocument = Jget.obj_opt json "textDocument" |> parse_textDocument;
        window = Jget.obj_opt json "window" |> parse_window;
        telemetry = Jget.obj_opt json "telemetry" |> parse_telemetry;
        client_experimental =
          Jget.obj_opt json "experimental" |> parse_client_experimental;
      }
    and parse_workspace json =
      {
        applyEdit = Jget.bool_d json "applyEdit" ~default:false;
        workspaceEdit = Jget.obj_opt json "workspaceEdit" |> parse_workspaceEdit;
        didChangeWatchedFiles =
          Jget.obj_opt json "didChangeWatchedFiles" |> parse_dynamicRegistration;
      }
    and parse_dynamicRegistration json =
      {
        dynamicRegistration =
          Jget.bool_d json "dynamicRegistration" ~default:false;
      }
    and parse_workspaceEdit json =
      { documentChanges = Jget.bool_d json "documentChanges" ~default:false }
    and parse_textDocument json =
      {
        synchronization =
          Jget.obj_opt json "synchronization" |> parse_synchronization;
        completion = Jget.obj_opt json "completion" |> parse_completion;
        codeAction = Jget.obj_opt json "codeAction" |> parse_codeAction;
        definition = Jget.obj_opt json "definition" |> parse_definition;
        typeDefinition =
          Jget.obj_opt json "typeDefinition" |> parse_typeDefinition;
        implementation =
          Jget.obj_opt json "implementation" |> parse_implementation;
        declaration = Jget.obj_opt json "declaration" |> parse_declaration;
      }
    and parse_synchronization json =
      {
        can_willSave = Jget.bool_d json "willSave" ~default:false;
        can_willSaveWaitUntil =
          Jget.bool_d json "willSaveWaitUntil" ~default:false;
        can_didSave = Jget.bool_d json "didSave" ~default:false;
      }
    and parse_completion json =
      {
        completionItem =
          Jget.obj_opt json "completionItem" |> parse_completionItem_;
      }
    and parse_completionItem_ json =
      { snippetSupport = Jget.bool_d json "snippetSupport" ~default:false }
    and parse_codeAction json =
      {
        codeAction_dynamicRegistration =
          Jget.bool_d json "dynamicRegistration" ~default:false;
        codeActionLiteralSupport =
          Jget.obj_opt json "codeActionLiteralSupport"
          |> parse_codeActionLiteralSupport;
      }
    and parse_codeActionLiteralSupport json =
      Option.(
        Jget.array_opt json "valueSet" >>= fun ls ->
        Some { codeAction_valueSet = parse_kinds ls })
    and parse_definition json =
      { definitionLinkSupport = Jget.bool_d json "linkSupport" ~default:false }
    and parse_typeDefinition json =
      {
        typeDefinitionLinkSupport =
          Jget.bool_d json "linkSupport" ~default:false;
      }
    and parse_implementation json =
      {
        implementationLinkSupport =
          Jget.bool_d json "linkSupport" ~default:false;
      }
    and parse_declaration json =
      { declarationLinkSupport = Jget.bool_d json "linkSupport" ~default:false }
    and parse_window json =
      { status = Jget.obj_opt json "status" |> Option.is_some }
    and parse_telemetry json =
      {
        connectionStatus =
          Jget.obj_opt json "connectionStatus" |> Option.is_some;
      }
    and parse_client_experimental json =
      ClientExperimentalCapabilities.
        {
          snippetTextEdit = Jget.bool_d json "snippetTextEdit" ~default:false;
          autoCloseJsx = Jget.bool_d json "autoCloseJsx" ~default:false;
        }
    in
    parse_initialize params)

let print_initializeError (r : Initialize.errorData) : Yojson.Safe.t =
  Initialize.(`Assoc [("retry", `Bool r.retry)])

let print_initialize (r : Initialize.result) : Yojson.Safe.t =
  Initialize.(
    let cap = r.server_capabilities in
    let sync = cap.textDocumentSync in
    `Assoc
      [
        ( "capabilities",
          let fields =
            List.filter_opt
              [
                Some
                  ( "textDocumentSync",
                    let sync_fields =
                      List.filter_opt
                        [
                          Some ("openClose", `Bool sync.want_openClose);
                          Some
                            ( "change",
                              `Int
                                (textDocumentSyncKind_to_enum sync.want_change)
                            );
                          Some ("willSave", `Bool sync.want_willSave);
                          Some
                            ( "willSaveWaitUntil",
                              `Bool sync.want_willSaveWaitUntil );
                          Option.map sync.want_didSave ~f:(fun save ->
                              ( "save",
                                `Assoc [("includeText", `Bool save.includeText)]
                              ));
                        ]
                    in
                    `Assoc sync_fields );
                Some ("hoverProvider", `Bool cap.hoverProvider);
                Option.map cap.completionProvider ~f:(fun comp ->
                    ( "completionProvider",
                      `Assoc
                        [
                          ( "resolveProvider",
                            `Bool comp.CompletionOptions.resolveProvider );
                          ( "triggerCharacters",
                            Jprint.string_array
                              comp
                                .CompletionOptions.completion_triggerCharacters
                          );
                        ] ));
                Option.map cap.signatureHelpProvider ~f:(fun shp ->
                    ( "signatureHelpProvider",
                      `Assoc
                        [
                          ( "triggerCharacters",
                            Jprint.string_array shp.sighelp_triggerCharacters );
                        ] ));
                Some ("definitionProvider", `Bool cap.definitionProvider);
                Some ("typeDefinitionProvider", `Bool cap.typeDefinitionProvider);
                Some ("referencesProvider", `Bool cap.referencesProvider);
                Some
                  ( "documentHighlightProvider",
                    `Bool cap.documentHighlightProvider );
                Some ("documentSymbolProvider", `Bool cap.documentSymbolProvider);
                Some
                  ("workspaceSymbolProvider", `Bool cap.workspaceSymbolProvider);
                Option.map cap.codeActionProvider ~f:(fun provider ->
                    ( "codeActionProvider",
                      `Assoc
                        [
                          ( "resolveProvider",
                            `Bool provider.CodeActionOptions.resolveProvider );
                        ] ));
                Option.map cap.codeLensProvider ~f:(fun codelens ->
                    ( "codeLensProvider",
                      `Assoc
                        [
                          ( "resolveProvider",
                            `Bool codelens.codelens_resolveProvider );
                        ] ));
                Some
                  ( "documentFormattingProvider",
                    `Bool cap.documentFormattingProvider );
                Some
                  ( "documentRangeFormattingProvider",
                    `Bool cap.documentRangeFormattingProvider );
                Option.map cap.documentOnTypeFormattingProvider ~f:(fun o ->
                    ( "documentOnTypeFormattingProvider",
                      `Assoc
                        [
                          ( "firstTriggerCharacter",
                            `String o.firstTriggerCharacter );
                          ( "moreTriggerCharacter",
                            Jprint.string_array o.moreTriggerCharacter );
                        ] ));
                Some ("renameProvider", `Bool cap.renameProvider);
                Option.map cap.documentLinkProvider ~f:(fun dlp ->
                    ( "documentLinkProvider",
                      `Assoc
                        [("resolveProvider", `Bool dlp.doclink_resolveProvider)]
                    ));
                Option.map cap.executeCommandProvider ~f:(fun p ->
                    ( "executeCommandProvider",
                      `Assoc [("commands", Jprint.string_array p.commands)] ));
                Some ("implementationProvider", `Bool cap.implementationProvider);
                Some ("rageProvider", `Bool cap.rageProviderFB);
                Option.map
                  cap.server_experimental
                  ~f:(fun experimental_capabilities ->
                    ( "experimental",
                      `Assoc
                        [
                          ( "snippetTextEdit",
                            `Bool
                              experimental_capabilities
                                .ServerExperimentalCapabilities.snippetTextEdit
                          );
                          ( "autoCloseJsx",
                            `Bool
                              experimental_capabilities
                                .ServerExperimentalCapabilities.autoCloseJsx );
                        ] ));
              ]
          in
          `Assoc fields );
      ])

(************************************************************************)

let print_registrationOptions (registerOptions : Lsp.lsp_registration_options) :
    Yojson.Safe.t =
  match registerOptions with
  | Lsp.DidChangeWatchedFilesRegistrationOptions registerOptions ->
    Lsp.DidChangeWatchedFiles.(
      `Assoc
        [
          ( "watchers",
            `List
              (List.map registerOptions.watchers ~f:(fun watcher ->
                   `Assoc
                     [
                       ("globPattern", `String watcher.globPattern);
                       ("kind", `Int 7);
                       (* all events: create, change, and delete *)
                     ])) );
        ])

let print_registerCapability (params : Lsp.RegisterCapability.params) :
    Yojson.Safe.t =
  Lsp.RegisterCapability.(
    `Assoc
      [
        ( "registrations",
          `List
            (List.map params.registrations ~f:(fun registration ->
                 `Assoc
                   [
                     ("id", `String registration.id);
                     ("method", `String registration.method_);
                     ( "registerOptions",
                       print_registrationOptions registration.registerOptions );
                   ])) );
      ])

let parse_didChangeWatchedFiles (json : Yojson.Safe.t option) :
    DidChangeWatchedFiles.params =
  let changes =
    Jget.array_exn json "changes"
    |> List.map ~f:(fun change ->
           let uri = Jget.string_exn change "uri" |> uri_of_string in
           let type_ = Jget.int_exn change "type" in
           let type_ =
             match DidChangeWatchedFiles.fileChangeType_of_enum type_ with
             | Some type_ -> type_
             | None ->
               failwith (Printf.sprintf "Invalid file change type %d" type_)
           in
           { DidChangeWatchedFiles.uri; type_ })
  in
  { DidChangeWatchedFiles.changes }

(************************************************************************)
(* universal parser+printer                                             *)
(************************************************************************)

let get_uri_opt (m : lsp_message) : Lsp.DocumentUri.t option =
  let open TextDocumentIdentifier in
  match m with
  | RequestMessage (_, DocumentCodeLensRequest p) ->
    Some p.DocumentCodeLens.textDocument.uri
  | RequestMessage (_, HoverRequest p) ->
    Some p.TextDocumentPositionParams.textDocument.uri
  | RequestMessage (_, DefinitionRequest p) ->
    Some p.TextDocumentPositionParams.textDocument.uri
  | RequestMessage (_, TypeDefinitionRequest p) ->
    Some p.TextDocumentPositionParams.textDocument.uri
  | RequestMessage (_, CodeActionRequest p) ->
    Some p.CodeActionRequest.textDocument.uri
  | RequestMessage (_, CodeActionResolveRequest p) ->
    Some p.CodeActionResolveRequest.data.CodeActionRequest.textDocument.uri
  | RequestMessage (_, CompletionRequest p) ->
    Some p.Completion.loc.TextDocumentPositionParams.textDocument.uri
  | RequestMessage (_, DocumentSymbolRequest p) ->
    Some p.DocumentSymbol.textDocument.uri
  | RequestMessage (_, FindReferencesRequest p) ->
    Some p.FindReferences.loc.TextDocumentPositionParams.textDocument.uri
  | RequestMessage (_, PrepareCallHierarchyRequest p) ->
    Some p.TextDocumentPositionParams.textDocument.uri
  (*Implement for CallHierarchy*)
  | RequestMessage (_, CallHierarchyIncomingCallsRequest p) ->
    Some p.CallHierarchyCallsRequestParam.item.CallHierarchyItem.uri
  | RequestMessage (_, CallHierarchyOutgoingCallsRequest p) ->
    Some p.CallHierarchyCallsRequestParam.item.CallHierarchyItem.uri
  | RequestMessage (_, ImplementationRequest p) ->
    Some p.TextDocumentPositionParams.textDocument.uri
  | RequestMessage (_, DocumentHighlightRequest p) ->
    Some p.TextDocumentPositionParams.textDocument.uri
  | RequestMessage (_, DocumentFormattingRequest p) ->
    Some p.DocumentFormatting.textDocument.uri
  | RequestMessage (_, DocumentRangeFormattingRequest p) ->
    Some p.DocumentRangeFormatting.textDocument.uri
  | RequestMessage (_, DocumentOnTypeFormattingRequest p) ->
    Some p.DocumentOnTypeFormatting.textDocument.uri
  | RequestMessage (_, RenameRequest p) -> Some p.Rename.textDocument.uri
  | RequestMessage (_, SignatureHelpRequest p) ->
    Some p.TextDocumentPositionParams.textDocument.uri
  | RequestMessage (_, AutoCloseRequest p) ->
    Some p.TextDocumentPositionParams.textDocument.uri
  | NotificationMessage (PublishDiagnosticsNotification p) ->
    Some p.PublishDiagnostics.uri
  | NotificationMessage (DidOpenNotification p) ->
    Some p.DidOpen.textDocument.TextDocumentItem.uri
  | NotificationMessage (DidCloseNotification p) ->
    Some p.DidClose.textDocument.uri
  | NotificationMessage (DidSaveNotification p) ->
    Some p.DidSave.textDocument.uri
  | NotificationMessage (DidChangeNotification p) ->
    Some p.DidChange.textDocument.VersionedTextDocumentIdentifier.uri
  | NotificationMessage (DidChangeWatchedFilesNotification p) -> begin
    match p.DidChangeWatchedFiles.changes with
    | [] -> None
    | { DidChangeWatchedFiles.uri; _ } :: _ -> Some uri
  end
  | RequestMessage
      ( _,
        ( HackTestStartServerRequestFB | HackTestStopServerRequestFB
        | HackTestShutdownServerlessRequestFB | UnknownRequest _
        | InitializeRequest _ | RegisterCapabilityRequest _ | ShutdownRequest
        | CodeLensResolveRequest _ | CompletionItemResolveRequest _
        | WorkspaceSymbolRequest _ | ShowMessageRequestRequest _
        | ShowStatusRequestFB _ | RageRequestFB | WillSaveWaitUntilRequest _
        | TopLevelDefNameAtPosRequest _ ) )
  | NotificationMessage
      ( ExitNotification | CancelRequestNotification _
      | LogMessageNotification _ | TelemetryNotification _
      | ShowMessageNotification _ | ConnectionStatusNotificationFB _
      | InitializedNotification | FindReferencesPartialResultNotification _
      | SetTraceNotification _ | SetTrace _ | LogTraceNotification
      | UnknownNotification _ )
  | ResponseMessage _ ->
    None

let request_name_to_string (request : lsp_request) : string =
  match request with
  | ShowMessageRequestRequest _ -> "window/showMessageRequest"
  | ShowStatusRequestFB _ -> "window/showStatus"
  | InitializeRequest _ -> "initialize"
  | RegisterCapabilityRequest _ -> "client/registerCapability"
  | ShutdownRequest -> "shutdown"
  | CodeLensResolveRequest _ -> "codeLens/resolve"
  | HoverRequest _ -> "textDocument/hover"
  | CodeActionRequest _ -> "textDocument/codeAction"
  | CodeActionResolveRequest _ -> "codeAction/resolve"
  | CompletionRequest _ -> "textDocument/completion"
  | CompletionItemResolveRequest _ -> "completionItem/resolve"
  | DefinitionRequest _ -> "textDocument/definition"
  | TypeDefinitionRequest _ -> "textDocument/typeDefinition"
  | ImplementationRequest _ -> "textDocument/implementation"
  | WorkspaceSymbolRequest _ -> "workspace/symbol"
  | DocumentSymbolRequest _ -> "textDocument/documentSymbol"
  | FindReferencesRequest _ -> "textDocument/references"
  | PrepareCallHierarchyRequest _ -> "textDocument/prepareCallHierarchy"
  | CallHierarchyIncomingCallsRequest _ -> "callHierarchy/incomingCalls"
  | CallHierarchyOutgoingCallsRequest _ -> "callHierarchy/outgoingCalls"
  | DocumentHighlightRequest _ -> "textDocument/documentHighlight"
  | DocumentFormattingRequest _ -> "textDocument/formatting"
  | DocumentRangeFormattingRequest _ -> "textDocument/rangeFormatting"
  | DocumentOnTypeFormattingRequest _ -> "textDocument/onTypeFormatting"
  | RageRequestFB -> "telemetry/rage"
  | RenameRequest _ -> "textDocument/rename"
  | DocumentCodeLensRequest _ -> "textDocument/codeLens"
  | SignatureHelpRequest _ -> "textDocument/signatureHelp"
  | AutoCloseRequest _ -> "flow/autoCloseJsx"
  | HackTestStartServerRequestFB -> "$test/startHhServer"
  | HackTestStopServerRequestFB -> "$test/stopHhServer"
  | HackTestShutdownServerlessRequestFB -> "$test/shutdownServerlessIde"
  | WillSaveWaitUntilRequest _ -> "textDocument/willSaveWaitUntil"
  | TopLevelDefNameAtPosRequest _ -> "custom/topLevelDefNameAtPos"
  | UnknownRequest (method_, _params) -> method_

let result_name_to_string (result : lsp_result) : string =
  match result with
  | ShowMessageRequestResult _ -> "window/showMessageRequest"
  | ShowStatusResultFB _ -> "window/showStatus"
  | InitializeResult _ -> "initialize"
  | ShutdownResult -> "shutdown"
  | CodeLensResolveResult _ -> "codeLens/resolve"
  | HoverResult _ -> "textDocument/hover"
  | CodeActionResult _ -> "textDocument/codeAction"
  | CodeActionResolveResult _ -> "codeAction/resolve"
  | CompletionResult _ -> "textDocument/completion"
  | CompletionItemResolveResult _ -> "completionItem/resolve"
  | DefinitionResult _ -> "textDocument/definition"
  | TypeDefinitionResult _ -> "textDocument/typeDefinition"
  | ImplementationResult _ -> "textDocument/implementation"
  | WorkspaceSymbolResult _ -> "workspace/symbol"
  | DocumentSymbolResult _ -> "textDocument/documentSymbol"
  | FindReferencesResult _ -> "textDocument/references"
  | PrepareCallHierarchyResult _ -> "textDocument/prepareCallHierarchy"
  | CallHierarchyIncomingCallsResult _ -> "callHierarchy/incomingCalls"
  | CallHierarchyOutgoingCallsResult _ -> "callHierarchy/outgoingCalls"
  | DocumentHighlightResult _ -> "textDocument/documentHighlight"
  | DocumentFormattingResult _ -> "textDocument/formatting"
  | DocumentRangeFormattingResult _ -> "textDocument/rangeFormatting"
  | DocumentOnTypeFormattingResult _ -> "textDocument/onTypeFormatting"
  | RageResultFB _ -> "telemetry/rage"
  | RenameResult _ -> "textDocument/rename"
  | DocumentCodeLensResult _ -> "textDocument/codeLens"
  | SignatureHelpResult _ -> "textDocument/signatureHelp"
  | AutoCloseResult _ -> "flow/autoCloseJsx"
  | HackTestStartServerResultFB -> "$test/startHhServer"
  | HackTestStopServerResultFB -> "$test/stopHhServer"
  | HackTestShutdownServerlessResultFB -> "$test/shutdownServerlessIde"
  | RegisterCapabilityRequestResult -> "client/registerCapability"
  | WillSaveWaitUntilResult _ -> "textDocument/willSaveWaitUntil"
  | TopLevelDefNameAtPosResult _ -> "custom/topLevelDefNameAtPos"
  | ErrorResult e -> "ERROR/" ^ e.Error.message

let notification_name_to_string (notification : lsp_notification) : string =
  match notification with
  | ExitNotification -> "exit"
  | CancelRequestNotification _ -> "$/cancelRequest"
  | PublishDiagnosticsNotification _ -> "textDocument/publishDiagnostics"
  | DidOpenNotification _ -> "textDocument/didOpen"
  | DidCloseNotification _ -> "textDocument/didClose"
  | DidSaveNotification _ -> "textDocument/didSave"
  | DidChangeNotification _ -> "textDocument/didChange"
  | DidChangeWatchedFilesNotification _ -> "workspace/didChangeWatchedFiles"
  | TelemetryNotification _ -> "telemetry/event"
  | LogMessageNotification _ -> "window/logMessage"
  | ShowMessageNotification _ -> "window/showMessage"
  | ConnectionStatusNotificationFB _ -> "telemetry/connectionStatus"
  | InitializedNotification -> "initialized"
  | FindReferencesPartialResultNotification _ -> "$/progress"
  | SetTraceNotification _ -> "$/setTraceNotification"
  | SetTrace _ -> "$/setTrace"
  | LogTraceNotification -> "$/logTraceNotification"
  | UnknownNotification (method_, _params) -> method_

let message_name_to_string (message : lsp_message) : string =
  match message with
  | RequestMessage (_, r) -> request_name_to_string r
  | NotificationMessage n -> notification_name_to_string n
  | ResponseMessage (_, r) -> result_name_to_string r

let denorm_message_to_string (message : lsp_message) : string =
  let uri =
    match get_uri_opt message with
    | Some (DocumentUri.Uri uri) -> "(" ^ uri ^ ")"
    | None -> ""
  in
  match message with
  | RequestMessage (id, r) ->
    Printf.sprintf
      "request %s %s%s"
      (id_to_string id)
      (request_name_to_string r)
      uri
  | NotificationMessage n ->
    Printf.sprintf "notification %s%s" (notification_name_to_string n) uri
  | ResponseMessage (id, ErrorResult e) ->
    Printf.sprintf "error %s %s %s" (id_to_string id) e.Error.message uri
  | ResponseMessage (id, r) ->
    Printf.sprintf
      "result %s %s%s"
      (id_to_string id)
      (result_name_to_string r)
      uri

let parse_lsp_request (method_ : string) (params : Yojson.Safe.t option) :
    lsp_request =
  match method_ with
  | "initialize" -> InitializeRequest (parse_initialize params)
  | "shutdown" -> ShutdownRequest
  | "codeLens/resolve" -> CodeLensResolveRequest (parse_codeLensResolve params)
  | "textDocument/hover" -> HoverRequest (parse_hover params)
  | "textDocument/codeAction" ->
    CodeActionRequest (parse_codeActionRequest params)
  | "codeAction/resolve" ->
    CodeActionResolveRequest (parse_codeActionResolveRequest params)
  | "textDocument/completion" -> CompletionRequest (parse_completion params)
  | "completionItem/resolve" ->
    CompletionItemResolveRequest (parse_completionItem params)
  | "textDocument/definition" ->
    DefinitionRequest (parse_textDocumentPositionParams params)
  | "textDocument/typeDefinition" ->
    TypeDefinitionRequest (parse_textDocumentPositionParams params)
  | "textDocument/implementation" ->
    ImplementationRequest (parse_textDocumentPositionParams params)
  | "workspace/symbol" -> WorkspaceSymbolRequest (parse_workspaceSymbol params)
  | "textDocument/documentSymbol" ->
    DocumentSymbolRequest (parse_documentSymbol params)
  | "textDocument/references" ->
    FindReferencesRequest (parse_findReferences params)
  | "textDocument/prepareCallHierarchy" ->
    PrepareCallHierarchyRequest (parse_textDocumentPositionParams params)
  | "callHierarchy/incomingCalls" ->
    CallHierarchyIncomingCallsRequest (parse_callHierarchyCalls params)
  | "callHierarchy/outgoingCalls" ->
    CallHierarchyOutgoingCallsRequest (parse_callHierarchyCalls params)
  | "textDocument/rename" -> RenameRequest (parse_documentRename params)
  | "textDocument/documentHighlight" ->
    DocumentHighlightRequest (parse_documentHighlight params)
  | "textDocument/formatting" ->
    DocumentFormattingRequest (parse_documentFormatting params)
  | "textDocument/rangeFormatting" ->
    DocumentRangeFormattingRequest (parse_documentRangeFormatting params)
  | "textDocument/onTypeFormatting" ->
    DocumentOnTypeFormattingRequest (parse_documentOnTypeFormatting params)
  | "textDocument/signatureHelp" ->
    SignatureHelpRequest (parse_signatureHelp params)
  | "flow/autoCloseJsx" -> AutoCloseRequest (parse_AutoClose params)
  | "textDocument/codeLens" ->
    DocumentCodeLensRequest (parse_documentCodeLens params)
  | "telemetry/rage" -> RageRequestFB
  | "$test/startHhServer" -> HackTestStartServerRequestFB
  | "$test/stopHhServer" -> HackTestStopServerRequestFB
  | "$test/shutdownServerlessIde" -> HackTestShutdownServerlessRequestFB
  | "textDocument/willSaveWaitUntil" ->
    WillSaveWaitUntilRequest (parse_willSaveWaitUntil params)
  | "custom/topLevelDefNameAtPos" ->
    TopLevelDefNameAtPosRequest
      (TopLevelDefNameAtPos.params_of_yojson @@ Option.value_exn params)
  | "window/showMessageRequest"
  | "window/showStatus"
  | _ ->
    UnknownRequest (method_, params)

let parse_lsp_notification (method_ : string) (params : Yojson.Safe.t option) :
    lsp_notification =
  match method_ with
  | "$/cancelRequest" -> CancelRequestNotification (parse_cancelRequest params)
  | "$/setTraceNotification" ->
    SetTraceNotification (parse_setTraceNotification params)
  | "$/setTrace" -> SetTrace (parse_setTrace params)
  | "$/logTraceNotification" -> LogTraceNotification
  | "initialized" -> InitializedNotification
  | "exit" -> ExitNotification
  | "textDocument/didOpen" -> DidOpenNotification (parse_didOpen params)
  | "textDocument/didClose" -> DidCloseNotification (parse_didClose params)
  | "textDocument/didSave" -> DidSaveNotification (parse_didSave params)
  | "textDocument/didChange" -> DidChangeNotification (parse_didChange params)
  | "workspace/didChangeWatchedFiles" ->
    DidChangeWatchedFilesNotification (parse_didChangeWatchedFiles params)
  | "textDocument/publishDiagnostics"
  | "window/logMessage"
  | "window/showMessage"
  | "window/progress"
  | "window/actionRequired"
  | "telemetry/connectionStatus"
  | _ ->
    UnknownNotification (method_, params)

let parse_lsp_result (request : lsp_request) (result : Yojson.Safe.t) :
    lsp_result =
  let method_ = request_name_to_string request in
  match request with
  | ShowMessageRequestRequest _ ->
    ShowMessageRequestResult (parse_result_showMessageRequest (Some result))
  | ShowStatusRequestFB _ -> ShowStatusResultFB ()
  | RegisterCapabilityRequest _ -> RegisterCapabilityRequestResult
  | InitializeRequest _
  | ShutdownRequest
  | CodeLensResolveRequest _
  | HoverRequest _
  | CodeActionRequest _
  | CodeActionResolveRequest _
  | CompletionRequest _
  | CompletionItemResolveRequest _
  | DefinitionRequest _
  | TypeDefinitionRequest _
  | ImplementationRequest _
  | WorkspaceSymbolRequest _
  | DocumentSymbolRequest _
  | FindReferencesRequest _
  | PrepareCallHierarchyRequest _
  | CallHierarchyIncomingCallsRequest _
  | CallHierarchyOutgoingCallsRequest _
  | DocumentHighlightRequest _
  | DocumentFormattingRequest _
  | DocumentRangeFormattingRequest _
  | DocumentOnTypeFormattingRequest _
  | RageRequestFB
  | RenameRequest _
  | DocumentCodeLensRequest _
  | SignatureHelpRequest _
  | AutoCloseRequest _
  | HackTestStartServerRequestFB
  | HackTestStopServerRequestFB
  | HackTestShutdownServerlessRequestFB
  | WillSaveWaitUntilRequest _
  | TopLevelDefNameAtPosRequest _
  | UnknownRequest _ ->
    raise
      (Error.LspException
         {
           Error.code = Error.ParseError;
           message = "Don't know how to parse LSP response " ^ method_;
           data = None;
         })

(* parse_lsp: non-jsonrpc inputs - will raise an exception                    *)
(* requests and notifications - will raise an exception if they're malformed, *)
(*   otherwise return Some                                                    *)
(* responses - will raise an exception if they're malformed, will return None *)
(*   if they're absent from the "outstanding" map, otherwise return Some.     *)
let parse_lsp (json : Yojson.Safe.t) (outstanding : lsp_id -> lsp_request) :
    lsp_message =
  let json = Some json in
  let id = Jget.val_opt json "id" |> parse_id_opt in
  let method_opt = Jget.string_opt json "method" in
  let params = Jget.val_opt json "params" in
  let result = Jget.val_opt json "result" in
  let error = Jget.val_opt json "error" in
  match (id, method_opt, result, error) with
  | (None, Some method_, _, _) ->
    NotificationMessage (parse_lsp_notification method_ params)
  | (Some id, Some method_, _, _) ->
    RequestMessage (id, parse_lsp_request method_ params)
  | (Some id, _, Some result, _) ->
    let request = outstanding id in
    ResponseMessage (id, parse_lsp_result request result)
  | (Some id, _, _, Some error) ->
    ResponseMessage (id, ErrorResult (parse_error error))
  | (_, _, _, _) ->
    raise
      (Error.LspException
         { Error.code = Error.ParseError; message = "Not JsonRPC"; data = None })

let print_lsp_request (id : lsp_id) (request : lsp_request) : Yojson.Safe.t =
  let method_ = request_name_to_string request in
  let params =
    match request with
    | ShowMessageRequestRequest r -> print_showMessageRequest r
    | ShowStatusRequestFB r -> print_showStatus r
    | RegisterCapabilityRequest r -> print_registerCapability r
    | InitializeRequest _
    | ShutdownRequest
    | HoverRequest _
    | CodeActionRequest _
    | CodeActionResolveRequest _
    | CodeLensResolveRequest _
    | CompletionRequest _
    | CompletionItemResolveRequest _
    | DefinitionRequest _
    | TypeDefinitionRequest _
    | ImplementationRequest _
    | WorkspaceSymbolRequest _
    | DocumentSymbolRequest _
    | FindReferencesRequest _
    | PrepareCallHierarchyRequest _
    | CallHierarchyIncomingCallsRequest _
    | CallHierarchyOutgoingCallsRequest _
    | DocumentHighlightRequest _
    | DocumentFormattingRequest _
    | DocumentRangeFormattingRequest _
    | DocumentOnTypeFormattingRequest _
    | RageRequestFB
    | RenameRequest _
    | DocumentCodeLensRequest _
    | SignatureHelpRequest _
    | AutoCloseRequest _
    | HackTestStartServerRequestFB
    | HackTestStopServerRequestFB
    | HackTestShutdownServerlessRequestFB
    | WillSaveWaitUntilRequest _
    | TopLevelDefNameAtPosRequest _
    | UnknownRequest _ ->
      failwith ("Don't know how to print request " ^ method_)
  in
  `Assoc
    [
      ("jsonrpc", `String "2.0");
      ("id", print_id id);
      ("method", `String method_);
      ("params", params);
    ]

let print_lsp_response (id : lsp_id) (result : lsp_result) : Yojson.Safe.t =
  let method_ = result_name_to_string result in
  let json =
    match result with
    | InitializeResult r -> print_initialize r
    | ShutdownResult -> print_shutdown ()
    | CodeLensResolveResult r -> print_codeLensResolve r
    | HoverResult r -> print_hover r
    | CodeActionResult (r, p) -> print_codeActionResult r p
    | CodeActionResolveResult r -> print_codeActionResolveResult r
    | CompletionResult r -> print_completion r
    | CompletionItemResolveResult r -> print_completionItem r
    | DefinitionResult r -> print_definition_locations r
    | TypeDefinitionResult r -> print_definition_locations r
    | ImplementationResult r -> print_locations r
    | WorkspaceSymbolResult r -> print_workspaceSymbol r
    | DocumentSymbolResult r -> print_documentSymbol r
    | FindReferencesResult r -> print_locations r
    | PrepareCallHierarchyResult r -> print_PrepareCallHierarchyResult r
    | CallHierarchyIncomingCallsResult r ->
      print_CallHierarchyIncomingCallsResult r
    | CallHierarchyOutgoingCallsResult r ->
      print_CallHierarchyOutgoingCallsResult r
    | DocumentHighlightResult r -> print_documentHighlight r
    | DocumentFormattingResult r -> print_documentFormatting r
    | DocumentRangeFormattingResult r -> print_documentRangeFormatting r
    | DocumentOnTypeFormattingResult r -> print_documentOnTypeFormatting r
    | RageResultFB r -> print_rage r
    | RenameResult r -> print_workspaceEdit r
    | DocumentCodeLensResult r -> print_documentCodeLens r
    | SignatureHelpResult r -> print_signatureHelp r
    | AutoCloseResult r -> print_AutoClose r
    | HackTestStartServerResultFB -> `Null
    | HackTestStopServerResultFB -> `Null
    | HackTestShutdownServerlessResultFB -> `Null
    | ShowMessageRequestResult _
    | ShowStatusResultFB _
    | RegisterCapabilityRequestResult ->
      failwith ("Don't know how to print result " ^ method_)
    | WillSaveWaitUntilResult r -> print_textEdits r
    | TopLevelDefNameAtPosResult r -> TopLevelDefNameAtPos.yojson_of_result r
    | ErrorResult e -> print_error e
  in
  match result with
  | ErrorResult _ ->
    `Assoc [("jsonrpc", `String "2.0"); ("id", print_id id); ("error", json)]
  | _ ->
    `Assoc [("jsonrpc", `String "2.0"); ("id", print_id id); ("result", json)]

let print_lsp_notification (notification : lsp_notification) : Yojson.Safe.t =
  let method_ = notification_name_to_string notification in
  let params =
    match notification with
    | CancelRequestNotification r -> print_cancelRequest r
    | SetTraceNotification r -> print_setTraceNotification r
    | SetTrace r -> print_setTrace r
    | PublishDiagnosticsNotification r -> print_diagnostics r
    | FindReferencesPartialResultNotification (token, r) ->
      print_findReferencesPartialResult token r
    | TelemetryNotification (r, extras) -> print_telemetryNotification r extras
    | LogMessageNotification r -> print_logMessage r.type_ r.message
    | ShowMessageNotification r -> print_showMessage r.type_ r.message
    | ConnectionStatusNotificationFB r -> print_connectionStatus r
    | ExitNotification
    | InitializedNotification
    | LogTraceNotification
    | DidOpenNotification _
    | DidCloseNotification _
    | DidSaveNotification _
    | DidChangeNotification _
    | DidChangeWatchedFilesNotification _
    | UnknownNotification _ ->
      failwith ("Don't know how to print notification " ^ method_)
  in
  `Assoc
    [
      ("jsonrpc", `String "2.0"); ("method", `String method_); ("params", params);
    ]

let print_lsp (message : lsp_message) : Yojson.Safe.t =
  match message with
  | RequestMessage (id, request) -> print_lsp_request id request
  | ResponseMessage (id, result) -> print_lsp_response id result
  | NotificationMessage notification -> print_lsp_notification notification
