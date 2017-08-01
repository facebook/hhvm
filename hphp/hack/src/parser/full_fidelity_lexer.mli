(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type t
val make : Full_fidelity_source_text.t -> t
val errors : t -> Full_fidelity_syntax_error.t list
val start_offset : t -> int
val end_offset : t -> int
val current_text_at : t -> int -> int -> string
val next_token : t -> t * Full_fidelity_minimal_token.t
val next_token_no_trailing : t -> t * Full_fidelity_minimal_token.t
val next_token_in_string : t -> string -> t * Full_fidelity_minimal_token.t
val scan_markup: t ->
  is_leading_section:bool ->
  (* lexer *)
  t *
  (* markup text *)
  Full_fidelity_minimal_token.t *
  (* optional suffix that consist of mandatory '<?' and optional 'name'
  which can be either
  - language 'hh', 'php'
  - '=' is case of short '<?=' tag *)
  (Full_fidelity_minimal_token.t * Full_fidelity_minimal_token.t option) option
val next_token_as_name : t -> t * Full_fidelity_minimal_token.t
val next_token_in_type : t -> t * Full_fidelity_minimal_token.t
val next_docstring_header : t -> t * Full_fidelity_minimal_token.t * String.t
val next_xhp_element_token : no_trailing:bool -> t
  -> t * Full_fidelity_minimal_token.t * String.t
val next_xhp_body_token : t -> t * Full_fidelity_minimal_token.t
val next_xhp_class_name : t -> t * Full_fidelity_minimal_token.t
val is_next_xhp_class_name : t -> bool
val is_next_name : t -> bool
val next_xhp_name : t -> t * Full_fidelity_minimal_token.t
val is_next_xhp_category_name : t -> bool
val next_xhp_category_name : t -> t * Full_fidelity_minimal_token.t
