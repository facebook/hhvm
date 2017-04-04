(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Core
open Lsp
open Hh_json


(************************************************************************)
(** Helpers for parsing & printing                                     **)
(************************************************************************)

module Jget = struct
  (* Helpers for the various "option" monads in use for Json, to succinctly
     capture the spirit of JSON (tolerance for missing values) and the spirit
     of LSP (loads of nested optional members with obvious defaults)
     and the usefuless of error-checking (in case a required field is absent)...
     - We use "json option" throughout. Things which you might expect to return
       a json are instead lifted to return a json option, so you can use all the
       accessors on them more easily. When you attempt to get "o.m", either
       it's present both because "o" is Some, and because "m" is a member.
       Or it's absent because either of those things is false...
     - The "_opt" accessors uniformally return Some (present) or None (absent),
       regardless of which of the two things caused absence.
     - The "_d" accessors uniformally return a value (present) or default.
     - The "_exn" accessors uniformally return a value (present) or throw.

     The effect of this is you lose precise information about what exactly
     caused an absence (which is usually only of marginal benefit), and in
     return you gain a consistent way to handle both optionals and requireds.
     Note: if you wish to get an int, and it's present but not parseable as
     an int, then this always throws.
     *)

  let get_opt hhjson_getter json key = match json with
    | None -> None
    | Some json -> match hhjson_getter key (json, []) with
                   | Result.Ok (r, _keytrace) -> Some r
                   | _ -> None

  let get_exn opt_getter json key = match opt_getter json key with
    | None -> raise (Error.Parse key)
    | Some v -> v

  let int_string_opt (s: string option) : int option = match s with
    | None -> None
    | Some s -> try Some (int_of_string s)
                with Failure _ -> raise (Error.Parse ("not an int: " ^ s))

  let list_opt (l: 'a list option) : 'a option list option = match l with
    | None -> None
    | Some x -> Some (List.map x ~f:(fun a -> Some a))

  (* Accessors which return None on absence *)
  let string_opt = get_opt Access.get_string
  let bool_opt = get_opt Access.get_bool
  let obj_opt = get_opt Access.get_obj
  let val_opt = get_opt Access.get_val
  let int_opt json key = get_opt Access.get_number json key |> int_string_opt
  let array_opt json key = get_opt Access.get_array json key |> list_opt
    (* array_opt lifts all the array's members into the "json option" monad *)

  (* Accessors which return a supplied default on absence *)
  let string_d json key ~default = Option.value (string_opt json key) ~default
  let bool_d json key ~default = Option.value (bool_opt json key) ~default
  let int_d json key ~default = Option.value (int_opt json key) ~default
  let array_d json key ~default = Option.value (array_opt json key) ~default

  (* Accessors which throw "Error.Parse key" on absence *)
  let string_exn = get_exn string_opt
  let int_exn = get_exn int_opt
  let obj_exn json key = Some (get_exn obj_opt json key)
    (* obj_exn lifts the result into the "json option" monad *)
end

module Jprint = struct
  (* object_opt is like Hh_json.JSON_Object constructor except it takes
     key * (value option): if a value is None, then it omits this member. *)
  let object_opt (keyvalues : (string * (json option)) list) : json =
    let make_pair_option = function
      | (k, Some v) -> Some (k,v)
      | _ -> None
    in
    JSON_Object (List.filter_map keyvalues ~f:make_pair_option)

  (* Convenience function to convert string list to JSON_Array *)
  let string_array (l: string list) : json =
    JSON_Array (List.map l ~f:string_)
end


(************************************************************************)
(** Miscellaneous LSP structures                                       **)
(************************************************************************)

let parse_position (json: json option) : position =
  {
    line = Jget.int_exn json "line";
    character = Jget.int_exn json "character";
  }

let print_position (position: position) : json =
  JSON_Object [
    "line", position.line |> int_;
    "character", position.character |> int_;
  ]

let print_range (range: range) : json =
  JSON_Object [
    "start", print_position range.start;
    "end", print_position range.end_;
  ]

let print_location (location: Location.t) : json =
  let open Location in
  JSON_Object [
    "uri", JSON_String location.uri;
    "range", print_range location.range;
  ]

let parse_range_opt (json: json option) : range option =
  Option.map json ~f:(fun _ ->
  {
    start = Jget.obj_exn json "start" |> parse_position;
    end_ = Jget.obj_exn json "end" |> parse_position;
  })

let parse_text_document_identifier (json: json option)
  : Text_document_identifier.t =
  let open Text_document_identifier in
  {
    uri = Jget.string_exn json "uri";
  }

let parse_versioned_text_document_identifier (json: json option)
  : Versioned_text_document_identifier.t =
  let open Versioned_text_document_identifier in
  {
    uri = Jget.string_exn json "uri";
    version = Jget.int_d json "version" 0;
  }

let parse_text_document_item (json: json option) : Text_document_item.t =
  let open Text_document_item in
  {
    uri = Jget.string_exn json "uri";
    language_id = Jget.string_d json "languageId" "";
    version = Jget.int_d json "version" 0;
    text = Jget.string_exn json "text";
  }

let print_marked_item (item: marked_string) : json =
  match item with
  | Marked_string s -> JSON_String s
  | Marked_code (language, value) -> JSON_Object [
      "language", JSON_String language;
      "value", JSON_String value;
    ]

let parse_text_document_position_params (params: json option)
  : Text_document_position_params.t =
  let open Text_document_position_params in
  {
    text_document = Jget.obj_exn params "textDocument"
      |> parse_text_document_identifier;
    position = Jget.obj_exn params "position" |> parse_position;
  }

let print_text_edit (edit: Text_edit.t) : json =
  let open Text_edit in
  JSON_Object [
    "range", print_range edit.range;
    "newText", JSON_String edit.new_text;
  ]

let print_command (command: Command.t) : json =
  let open Command in
  JSON_Object [
    "title", JSON_String command.title;
    "command", JSON_String command.command;
    "arguments", JSON_Array command.arguments;
  ]

let parse_command (json: json option) : Command.t =
  let open Command in
  {
    title = Jget.string_d json "title" "";
    command = Jget.string_d json "command" "";
    arguments = Jget.array_d json "arguments" ~default:[] |> List.filter_opt;
  }

let print_symbol_information (info: Symbol_information.t) : json =
  let open Symbol_information in
  let print_symbol_kind = function
    | File -> int_ 1
    | Module -> int_ 2
    | Namespace -> int_ 3
    | Package -> int_ 4
    | Class -> int_ 5
    | Method -> int_ 6
    | Property -> int_ 7
    | Field -> int_ 8
    | Constructor -> int_ 9
    | Enum -> int_ 10
    | Interface -> int_ 11
    | Function -> int_ 12
    | Variable -> int_ 13
    | Constant -> int_ 14
    | String -> int_ 15
    | Number -> int_ 16
    | Boolean -> int_ 17
    | Array -> int_ 18
  in
  Jprint.object_opt [
    "name", Some (JSON_String info.name);
    "kind", Some (print_symbol_kind info.kind);
    "location", Some (print_location info.location);
    "containerName", Option.map info.container_name string_;
  ]


(************************************************************************)
(** shutdown request                                                   **)
(************************************************************************)

let print_shutdown (_r: Shutdown.result) : json =
  JSON_Null


(************************************************************************)
(** textDocument/didOpen notification                                  **)
(************************************************************************)

let parse_did_open (params: json option) : Did_open.params =
  let open Did_open in
  {
    text_document = Jget.obj_exn params "text_document"
                      |> parse_text_document_item;
  }


(************************************************************************)
(** textDocument/didClose notification                                 **)
(************************************************************************)

let parse_did_close (params: json option) : Did_close.params =
  let open Did_close in
  {
    text_document = Jget.obj_exn params "text_document"
                      |> parse_text_document_identifier;
  }


(************************************************************************)
(** textDocument/didChange notification                                **)
(************************************************************************)

let parse_did_change (params: json option) : Did_change.params =
  let open Did_change in
  let parse_text_document_content_change_event json =
    {
      range = Jget.obj_opt json "range" |> parse_range_opt;
      range_length = Jget.int_opt json "rangeLength";
      text = Jget.string_exn json "text";
    }
  in
  {
    text_document = Jget.obj_exn params "textDocument"
                    |> parse_versioned_text_document_identifier;
    content_changes = Jget.array_d params "contentChanges" ~default:[]
                      |> List.map ~f:parse_text_document_content_change_event;
  }


(************************************************************************)
(** textDocument/publishDiagnostics notification                       **)
(************************************************************************)

let print_diagnostics (r: Publish_diagnostics.params) : json =
  let open Publish_diagnostics in
  let print_diagnostic_severity = function
    | Publish_diagnostics.Error -> int_ 1
    | Publish_diagnostics.Warning -> int_ 2
    | Publish_diagnostics.Information -> int_ 3
    | Publish_diagnostics.Hint -> int_ 4
  in
  let print_diagnostic (diagnostic: diagnostic) : json =
    Jprint.object_opt [
      "range", Some (print_range diagnostic.range);
      "severity", Option.map diagnostic.severity print_diagnostic_severity;
      "code", Option.map diagnostic.code int_;
      "source", Option.map diagnostic.source string_;
      "message", Some (JSON_String diagnostic.message);
    ]
  in
  JSON_Object [
    "uri", JSON_String r.uri;
    "diagnostics", JSON_Array (List.map r.diagnostics ~f:print_diagnostic)
  ]


(************************************************************************)
(** textDocument/hover request                                         **)
(************************************************************************)

let parse_hover (params: json option) : Hover.params =
  parse_text_document_position_params params

let print_hover (r: Hover.result) : json =
  let open Hover in
  Jprint.object_opt [
    "contents", Some (JSON_Array
                        (List.map r.Hover.contents ~f:print_marked_item));
    "range", Option.map r.range ~f:print_range;
  ]


(************************************************************************)
(** textDocument/definition request                                    **)
(************************************************************************)

let parse_definition (params: json option) : Definition.params =
  parse_text_document_position_params params

let print_definition (r: Definition.result) : json =
  JSON_Array (List.map r ~f:print_location)


(************************************************************************)
(** textDocument/completion request                                    **)
(************************************************************************)

let parse_completion (params: json option) : Definition.params =
  parse_text_document_position_params params

let print_completion (r: Completion.result) : json =
  let open Completion in
  let rec print_completion_item (item: Completion.completion_item) : json =
    Jprint.object_opt [
      "label", Some (JSON_String item.label);
      "kind", Option.map item.kind print_completion_item_kind;
      "detail", Option.map item.detail string_;
      "documentation", Option.map item.documentation string_;
      "sortText", Option.map item.sort_text string_;
      "filterText", Option.map item.filter_text string_;
      "insertText", Option.map item.insert_text string_;
      "insertTextFormat", Some (print_insert_format item.insert_text_format);
      "textEdit", Option.map (List.hd item.text_edits) print_text_edit;
      "additionalTextEdit", (match (List.tl item.text_edits) with
              | None | Some [] -> None
              | Some l -> Some (JSON_Array (List.map l ~f:print_text_edit)));
      "command", Option.map item.command print_command;
      "data", item.data;
    ]
  and print_completion_item_kind = function
    | Text -> int_ 1
    | Method -> int_ 2
    | Function -> int_ 3
    | Constructor -> int_ 4
    | Field -> int_ 5
    | Variable -> int_ 6
    | Class -> int_ 7
    | Interface -> int_ 8
    | Module -> int_ 9
    | Property -> int_ 10
    | Unit -> int_ 11
    | Value -> int_ 12
    | Enum -> int_ 13
    | Keyword -> int_ 14
    | Snippet -> int_ 15
    | Color -> int_ 16
    | File -> int_ 17
    | Reference -> int_ 18
  and print_insert_format = function
    | PlainText -> int_ 1
    | SnippetFormat -> int_ 2
  in
  JSON_Object [
    "isIncomplete", JSON_Bool r.is_incomplete;
    "items", JSON_Array (List.map r.items ~f:print_completion_item);
  ]


(************************************************************************)
(** workspace/symbol request                                           **)
(************************************************************************)


let parse_workspace_symbol (params: json option) : Workspace_symbol.params =
  let open Workspace_symbol in
  {
    query = Jget.string_exn params "query";
  }

let print_workspace_symbol (r: Workspace_symbol.result) : json =
  JSON_Array (List.map r ~f:print_symbol_information)


(************************************************************************)
(** textDocument/documentSymbol request                                **)
(************************************************************************)

let parse_document_symbol (params: json option) : Document_symbol.params =
  let open Document_symbol in
  {
    text_document = Jget.obj_exn params "textDocument"
                    |> parse_text_document_identifier;
  }

let print_document_symbol (r: Document_symbol.result) : json =
  JSON_Array (List.map r ~f:print_symbol_information)


(************************************************************************)
(** textDocument/references request                                    **)
(************************************************************************)

let parse_find_references (params: json option) : Find_references.params =
  let open Text_document_position_params in
  let as_position_params = parse_text_document_position_params params in
  let context = Jget.obj_opt params "context" in
  { Find_references.
    text_document = as_position_params.text_document;
    position = as_position_params.position;
    context = { Find_references.
      include_declaration = Jget.bool_d context "includeDeclaration" true;
    }
  }

let print_find_references (r: Location.t list) : json =
  JSON_Array (List.map r ~f:print_location)

(************************************************************************)
(** initialize request                                                 **)
(************************************************************************)

let parse_initialize (params: json option) : Initialize.params =
  let open Initialize in
  let rec parse_initialize json =
    {
      process_id = Jget.int_opt json "processId";
      root_path = Jget.string_opt json "rootPath";
      root_uri = Jget.string_opt json "rootUri";
      client_capabilities = Jget.obj_opt json "capabilities"
                            |> parse_capabilities;
      trace = Jget.string_opt json "trace" |> parse_trace;
    }
  and parse_trace (s : string option) : trace = match s with
    | Some "messages" -> Messages
    | Some "verbose" -> Verbose
    | _ -> Off
  and parse_capabilities json =
    {
      workspace = Jget.obj_opt json "workspace" |> parse_workspace;
      text_document = Jget.obj_opt json "textDocument" |> parse_text_document;
    }
  and parse_workspace json =
    {
      apply_edit = Jget.bool_d json "applyEdit" ~default:false;
      workspace_edit = Jget.obj_opt json "workspaceEdit"
                       |> parse_workspace_edit;
    }
  and parse_workspace_edit json =
    {
      document_changes = Jget.bool_d json "documentChanges" ~default:false;
    }
  and parse_text_document json =
    {
      synchronization =
        Jget.obj_opt json "synchronization" |> parse_synchronization;
      completion = Jget.obj_opt json "completion" |> parse_completion;
    }
  and parse_synchronization json =
    {
      can_will_save = Jget.bool_d json "willSave" ~default:false;
      can_will_save_wait_until =
        Jget.bool_d json "willSaveWaitUntil" ~default:false;
      can_did_save = Jget.bool_d json "didSave" ~default:false;
    }
  and parse_completion json =
    { completion_item =
        Jget.obj_opt json "completionItem" |> parse_completion_item;
    }
  and parse_completion_item json =
    { snippet_support = Jget.bool_d json "snippetSupport" ~default:false;
    }
  in
  parse_initialize params

let print_initialize_error (r: Initialize.error_data) : json =
  let open Initialize in
  JSON_Bool r.retry

let print_initialize (r: Initialize.result) : json =
  let open Initialize in
  let print_text_document_sync_kind = function
    | NoSync -> int_ 0
    | FullSync -> int_ 1
    | IncrementalSync -> int_ 2 in
  let cap = r.server_capabilities in
  let sync = cap.text_document_sync
  in
  JSON_Object [
    "capabilities", Jprint.object_opt [
      "textDocumentSync", Some (JSON_Object [
        "openClose", JSON_Bool sync.want_open_close;
        "change", print_text_document_sync_kind sync.want_change;
        "willSave", JSON_Bool sync.want_will_save;
        "willSaveWaitUntil", JSON_Bool sync.want_will_save_wait_until;
        "save", JSON_Object [
          "includeText", JSON_Bool sync.want_did_save.include_text;
        ];
      ]);
      "hoverProvider", Some (JSON_Bool cap.hover_provider);
      "completionProvider",
        Option.map cap.completion_provider ~f:(fun comp -> JSON_Object [
        "resolveProvider", JSON_Bool comp.resolve_provider;
        "triggerCharacters",
          Jprint.string_array comp.completion_trigger_characters;
        ]);
      "signatureHelpProvider",
        Option.map cap.signature_help_provider ~f:(fun shp -> JSON_Object [
        "triggerCharacters",
          Jprint.string_array shp.sighelp_trigger_characters;
        ]);
      "definitionProvider", Some (JSON_Bool cap.definition_provider);
      "referencesProvider", Some (JSON_Bool cap.references_provider);
      "documentHighlightProvider",
        Some (JSON_Bool cap.document_highlight_provider);
      "documentSymbolProvider",
        Some (JSON_Bool cap.document_symbol_provider);
      "workspaceSymbolProvider",
        Some (JSON_Bool cap.workspace_symbol_provider);
      "codeActionProvider",
        Some (JSON_Bool cap.code_action_provider);
      "codeLensProvider",
        Option.map cap.code_lens_provider ~f:(fun codelens -> JSON_Object [
        "resolveProvider", JSON_Bool codelens.code_lens_resolve_provider;
        ]);
      "documentFormattingProvider",
        Some (JSON_Bool cap.document_formatting_provider);
      "documentRangeFormattingProvider",
        Some (JSON_Bool cap.document_range_formatting_provider);
      "documentOnTypeFormattingProvider", Option.map
        cap.document_on_type_formatting_provider ~f:(fun o -> JSON_Object [
        "firstTriggerCharacter", JSON_String o.first_trigger_character;
        "moreTriggerCharacter",
          Jprint.string_array o.more_trigger_characters;
        ]);
      "renameProvider", Some (JSON_Bool cap.rename_provider);
      "documentLinkProvider",
        Option.map cap.document_link_provider ~f:(fun dlp -> JSON_Object [
          "resolveProvider", JSON_Bool dlp.document_link_resolve_provider;
        ]);
      "executeCommandProvider",
        Option.map cap.execute_command_provider ~f:(fun p -> JSON_Object [
          "commands", Jprint.string_array p.commands;
        ]);
    ];
  ]


(************************************************************************)
(** error response                                                     **)
(************************************************************************)

let print_error (e: exn) : json =
  let open Hh_json in
  let (code, message, data) = match e with
    | Error.Parse message -> (-32700, message, None)
    | Error.Invalid_request message -> (-32600, message, None)
    | Error.Method_not_found message -> (-32601, message, None)
    | Error.Invalid_params message -> (-32602, message, None)
    | Error.Internal_error message -> (-32603, message, None)
    | Error.Server_error_start (message, data) ->
        (-32603, message, Some (print_initialize_error data))
    | Error.Server_error_end message -> (-32000, message, None)
    | Error.Server_not_initialized message -> (-32002, message, None)
    | Error.Unknown message -> (-32001, message, None)
    | _ -> (-32001, Printexc.to_string e, None)
  in
  Jprint.object_opt [
    "code", Some (int_ code);
    "message", Some (string_ message);
    "data", data;
  ]
