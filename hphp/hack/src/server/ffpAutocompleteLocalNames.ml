(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module EditableSyntax = Full_fidelity_editable_syntax
module MinimalSyntax = Full_fidelity_minimal_syntax
module SyntaxTree = Full_fidelity_syntax_tree
open FfpAutocompleteContextParser
open Core
open String_utils

(* TODO: Ensure this covers all cases *)
(* TODO: Add in "$this" when it's valid *)
let local_variable_valid_in_context (context:context) (stub:string) : bool =
  let open ContextPredicates in
  is_expression_valid context &&
  (stub = "" || string_starts_with stub "$")

(* As of June 2017, the lambda analyzer only works with editable syntax trees
   for no specific reason. In the future, if the lambda analyzer is
   functorized to accept any tree with the ability to get the text of a
   token, this may be able to accept something other than a minimal tree that
   is converted to an editable tree. *)
let autocomplete_local (context:context) (stub:string) (tree:SyntaxTree.t)
  (offset:int) : string list =
  if local_variable_valid_in_context context stub then
    let open EditableSyntax in
    let editable_tree = from_tree tree in
    let parents = parentage editable_tree offset in
    let local_vars = List.fold_left parents
      ~f:Lambda_analyzer.local_variables
      ~init:SSet.empty
    in
    SSet.elements local_vars
  else
    []
