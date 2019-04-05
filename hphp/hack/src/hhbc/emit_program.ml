(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Core_kernel
open Instruction_sequence
open Hhbc_ast

let emit_main is_evaled debugger_modify_program popt defs =
  let body, _is_generator, _is_pair_generator =
    Emit_body.emit_body
      ~pos:Pos.none
      ~namespace:(Namespace_env.empty popt)
      ~is_closure_body:false
      ~is_memoize:false
      ~is_native:false
      ~is_async:false
      ~is_rx_body:false
      ~debugger_modify_program
      ~deprecation_info:None
      ~skipawaitable:false
      ~scope:Ast_scope.Scope.toplevel
      ~return_value:(if is_evaled then instr_null else instr_int 1)
      ~default_dropthrough:None
      ~doc_comment:None
      [] [] None defs
  in
    body

open Closure_convert

let emit_fatal_program ~ignore_message op pos message =
  Iterator.reset_iterator ();
  let body = Emit_body.make_body
    (
      gather [
        optional ignore_message
          [instr (IComment "Ignore Fatal Parse message")];
        Emit_fatal.emit_fatal op pos message
      ]
    )
    [] (* decl_vars *)
    false (*is_memoize_wrapper*)
    false (*is_memoize_wrapper_lsb*)
    [] (* params *)
    None (* return_type_info *)
    None (* doc *)
    None (* env *)
  in
  Hhas_program.make
    true [] [] [] [] [] body Emit_symbol_refs.empty_symbol_refs None

let debugger_eval_should_modify ast =
  (* The AST currently always starts with a Markup statement, so a length of 2
     means there was 1 user def (statement, function, etc.); we assert that
     the first thing is a Markup statement, and we only want to modify if
     there was exactly one user def (both 0 user defs and > 1 user def are
     valid situations where we pass the program through unmodififed) *)
  begin match (List.hd ast) with
    | Some (Ast.Stmt (_, Ast.Markup _)) -> ()
    | _ -> failwith "Lowered AST did not start with a Markup statement"
  end;
  if List.length ast <> 2 then false else
  match List.nth_exn ast 1 with
    | Ast.Stmt (_, Ast.Expr _) -> true
    | _ -> false

let from_ast ~is_hh_file ?(is_js_file = false) ~is_evaled ~for_debugger_eval ~popt ast =
  Utils.try_finally
  ~f:begin fun () ->
    try
      Emit_env.set_is_hh_file is_hh_file;
      Emit_env.set_is_js_file is_js_file;
      (* Convert closures to top-level classes;
       * also hoist inner classes and functions *)
      let { ast_defs = closed_ast; global_state; strict_types } =
        convert_toplevel_prog ~popt ast in
      let strict_types =
        (* is scalar_types is set - always assume strict_types to have value *)
        if Hhbc_options.php7_scalar_types !(Hhbc_options.compiler_options)
        then begin match strict_types with
          | None -> Some false
          | _ -> strict_types
          end
        else None in
      Emit_env.set_global_state global_state;
      let flat_closed_ast = List.map ~f:snd closed_ast in
      let debugger_modify_program = for_debugger_eval && debugger_eval_should_modify ast in
      let compiled_defs = emit_main is_evaled debugger_modify_program popt flat_closed_ast in
      let compiled_funs = Emit_function.emit_functions_from_program closed_ast in
      let compiled_classes = Emit_class.emit_classes_from_program closed_ast in
      let compiled_typedefs = Emit_typedef.emit_typedefs_from_program flat_closed_ast in
      let compiled_file_attributes = Emit_file_attributes.emit_file_attributes_from_program flat_closed_ast in
      let adata = Emit_adata.get_adata () in
      let symbol_refs = Emit_symbol_refs.get_symbol_refs () in
      let hhas = Hhas_program.make is_hh_file adata compiled_funs
        compiled_classes compiled_typedefs compiled_file_attributes
        compiled_defs symbol_refs strict_types in
      hhas
    with Emit_fatal.IncludeTimeFatalException (op, pos, message) ->
      emit_fatal_program ~ignore_message:false op pos message
  end
  ~finally:begin fun () ->
    Emit_adata.reset ();
    Emit_symbol_refs.reset ();
    Emit_env.clear_global_state ();
  end
