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
module ACKeyword = FfpAutocompleteKeywords
module ACLocal = FfpAutocompleteLocalNames
module SyntaxKind = Full_fidelity_syntax_kind

(*
 * TODO: The following types of completions are not yet implemented:
 * - Function Invocations
 * - :: and \ invocations
 *)
let autocomplete_word (tree:SyntaxTree.t) offset stub =
  let open MinimalSyntax in
  let autocomplete_child =
    List.hd_exn @@ parentage (SyntaxTree.root tree) offset in
  let result = match syntax autocomplete_child with
  (* TODO: Handle function invocation, class name, and other completions here *)
  (* TODO: Add test cases to make sure the right type of completion is taken for a given token *)
  | Token {
      MinimalToken.kind = TokenKind.Name; _
    } -> ACKeyword.autocomplete_keyword tree offset stub
  | Token {
      MinimalToken.kind = TokenKind.Variable; _
    }
  | Token {
      MinimalToken.kind = TokenKind.Dollar; _
    } -> ACLocal.autocomplete_local tree offset
  | Token {
      MinimalToken.kind = TokenKind.MinusGreaterThan (* This token: -> *); _
    } -> [] (* TODO: Not implemented yet *)
  | Token {
      MinimalToken.kind = TokenKind.ColonColon (* This token: :: *); _
    } -> [] (* TODO: Not implemented yet *)
  | Token {
      MinimalToken.kind = TokenKind.NamespacePrefix; _
    } -> [] (* TODO: Not implemented yet *)
  | _ -> [] (* Unimplemented completion type *)
  in
  match result with
  | [] -> None
  | x -> Some x

(* Get the token we wish to complete. This is necessary because the keyword autocompletion filters
   results based on what we have typed so far. This will potentially be removed in the future.*)
let get_autocomplete_stub (syntax_tree:SyntaxTree.t) offset =
  let open PositionedSyntax in
  let positioned_tree = from_tree syntax_tree in
  let autocomplete_child = List.hd_exn @@ parentage positioned_tree offset in
  text autocomplete_child

let auto_complete (file:string) (row, col) =
  let source_text = SourceText.make file in
  let syntax_tree = SyntaxTree.make source_text in

  let offset = SourceText.position_to_offset source_text (row, col) in
  let stub = get_autocomplete_stub syntax_tree offset in
  autocomplete_word syntax_tree offset stub
