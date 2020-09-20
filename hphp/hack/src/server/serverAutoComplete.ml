(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(*****************************************************************************)
(* Code for auto-completion *)
(*****************************************************************************)

let context_xhp_classname_regex = Str.regexp ".*<[a-zA-Z_0-9:]*$"

let context_xhp_member_regex = Str.regexp ".*->[a-zA-Z_0-9:]*$"

(* For identifying case statements from text context *)
let context_after_single_colon_regex = Str.regexp ".*[a-zA-Z_0-9\"']:$"

(* For identifying user attributes *)
let context_after_double_right_angle_bracket_regex =
  Str.regexp ".*[a-zA-z_0-9\"' ,)]>>$"

(* For identifying shape keys *)
let context_after_quote = Str.regexp ".*['\"]$"

let get_autocomplete_context
    ~(file_content : string)
    ~(pos : File_content.position)
    ~(is_manually_invoked : bool) :
    AutocompleteTypes.legacy_autocomplete_context =
  (* This function retrieves the current line of text up to the position,   *)
  (* and determines whether it's something like "<nt:te" or "->:attr".      *)
  (* This is a dumb implementation. Would be better to replace it with FFP. *)
  if pos.File_content.column = 1 then
    {
      AutocompleteTypes.is_manually_invoked;
      is_xhp_classname = false;
      is_instance_member = false;
      is_after_single_colon = false;
      is_after_double_right_angle_bracket = false;
      is_after_open_square_bracket = false;
      is_after_quote = false;
      is_before_apostrophe = false;
      char_at_pos = ' ';
    }
  else
    let pos_start = { pos with File_content.column = 1 } in
    let (offset_start, offset) =
      File_content.get_offsets file_content (pos_start, pos)
    in
    let leading_text =
      String.sub file_content offset_start (offset - offset_start)
    in
    (* text is the text from the start of the line up to the caret position *)
    let is_xhp_classname =
      Str.string_match context_xhp_classname_regex leading_text 0
    in
    let is_instance_member =
      Str.string_match context_xhp_member_regex leading_text 0
    in
    let is_after_single_colon =
      Str.string_match context_after_single_colon_regex leading_text 0
    in
    let is_after_double_right_angle_bracket =
      Str.string_match
        context_after_double_right_angle_bracket_regex
        leading_text
        0
    in
    let is_after_open_square_bracket =
      String.length leading_text >= 1
      && String.equal (Str.last_chars leading_text 1) "["
    in
    let is_after_quote = Str.string_match context_after_quote leading_text 0 in
    (* Detect what comes next *)
    let char_at_pos =
      try file_content.[offset + AutocompleteTypes.autocomplete_token_length]
      with _ -> ' '
    in
    let is_before_apostrophe = Char.equal char_at_pos '\'' in
    {
      AutocompleteTypes.is_manually_invoked;
      is_xhp_classname;
      is_instance_member;
      is_after_single_colon;
      is_after_double_right_angle_bracket;
      is_after_open_square_bracket;
      is_after_quote;
      is_before_apostrophe;
      char_at_pos;
    }

(** Call this function if you have an edited text file that ALREADY INCLUDES "AUTO332" *)
let go_at_auto332_ctx
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(sienv : SearchUtils.si_env)
    ~(autocomplete_context : AutocompleteTypes.legacy_autocomplete_context) :
    AutocompleteTypes.complete_autocomplete_result list
    Utils.With_complete_flag.t =
  AutocompleteService.go_ctx ~ctx ~entry ~sienv ~autocomplete_context

(** Call this function if you have a raw text file that DOES NOT have "AUTO332" in it *)
let go_ctx
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(sienv : SearchUtils.si_env)
    ~(is_manually_invoked : bool)
    ~(line : int)
    ~(column : int) : AutocompleteTypes.ide_result =
  let open File_content in
  (* We have to edit the file content to add the text AUTO332.
    TODO: Switch to FFP Autocomplete to avoid doing this file edit *)
  let file_content = Provider_context.read_file_contents_exn entry in
  let pos = { line; column } in
  let edits =
    [
      {
        range = Some { st = pos; ed = pos };
        text = AutocompleteTypes.autocomplete_token;
      };
    ]
  in
  let auto332_content = File_content.edit_file_unsafe file_content edits in
  let (modified_auto332_context, modified_auto332_entry) =
    Provider_context.add_or_overwrite_entry_contents
      ~ctx
      ~path:entry.Provider_context.path
      ~contents:auto332_content
  in
  let autocomplete_context =
    get_autocomplete_context
      ~file_content:auto332_content
      ~pos
      ~is_manually_invoked
  in
  let result =
    go_at_auto332_ctx
      ~ctx:modified_auto332_context
      ~entry:modified_auto332_entry
      ~sienv
      ~autocomplete_context
  in
  {
    AutocompleteTypes.completions = result.Utils.With_complete_flag.value;
    char_at_pos = autocomplete_context.AutocompleteTypes.char_at_pos;
    is_complete = result.Utils.With_complete_flag.is_complete;
  }
