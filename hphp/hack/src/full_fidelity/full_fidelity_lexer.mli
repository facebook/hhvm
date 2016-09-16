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
val next_token : t -> t * Full_fidelity_minimal_token.t
val next_token_as_name : t -> t * Full_fidelity_minimal_token.t
val next_token_in_type : t -> t * Full_fidelity_minimal_token.t
val next_xhp_element_token : t -> t * Full_fidelity_minimal_token.t * String.t
val next_xhp_body_token : t -> t * Full_fidelity_minimal_token.t
val next_xhp_class_name : t -> t * Full_fidelity_minimal_token.t
val is_next_xhp_class_name : t -> bool
val is_next_name : t -> bool
val next_xhp_name : t -> t * Full_fidelity_minimal_token.t
val is_next_xhp_category_name : t -> bool
val next_xhp_category_name : t -> t * Full_fidelity_minimal_token.t
