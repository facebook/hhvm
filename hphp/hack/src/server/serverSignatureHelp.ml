(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Option.Monad_infix

module SourceText = Full_fidelity_source_text
module Syntax = Full_fidelity_positioned_syntax

(** Returns ((symbol_line, symbol_char), argument_idx) where:
    - symbol_line: the line number of the function symbol
    - symbol_char: the column number of the function symbol
    - argument_idx: index of the function argument that contains the offset.

    For example, given this line:

        25:    $myObject->foo(true, null);
        offset:                    ^

    We would return the following:

        Some ((25, 16), 1)

    Returns None if the given offset is not inside a function call.
*)
let get_positional_info (cst : Syntax.t) (file_offset : int) : ((int * int) * int) option =
  let open Syntax in
  parentage cst file_offset
  |> List.find_map ~f:begin fun syntax ->
    match syntax.syntax with
    | FunctionCallExpression children ->
      Some (children.function_call_receiver, children.function_call_argument_list)
    | ConstructorCall children ->
      Some (children.constructor_call_type, children.constructor_call_argument_list)
    | _ -> None
  end
  >>= fun (callee_node, argument_list) ->
  trailing_token callee_node
  >>= fun callee_trailing_token ->
  let function_symbol_offset = Token.end_offset callee_trailing_token in
  let pos =
    SourceText.offset_to_position callee_trailing_token.Token.source_text function_symbol_offset
  in
  let arguments = children argument_list in
  (* Add 1 to counteract the -1 in trailing_end_offset. *)
  let in_args_area =
    leading_start_offset argument_list <= file_offset &&
    file_offset <= trailing_end_offset argument_list + 1
  in
  if not in_args_area then None else
  match arguments with
  | [] ->
    Some (pos, 0)
  | arguments ->
    arguments
    |> List.mapi ~f:(fun idx elem -> (idx, elem))
    |> List.find_map ~f:begin fun (idx, child) ->
      (* Don't bother range checking if we're in the final argument, since we
         already checked that in in_args_area up above. *)
      let matches_end =
        idx = List.length arguments - 1 ||
        file_offset < trailing_end_offset child
      in
      if matches_end then Some (pos, idx) else None
    end

let get_occurrence_info ast tast (line, char) occurrence =
  let def_opt = ServerSymbolDefinition.go ast occurrence in
  ServerInferType.type_at_pos tast line char
  >>= fun (env, ty) ->
  let open Typing_defs in
  begin match snd ty with
  | Tfun ft -> Some ft
  | _ -> None
  end
  >>| fun ft -> (occurrence, env, ft, def_opt)

let go env (file, line, char) ~basic_only =
  let ServerEnv.{tcopt; _} = env in
  let tcopt = {
    tcopt with
    GlobalOptions.tco_dynamic_view = ServerDynamicView.dynamic_view_on ();
  } in
  let source_text = ServerCommandTypesUtils.source_tree_of_file_input file in
  let relative_path = source_text.SourceText.file_path in
  let parser_env = Full_fidelity_ast.make_env relative_path in
  let (mode, tree) = Full_fidelity_ast.parse_text parser_env source_text in
  let results = Full_fidelity_ast.lower_tree parser_env source_text mode tree in
  let ast = results.Full_fidelity_ast.ast in
  let tast = ServerIdeUtils.check_ast tcopt ast in
  let offset = SourceText.position_to_offset source_text (line, char) in
  get_positional_info (Full_fidelity_ast.PositionedSyntaxTree.root tree) offset
  >>= fun ((symbol_line, symbol_char), argument_idx) ->
  let results = IdentifySymbolService.go tast symbol_line symbol_char in
  List.hd results
  >>= get_occurrence_info ast tast (symbol_line, symbol_char)
  >>| fun (occurrence, typing_env, ft, def_opt) ->
  let open Typing_defs in
  let open Lsp.SignatureHelp in
  let siginfo_label =
    Tast_env.print_ty_with_identity typing_env (Reason.Rnone, Tfun ft) occurrence def_opt in
  let params =
    ft.ft_params
    |> List.map ~f:begin fun param ->
      let parinfo_label =
        match param.fp_name with
        | Some s -> s
        | None -> Tast_env.print_ty typing_env ft.ft_ret
      in
      { parinfo_label; parinfo_documentation = None }
    end
  in
  let siginfo_documentation =
    let base_class_name = SymbolOccurrence.enclosing_class occurrence in
    def_opt
    >>= fun def ->
    let file =
      ServerCommandTypes.FileName (def.SymbolDefinition.pos |> Pos.to_absolute |> Pos.filename) in
    ServerDocblockAt.go_def def ~base_class_name ~file ~basic_only
  in
  let signature_information = {
    siginfo_label;
    siginfo_documentation;
    parameters = params;
  }
  in
  {
    signatures = [signature_information];
    activeSignature = 0;
    activeParameter = argument_idx;
  }
