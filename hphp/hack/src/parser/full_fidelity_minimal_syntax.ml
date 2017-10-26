(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(**
  * Minimal syntax tree
  *
  * Every node in the tree knows its full width, and can compute its leading
  * trivia width, trailing trivia width, and width without trivia.
  *)

module Token = Full_fidelity_minimal_token

module SyntaxWithMinimalToken =
  Full_fidelity_syntax.WithToken(Token)

module MinimalSyntaxValue = struct
  type t = { full_width: int }
  let make w = { full_width = w }
  let full_width n = n.full_width
end

module MinimalSyntax =
  SyntaxWithMinimalToken.WithSyntaxValue(MinimalSyntaxValue)

module MinimalValueBuilder = struct
  let value_from_children _kind nodes =
    let folder sum node =
      let v = MinimalSyntax.value node in
      let w = MinimalSyntaxValue.full_width v in
      sum + w in
    let width = List.fold_left folder 0 nodes in
    MinimalSyntaxValue.make width

  let value_from_token token =
    MinimalSyntaxValue.make (Token.full_width token)

  let value_from_syntax syntax =
    let folder sum child =
      let v = MinimalSyntax.value child in
      let w = MinimalSyntaxValue.full_width v in
      sum + w in
    let width = (MinimalSyntax.fold_over_children folder 0 syntax) in
    MinimalSyntaxValue.make width
end

include MinimalSyntax
include MinimalSyntax.WithValueBuilder(MinimalValueBuilder)

let full_width node =
  MinimalSyntaxValue.full_width (value node)

let leading_width node =
  match leading_token node with
  | None -> 0
  | Some token -> Token.leading_width token

let trailing_width node =
  match trailing_token node with
  | None -> 0
  | Some token -> Token.trailing_width token

let width node =
  (full_width node) - (leading_width node) - (trailing_width node)

(* Takes a node and an offset; produces the descent through the parse tree
   to that position. *)
let parentage node position =
  let rec aux nodes position acc =
    match nodes with
    | [] -> acc
    | h :: t ->
      let width = full_width h in
      if position < width then
        aux (children h) position (h :: acc)
      else
        aux t (position - width) acc in
  aux [node] position []

let is_in_body node position =
  let rec aux parents =
    match parents with
    | [] -> false
    | h1 :: t1 ->
      if is_compound_statement h1 then
        match t1 with
        | [] -> false
        | h2 :: _ ->
          if ((is_methodish_declaration h2) || is_function_declaration h2) then
            true
          else
            aux t1
      else
        aux t1 in
  let parents = parentage node position in
  aux parents
