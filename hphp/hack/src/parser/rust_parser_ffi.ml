(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
module SourceText = Full_fidelity_source_text
module SyntaxError = Full_fidelity_syntax_error
module Env = Full_fidelity_parser_env
module PositionedSyntax = Full_fidelity_positioned_syntax

external parse_mode : SourceText.t -> FileInfo.mode option = "rust_parse_mode"

type ('a, 'b) result = 'a * 'b * SyntaxError.t list * Rust_pointer.t option

external parse_positioned :
  SourceText.t -> Env.t -> (unit, PositionedSyntax.t) result
  = "parse_positioned_by_ref"

let parse_positioned text env = parse_positioned text env

let init () =
  Full_fidelity_positioned_syntax.rust_parse_ref := parse_positioned;
  ()
