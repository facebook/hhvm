(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Instruction_sequence
open Hhbc_ast

type t = {
  hhas_adata       : Hhas_adata.t list;
  hhas_fun         : Hhas_function.t list;
  hhas_classes     : Hhas_class.t list;
  hhas_typedefs    : Hhas_typedef.t list;
  hhas_main        : Hhas_body.t;
  hhas_symbol_refs : Hhas_symbol_refs.t;
}

let make hhas_adata hhas_fun hhas_classes hhas_typedefs hhas_main hhas_symbol_refs =
  { hhas_adata; hhas_fun; hhas_classes; hhas_typedefs; hhas_main; hhas_symbol_refs; }

let functions hhas_prog =
  hhas_prog.hhas_fun

let classes hhas_prog =
  hhas_prog.hhas_classes

let typedefs hhas_prog =
  hhas_prog.hhas_typedefs

let main hhas_prog =
  hhas_prog.hhas_main

let adata hhas_prog =
  hhas_prog.hhas_adata

let symbol_refs hhas_prog =
  hhas_prog.hhas_symbol_refs

let with_main hhas_prog hhas_main =
  {hhas_prog with hhas_main}

let with_fun hhas_prog hhas_fun =
  {hhas_prog with hhas_fun}

let with_classes hhas_prog hhas_classes =
  {hhas_prog with hhas_classes}

let with_typedefs hhas_prog hhas_typedefs =
  {hhas_prog with hhas_typedefs}

let with_adata hhas_prog hhas_adata =
  {hhas_prog with hhas_adata}

let emit_main defs =
  let body, _is_generator, _is_pair_generator =
    Emit_body.emit_body
      ~pos:Pos.none
      ~namespace:Namespace_env.empty_with_default_popt
      ~is_closure_body:false
      ~is_memoize:false
      ~is_async:false
      ~deprecation_info:None
      ~skipawaitable:false
      ~is_return_by_ref:false
      ~scope:Ast_scope.Scope.toplevel
      ~return_value:(instr_int 1)
      ~default_dropthrough:None
      ~doc_comment:None
      [] None defs
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
    [] (* params *)
    None (* return_type_info *)
    [] (* static_inits static_inits  *)
    None (* doc *)
  in
    make [] [] [] [] body Emit_symbol_refs.empty_symbol_refs

let pick_top_function_name_span f =
  match f with
  | true, A.Fun f -> Some ((snd f.A.f_name), f.A.f_span)
  | _ -> None

let ensure_functions_not_redeclared l =
  match Emit_function.find_first_redeclaration pick_top_function_name_span l with
  | None -> ()
  | Some (name, original_span, conflicting_span) ->
    let message =
      Printf.sprintf "Cannot redeclare %s() (previously declared in %s:%d)"
        name
        (Pos.filename (Pos.to_absolute original_span))
        (File_pos.line (Pos.pos_start original_span)) in
    Emit_fatal.raise_fatal_runtime
      conflicting_span
      message

let from_ast is_hh_file ast =
  Utils.try_finally
  ~f:begin fun () ->
    try
      Emit_env.set_is_hh_file is_hh_file;
      (* Convert closures to top-level classes;
       * also hoist inner classes and functions *)
      let closed_ast, global_state =
        convert_toplevel_prog ast in
      Emit_env.set_global_state global_state;
      let flat_closed_ast = List.map snd closed_ast in

      ensure_functions_not_redeclared closed_ast;

      let compiled_defs = emit_main flat_closed_ast in
      let compiled_funs = Emit_function.emit_functions_from_program closed_ast in
      let compiled_classes = Emit_class.emit_classes_from_program closed_ast in
      let compiled_typedefs = Emit_typedef.emit_typedefs_from_program flat_closed_ast in
      let adata = Emit_adata.get_adata () in
      let symbol_refs = Emit_symbol_refs.get_symbol_refs () in
      let hhas = make
        adata compiled_funs compiled_classes
        compiled_typedefs compiled_defs symbol_refs in
      hhas
    with Emit_fatal.IncludeTimeFatalException (op, pos, message) ->
      emit_fatal_program ~ignore_message:false op pos message
  end
  ~finally:begin fun () ->
    Emit_adata.reset ();
    Emit_symbol_refs.reset ();
    Emit_env.clear_global_state ();
  end
