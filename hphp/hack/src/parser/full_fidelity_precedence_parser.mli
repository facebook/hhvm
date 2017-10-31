(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
module WithSyntax : functor (Syntax : Syntax_sig.Syntax_S) -> sig
module WithLexer : functor (Lexer : Full_fidelity_lexer_sig.WithToken(Syntax.Token).Lexer_S) -> sig
type t = {
  lexer : Lexer.t;
  errors : Full_fidelity_syntax_error.t list;
  context:
    Full_fidelity_parser_context.WithToken(Syntax.Token).t;
  precedence : int;
  env : Full_fidelity_parser_env.t;
}

val make : Full_fidelity_parser_env.t
  -> Lexer.t
  -> Full_fidelity_syntax_error.t list
  -> Full_fidelity_parser_context.WithToken(Syntax.Token).t
  -> t

val errors : t -> Full_fidelity_syntax_error.t list

val env : t -> Full_fidelity_parser_env.t

val with_errors : t -> Full_fidelity_syntax_error.t list -> t

val with_lexer : t -> Lexer.t -> t

val lexer : t -> Lexer.t

val context : t ->
  Full_fidelity_parser_context.WithToken(Syntax.Token).t

val with_context : t ->
  Full_fidelity_parser_context.WithToken(Syntax.Token).t -> t

val skipped_tokens : t -> Syntax.Token.t list

val with_skipped_tokens : t -> Syntax.Token.t list -> t

val clear_skipped_tokens : t -> t

(** Wrapper functions for interfacing with parser context **)

val expect : t -> Full_fidelity_token_kind.t list -> t

val expect_in_new_scope : t -> Full_fidelity_token_kind.t list -> t

val expects : t -> Full_fidelity_token_kind.t -> bool

val expects_here : t -> Full_fidelity_token_kind.t -> bool

val pop_scope : t -> Full_fidelity_token_kind.t list -> t

val print_expected : t -> unit

(** Precedence functions **)

val with_precedence : t -> int -> t

val with_numeric_precedence : t -> int -> (t -> t * 'a) -> t * 'a

val with_operator_precedence : t ->
  Full_fidelity_operator.t -> (t -> t * 'a) -> t * 'a

val with_reset_precedence : t -> (t -> t * 'a) -> t * 'a

val next_xhp_element_token :
  ?no_trailing:bool -> ?attribute:bool -> t
  -> t * Syntax.Token.t * String.t

val next_xhp_body_token : t -> t * Syntax.Token.t
end
end
