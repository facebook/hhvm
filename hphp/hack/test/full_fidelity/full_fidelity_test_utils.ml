(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module SyntaxKind = Full_fidelity_syntax_kind
module SyntaxTree = Full_fidelity_syntax_tree
module TriviaKind = Full_fidelity_trivia_kind
module TokenKind = Full_fidelity_token_kind
module MinimalSyntax = Full_fidelity_minimal_syntax
module MinimalToken = Full_fidelity_minimal_token
module MinimalTrivia = Full_fidelity_minimal_trivia
module Rewriter = Full_fidelity_rewriter.WithSyntax(MinimalSyntax)

open Core

let identity x = x

let rewrite_tree_no_trivia node =
  let rewrite n =
    match MinimalSyntax.syntax n with
    | MinimalSyntax.Token t ->
      let kind = MinimalToken.kind t in
      let width = MinimalToken.width t in
      let token = MinimalToken.make kind width [] [] in
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
        (width t)
        (filter_whitespace (leading t))
        (filter_whitespace (trailing t))
      ) in
      Rewriter.Replace (MinimalSyntax.make_token token)
    | _ -> Rewriter.Keep in
  Rewriter.rewrite_post rewrite node

let minimal_trivia_to_string trivia =
  let name = TriviaKind.to_string (MinimalTrivia.kind trivia) in
  Printf.sprintf "(%s)" name

let minimal_trivia_list_to_string trivia_list =
  String.concat "" (List.map trivia_list ~f:minimal_trivia_to_string)

let minimal_token_to_string token =
  let leading = minimal_trivia_list_to_string (MinimalToken.leading token) in
  let name = TokenKind.to_string (MinimalToken.kind token) in
  let name =
    if name = "(" then "lparen"
    else if name = ")" then "rparen"
    else name in
  let trailing = minimal_trivia_list_to_string (MinimalToken.trailing token) in
  Printf.sprintf "(%s(%s)%s)" leading name trailing

let rec minimal_to_string node =
  match MinimalSyntax.syntax node with
  | MinimalSyntax.Token token ->
    minimal_token_to_string token
  | _ ->
    let name = SyntaxKind.to_string (MinimalSyntax.kind node) in
    let children = MinimalSyntax.children node in
    let children = List.map children ~f:minimal_to_string in
    let children = String.concat "" children in
    Printf.sprintf "(%s%s)" name children

let tree_to_string_ignore_trivia tree =
  let root = SyntaxTree.root tree in
  let new_root = rewrite_tree_no_trivia root in
  minimal_to_string new_root
