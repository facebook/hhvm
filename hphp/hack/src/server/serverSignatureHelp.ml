(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
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
let get_positional_info (cst : Syntax.t) (file_offset : int) :
    ((int * int) * int) option =
  Syntax.(
    let parent_tree = parentage cst file_offset in
    (* Search upwards through the parent tree.
     * If we find a function call or constructor, signature help should appear.
     * If we find a lambda first, don't offer help even if we are within a function call! *)
    let within_lambda =
      Option.value
        ~default:false
        (List.find_map parent_tree ~f:(fun syntax ->
             match syntax.syntax with
             | LambdaExpression _ -> Some true
             | ConstructorCall _
             | FunctionCallExpression _ ->
               Some false
             | _ -> None))
    in
    if within_lambda then
      None
    else
      parent_tree
      |> List.find_map ~f:(fun syntax ->
             match syntax.syntax with
             | FunctionCallExpression children ->
               Some
                 ( children.function_call_receiver,
                   children.function_call_argument_list )
             | ConstructorCall children ->
               Some
                 ( children.constructor_call_type,
                   children.constructor_call_argument_list )
             | _ -> None)
      >>= fun (callee_node, argument_list) ->
      trailing_token callee_node >>= fun callee_trailing_token ->
      let function_symbol_offset = Token.end_offset callee_trailing_token in
      let pos =
        SourceText.offset_to_position
          callee_trailing_token.Token.source_text
          function_symbol_offset
      in
      let arguments = children argument_list in
      (* Add 1 to counteract the -1 in trailing_end_offset. *)
      let in_args_area =
        leading_start_offset argument_list <= file_offset
        && file_offset <= trailing_end_offset argument_list + 1
      in
      if not in_args_area then
        None
      else
        match arguments with
        | [] -> Some (pos, 0)
        | arguments ->
          arguments
          |> List.mapi ~f:(fun idx elem -> (idx, elem))
          |> List.find_map ~f:(fun (idx, child) ->
                 (* Don't bother range checking if we're in the final argument, since we
                    already checked that in in_args_area up above. *)
                 let matches_end =
                   idx = List.length arguments - 1
                   || file_offset < trailing_end_offset child
                 in
                 if matches_end then
                   Some (pos, idx)
                 else
                   None))

let get_occurrence_info
    (ctx : Provider_context.t)
    (nast : Nast.program)
    (occurrence : Relative_path.t SymbolOccurrence.t) =
  let module SO = SymbolOccurrence in
  let (ft_opt, full_occurrence) =
    (* Handle static methods, instance methods, and constructors *)
    match occurrence.SO.type_ with
    | SO.Method (SO.ClassName classname, methodname) ->
      let classname = Utils.add_ns classname in
      let ft =
        Decl_provider.get_class ctx classname
        |> Decl_entry.to_option
        |> Option.bind ~f:(fun cls ->
               if String.equal methodname "__construct" then
                 Decl_provider.Class.construct cls |> fst
               else
                 Option.first_some
                   (Decl_provider.Class.get_method cls methodname)
                   (Decl_provider.Class.get_smethod cls methodname))
        |> Option.map ~f:(fun class_elt ->
               (* We'll convert class_elt to fun_decl here solely as a lazy
                  convenience, so that the "display" code below can display
                  both class_elt and fun_decl uniformally. *)
               {
                 fe_module = None;
                 fe_internal = false;
                 Typing_defs.fe_pos = Lazy.force class_elt.Typing_defs.ce_pos;
                 fe_type = Lazy.force class_elt.Typing_defs.ce_type;
                 fe_deprecated = class_elt.Typing_defs.ce_deprecated;
                 fe_php_std_lib = false;
                 fe_support_dynamic_type =
                   Typing_defs.get_ce_support_dynamic_type class_elt;
                 fe_no_auto_dynamic = false;
                 fe_no_auto_likes = false;
               })
      in
      (ft, occurrence)
    | _ ->
      let fun_name =
        Utils.expand_namespace
          (ParserOptions.auto_namespace_map (Provider_context.get_popt ctx))
          occurrence.SO.name
      in
      let ft = Decl_provider.get_fun ctx fun_name |> Decl_entry.to_option in
      let full_occurrence =
        match occurrence.SO.type_ with
        | SO.Function -> { occurrence with SO.name = fun_name }
        | _ -> occurrence
      in
      (ft, full_occurrence)
  in
  let def_opt = ServerSymbolDefinition.go ctx (Some nast) full_occurrence in
  match ft_opt with
  | None -> None
  | Some ft -> Some (occurrence, ft, def_opt)

let go_quarantined
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(line : int)
    ~(column : int) : Lsp.SignatureHelp.result =
  let source_text = Ast_provider.compute_source_text ~entry in
  let offset = SourceText.position_to_offset source_text (line, column) in
  let cst = Ast_provider.compute_cst ~ctx ~entry in
  match
    get_positional_info (Provider_context.PositionedSyntaxTree.root cst) offset
  with
  | None -> None
  | Some ((symbol_line, symbol_char), argument_idx) ->
    let results =
      IdentifySymbolService.go_quarantined
        ~ctx
        ~entry
        ~line:symbol_line
        ~column:symbol_char
    in
    let results =
      List.filter results ~f:(fun r ->
          match r.SymbolOccurrence.type_ with
          | SymbolOccurrence.Method _
          | SymbolOccurrence.Function ->
            true
          | _ -> false)
    in
    (match List.hd results with
    | None -> None
    | Some head_result ->
      let ast =
        Ast_provider.compute_ast ~popt:(Provider_context.get_popt ctx) ~entry
      in
      (match get_occurrence_info ctx ast head_result with
      | None -> None
      | Some (occurrence, fe, def_opt) ->
        let open Typing_defs in
        let open Lsp.SignatureHelp in
        let tast_env = Tast_env.empty ctx in
        let siginfo_label =
          Tast_env.print_ty_with_identity
            tast_env
            (DeclTy fe.fe_type)
            occurrence
            def_opt
        in
        let siginfo_documentation =
          let base_class_name = SymbolOccurrence.enclosing_class occurrence in
          def_opt >>= fun def ->
          let path = def.SymbolDefinition.pos |> Pos.filename in
          let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
          ServerDocblockAt.go_comments_for_symbol_ctx
            ~ctx
            ~entry
            ~def
            ~base_class_name
        in
        let param_docs =
          match siginfo_documentation with
          | Some siginfo_documentation ->
            Some
              (Docblock_parser.get_param_docs ~docblock:siginfo_documentation)
          | None -> None
        in
        let ft_params =
          match get_node fe.fe_type with
          | Tfun ft -> ft.ft_params
          | _ -> []
        in
        let params =
          List.map ft_params ~f:(fun param ->
              let parinfo_label =
                match param.fp_name with
                | Some s -> s
                | None -> Tast_env.print_decl_ty tast_env fe.fe_type
              in
              let parinfo_documentation =
                match param_docs with
                | Some param_docs -> Map.find param_docs parinfo_label
                | None -> None
              in
              { parinfo_label; parinfo_documentation })
        in
        let signature_information =
          { siginfo_label; siginfo_documentation; parameters = params }
        in
        Some
          {
            signatures = [signature_information];
            activeSignature = 0;
            activeParameter = argument_idx;
          }))
