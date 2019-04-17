(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
module SourceText = Full_fidelity_source_text
module MinimalSyntax = Full_fidelity_minimal_syntax
module Env = Full_fidelity_parser_env

(* We could pass ParserEnv, but the less complicated structs we need to
 * synchronize on the boundary between Rust and OCaml, the better. *)
type parser_opts = (
  bool * (* is_experimental mode *)
  bool * (* enable_stronger_await_binding *)
  bool * (* disable_unsafe_expr *)
  bool * (* enable_unsafe_block *)
  bool * (* force_hh *)
  bool * (* enable_xhp *)
  bool * (* hhvm_compat_mode *)
  bool   (* php5_compat_mode *)
)
let env_to_opts env = (
  (Env.is_experimental_mode env),
  (Env.enable_stronger_await_binding env),
  (Env.disable_unsafe_expr env),
  (Env.disable_unsafe_block env),
  (Env.force_hh env),
  (Env.enable_xhp env),
  (Env.hhvm_compat_mode env),
  (Env.php5_compat_mode env)
)

external parse_minimal: SourceText.t -> parser_opts -> MinimalSyntax.t = "parse_minimal"
let parse_minimal text env = parse_minimal text (env_to_opts env)
