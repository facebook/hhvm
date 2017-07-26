(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

module MinimalSyntax = Full_fidelity_minimal_syntax
module PositionedSyntax = Full_fidelity_positioned_syntax
module SourceText = Full_fidelity_source_text
module SyntaxKind = Full_fidelity_syntax_kind
module SyntaxTree = Full_fidelity_syntax_tree
module TokenKind = Full_fidelity_token_kind

open Core
open AutocompleteTypes

let make_keyword_completion (keyword_name:string) =
  {
    res_pos = Pos.none |> Pos.to_absolute;
    res_ty = "keyword";
    res_name = keyword_name;
    res_kind = Keyword_kind;
    func_details = None;
  }

let make_local_var_completion (var_name:string) =
  {
    res_pos = Pos.none |> Pos.to_absolute;
    res_ty = ""; (* TODO: Doing local variable completion syntactically means
      that we don't know the type when completing it. Do we care about it? *)
    res_name = var_name;
    res_kind = Variable_kind;
    func_details = None;
  }

let auto_complete
  (tcopt:TypecheckerOptions.t)
  (file_content:string)
  (pos:Ide_api_types.position)
  ~(filter_by_token:bool) : result =
  let source_text = SourceText.make file_content in
  let syntax_tree = SyntaxTree.make source_text in

  let open Ide_api_types in
  let position_tuple = (pos.line, pos.column) in
  let offset = SourceText.position_to_offset source_text position_tuple in
  let (context, stub) =
    FfpAutocompleteContextParser.get_context_and_stub syntax_tree offset in
  (* If we are running a test, filter the keywords and local variables based on
  the token we are completing. *)
  let filter_results res = List.filter res ~f:begin fun res ->
    if filter_by_token
    then String_utils.string_starts_with res stub
    else true
  end in
  (* Delegate to each type of completion to determine whether or not that
     type is valid in the current context *)
  let keyword_completions =
    FfpAutocompleteKeywords.autocomplete_keyword context
    |> filter_results
    |> List.map ~f:make_keyword_completion
  in
  let local_var_completions =
    FfpAutocompleteLocalNames.autocomplete_local context stub syntax_tree offset
    |> filter_results
    |> List.map ~f:make_local_var_completion
  in
  let class_member_completions =
    FfpAutocompleteClassMembers.autocomplete_class_member
    ~context
    ~pos
    ~file_content
    ~tcopt
  in
  keyword_completions @ local_var_completions @ class_member_completions
  |> List.sort ~cmp:(fun a b -> compare a.res_name b.res_name)
