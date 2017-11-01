(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
module type LexableToken_S = sig
  module Trivia : Lexable_trivia_sig.LexableTrivia_S
  type t
  val make:
    Full_fidelity_token_kind.t -> (* kind *)
    Full_fidelity_source_text.t -> (* source *)
    int -> (* offset *)
    int -> (* width *)
    Trivia.t list -> (* leading *)
    Trivia.t list -> (* trailing *)
    t
  val kind: t -> Full_fidelity_token_kind.t
  val width: t -> int
  val trailing_width: t -> int
  val leading_width: t -> int
  val trailing: t -> Trivia.t list
  val leading: t -> Trivia.t list
  val with_leading: Trivia.t list -> t -> t
  val with_kind: t -> Full_fidelity_token_kind.t -> t
end
