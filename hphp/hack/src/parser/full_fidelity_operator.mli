(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module TokenKind : sig
  type t = Full_fidelity_token_kind.t
end

include Full_fidelity_operator_generated.Sig

type assoc =
  | LeftAssociative
  | RightAssociative
  | NotAssociative

val precedence : Full_fidelity_parser_env.t -> t -> int

val precedence_for_assignment_in_expressions : int

val associativity : Full_fidelity_parser_env.t -> t -> assoc

val prefix_unary_from_token : TokenKind.t -> t

val is_trailing_operator_token : TokenKind.t -> bool

val trailing_from_token : TokenKind.t -> t

val is_binary_operator_token : TokenKind.t -> bool

val is_assignment : t -> bool

val is_comparison : t -> bool
