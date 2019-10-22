(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Code for auto-completion *)
(*****************************************************************************)

let get_results
    ~(tcopt : TypecheckerOptions.t)
    ~(autocomplete_context : AutocompleteTypes.legacy_autocomplete_context)
    ~(sienv : SearchUtils.si_env)
    (_ : Relative_path.t)
    (_ : FileInfo.t)
    (tast : Tast.program) :
    AutocompleteTypes.complete_autocomplete_result list
    Utils.With_complete_flag.t =
  AutocompleteService.go tast ~tcopt ~autocomplete_context ~sienv

let auto_complete
    ~(tcopt : TypecheckerOptions.t)
    ~(autocomplete_context : AutocompleteTypes.legacy_autocomplete_context)
    ~(sienv : SearchUtils.si_env)
    (content : string) :
    AutocompleteTypes.complete_autocomplete_result list
    Utils.With_complete_flag.t =
  let result =
    ServerIdeUtils.declare_and_check
      content
      ~f:(get_results ~tcopt ~autocomplete_context ~sienv)
      tcopt
  in
  result

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
    (content : string)
    (pos : File_content.position)
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
    }
  else
    let pos_start = { pos with File_content.column = 1 } in
    let (offset_start, offset) =
      File_content.get_offsets content (pos_start, pos)
    in
    let leading_text =
      String.sub content offset_start (offset - offset_start)
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
      String.length leading_text >= 1 && Str.last_chars leading_text 1 = "["
    in
    let is_after_quote = Str.string_match context_after_quote leading_text 0 in
    (* Detect what comes next *)
    let next_character =
      (try String.sub content (offset + 7) 1 with _ -> "")
    in
    let is_before_apostrophe = next_character = "'" in
    {
      AutocompleteTypes.is_manually_invoked;
      is_xhp_classname;
      is_instance_member;
      is_after_single_colon;
      is_after_double_right_angle_bracket;
      is_after_open_square_bracket;
      is_after_quote;
      is_before_apostrophe;
    }

let auto_complete_at_position
    ~(is_manually_invoked : bool)
    ~(file_content : string)
    ~(pos : File_content.position)
    ~(tcopt : TypecheckerOptions.t)
    ~(sienv : SearchUtils.si_env) :
    AutocompleteTypes.complete_autocomplete_result list
    Utils.With_complete_flag.t =
  File_content.(
    (* TODO: Avoid doing the "AUTO332" thing by modifying autocomplete service to accept a position *)
    let autocomplete_context =
      get_autocomplete_context file_content pos ~is_manually_invoked
    in
    let edits = [{ range = Some { st = pos; ed = pos }; text = "AUTO332" }] in
    let content = File_content.edit_file_unsafe file_content edits in
    auto_complete ~tcopt ~autocomplete_context ~sienv content)
