(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Core

module TokenKind = Full_fidelity_token_kind
module SyntaxTree = Full_fidelity_syntax_tree
module MinimalSyntax = Full_fidelity_minimal_syntax
module PositionedSyntax = Full_fidelity_positioned_syntax
module SourceText = Full_fidelity_source_text
module SyntaxKind = Full_fidelity_syntax_kind

(* The type returned to the client *)
(* TODO: Add all the additional autocomplete info to this,
   not just the autocomplete word *)
type autocomplete_result = {
  name : string;
}

type result = autocomplete_result list

(* TODO: Return autocomplete_results from each type of autocomplete directly
   instead of wrapping them here *)
let make_result (res:string) : autocomplete_result =
  { name = res }

let auto_complete
  (tcopt:TypecheckerOptions.t)
  (file_content:string)
  (pos:Ide_api_types.position) : result =
  let source_text = SourceText.make file_content in
  let syntax_tree = SyntaxTree.make source_text in

  let open Ide_api_types in
  let position_tuple = (pos.line, pos.column) in
  let offset = SourceText.position_to_offset source_text position_tuple in
  let (context, stub) =
    FfpAutocompleteContextParser.get_context_and_stub syntax_tree offset in
  (* Delegate to each type of completion to determine whether or not that
     type is valid in the current context *)
  let keywords = FfpAutocompleteKeywords.autocomplete_keyword context stub in
  let local_vars =
    FfpAutocompleteLocalNames.autocomplete_local context stub syntax_tree offset
  in
  let class_members =
    FfpAutocompleteClassMembers.autocomplete_class_member ~pos ~file_content ~tcopt
  in
  let results = keywords @ local_vars @ class_members in
  results
    |> List.sort ~cmp:Pervasives.compare
    |> List.remove_consecutive_duplicates ~equal:(=)
    |> List.map ~f:make_result
