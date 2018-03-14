(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module Syntax = Full_fidelity_positioned_syntax
module SyntaxTree = Full_fidelity_syntax_tree.WithSyntax(Syntax)
module Token = Syntax.Token
module Trivia = Token.Trivia
module SourceText = Full_fidelity_source_text
module SyntaxKind = Full_fidelity_syntax_kind
module TriviaKind = Full_fidelity_trivia_kind
module TokenKind = Full_fidelity_token_kind
module Rewriter = Full_fidelity_rewriter.WithSyntax(Syntax)

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
      let token = EditableToken.create kind text [] [] in
      trivia := !trivia @ leading @ trailing;
      EditableRewriter.Replace (EditableSyntax.make_token token)
    | _ -> EditableRewriter.Keep in
  let no_trivia_tree = EditableRewriter.rewrite_post rewrite node in
  (no_trivia_tree, !trivia)

let rewrite_tree_no_trivia node =
  let rewrite n =
    match Syntax.syntax n with
    | Syntax.Token t ->
      let kind = Token.kind t in
      let width = Token.width t in
      let token = Token.make kind SourceText.empty 0 width [] [] in
      Rewriter.Replace (Syntax.make_token token)
    | _ -> Rewriter.Keep in
  Rewriter.rewrite_post rewrite node

let rewrite_tree_no_whitespace node =
  let filter_whitespace trivia_list =
    List.filter
      trivia_list
      ~f:(fun t ->
        match Trivia.kind t with
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
    match Syntax.syntax n with
    | Syntax.Token t ->
      let token = Token.(make
        (kind t)
        SourceText.empty
        0
        (width t)
        (filter_whitespace (leading t))
        (filter_whitespace (trailing t))
      ) in
      Rewriter.Replace (Syntax.make_token token)
    | _ -> Rewriter.Keep in
  Rewriter.rewrite_post rewrite node

let trivia_to_sexp trivia =
  let name = TriviaKind.to_string (Trivia.kind trivia) in
  Sexp.List [ Sexp.Atom name ]

let trivia_list trivia_list =
  List.map trivia_list ~f:trivia_to_sexp

let token_to_sexp token =
  let leading = trivia_list (Token.leading token) in
  let name = TokenKind.to_string (Token.kind token) in
  let name =
    if name = "(" then "lparen"
    else if name = ")" then "rparen"
    else name in
  let trailing = trivia_list (Token.trailing token) in
  let name = Sexp.List [Sexp.Atom name] in
  Sexp.List (leading @ [name] @ trailing)

let rec to_sexp node =
  match Syntax.syntax node with
  | Syntax.Token token ->
    token_to_sexp token
  | _ ->
    let name = SyntaxKind.to_string (Syntax.kind node) in
    let children = Syntax.children node in
    let children = List.map children ~f:to_sexp in
    Sexp.List ((Sexp.Atom name) :: children)

let to_formatted_sexp_string node =
  to_sexp node
  |> Sexp.to_string_hum

let tree_to_sexp_string_ignore_trivia tree =
  tree
  |> SyntaxTree.root
  |> rewrite_tree_no_trivia
  |> to_sexp
  |> Sexp.to_string_mach

let tree_dump_node node =
  let print level text =
    let buf = Buffer.create (level * 2 + String.length text) in
    let () =
      for i = 1 to level do Buffer.add_string buf "> " done;
      Buffer.add_string buf text
    in
    Buffer.contents buf
  in
  let rec aux level node =
    match Syntax.syntax node with
    | Syntax.Token token ->
      [print level (TokenKind.to_string (Token.kind token))]
    | _ ->
      let children =
        List.concat_map ~f:(aux @@ level + 1) (Syntax.children node)
      in
      let name = print level (SyntaxKind.to_string (Syntax.kind node)) in
      children @ [name]
  in
  aux 0 node

let tree_dump_list lst =
  List.concat_map ~f:tree_dump_node lst
  |> String.concat "\n"

let printer w1 w2 s1 s2 =
  let fmt w s =
    let len = (w - (String.length s)) in
    s ^ String.make len ' '
  in
  "| " ^ fmt w1 s1 ^ " | " ^ fmt w2 s2 ^ " |\n"

let width l =
  let max_length m s = max m (String.length s) in
  List.fold_left ~f:max_length ~init:0 l
  |> max 8

let adjust l1 l2 =
  let aux lst len =
    if List.length lst < len
    then lst @ List.init ~f:(fun _ -> "") (len - (List.length lst))
    else lst
  in
  let len = max (List.length l1) (List.length l2) in
  aux l1 len, aux l2 len

let dump_diff expected actual =
  let l1 = List.concat_map ~f:tree_dump_node expected in
  let l2 = List.concat_map ~f:tree_dump_node actual in
  let l1, l2 = adjust l1 l2 in
  let w1, w2 = width l1, width l2 in
  let separator = String.make (w1 + w2 + 7) '-'  ^ "\n" in
  let printer = printer w1 w2 in
  let header = printer "EXPECTED" "ACTUAL" in
  let body =
    List.map2_exn ~f:printer l1 l2
    |> String.concat ""
  in
  separator ^ header ^ body ^ separator
