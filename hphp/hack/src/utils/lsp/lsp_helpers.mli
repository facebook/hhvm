(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* This `.mli` file was generated automatically. It may include extra
   definitions that should not actually be exposed to the caller. If you notice
   that this interface file is a poor interface, please take a few minutes to
   clean it up manually, and then delete this comment once the interface is in
   shape. *)

type range_replace = {
  remove_range: Lsp.range;
  insert_lines: int;
  insert_chars_on_final_line: int;
}

val progress_and_actionRequired_counter : int ref

val url_scheme_regex : Str.regexp

val lsp_uri_to_path : Lsp.DocumentUri.t -> string

(** If given a relative path with a dummy prefix, the file
uri will include a fake prefix like '/dummy_from_path_to_lsp_uri/' *)
val path_to_lsp_uri : Relative_path.t -> Lsp.DocumentUri.t

(** prefer [path_to_lsp_uri] *)
val path_string_to_lsp_uri : string -> default_path:string -> Lsp.DocumentUri.t

val lsp_textDocumentIdentifier_to_filename :
  Lsp.TextDocumentIdentifier.t -> string

val lsp_position_to_fc : Lsp.position -> File_content.position

val lsp_range_to_fc : Lsp.range -> File_content.range

val lsp_range_to_pos :
  line_to_offset:(int -> int) -> Relative_path.t -> Lsp.range -> Pos.t

val lsp_edit_to_fc :
  Lsp.DidChange.textDocumentContentChangeEvent -> File_content.text_edit

val apply_changes :
  string ->
  Lsp.DidChange.textDocumentContentChangeEvent list ->
  (string, string * Exception.t) result

val get_char_from_lsp_position : string -> Lsp.position -> char

val apply_changes_unsafe :
  string -> Lsp.DidChange.textDocumentContentChangeEvent list -> string

val sym_occ_kind_to_lsp_sym_info_kind :
  SymbolOccurrence.kind -> Lsp.SymbolInformation.symbolKind

(** Correctly handles our various positions:
  * - real positions
  * - .hhconfig error positions, which have dummy start/ends
  * - and [Pos.none]
  * Special handling is required, as the LSP
  * specification requires line and character >= 0, and VSCode silently
  * drops diagnostics that violate the spec in this way *)
val hack_pos_to_lsp_range : equal:('a -> 'a -> bool) -> 'a Pos.pos -> Lsp.range

(** You probably want [hack_pos_to_lsp_range].
 * Equivalent to `[hack_pos_to_lsp_range] sans handling of special positions and with the following transformation:
 * {r with start = {line = r.start.line + 1; character = r.start.character + 1}; end_ = {r.end_ with line = r.end_.line + 1}}
 * where `r` is a range produced by [hack_pos_to_lsp_range] *)
val hack_pos_to_lsp_range_adjusted : 'a Pos.pos -> Lsp.range

val symbol_to_lsp_call_item :
  Relative_path.t SymbolOccurrence.t ->
  Relative_path.t SymbolDefinition.t option ->
  Lsp.CallHierarchyItem.t

val pos_compare : Lsp.position -> Lsp.position -> int

type range_overlap =
  | Selection_before_start_of_squiggle
  | Selection_overlaps_start_of_squiggle
  | Selection_covers_whole_squiggle
  | Selection_in_middle_of_squiggle
  | Selection_overlaps_end_of_squiggle
  | Selection_after_end_of_squiggle

val get_range_overlap : Lsp.range -> Lsp.range -> range_overlap

val update_pos_due_to_prior_replace :
  Lsp.position -> range_replace -> Lsp.position

val update_range_due_to_replace : Lsp.range -> range_replace -> Lsp.range option

val update_diagnostics_due_to_change :
  Lsp.PublishDiagnostics.diagnostic list ->
  Lsp.DidChange.params ->
  Lsp.PublishDiagnostics.diagnostic list

val get_root : Lsp.Initialize.params -> string

val supports_status : Lsp.Initialize.params -> bool

val supports_snippets : Lsp.Initialize.params -> bool

val supports_connectionStatus : Lsp.Initialize.params -> bool

val telemetry :
  Jsonrpc.writer ->
  Lsp.MessageType.t ->
  (string * Hh_json.json) list ->
  string ->
  unit

val telemetry_error :
  Jsonrpc.writer -> ?extras:(string * Hh_json.json) list -> string -> unit

val telemetry_log :
  Jsonrpc.writer -> ?extras:(string * Hh_json.json) list -> string -> unit

val log : Jsonrpc.writer -> Lsp.MessageType.t -> string -> unit

val log_error : Jsonrpc.writer -> string -> unit

val log_warning : Jsonrpc.writer -> string -> unit

val log_info : Jsonrpc.writer -> string -> unit

val showMessage_info : Jsonrpc.writer -> string -> unit

val showMessage_warning : Jsonrpc.writer -> string -> unit

val showMessage_error : Jsonrpc.writer -> string -> unit

val title_of_command_or_action : 'a Lsp.CodeAction.command_or_action_ -> string
