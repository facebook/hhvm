(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module PositionedSyntax = Full_fidelity_positioned_syntax
module SourceText = Full_fidelity_source_text
module SyntaxKind = Full_fidelity_syntax_kind
module SyntaxTree = Full_fidelity_syntax_tree.WithSyntax (PositionedSyntax)
module TokenKind = Full_fidelity_token_kind
open Core_kernel
open AutocompleteTypes

let empty_autocomplete_token = "PLACEHOLDER"

let make_keyword_completion
    (replace_pos : Ide_api_types.range) (keyword_name : string) =
  {
    res_pos = Pos.none |> Pos.to_absolute;
    res_replace_pos = replace_pos;
    res_base_class = None;
    res_ty = "keyword";
    res_name = keyword_name;
    res_fullname = keyword_name;
    res_kind = SearchUtils.SI_Keyword;
    func_details = None;
    ranking_details = None;
  }

let handle_empty_autocomplete (pos : File_content.position) file_content =
  File_content.(
    let offset = File_content.get_offset file_content pos in
    let prev_char = File_content.get_char file_content (offset - 1) in
    let next_char = File_content.get_char file_content offset in
    let is_whitespace = function
      | ' '
      | '\n'
      | '\r'
      | '\t' ->
        true
      | _ -> false
    in
    if is_whitespace prev_char && is_whitespace next_char then
      let edits =
        [
          {
            range = Some { st = pos; ed = pos };
            text = empty_autocomplete_token;
          };
        ]
      in
      File_content.edit_file_unsafe file_content edits
    else
      file_content)

let auto_complete_all_completion_types
    ~(old_ctx : Provider_context.t)
    ~(old_entry : Provider_context.entry)
    ~(new_ctx : Provider_context.t)
    ~(new_entry : Provider_context.entry)
    ~(pos : File_content.position)
    ~(filter_by_token : bool)
    ~(sienv : SearchUtils.si_env) : result =
  let source_text = Ast_provider.compute_source_text ~entry:new_entry in
  let offset =
    SourceText.position_to_offset
      source_text
      (pos.File_content.line, pos.File_content.column)
  in
  let syntax_tree = Ast_provider.compute_cst ~ctx:new_ctx ~entry:new_entry in
  let positioned_tree = SyntaxTree.root syntax_tree in
  let replace_pos =
    let syntax =
      List.hd_exn (PositionedSyntax.parentage positioned_tree offset)
    in
    let (start_line, start_col) =
      SourceText.offset_to_position
        source_text
        (PositionedSyntax.start_offset syntax)
    in
    let (end_line, end_col) =
      SourceText.offset_to_position
        source_text
        (PositionedSyntax.end_offset syntax)
    in
    {
      Ide_api_types.st = { Ide_api_types.line = start_line; column = start_col };
      ed = { Ide_api_types.line = end_line; column = end_col };
    }
  in
  let (context, stub) =
    FfpAutocompleteContextParser.get_context_and_stub positioned_tree offset
  in
  (* If we are running a test, filter the keywords and local variables based on
  the token we are completing. *)
  let stub =
    if
      old_entry.Provider_context.contents <> new_entry.Provider_context.contents
    then
      String_utils.rstrip stub empty_autocomplete_token
    else
      stub
  in
  let filter_results res =
    List.filter res ~f:(fun res ->
        if filter_by_token then
          String_utils.string_starts_with res.res_name stub
        else
          true)
  in
  (* Delegate to each type of completion to determine whether or not that
     type is valid in the current context *)
  let keyword_completions =
    FfpAutocompleteKeywords.autocomplete_keyword context
    |> List.map ~f:(make_keyword_completion replace_pos)
  in
  let type_based_completions =
    FfpAutocompleteTypeCheck.run
      ~context
      ~ctx:old_ctx
      ~entry:old_entry
      ~stub
      ~pos
      ~sienv
  in
  let global_completions =
    FfpAutocompleteGlobals.get_globals
      sienv
      context
      stub
      positioned_tree
      replace_pos
  in
  [keyword_completions; type_based_completions; global_completions]
  |> List.concat_no_order
  |> filter_results
  |> List.sort ~compare:(fun a b -> compare a.res_name b.res_name)
  |> List.remove_consecutive_duplicates ~equal:(fun a b ->
         a.res_name = b.res_name)

let auto_complete
    (ctx : Provider_context.t)
    (entry : Provider_context.entry)
    (pos : File_content.position)
    ~(filter_by_token : bool)
    ~(sienv : SearchUtils.si_env) : result =
  (* Some code paths require the empty autocomplete token to be present, and
  some require that it isn't, so make a separate entry for each scenario. *)
  let old_file_content = entry.Provider_context.contents in
  let (old_ctx, old_entry) =
    Provider_context.add_entry_from_file_contents
      ~ctx
      ~path:entry.Provider_context.path
      ~contents:old_file_content
  in
  (* The part of the line from the far left end to the point where the caret is. *)
  let new_file_content = handle_empty_autocomplete pos old_file_content in
  let (new_ctx, new_entry) =
    Provider_context.add_entry_from_file_contents
      ~ctx
      ~path:entry.Provider_context.path
      ~contents:new_file_content
  in
  auto_complete_all_completion_types
    ~old_ctx
    ~old_entry
    ~new_ctx
    ~new_entry
    ~pos
    ~filter_by_token
    ~sienv
