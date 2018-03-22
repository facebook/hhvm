(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
module SourceData = Full_fidelity_editable_positioned_original_source_data

module type LexablePositionedToken_S = sig
  module Trivia : module type of Full_fidelity_positioned_trivia

  include Lexable_token_sig.LexableToken_S
    with module Trivia := Trivia

  val text : t -> string
  val concatenate : t -> t -> t
  val trim_left : n:int -> t -> t
  val trim_right : n:int -> t -> t
end (* LexablePositionedToken_S *)
