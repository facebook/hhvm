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

type t = Full_fidelity_lexer.t

let start_offset = Full_fidelity_lexer.start_offset
let end_offset = Full_fidelity_lexer.end_offset
let next_token = Full_fidelity_lexer.next_token_in_type
let next_token_as_name = Full_fidelity_lexer.next_token_as_name
let next_xhp_class_name = Full_fidelity_lexer.next_xhp_class_name
let is_next_xhp_class_name = Full_fidelity_lexer.is_next_xhp_class_name
let is_next_name = Full_fidelity_lexer.is_next_name
let next_xhp_name = Full_fidelity_lexer.next_xhp_name
let is_next_xhp_category_name = Full_fidelity_lexer.is_next_xhp_category_name
let next_xhp_category_name = Full_fidelity_lexer.next_xhp_category_name
let errors = Full_fidelity_lexer.errors
