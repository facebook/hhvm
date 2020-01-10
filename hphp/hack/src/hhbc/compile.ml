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

module SyntaxError = Full_fidelity_syntax_error
module SourceText = Full_fidelity_source_text

type result = {
  bytecode_segments: string list;
  parsing_t: float;
  codegen_t: float;
  printing_t: float;
}

type env = {
  filepath: Relative_path.t;
  is_systemlib: bool;
  is_evaled: bool;
  for_debugger_eval: bool;
  dump_symbol_refs: bool;
  config_jsons: Hh_json.json option list;
  config_list: string list;
}

let add_to_time t0 =
  let t = Unix.gettimeofday () in
  (t, t -. t0)

let with_global_state env f =
  let hhbc_options =
    Hhbc_options.apply_config_overrides_statelessly
      env.config_list
      env.config_jsons
  in
  Hhbc_options.set_compiler_options hhbc_options;
  Emit_env.set_is_systemlib env.is_systemlib;
  let ret = f hhbc_options in
  (* TODO(hrust) investigate if we can revert it here, or some parts of
  global state carry over across multiple emit requests  *)
  ret

let with_compilation_times env f =
  let t = Unix.gettimeofday () in
  let hhas_prog = f () in
  let (t, codegen_t) = add_to_time t in
  let bytecode_segments =
    Hhbc_hhas.to_segments
      ~path:env.filepath
      ~dump_symbol_refs:env.dump_symbol_refs
      hhas_prog
  in
  let (_, printing_t) = add_to_time t in
  { bytecode_segments; codegen_t; printing_t; parsing_t = Float.nan }

let parse_text ~hhbc_options popt filename text =
  let php5_compat_mode =
    not (Hhbc_options.enable_uniform_variable_syntax hhbc_options)
  in
  let hacksperimental = Hhbc_options.hacksperimental hhbc_options in
  let lower_coroutines = Hhbc_options.enable_coroutines hhbc_options in
  let env =
    Full_fidelity_ast.make_env
      ~parser_options:popt
      ~codegen:true
      ~fail_open:false
      ~php5_compat_mode
      ~hacksperimental
      ~keep_errors:false
      ~lower_coroutines
      filename
  in
  let source_text = SourceText.make filename text in
  let (ast, file_mode) =
    Full_fidelity_ast.from_text_to_empty_tast env source_text
  in
  let elaborate_namespaces =
    new Naming_elaborate_namespaces_endo.generic_elaborator
  in
  let nsenv = Namespace_env.empty_from_popt popt in
  let ast =
    elaborate_namespaces#on_program
      (Naming_elaborate_namespaces_endo.make_env nsenv)
      ast
  in
  (ast, file_mode)

let parse_file ~hhbc_options filename text =
  let popt =
    Hhbc_options.(
      let co = hhbc_options in
      ParserOptions.make
        ~auto_namespace_map:(aliased_namespaces co)
        ~codegen:true
        ~disallow_execution_operator:(phpism_disallow_execution_operator co)
        ~disable_nontoplevel_declarations:
          (phpism_disable_nontoplevel_declarations co)
        ~disable_static_closures:(phpism_disable_static_closures co)
        ~disable_lval_as_an_expression:(disable_lval_as_an_expression co)
        ~enable_class_level_where_clauses:(enable_class_level_where_clauses co)
        ~disable_legacy_soft_typehints:(disable_legacy_soft_typehints co)
        ~allow_new_attribute_syntax:(allow_new_attribute_syntax co)
        ~disable_legacy_attribute_syntax:(disable_legacy_attribute_syntax co)
        ~const_default_func_args:(const_default_func_args co)
        ~disallow_silence:false
        ~const_static_props:(const_static_props co)
        ~abstract_static_props:(abstract_static_props co)
        ~disable_unset_class_const:(disable_unset_class_const co)
        ~disallow_func_ptrs_in_constants:(disallow_func_ptrs_in_constants co)
        ~rust_lowerer:(rust_lowerer co))
  in
  ( (try
       `ParseResult
         (Errors.do_ (fun () -> parse_text popt filename text ~hhbc_options))
     with
    (* FFP failed to parse *)
    | Failure s -> `ParseFailure (SyntaxError.make 0 0 s, Pos.none)
    (* FFP generated an error *)
    | SyntaxError.ParserFatal (e, p) -> `ParseFailure (e, p)),
    popt )

let from_ast ~env ~is_hh_file ~empty_namespace ~hhbc_options tast =
  with_compilation_times env (fun _ ->
      let tast =
        if Hhbc_options.enable_pocket_universes hhbc_options then
          Pocket_universes.translate tast
        else
          tast
      in
      Emit_program.from_ast
        ~is_evaled:env.is_evaled
        ~for_debugger_eval:env.for_debugger_eval
        ~empty_namespace
        ~is_hh_file
        tast)

let handle_conversion_errors errors =
  List.filter errors ~f:(fun error ->
      match Errors.get_code error with
      (* Ignore these errors to match legacy AST behavior *)
      | 2086
      (* Naming.MethodNeedsVisibility *)
      | 2102
      (* Naming.UnsupportedTraitUseAs *)
      | 2103 (* Naming.UnsupportedInsteadOf *) ->
        false
      | _ (* Emit fatal parse otherwise *) -> true)

let fatal ~env ~is_runtime_error pos message =
  with_compilation_times env (fun _ ->
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

let from_text (source_text : string) (env : env) : result =
  with_global_state env (fun hhbc_options ->
      let t = Unix.gettimeofday () in
      let (fail_or_tast, popt) =
        parse_file env.filepath source_text ~hhbc_options
      in
      let (_, parsing_t) = add_to_time t in
      let empty_namespace = Namespace_env.empty_from_popt popt in
      let (tast_with_file_mode, is_runtime_error, error) =
        match fail_or_tast with
        | `ParseFailure (e, pos) ->
          let is_runtime_error =
            match SyntaxError.error_type e with
            | SyntaxError.ParseError -> false
            | SyntaxError.RuntimeError -> true
          in
          (None, is_runtime_error, Some (pos, Some (SyntaxError.message e)))
        | `ParseResult (errors, tast) ->
          let error_list = Errors.get_error_list errors in
          let error_list = handle_conversion_errors error_list in
          List.iter error_list (fun e ->
              Printf.eprintf "%s\n%!" (Errors.to_string (Errors.to_absolute e)));
          if List.is_empty error_list then
            (Some tast, false, None)
          else
            (None, false, Some (Pos.none, None))
      in
      let ret =
        match (tast_with_file_mode, error) with
        | (Some (tast, file_mode), _) ->
          let is_hh_file = FileInfo.is_hh_file file_mode in
          from_ast ~env ~is_hh_file ~empty_namespace ~hhbc_options tast
        | (_, Some (pos, msg)) -> fatal ~env ~is_runtime_error pos msg
        | _ -> failwith "Impossible case: emits program or fatals"
      in
      { ret with parsing_t })
