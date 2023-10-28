(* A few helpful wrappers around LSP *)

open Hh_prelude
open Lsp
open Lsp_fmt

let progress_and_actionRequired_counter = ref 0

(************************************************************************)
(* Conversions                                                          *)
(************************************************************************)

let url_scheme_regex = Str.regexp "^\\([a-zA-Z][a-zA-Z0-9+.-]+\\):"

(* this requires schemes with 2+ characters, so "c:\path" isn't considered a scheme *)

let lsp_uri_to_path (uri : DocumentUri.t) : string =
  let uri = string_of_uri uri in
  if Str.string_match url_scheme_regex uri 0 then
    let scheme = Str.matched_group 1 uri in
    if String.equal scheme "file" then
      File_url.parse uri
    else
      raise
        (Error.LspException
           {
             Error.code = Error.InvalidParams;
             message = Printf.sprintf "Not a valid file url '%s'" uri;
             data = None;
           })
  else
    uri

let path_to_lsp_uri (path : string) ~(default_path : string) : Lsp.DocumentUri.t
    =
  if String.equal path "" then begin
    HackEventLogger.invariant_violation_bug "missing path";
    Hh_logger.log
      "missing path %s"
      (Exception.get_current_callstack_string 99 |> Exception.clean_stack);
    File_url.create default_path |> uri_of_string
  end else
    File_url.create path |> uri_of_string

let lsp_textDocumentIdentifier_to_filename
    (identifier : Lsp.TextDocumentIdentifier.t) : string =
  Lsp.TextDocumentIdentifier.(lsp_uri_to_path identifier.uri)

let lsp_position_to_fc (pos : Lsp.position) : File_content.position =
  {
    File_content.line = pos.Lsp.line + 1;
    (* LSP is 0-based; File_content is 1-based. *)
    column = pos.Lsp.character + 1;
  }

let lsp_range_to_fc (range : Lsp.range) : File_content.range =
  {
    File_content.st = lsp_position_to_fc range.Lsp.start;
    ed = lsp_position_to_fc range.Lsp.end_;
  }

let lsp_range_to_pos ~line_to_offset path (range : Lsp.range) : Pos.t =
  let triple_of_endpoint Lsp.{ line; character } =
    let bol = line_to_offset line in
    (line, bol, bol + character)
  in
  Pos.make_from_lnum_bol_offset
    ~pos_file:path
    ~pos_start:(triple_of_endpoint range.Lsp.start)
    ~pos_end:(triple_of_endpoint range.Lsp.end_)

let lsp_range_is_selection (range : Lsp.range) =
  range.start.line < range.end_.line
  || range.start.line = range.end_.line
     && range.start.character < range.end_.character

let lsp_edit_to_fc (edit : Lsp.DidChange.textDocumentContentChangeEvent) :
    File_content.text_edit =
  {
    File_content.range = Option.map edit.DidChange.range ~f:lsp_range_to_fc;
    text = edit.DidChange.text;
  }

let apply_changes
    (text : string)
    (contentChanges : DidChange.textDocumentContentChangeEvent list) :
    (string, string * Exception.t) result =
  let edits = List.map ~f:lsp_edit_to_fc contentChanges in
  File_content.edit_file text edits

let get_char_from_lsp_position (content : string) (position : Lsp.position) :
    char =
  let fc_position = lsp_position_to_fc position in
  File_content.(get_char content (get_offset content fc_position))

let apply_changes_unsafe
    text (contentChanges : DidChange.textDocumentContentChangeEvent list) :
    string =
  match apply_changes text contentChanges with
  | Ok r -> r
  | Error (e, _stack) -> failwith e

let sym_occ_kind_to_lsp_sym_info_kind (sym_occ_kind : SymbolOccurrence.kind) :
    Lsp.SymbolInformation.symbolKind =
  let open Lsp.SymbolInformation in
  let open SymbolOccurrence in
  match sym_occ_kind with
  | Class _ -> Class
  | BuiltInType _ -> Class
  | Function -> Function
  | Method _ -> Method
  | LocalVar -> Variable
  | TypeVar -> TypeParameter
  | Property _ -> Property
  | XhpLiteralAttr _ -> Property
  | ClassConst _ -> TypeParameter
  | Typeconst _ -> TypeParameter
  | GConst -> Constant
  | Attribute _ -> Class
  | EnumClassLabel _ -> EnumMember
  | Keyword _ -> Null
  | PureFunctionContext -> Null
  | BestEffortArgument _ -> Null
  | HhFixme -> Null
  | Module -> Module

let hack_pos_to_lsp_range ~(equal : 'a -> 'a -> bool) (pos : 'a Pos.pos) :
    Lsp.range =
  (* .hhconfig errors are Positions with a filename, but dummy start/end
   * positions. Handle that case - and Pos.none - specially, as the LSP
   * specification requires line and character >= 0, and VSCode silently
   * drops diagnostics that violate the spec in this way *)
  if Pos.equal_pos equal pos (Pos.make_from (Pos.filename pos)) then
    { start = { line = 0; character = 0 }; end_ = { line = 0; character = 0 } }
  else
    let (line1, col1, line2, col2) = Pos.destruct_range pos in
    {
      start = { line = line1 - 1; character = col1 - 1 };
      end_ = { line = line2 - 1; character = col2 - 1 };
    }

let hack_pos_to_lsp_range_adjusted (p : 'a Pos.pos) : range =
  let (line_start, line_end, character_start, character_end) =
    Pos.info_pos_extended p
  in
  let (start_position : position) =
    { line = line_start; character = character_start }
  in
  let (end_position : position) =
    { line = line_end; character = character_end }
  in
  { start = start_position; end_ = end_position }

let symbol_to_lsp_call_item
    (sym_occ : Relative_path.t SymbolOccurrence.t)
    (sym_def_opt : Relative_path.t SymbolDefinition.t option) :
    CallHierarchyItem.t =
  let open CallHierarchyItem in
  let open SymbolOccurrence in
  let (selectionRange_, range_, file_pos) =
    match sym_def_opt with
    | None ->
      let sym_range = hack_pos_to_lsp_range_adjusted sym_occ.pos in
      (sym_range, sym_range, sym_occ.pos)
    | Some sym_def ->
      ( hack_pos_to_lsp_range_adjusted sym_def.SymbolDefinition.pos,
        hack_pos_to_lsp_range_adjusted sym_def.SymbolDefinition.span,
        sym_def.SymbolDefinition.pos )
  in
  let filename_ = Pos.to_absolute file_pos |> Pos.filename in
  let uri_ = path_to_lsp_uri filename_ ~default_path:"www" in
  {
    name = sym_occ.name;
    kind = sym_occ_kind_to_lsp_sym_info_kind sym_occ.type_;
    detail = None;
    uri = uri_;
    range = range_;
    selectionRange = selectionRange_;
  }

(************************************************************************)
(* Range calculations                                                   *)
(************************************************************************)

(* We need to do intersection and other calculations on ranges.
 * The functions in the following module all assume LSP 0-based ranges,
 * and assume without testing that a range's start is equal to or before
 * its end. *)
let pos_compare (p1 : position) (p2 : position) : int =
  if p1.line < p2.line then
    -1
  else if p1.line > p2.line then
    1
  else
    p1.character - p2.character

(* Given a "selection" range A..B and a "squiggle" range a..b, how do they overlap?
 * There are 12 ways to order the four letters ABab, of which six
 * satisfy both A<=B and a<=b. Here they are. *)
type range_overlap =
  | Selection_before_start_of_squiggle (* ABab *)
  | Selection_overlaps_start_of_squiggle (* AaBb *)
  | Selection_covers_whole_squiggle (* AabB *)
  | Selection_in_middle_of_squiggle (* aABb *)
  | Selection_overlaps_end_of_squiggle (* aAbB *)
  (* abAB *)
  | Selection_after_end_of_squiggle

(* Computes how two ranges "selection" and "squiggle" overlap *)
let get_range_overlap (selection : range) (squiggle : range) : range_overlap =
  let selStart_leq_squiggleStart =
    pos_compare selection.start squiggle.start <= 0
  in
  let selStart_leq_squiggleEnd =
    pos_compare selection.start squiggle.end_ <= 0
  in
  let selEnd_lt_squiggleStart = pos_compare selection.end_ squiggle.start < 0 in
  let selEnd_lt_squiggleEnd = pos_compare selection.end_ squiggle.end_ < 0 in
  (* Q. Why does it test "<=" for the first two and "<" for the last two? *)
  (* Intuitively you can trust that it has something to do with how ranges are *)
  (* inclusive at their start and exclusive at their end. But the real reason *)
  (* is just that I did an exhaustive case analysis to look at all cases where *)
  (* A,B,a,b might be equal, and decided which outcome I wanted for each of them *)
  (* because of how I'm going to treat them in other functions, and retrofitted *)
  (* those answers into this function. For instance, if squiggleStart==selEnd, *)
  (* I'll want to handle it in the same way as squiggleStart<selEnd<squiggleEnd. *)
  (* The choices of "leq" and "lt" in this function embody those answers. *)
  match
    ( selStart_leq_squiggleStart,
      selStart_leq_squiggleEnd,
      selEnd_lt_squiggleStart,
      selEnd_lt_squiggleEnd )
  with
  | (true, true, true, true) -> Selection_before_start_of_squiggle
  | (true, true, false, true) -> Selection_overlaps_start_of_squiggle
  | (true, true, false, false) -> Selection_covers_whole_squiggle
  | (false, true, false, true) -> Selection_in_middle_of_squiggle
  | (false, true, false, false) -> Selection_overlaps_end_of_squiggle
  | (false, false, false, false) -> Selection_after_end_of_squiggle
  | (true, false, _, _) ->
    failwith "sel.start proves squiggle.start > squiggle.end_"
  | (_, _, true, false) ->
    failwith "sel.end proves squiggle.start > squiggle.end_"
  | (false, _, true, _) -> failwith "squiggle.start proves sel.start > sel.end_"
  | (_, false, _, true) -> failwith "squiggle.end_ proves sel.start > sel.end_"

(* this structure models a change where a certain range is replaced with
 * a block of text. For instance, if you merely insert a single character,
 * then remove_range.start==remove_range.end_ and insert_lines=0
 * and insert_chars_on_final_line=1. *)
type range_replace = {
  remove_range: range;
  insert_lines: int;
  insert_chars_on_final_line: int;
}

(* If you have a position "p", and some range before this point is replaced with
 * text of a certain number of lines, the last line having a certain number of characters,
 * then how will the position be shifted?
 * Note: this function assumes but doesn't verify that the range ends on or before
 * the position. *)
let update_pos_due_to_prior_replace (p : position) (replace : range_replace) :
    position =
  if replace.remove_range.end_.line < p.line then
    (* The replaced range doesn't touch the position, so position merely gets shifted up/down *)
    let line =
      p.line
      - (replace.remove_range.end_.line - replace.remove_range.start.line)
      + replace.insert_lines
    in
    { p with line }
  else if replace.insert_lines > 0 then
    (* The position is on the final line and multiple lines were inserted  *)
    let line =
      p.line
      - (replace.remove_range.end_.line - replace.remove_range.start.line)
      + replace.insert_lines
    in
    let character =
      replace.insert_chars_on_final_line
      + (p.character - replace.remove_range.end_.character)
    in
    { line; character }
  else
    (* The position is on the line where a few characters were inserted *)
    let line =
      p.line - (replace.remove_range.end_.line - replace.remove_range.start.line)
    in
    let character =
      replace.remove_range.start.character
      + replace.insert_chars_on_final_line
      + (p.character - replace.remove_range.end_.character)
    in
    { line; character }

(* If you have a squiggle, and some range in the document is replaced with a block
 * some lines long and with insert_chars on the final line, then what's the new
 * range of the squiggle? *)
let update_range_due_to_replace (squiggle : range) (replace : range_replace) :
    range option =
  match get_range_overlap replace.remove_range squiggle with
  | Selection_before_start_of_squiggle ->
    let start = update_pos_due_to_prior_replace squiggle.start replace in
    let end_ = update_pos_due_to_prior_replace squiggle.end_ replace in
    Some { start; end_ }
  | Selection_overlaps_start_of_squiggle ->
    let line = replace.remove_range.start.line + replace.insert_lines in
    let character =
      if replace.insert_lines = 0 then
        replace.remove_range.start.character
        + replace.insert_chars_on_final_line
      else
        replace.insert_chars_on_final_line
    in
    let start = { line; character } in
    let end_ = update_pos_due_to_prior_replace squiggle.end_ replace in
    Some { start; end_ }
  | Selection_covers_whole_squiggle -> None
  | Selection_in_middle_of_squiggle ->
    let start = squiggle.start in
    let end_ = update_pos_due_to_prior_replace squiggle.end_ replace in
    Some { start; end_ }
  | Selection_overlaps_end_of_squiggle ->
    let start = squiggle.start in
    let end_ = replace.remove_range.start in
    Some { start; end_ }
  | Selection_after_end_of_squiggle -> Some squiggle

(* Moves all diagnostics in response to an LSP change.
 * The change might insert text before a diagnostic squiggle (so the squiggle
 * has to be moved down or to the right); it might delete text before the squiggle;
 * it might modify text inside the squiggle; it might replace text that overlaps
 * the squiggle in which case the squiggle gets truncated/moved; it might replace
 * the squiggle in its entirety in which case the squiggle gets removed.
 * Note that an LSP change is actually a set of changes, applied in order. *)
let update_diagnostics_due_to_change
    (diagnostics : PublishDiagnostics.diagnostic list)
    (change : Lsp.DidChange.params) : PublishDiagnostics.diagnostic list =
  PublishDiagnostics.(
    let replace_of_change change =
      match change.DidChange.range with
      | None -> None
      | Some remove_range ->
        let offset = String.length change.DidChange.text in
        let pos =
          File_content.offset_to_position change.DidChange.text offset
        in
        (* 1-based *)
        let insert_lines = pos.File_content.line - 1 in
        let insert_chars_on_final_line = pos.File_content.column - 1 in
        Some { remove_range; insert_lines; insert_chars_on_final_line }
    in
    let apply_replace diagnostic_opt replace_opt =
      match (diagnostic_opt, replace_opt) with
      | (Some diagnostic, Some replace) ->
        let range = update_range_due_to_replace diagnostic.range replace in
        Option.map range ~f:(fun range -> { diagnostic with range })
      | _ -> None
    in
    let replaces =
      Base.List.map change.DidChange.contentChanges ~f:replace_of_change
    in
    let apply_all_replaces diagnostic =
      Base.List.fold replaces ~init:(Some diagnostic) ~f:apply_replace
    in
    Base.List.filter_map diagnostics ~f:apply_all_replaces)

(************************************************************************)
(* Accessors                                                            *)
(************************************************************************)

let get_root (p : Lsp.Initialize.params) : string =
  Lsp.Initialize.(
    match (p.rootUri, p.rootPath) with
    | (Some uri, _) -> lsp_uri_to_path uri
    | (None, Some path) -> path
    | (None, None) -> failwith "Initialize params missing root")

let supports_status (p : Lsp.Initialize.params) : bool =
  Lsp.Initialize.(p.client_capabilities.window.status)

let supports_snippets (p : Lsp.Initialize.params) : bool =
  Lsp.Initialize.(
    p.client_capabilities.textDocument.completion.completionItem.snippetSupport)

let supports_connectionStatus (p : Lsp.Initialize.params) : bool =
  Lsp.Initialize.(p.client_capabilities.telemetry.connectionStatus)

(************************************************************************)
(* Wrappers for some LSP methods                                        *)
(************************************************************************)

let telemetry
    (writer : Jsonrpc.writer)
    (type_ : MessageType.t)
    (extras : (string * Hh_json.json) list)
    (message : string) : unit =
  let params = { LogMessage.type_; message } in
  let notification = TelemetryNotification (params, extras) in
  notification |> print_lsp_notification |> writer

let telemetry_error
    (writer : Jsonrpc.writer)
    ?(extras : (string * Hh_json.json) list = [])
    (message : string) : unit =
  telemetry writer MessageType.ErrorMessage extras message

let telemetry_log
    (writer : Jsonrpc.writer)
    ?(extras : (string * Hh_json.json) list = [])
    (message : string) : unit =
  telemetry writer MessageType.LogMessage extras message

let log (writer : Jsonrpc.writer) (type_ : MessageType.t) (message : string) :
    unit =
  let params = { LogMessage.type_; message } in
  let notification = LogMessageNotification params in
  notification |> print_lsp_notification |> writer

let log_error (writer : Jsonrpc.writer) = log writer MessageType.ErrorMessage

let log_warning (writer : Jsonrpc.writer) =
  log writer MessageType.WarningMessage

let log_info (writer : Jsonrpc.writer) = log writer MessageType.InfoMessage

let showMessage
    (writer : Jsonrpc.writer) (type_ : MessageType.t) (message : string) : unit
    =
  let params = { ShowMessage.type_; message } in
  let notification = ShowMessageNotification params in
  notification |> print_lsp_notification |> writer

let showMessage_info (writer : Jsonrpc.writer) =
  showMessage writer MessageType.InfoMessage

let showMessage_warning (writer : Jsonrpc.writer) =
  showMessage writer MessageType.WarningMessage

let showMessage_error (writer : Jsonrpc.writer) =
  showMessage writer MessageType.ErrorMessage

let title_of_command_or_action =
  Lsp.CodeAction.(
    function
    | Command Command.{ title; _ } -> title
    | Action { title; _ } -> title)
