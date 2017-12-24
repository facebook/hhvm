(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module SourceText = Full_fidelity_source_text
module SyntaxKind = Full_fidelity_syntax_kind
module SyntaxTree = Full_fidelity_syntax_tree
  .WithSyntax(Full_fidelity_minimal_syntax)
module TriviaKind = Full_fidelity_trivia_kind
module TokenKind = Full_fidelity_token_kind
module MinimalSyntax = Full_fidelity_minimal_syntax
module MinimalToken = Full_fidelity_minimal_token
module MinimalTrivia = Full_fidelity_minimal_trivia
module Rewriter = Full_fidelity_rewriter.WithSyntax(MinimalSyntax)

module EditableSyntax = Full_fidelity_editable_syntax
module EditableToken = Full_fidelity_editable_token
module EditableRewriter = Full_fidelity_rewriter.WithSyntax(EditableSyntax)

open Hh_core

let identity x = x

let rewrite_editable_tree_no_trivia node =
  let trivia = ref [] in
  let rewrite n =
    match EditableSyntax.syntax n with
    | EditableSyntax.Token t ->
      let kind = EditableToken.kind t in
      let text = EditableToken.text t in
      let leading = EditableToken.leading t in
      let trailing = EditableToken.trailing t in
      let token = EditableToken.make kind text [] [] in
      trivia := !trivia @ leading @ trailing;
      EditableRewriter.Replace (EditableSyntax.make_token token)
    | _ -> EditableRewriter.Keep in
  let no_trivia_tree = EditableRewriter.rewrite_post rewrite node in
  (no_trivia_tree, !trivia)

let rewrite_tree_no_trivia node =
  let rewrite n =
    match MinimalSyntax.syntax n with
    | MinimalSyntax.Token t ->
      let kind = MinimalToken.kind t in
      let width = MinimalToken.width t in
      let token = MinimalToken.make kind SourceText.empty 0 width [] [] in
      Rewriter.Replace (MinimalSyntax.make_token token)
    | _ -> Rewriter.Keep in
  Rewriter.rewrite_post rewrite node

let rewrite_tree_no_whitespace node =
  let filter_whitespace trivia_list =
    List.filter
      trivia_list
      ~f:(fun t ->
        match MinimalTrivia.kind t with
          | TriviaKind.ExtraTokenError
          | TriviaKind.FallThrough
          | TriviaKind.Unsafe
          | TriviaKind.IgnoreError
          | TriviaKind.UnsafeExpression
          | TriviaKind.FixMe
          | TriviaKind.SingleLineComment
          | TriviaKind.AfterHaltCompiler
          | TriviaKind.DelimitedComment -> true
          | TriviaKind.EndOfLine
          | TriviaKind.WhiteSpace -> false
      )
  in

  let rewrite n =
    match MinimalSyntax.syntax n with
    | MinimalSyntax.Token t ->
      let token = MinimalToken.(make
        (kind t)
        SourceText.empty
        0
        (width t)
        (filter_whitespace (leading t))
        (filter_whitespace (trailing t))
      ) in
      Rewriter.Replace (MinimalSyntax.make_token token)
    | _ -> Rewriter.Keep in
  Rewriter.rewrite_post rewrite node

let minimal_trivia_to_sexp trivia =
  let name = TriviaKind.to_string (MinimalTrivia.kind trivia) in
  Sexp.List [ Sexp.Atom name ]

let minimal_trivia_list trivia_list =
  List.map trivia_list ~f:minimal_trivia_to_sexp

let minimal_token_to_sexp token =
  let leading = minimal_trivia_list (MinimalToken.leading token) in
  let name = TokenKind.to_string (MinimalToken.kind token) in
  let name =
    if name = "(" then "lparen"
    else if name = ")" then "rparen"
    else name in
  let trailing = minimal_trivia_list (MinimalToken.trailing token) in
  let name = Sexp.List [Sexp.Atom name] in
  Sexp.List (leading @ [name] @ trailing)

let rec minimal_to_sexp node =
  match MinimalSyntax.syntax node with
  | MinimalSyntax.Token token ->
    minimal_token_to_sexp token
  | _ ->
    let name = SyntaxKind.to_string (MinimalSyntax.kind node) in
    let children = MinimalSyntax.children node in
    let children = List.map children ~f:minimal_to_sexp in
    Sexp.List ((Sexp.Atom name) :: children)

let minimal_to_formatted_sexp_string node =
  minimal_to_sexp node
  |> Sexp.to_string_hum

let tree_to_sexp_string_ignore_trivia tree =
  tree
  |> SyntaxTree.root
  |> rewrite_tree_no_trivia
  |> minimal_to_sexp
  |> Sexp.to_string_mach
