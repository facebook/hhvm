(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type snippet_with_fallback = {
  snippet: string;
  fallback: string;
}

(** Whether the inserted text should be treated as literal text or a template with placeholders.

    https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#insertTextFormat *)
type insert_text =
  | InsertLiterally of string
  | InsertAsSnippet of snippet_with_fallback

type autocomplete_item = {
  (* The position of the declaration we're returning. *)
  res_decl_pos: Pos.absolute;
  (* The position in the opened file that we're replacing with res_insert_text. *)
  res_replace_pos: Ide_api_types.range;
  (* If we're autocompleting a method, store the class name of the variable
        we're calling the method on (for doc block fallback in autocomplete
        resolution). *)
  res_base_class: string option;
  (* These strings correspond to the LSP fields in CompletionItem. *)
  res_label: string;
  res_insert_text: insert_text;
  res_detail: string;
  res_filter_text: string option;
  res_additional_edits: (string * Ide_api_types.range) list;
  (* res_fullname is res_label without trimming the namespace. *)
  res_fullname: string;
  res_kind: FileInfo.si_kind;
  (* documentation (in markdown); if absent, then it will be resolved on-demand later *)
  res_documentation: string option;
  (* res_sortText is the string used to compare or order for autocomplete results  *)
  res_sortText: string option;
}

(* The type returned to the client *)
type ide_result = {
  completions: autocomplete_item list;
  char_at_pos: char;
  is_complete: bool;
}

type result = autocomplete_item list

type legacy_autocomplete_context = {
  is_manually_invoked: bool;
  is_after_single_colon: bool;
  is_after_double_right_angle_bracket: bool;
  is_after_open_square_bracket: bool;
  is_after_quote: bool;
  is_before_apostrophe: bool;
  is_open_curly_without_equals: bool;
  char_at_pos: char;
}

(* The standard autocomplete token, which is currently "AUTO332" *)
val autocomplete_token : string

(* The length of the standard autocomplete token *)
val autocomplete_token_length : int
