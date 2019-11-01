(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type result = {
  bytecode_segments: string list;
  codegen_t: float;
  printing_t: float;
}

type env = {
  filepath: Relative_path.t;
  is_systemlib: bool;
  is_evaled: bool;
  for_debugger_eval: bool;
  dump_symbol_refs: bool;
  empty_namespace: Namespace_env.env;
  hhbc_options: Hhbc_options.t;
}

val from_ast : env:env -> is_hh_file:bool -> Tast.program -> result

val fatal :
  env:env -> is_runtime_error:bool -> Pos.t -> string option -> result
