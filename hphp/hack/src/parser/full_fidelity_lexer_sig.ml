(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module WithToken(Token : Lexable_token_sig.LexableToken_S) = struct
  module type Lexer_S = sig
    type t
    val source : t -> Full_fidelity_source_text.t
    val start_offset : t -> int
    val end_offset : t -> int
    val current_text_at : t -> int -> int -> string
    val next_token : t -> t * Token.t
    val next_token_no_trailing : t -> t * Token.t
    val next_token_as_name : t -> t * Token.t
    val next_docstring_header : t -> t * Token.t * string
    val next_token_in_string : t -> string -> t * Token.t
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
    val errors : t -> Full_fidelity_syntax_error.t list
    val next_xhp_class_name : t -> t * Token.t
    val is_next_xhp_class_name : t -> bool
    val is_next_name : t -> bool
    val next_xhp_name : t -> t * Token.t
    val is_next_xhp_category_name : t -> bool
    val next_xhp_category_name : t -> t * Token.t
    val next_xhp_element_token : no_trailing:bool ->  attribute:bool ->t
      -> t * Token.t * String.t
    val next_xhp_body_token : t -> t * Token.t
  end (* Lexer_S *)
end (* WithToken *)

module type MinimalLexer_S = WithToken(Full_fidelity_minimal_token).Lexer_S
