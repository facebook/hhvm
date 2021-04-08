(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type rust_output_config = {
  include_header: bool;
  output_file: string option;
}

type rust_env = {
  re_filepath: Relative_path.t;
  re_config_jsons: string list;
  re_config_list: string list;
  re_flags: int;
}

external rust_from_text_ffi :
  bool (* use_hhbc_by_ref *) ->
  rust_env ->
  rust_output_config ->
  Full_fidelity_source_text.t ->
  (unit, string) result = "compile_from_text_ffi"

external desugar_and_print_expr_trees : rust_env -> unit
  = "desugar_and_print_expr_trees"
