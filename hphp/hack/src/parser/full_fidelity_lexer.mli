(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Env : sig
  val set : force_hh:bool -> enable_xhp:bool -> unit
end

module WithToken : functor (Token : Lexable_token_sig.LexableToken_S) -> sig
  type t [@@deriving show]
  type string_literal_kind =
    | Literal_execution_string
    | Literal_double_quoted
    | Literal_heredoc of string [@@deriving show]
  val make : ?is_experimental_mode:bool -> Full_fidelity_source_text.t -> t
  val make_at : ?is_experimental_mode:bool -> Full_fidelity_source_text.t -> int -> t
  val source : t -> Full_fidelity_source_text.t
  val errors : t -> Full_fidelity_syntax_error.t list
  val start_offset : t -> int
  val end_offset : t -> int
  val is_experimental_mode : t -> bool
  val next_token : t -> t * Token.t
  val next_token_no_trailing : t -> t * Token.t
  val next_token_in_string : t -> string_literal_kind -> t * Token.t
  val scan_markup: t ->
    is_leading_section:bool ->
    (* lexer *)
    t *
    (* markup text *)
    Token.t *
    (* optional suffix that consist of mandatory '<?' and optional 'name'
    which can be either
    - language 'hh', 'php'
    - '=' is case of short '<?=' tag *)
    (Token.t * Token.t option) option
  val next_token_as_name : t -> t * Token.t
  val next_token_in_type : t -> t * Token.t
  val next_docstring_header : t -> t * Token.t * String.t
  val next_xhp_element_token : no_trailing:bool -> t
    -> t * Token.t * String.t
  val next_xhp_body_token : t -> t * Token.t
  val next_xhp_class_name : t -> t * Token.t
  val is_next_xhp_class_name : t -> bool
  val is_next_name : t -> bool
  val next_xhp_name : t -> t * Token.t
  val is_next_xhp_category_name : t -> bool
  val next_xhp_category_name : t -> t * Token.t
  val rescan_halt_compiler : t -> Token.t -> t * Token.t
  val scan_leading_php_trivia : t -> t * Token.Trivia.t list
  val scan_trailing_php_trivia : t -> t * Token.Trivia.t list
  val scan_leading_xhp_trivia : t -> t * Token.Trivia.t list
  val scan_trailing_xhp_trivia : t -> t * Token.Trivia.t list
end
