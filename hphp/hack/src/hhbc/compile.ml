(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

[@@@warning "-33"]

open Core_kernel (* ensure forward compatible *)

[@@@warning "+33"]

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
  (* TODO(hrust) replace with these avoid two-way conversion of Hhbc_options
  config_jsons: Hh_json.json option list;
  config_list: string list; *)
  hhbc_options: Hhbc_options.t;
}

let add_to_time t0 =
  let t = Unix.gettimeofday () in
  (t, t -. t0)

let with_global_state env f =
  Hhbc_options.set_compiler_options env.hhbc_options;
  Emit_env.set_is_systemlib env.is_systemlib;
  let t = Unix.gettimeofday () in
  let hhas_prog = f () in
  let (t, codegen_t) = add_to_time t in
  (* TODO(hrust) investigate if we can revert it here, or some parts of
  global state carry over across multiple emit requests  *)
  let bytecode_segments =
    Hhbc_hhas.to_segments
      ~path:env.filepath
      ~dump_symbol_refs:env.dump_symbol_refs
      hhas_prog
  in
  let (_, printing_t) = add_to_time t in
  { bytecode_segments; codegen_t; printing_t }

let from_ast ~env ~is_hh_file tast =
  with_global_state env (fun () ->
      let tast =
        if Hhbc_options.enable_pocket_universes env.hhbc_options then
          Pocket_universes.translate tast
        else
          tast
      in
      Emit_program.from_ast
        ~is_evaled:env.is_evaled
        ~for_debugger_eval:env.for_debugger_eval
        ~empty_namespace:env.empty_namespace
        ~is_hh_file
        tast)

let fatal ~env ~is_runtime_error pos message =
  with_global_state env (fun () ->
      let error_t =
        if is_runtime_error then
          Hhbc_ast.FatalOp.Runtime
        else
          Hhbc_ast.FatalOp.Parse
      in
      let (message, ignore_message) =
        match message with
        | Some message -> (message, false)
        | None -> ("Syntax error", true)
      in
      Emit_program.emit_fatal_program ~ignore_message error_t pos message)
