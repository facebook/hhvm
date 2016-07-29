(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module type Lexer_S = sig
  type t
  val start_offset : t -> int
  val end_offset : t -> int
  val next_token : t -> t * Full_fidelity_minimal_token.t
  val next_token_as_name : t -> t * Full_fidelity_minimal_token.t
  val errors : t -> Full_fidelity_syntax_error.t list
end
