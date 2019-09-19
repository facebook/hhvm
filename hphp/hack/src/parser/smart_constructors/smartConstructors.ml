(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional
 * directory.
 *
 **
 *
 * THIS FILE IS @generated; DO NOT EDIT IT
 * To regenerate this file, run
 *
 *   buck run //hphp/hack/src:generate_full_fidelity
 *
 **
 *
 * This module contains a signature which can be used to describe smart
 * constructors.
  
 *)

module ParserEnv = Full_fidelity_parser_env

module type SmartConstructors_S = sig
  module Token : Lexable_token_sig.LexableToken_S

  type t (* state *) [@@deriving show]

  type r (* smart constructor return type *) [@@deriving show]

  val rust_parse :
    Full_fidelity_source_text.t ->
    ParserEnv.t ->
    t * r * Full_fidelity_syntax_error.t list * Rust_pointer.t option

  val initial_state : ParserEnv.t -> t
end
