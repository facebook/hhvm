(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open String_utils

module EditableTrivia = Full_fidelity_editable_trivia
module TriviaKind = Full_fidelity_trivia_kind
module SyntaxTree = Full_fidelity_syntax_tree
module PositionedSyntax = Full_fidelity_positioned_syntax
module SynUtil = Full_fidelity_syntax_utilities.WithSyntax(PositionedSyntax)
module SourceText = Full_fidelity_source_text

let parse_flags flag_comment =
  let args_regex = Str.regexp "[A-Za-z-_]+ [1-9][0-9]* [1-9][0-9]*" in
  try
    let _ = Str.search_forward args_regex flag_comment 0 in
    let raw_flags = Str.matched_string flag_comment in
    match split ' ' raw_flags with
    | [k; r; c] -> Some (k, int_of_string r, int_of_string c)
    | _ -> None
  with
    Not_found -> None

let process_node acc n =
  match acc with
  | Some flags -> Some flags
  | None ->
    let comments = PositionedSyntax.leading_text n in
    parse_flags comments

let auto_complete filename =
  let file = Relative_path.create Relative_path.Dummy filename in
  let source_text = SourceText.from_file file in
  let syntax_tree = SyntaxTree.make source_text in
  let syntax_tree_root = PositionedSyntax.from_tree syntax_tree in
  match SynUtil.fold process_node None syntax_tree_root with
  | None -> failwith "Invalid test file: no flags found"
  | Some a -> a
