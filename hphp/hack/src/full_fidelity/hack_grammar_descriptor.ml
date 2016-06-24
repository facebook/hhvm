(**
* Copyright (c) 2016, Facebook, Inc.
* All rights reserved.
*
* This source code is licensed under the BSD-style license found in the
* LICENSE file in the "hack" directory of this source tree. An additional grant
* of patent rights can be found in the PATENTS file in the same directory.
*
*)

module type Grammar = sig

  type term

  type nonterm

  type symbol =
  | Term of term
  | NonTerm of nonterm

  val start : nonterm

  val grammar : nonterm -> symbol list list

  val to_string : term -> string

  val non_term_to_string : nonterm -> string

end

module HackGrammar = struct
  (* this module defines the grammar of hack using the [grammar] function, which
   * returns all productions for each non-terminal in the grammar *)
  module TokenKind = Full_fidelity_token_kind

  type term = TokenKind.t

  type nonterm =
  (* TODO complete the non-terminals *)
  | Statement
  | StatementList
  | ExpressionStatement
  | Expression

  type symbol =
  | Term of term
  | NonTerm of nonterm

  let to_string = TokenKind.to_string

  open TokenKind
  let grammar = function
  (* TODO complete the grammar *)
  | Statement -> [[Term LeftBrace; NonTerm StatementList; Term RightBrace];
                  [NonTerm ExpressionStatement]]
  | StatementList -> [[NonTerm StatementList; NonTerm ExpressionStatement];
                      [NonTerm ExpressionStatement]]
  | ExpressionStatement -> [[NonTerm Expression; Term Semicolon]]
  | Expression -> [[Term LeftParen; NonTerm Expression; Term RightParen];
                   [Term Do; Term DollarDollar];
                   [Term Dollar; Term As]]

  let start = Statement

  let non_term_to_string = function
  | Statement -> "statement"
  | Expression -> "expression"
  | ExpressionStatement -> "expression_statement"
  | StatementList -> "statement_list"

end
