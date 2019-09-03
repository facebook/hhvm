(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
module TriviaKind = Full_fidelity_trivia_kind

module type LexableToken_S = sig
  module Trivia : Lexable_trivia_sig.LexableTrivia_S

  type t [@@deriving show]

  val make :
    Full_fidelity_token_kind.t ->
    (* kind *)
    Full_fidelity_source_text.t ->
    (* source *)
    int ->
    (* offset *)
    int ->
    (* width *)
    Trivia.t list ->
    (* leading *)
    Trivia.t list ->
    (* trailing *)
    t

  val kind : t -> Full_fidelity_token_kind.t

  val leading_start_offset : t -> int

  val text : t -> string

  val source_text : t -> Full_fidelity_source_text.t

  val width : t -> int

  val trailing_width : t -> int

  val leading_width : t -> int

  val trailing : t -> Trivia.t list

  val leading : t -> Trivia.t list

  val with_leading : Trivia.t list -> t -> t

  val with_kind : t -> Full_fidelity_token_kind.t -> t

  val with_trailing : Trivia.t list -> t -> t

  val filter_leading_trivia_by_kind : t -> TriviaKind.t -> Trivia.t list

  val has_trivia_kind : t -> TriviaKind.t -> bool
end
