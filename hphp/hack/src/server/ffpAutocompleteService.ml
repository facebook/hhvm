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
open Core

module EditableTrivia = Full_fidelity_editable_trivia
module TriviaKind = Full_fidelity_trivia_kind
module TokenKind = Full_fidelity_token_kind
module SyntaxKind = Full_fidelity_syntax_kind
module SyntaxTree = Full_fidelity_syntax_tree
module PositionedSyntax = Full_fidelity_positioned_syntax
module PositionedToken = Full_fidelity_positioned_token
module SynUtil = Full_fidelity_syntax_utilities.WithSyntax(PositionedSyntax)
module SourceText = Full_fidelity_source_text
module ACKeyword = FfpAutocompleteKeywords

let print_parse_tree_descent_debug node position =
  let open PositionedSyntax in
  let parents = parentage node position in
  let () = Printf.printf "%s\n"
    (Hh_json.json_to_string @@ to_json node) in
  let print_node idx node = Printf.printf "%d. %s\n" idx
    (SyntaxKind.to_string @@ kind node)
  in
  List.iteri ~f:print_node (List.rev parents)

let get_parentage_and_predecessor root position =
  (*let () = print_parse_tree_descent_debug root position in*)
  let rec get_next_child ~current_level ~position ~parentage ~predecessor =
    match current_level with
    | [] -> (parentage, predecessor)
    | h :: t ->
      let width = PositionedSyntax.full_width h in
      if position < width then
        get_next_child ~current_level:(PositionedSyntax.children h) ~position
          ~parentage:(h :: parentage) ~predecessor
      else
        let prev = if PositionedSyntax.kind h = SyntaxKind.Missing
          then predecessor
          else (Some h)
        in
        get_next_child ~current_level:t ~position:(position - width) ~parentage
          ~predecessor:prev
  in
  get_next_child ~current_level:[root] ~position ~parentage:[] ~predecessor:None

let get_autocomplete_position syntax_tree =
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
  in
  let process_node acc n =
    match acc with
    | Some flags -> Some flags
    | None -> parse_flags @@ PositionedSyntax.leading_text n
  in
  match SynUtil.fold process_node None syntax_tree with
  | None -> failwith "Invalid test file: no flags found"
  | Some a -> a

let autocomplete_word context stub predecessor =
  let result = ACKeyword.autocomplete_keyword ~context ~predecessor ~stub in
  match result with
  | [] -> None
  | x -> Some x

let create_syntax_tree_from_file filename =
  let file = Relative_path.create Relative_path.Dummy filename in
  let source_text = SourceText.from_file file in
  let syntax_tree = SyntaxTree.make source_text in
  PositionedSyntax.from_tree syntax_tree

let auto_complete filename =
  let syntax_tree_root = create_syntax_tree_from_file filename in
  let (_, row, col) = get_autocomplete_position syntax_tree_root in
  let offset = SourceText.position_to_offset
    (PositionedSyntax.source_text syntax_tree_root) (row, col) in
  let (parents, autocomplete_predecessor) =
    get_parentage_and_predecessor syntax_tree_root offset in
  let autocomplete_child = List.hd_exn parents in
  let autocomplete_target = PositionedSyntax.text autocomplete_child in
  autocomplete_word (List.rev parents)
    autocomplete_target autocomplete_predecessor
