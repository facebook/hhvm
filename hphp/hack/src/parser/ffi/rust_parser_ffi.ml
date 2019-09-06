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

(* We could pass ParserEnv, but the less complicated structs we need to
 * synchronize on the boundary between Rust and OCaml, the better. *)
type parser_opts =
  (* is_experimental mode *)
  bool
  * (* hhvm_compat_mode *) bool
  * (* php5_compat_mode *) bool
  * (* codegen *) bool
  * (* allow_new_attribute_syntax *) bool
  * (* leak_rust_tree *) bool

let env_to_opts env =
  ( Env.is_experimental_mode env,
    Env.hhvm_compat_mode env,
    Env.php5_compat_mode env,
    Env.codegen env,
    Env.allow_new_attribute_syntax env,
    Env.leak_rust_tree env )

exception RustException of string

external parse_mode : SourceText.t -> FileInfo.mode option = "rust_parse_mode"

type ('a, 'b) result = 'a * 'b * SyntaxError.t list * Rust_pointer.t option

external parse_minimal :
  SourceText.t -> parser_opts -> (unit, MinimalSyntax.t) result
  = "parse_minimal"

let parse_minimal text env = parse_minimal text (env_to_opts env)

external parse_positioned :
  SourceText.t -> parser_opts -> (unit, PositionedSyntax.t) result
  = "parse_positioned"

let parse_positioned text env = parse_positioned text (env_to_opts env)

external parse_positioned_with_decl_mode_sc :
  SourceText.t -> parser_opts -> (bool list, PositionedSyntax.t) result
  = "parse_positioned_with_decl_mode_sc"

let parse_positioned_with_decl_mode_sc text env =
  parse_positioned_with_decl_mode_sc text (env_to_opts env)

external parse_positioned_with_coroutine_sc :
  SourceText.t -> parser_opts -> (bool, PositionedSyntax.t) result
  = "parse_positioned_with_coroutine_sc"

external parse_positioned_with_coroutine_sc_leak_tree :
  SourceText.t -> parser_opts -> (bool, PositionedSyntax.t) result
  = "parse_positioned_with_coroutine_sc_leak_tree"

let parse_positioned_with_coroutine_sc text env =
  if Env.leak_rust_tree env then
    parse_positioned_with_coroutine_sc_leak_tree text (env_to_opts env)
  else
    parse_positioned_with_coroutine_sc text (env_to_opts env)

external parse_positioned_with_verify_sc :
  SourceText.t ->
  parser_opts ->
  (PositionedSyntax.t list, PositionedSyntax.t) result
  = "parse_positioned_with_verify_sc"

let parse_positioned_with_verify_sc text env =
  parse_positioned_with_verify_sc text (env_to_opts env)

let init () =
  Callback.register_exception "rust exception" (RustException "");
  Full_fidelity_minimal_syntax.rust_parse_ref := parse_minimal;
  Full_fidelity_positioned_syntax.rust_parse_ref := parse_positioned;
  Full_fidelity_positioned_syntax.rust_parse_with_coroutine_sc_ref :=
    parse_positioned_with_coroutine_sc;
  Full_fidelity_positioned_syntax.rust_parse_with_decl_mode_sc_ref :=
    parse_positioned_with_decl_mode_sc;
  Full_fidelity_positioned_syntax.rust_parse_with_verify_sc_ref :=
    parse_positioned_with_verify_sc;
  ()
