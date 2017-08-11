(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

module PositionedSyntax = Full_fidelity_positioned_syntax
module SourceText = Full_fidelity_source_text
module SyntaxKind = Full_fidelity_syntax_kind
module SyntaxTree = Full_fidelity_syntax_tree
module TokenKind = Full_fidelity_token_kind

open Core
open AutocompleteTypes

let empty_autocomplete_token = "PLACEHOLDER"

let make_keyword_completion (keyword_name:string) =
  {
    res_pos = Pos.none |> Pos.to_absolute;
    res_ty = "keyword";
    res_name = keyword_name;
    res_kind = Keyword_kind;
    func_details = None;
  }

let handle_empty_autocomplete pos file_content =
  let open Ide_api_types in
  let offset = File_content.get_offset file_content pos in
  let prev_char = File_content.get_char file_content (offset-1) in
  let next_char = File_content.get_char file_content offset in
  let is_whitespace = function ' ' | '\n' | '\r' | '\t' -> true | _ -> false in
  if is_whitespace prev_char && is_whitespace next_char then
    let edits = [{range = Some {st = pos; ed = pos}; text = empty_autocomplete_token}] in
    File_content.edit_file_unsafe file_content edits
  else
    file_content

let auto_complete
  (tcopt:TypecheckerOptions.t)
  (file_content:string)
  (pos:Ide_api_types.position)
  ~(filter_by_token:bool) : result =
  let open Ide_api_types in
  let new_file_content = handle_empty_autocomplete pos file_content in
  let source_text = SourceText.make new_file_content in
  let offset = SourceText.position_to_offset source_text (pos.line, pos.column) in
  let syntax_tree = SyntaxTree.make source_text in
  let positioned_tree = PositionedSyntax.from_tree syntax_tree in

  let (context, stub) = FfpAutocompleteContextParser.get_context_and_stub positioned_tree offset in
  (* If we are running a test, filter the keywords and local variables based on
  the token we are completing. *)
  let stub = if file_content <> new_file_content then
      String_utils.rstrip stub empty_autocomplete_token
    else
      stub
  in
  let filter_results res = List.filter res ~f:begin fun res ->
    if filter_by_token
    then String_utils.string_starts_with res.res_name stub
    else true
  end in
  (* Delegate to each type of completion to determine whether or not that
     type is valid in the current context *)
  let keyword_completions =
    FfpAutocompleteKeywords.autocomplete_keyword context
    |> List.map ~f:make_keyword_completion
  in
  let type_based_completions =
    FfpAutocompleteTypeCheck.run ~context ~file_content ~stub ~pos ~tcopt
  in
  let global_completions =
    FfpAutocompleteGlobals.get_globals context stub positioned_tree
  in
  [keyword_completions; type_based_completions; global_completions]
  |> List.concat_no_order
  |> filter_results
  |> List.sort ~cmp:(fun a b -> compare a.res_name b.res_name)
  |> List.remove_consecutive_duplicates ~equal:(fun a b -> a.res_name = b.res_name)
