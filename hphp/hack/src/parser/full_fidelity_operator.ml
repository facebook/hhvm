(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module TokenKind = Full_fidelity_token_kind
include Full_fidelity_operator_generated.Impl

type assoc =
  | LeftAssociative
  | RightAssociative
  | NotAssociative

(* helper functions for Rust forwarding *)
external rust_precedence_helper : t -> int = "rust_precedence_helper"

external rust_precedence_for_assignment_in_expressions_helper : unit -> int
  = "rust_precedence_for_assignment_in_expressions_helper"

external rust_associativity_helper : t -> assoc = "rust_associativity_helper"

(* NOTE: rust_precedence_helper does not use Full_fidelity_parser_env '_env'.
 * This call must be updated if operator::precedence in operator.rs starts to depend on
 * the env parameter *)
let precedence _env operator = rust_precedence_helper operator

let precedence_for_assignment_in_expressions : int =
  rust_precedence_for_assignment_in_expressions_helper ()

(* NOTE: rust_associativity_helper does not use Full_fidelity_parser_env '_env'.
 * This call must be updated if operator::associativity in operator.rs starts to depend on
 * the env parameter *)
let associativity _env operator = rust_associativity_helper operator

external prefix_unary_from_token : TokenKind.t -> t = "prefix_unary_from_token"

(* Is this a token that can appear after an expression? *)
external is_trailing_operator_token : TokenKind.t -> bool
  = "is_trailing_operator_token"

external trailing_from_token : TokenKind.t -> t = "trailing_from_token"

external is_binary_operator_token : TokenKind.t -> bool
  = "is_binary_operator_token"

external is_assignment : t -> bool = "is_assignment"

external is_comparison : t -> bool = "is_comparison"
