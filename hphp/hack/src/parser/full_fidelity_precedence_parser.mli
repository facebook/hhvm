(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
module WithLexer :
  functor (Lexer : Full_fidelity_lexer_sig.MinimalLexer_S) -> sig

type t = {
  lexer : Lexer.t;
  errors : Full_fidelity_syntax_error.t list;
  context:
    Full_fidelity_parser_context.WithToken(Full_fidelity_minimal_token).t;
  precedence : int;
  hhvm_compat_mode : bool;
}

val make : ?hhvm_compat_mode:bool
  -> Lexer.t
  -> Full_fidelity_syntax_error.t list
  -> Full_fidelity_parser_context.WithToken(Full_fidelity_minimal_token).t
  -> t

val errors : t -> Full_fidelity_syntax_error.t list

val hhvm_compat_mode : t -> bool

val with_errors : t -> Full_fidelity_syntax_error.t list -> t

val with_lexer : t -> Lexer.t -> t

val lexer : t -> Lexer.t

val context : t ->
  Full_fidelity_parser_context.WithToken(Full_fidelity_minimal_token).t

val with_context : t ->
  Full_fidelity_parser_context.WithToken(Full_fidelity_minimal_token).t -> t

val skipped_tokens : t -> Full_fidelity_minimal_token.t list

val with_skipped_tokens : t -> Full_fidelity_minimal_token.t list -> t

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
  -> t * Full_fidelity_minimal_token.t * String.t

val next_xhp_body_token : t -> t * Full_fidelity_minimal_token.t
end
