(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(* This lexer is exactly the same as a regular lexer, except that it tokenizes
   C<D<E>> as C < D < E > >, and not C < D < E >> *)

module Lexer = Full_fidelity_minimal_lexer

type t = Lexer.t

let source = Lexer.source
let start_offset = Lexer.start_offset
let end_offset = Lexer.end_offset
let next_token = Lexer.next_token_in_type
let next_token_no_trailing = Lexer.next_token_no_trailing
let next_token_in_string = Lexer.next_token_in_string
let next_token_as_name = Lexer.next_token_as_name
let next_docstring_header = Lexer.next_docstring_header
let next_xhp_class_name = Lexer.next_xhp_class_name
let is_next_xhp_class_name = Lexer.is_next_xhp_class_name
let is_next_name = Lexer.is_next_name
let next_xhp_name = Lexer.next_xhp_name
let scan_markup = Lexer.scan_markup
let is_next_xhp_category_name = Lexer.is_next_xhp_category_name
let next_xhp_category_name = Lexer.next_xhp_category_name
let errors = Lexer.errors
let current_text_at = Lexer.current_text_at
