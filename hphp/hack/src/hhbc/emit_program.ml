(*
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

let emit_main is_evaled debugger_modify_program namespace (defs : Tast.program)
    =
  let (body, _is_generator, _is_pair_generator) =
    Emit_body.emit_body
      ~pos:Pos.none
      ~namespace
      ~is_closure_body:false
      ~is_memoize:false
      ~is_native:false
      ~is_async:false
      ~is_rx_body:false
      ~debugger_modify_program
      ~deprecation_info:None
      ~skipawaitable:false
      ~scope:Ast_scope.Scope.toplevel
      ~return_value:
        ( if is_evaled then
          instr_null
        else
          instr_int 1 )
      ~default_dropthrough:None
      ~doc_comment:None
      ~immediate_tparams:[]
      ~class_tparam_names:[]
      []
      None
      defs
  in
  body

open Closure_convert

let emit_fatal_program ~ignore_message op pos message =
  Iterator.reset_iterator ();
  let body =
    Emit_body.make_body
      (gather
         [
           optional
             ignore_message
             [instr (IComment "Ignore Fatal Parse message")];
           Emit_fatal.emit_fatal op pos message;
         ])
      [] (* decl_vars *)
      false (*is_memoize_wrapper*)
      false (*is_memoize_wrapper_lsb*)
      [] (* upper bounds *)
      [] (* shadowed_tparams *)
      [] (* params *)
      None (* return_type_info *)
      None (* doc *)
      None
    (* env *)
  in
  Hhas_program.make
    true
    []
    []
    []
    []
    []
    []
    []
    body
    Emit_symbol_refs.empty_symbol_refs

let debugger_eval_should_modify ast =
  (* The AST currently always starts with a Markup statement, so a length of 2
     means there was 1 user def (statement, function, etc.); we assert that
     the first thing is a Markup statement, and we only want to modify if
     there was exactly one user def (both 0 user defs and > 1 user def are
     valid situations where we pass the program through unmodififed) *)
  begin
    match List.hd ast with
    | Some (Aast.Stmt (_, Aast.Markup _)) -> ()
    | _ -> failwith "Lowered AST did not start with a Markup statement"
  end;
  if List.length ast <> 2 then
    false
  else
    match List.nth_exn ast 1 with
    | Aast.Stmt (_, Aast.Expr _) -> true
    | _ -> false

let emit_program ~is_hh_file ~is_evaled ~for_debugger_eval ~empty_namespace tast
    =
  Utils.try_finally
    ~f:
      begin
        fun () ->
        try
          (* Convert closures to top-level classes;
           * also hoist inner classes and functions *)
          let { ast_defs = closed_ast; global_state } =
            convert_toplevel_prog ~empty_namespace ~for_debugger_eval tast
          in
          Emit_env.set_global_state global_state;
          let flat_closed_ast = List.map ~f:snd closed_ast in
          let debugger_modify_program =
            for_debugger_eval && debugger_eval_should_modify tast
          in
          let compiled_main =
            emit_main
              is_evaled
              debugger_modify_program
              empty_namespace
              flat_closed_ast
          in
          let compiled_funs =
            Emit_function.emit_functions_from_program closed_ast
          in
          let compiled_classes =
            Emit_class.emit_classes_from_program closed_ast
          in
          let compiled_records =
            Emit_record_def.emit_record_defs_from_program closed_ast
          in
          let compiled_typedefs =
            Emit_typedef.emit_typedefs_from_program flat_closed_ast
          in
          let env = Emit_env.(empty |> with_namespace empty_namespace) in
          let (compiled_constants, compiled_constant_funs) =
            Emit_constant.emit_constants_from_program env flat_closed_ast
          in
          let compiled_file_attributes =
            Emit_file_attributes.emit_file_attributes_from_program
              flat_closed_ast
          in
          let adata = Emit_adata.get_adata () in
          let symbol_refs = Emit_symbol_refs.get_symbol_refs () in
          let hhas =
            Hhas_program.make
              is_hh_file
              adata
              (compiled_funs @ compiled_constant_funs)
              compiled_classes
              compiled_records
              compiled_typedefs
              compiled_constants
              compiled_file_attributes
              compiled_main
              symbol_refs
          in
          hhas
        with Emit_fatal.IncludeTimeFatalException (op, pos, message) ->
          emit_fatal_program ~ignore_message:false op pos message
      end
    ~finally:
      begin
        fun () ->
        Emit_adata.reset ();
        Emit_symbol_refs.reset ();
        Emit_env.clear_global_state ()
      end
