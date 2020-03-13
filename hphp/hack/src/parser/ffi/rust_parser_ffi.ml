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
module MinimalSyntax = Full_fidelity_minimal_syntax
module Env = Full_fidelity_parser_env
module PositionedSyntax = Full_fidelity_positioned_syntax

external parse_mode : SourceText.t -> FileInfo.mode option = "rust_parse_mode"

type ('a, 'b) result = 'a * 'b * SyntaxError.t list * Rust_pointer.t option

external parse_minimal : SourceText.t -> Env.t -> (unit, MinimalSyntax.t) result
  = "parse_minimal"

let parse_minimal text env = parse_minimal text env

external parse_positioned :
  SourceText.t -> Env.t -> (unit, PositionedSyntax.t) result
  = "parse_positioned"

let parse_positioned text env = parse_positioned text env

external parse_positioned_with_decl_mode_sc :
  SourceText.t -> Env.t -> (bool list, PositionedSyntax.t) result
  = "parse_positioned_with_decl_mode_sc"

let parse_positioned_with_decl_mode_sc text env =
  parse_positioned_with_decl_mode_sc text env

external parse_positioned_with_coroutine_sc :
  SourceText.t -> Env.t -> (bool, PositionedSyntax.t) result
  = "parse_positioned_with_coroutine_sc"

external parse_positioned_with_coroutine_sc_leak_tree :
  SourceText.t -> Env.t -> (bool, PositionedSyntax.t) result
  = "parse_positioned_with_coroutine_sc_leak_tree"

let parse_positioned_with_coroutine_sc text env =
  if Env.leak_rust_tree env then
    parse_positioned_with_coroutine_sc_leak_tree text env
  else
    parse_positioned_with_coroutine_sc text env

external parse_positioned_with_verify_sc :
  SourceText.t -> Env.t -> (PositionedSyntax.t list, PositionedSyntax.t) result
  = "parse_positioned_with_verify_sc"

let parse_positioned_with_verify_sc text env =
  parse_positioned_with_verify_sc text env

let init () =
  Full_fidelity_minimal_syntax.rust_parse_ref := parse_minimal;
  Full_fidelity_positioned_syntax.rust_parse_ref := parse_positioned;
  Full_fidelity_positioned_syntax.rust_parse_with_coroutine_sc_ref :=
    parse_positioned_with_coroutine_sc;
  Full_fidelity_positioned_syntax.rust_parse_with_decl_mode_sc_ref :=
    parse_positioned_with_decl_mode_sc;
  Full_fidelity_positioned_syntax.rust_parse_with_verify_sc_ref :=
    parse_positioned_with_verify_sc;
  ()
