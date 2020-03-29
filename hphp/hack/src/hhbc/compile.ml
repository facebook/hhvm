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
  hhbc_options: Hhbc_options.t;
}

type env = {
  filepath: Relative_path.t;
  is_systemlib: bool;
  is_evaled: bool;
  for_debugger_eval: bool;
  dump_symbol_refs: bool;
  config_jsons: string list;
  config_list: string list;
  disable_toplevel_elaboration: bool;
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

let with_compilation_times ~hhbc_options env f =
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
  {
    bytecode_segments;
    codegen_t;
    printing_t;
    hhbc_options;
    parsing_t = Float.nan;
  }

let elaborate_namespaces popt aast =
  let elaborator = new Naming_elaborate_namespaces_endo.generic_elaborator in
  let nsenv = Namespace_env.empty_from_popt popt in
  elaborator#on_program (Naming_elaborate_namespaces_endo.make_env nsenv) aast

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

let pos_of_error path source_text error =
  SourceText.relative_pos
    path
    source_text
    (SyntaxError.start_offset error)
    (SyntaxError.end_offset error)

let parse_file ~hhbc_options env text :
    (Tast.program * bool, Pos.t * string * bool) Either.t * ParserOptions.t =
  let filename = env.filepath in
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
        ~disable_xhp_element_mangling:(disable_xhp_element_mangling co)
        ~disable_xhp_children_declarations:
          (disable_xhp_children_declarations co)
        ~enable_xhp_class_modifier:(enable_xhp_class_modifier co)
        ~rust_top_level_elaborator:(rust_top_level_elaborator co)
        ~enable_first_class_function_pointers:
          (enable_first_class_function_pointers co)
        ~disable_modes:(disable_modes co)
        ~disable_array:(disable_array co))
  in
  let env =
    Full_fidelity_ast.make_env
      ~parser_options:popt
      ~codegen:true
      ~fail_open:false
      ~php5_compat_mode:
        (not (Hhbc_options.enable_uniform_variable_syntax hhbc_options))
      ~keep_errors:false
      ~lower_coroutines:(Hhbc_options.enable_coroutines hhbc_options)
      ~elaborate_namespaces:(not env.disable_toplevel_elaboration)
      filename
  in
  let source_text = SourceText.make filename text in
  let result =
    try
      let tast_result =
        Full_fidelity_ast.from_text_to_empty_tast env source_text
      in
      Rust_aast_parser_types.(
        match tast_result with
        | { syntax_errors = e :: _ as errors; _ } ->
          let pos_of_error = pos_of_error filename source_text in
          let runtime_errors =
            List.filter
              errors
              ~f:SyntaxError.((fun e -> error_type e = RuntimeError))
          in
          (match runtime_errors with
          | e :: _ -> Either.Second (pos_of_error e, SyntaxError.message e, true)
          | _ -> Either.Second (pos_of_error e, SyntaxError.message e, false))
        | { lowpri_errors = (p, msg) :: _; _ } -> Either.Second (p, msg, false)
        | {
         errors;
         aast;
         scoured_comments = { Scoured_comments.sc_fixmes; _ };
         file_mode;
         _;
        } ->
          let errors =
            List.filter errors ~f:(fun e ->
                Scoured_comments.get_fixme_pos
                  sc_fixmes
                  (Errors.get_pos e)
                  (Errors.get_code e)
                |> Option.is_none)
          in
          let errors = handle_conversion_errors errors in
          List.iter errors (fun e ->
              Printf.eprintf "%s\n%!" (Errors.to_string (Errors.to_absolute e)));
          (match (errors, aast) with
          | ([], Ok aast) -> Either.First (aast, FileInfo.is_hh_file file_mode)
          | ([], Error msg) -> Either.Second (Pos.none, msg, false)
          | _ -> Either.Second (Pos.none, "", false)))
    with
    | Failure s -> Either.Second (Pos.none, s, false)
    | SyntaxError.ParserFatal (e, p) ->
      Either.Second (p, SyntaxError.message e, false)
  in
  (result, popt)

let emit ~env ~is_hh_file ~empty_namespace ~hhbc_options tast =
  with_compilation_times ~hhbc_options env (fun _ ->
      let tast =
        if Hhbc_options.enable_pocket_universes hhbc_options then
          Pocket_universes.translate tast
        else
          tast
      in
      Emit_program.emit_program
        ~is_evaled:env.is_evaled
        ~for_debugger_eval:env.for_debugger_eval
        ~empty_namespace
        ~is_hh_file
        tast)

let emit_fatal ~env ~is_runtime_error ~hhbc_options pos message =
  with_compilation_times ~hhbc_options env (fun _ ->
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
      let (fail_or_tast, popt) = parse_file env source_text ~hhbc_options in
      let (_, parsing_t) = add_to_time t in
      let ret =
        match fail_or_tast with
        | Either.First (tast, is_hh_file) ->
          let empty_namespace = Namespace_env.empty_from_popt popt in
          let tast = elaborate_namespaces popt tast in
          emit ~env ~is_hh_file ~empty_namespace ~hhbc_options tast
        | Either.Second (pos, msg, is_runtime_error) ->
          emit_fatal ~env ~is_runtime_error ~hhbc_options pos (Some msg)
      in
      { ret with parsing_t })
