(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Sexplib

module WithSyntax (Syntax : Syntax_sig.Syntax_S) = struct
  module Token = Syntax.Token
  module Trivia = Token.Trivia

  let trivia_to_sexp trivia =
    let name = Full_fidelity_trivia_kind.to_string (Trivia.kind trivia) in
    let width = Printf.sprintf "%d" (Trivia.width trivia) in
    Sexp.List [Sexp.Atom name; Sexp.Atom width]

  let trivia_list_to_sexp trivia_list =
    Sexp.List (List.map trivia_to_sexp trivia_list)

  let token_to_sexp token =
    let leading = trivia_list_to_sexp (Token.leading token) in
    let name = Full_fidelity_token_kind.to_string (Token.kind token) in
    let width = Printf.sprintf "%d" (Token.width token) in
    let trailing = trivia_list_to_sexp (Token.trailing token) in
    Sexp.List [leading; Sexp.Atom name; Sexp.Atom width; trailing]

  let rec to_sexp node =
    match Syntax.syntax node with
    | Syntax.Token token -> token_to_sexp token
    | _ ->
      let width = Printf.sprintf "%d" (Syntax.width node) in
      let name = Full_fidelity_syntax_kind.to_string (Syntax.kind node) in
      let children = List.map to_sexp (Syntax.children node) in
      Sexp.List [Sexp.Atom name; Sexp.Atom width; Sexp.List children]

  let dump_syntax root = Sexp.List [to_sexp root] |> Sexp.to_string_hum
end

(* WithSyntax *)
