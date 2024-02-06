(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module Syntax = Full_fidelity_editable_positioned_syntax
module Rewriter = Full_fidelity_rewriter.WithSyntax (Syntax)
module PositionedTree =
  Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax)
open Syntax

let is_not_acceptable ty =
  let finder =
    object
      inherit [_] Type_visitor.locl_type_visitor

      method! on_tprim acc _ =
        function
        | Aast.Tnull
        | Aast.Tvoid
        | Aast.Tresource
        | Aast.Tnoreturn ->
          true
        | _ -> acc
    end
  in
  finder#on_type false ty

let print_ty ty =
  if is_not_acceptable ty then
    None
  else
    CodemodTypePrinter.print ty

let get_first_suggested_type_as_string file type_map node =
  Option.Monad_infix.(
    position file node >>= fun pos ->
    Tast_type_collector.get_from_pos_map (Pos.to_absolute pos) type_map
    >>= fun tys ->
    List.find_map tys ~f:(fun (env, phase_ty) ->
        match phase_ty with
        | Typing_defs.LoclTy ty ->
          let (env, ty) = Tast_env.simplify_unions env ty in
          let (env, ty) = Tast_env.expand_type env ty in
          begin
            match Typing_defs.deref ty with
            | (_, Typing_defs.Tnewtype ("HackSuggest", [ty], _)) ->
              let (env, ty) = Tast_env.simplify_unions env ty in
              let ty = Tast_env.fully_expand env ty in
              begin
                match print_ty ty with
                | Some type_str -> Some type_str
                | None ->
                  Hh_logger.log
                    "%s failed to rewrite lambda parameter %s: the suggested type %s is non-denotable"
                    (Pos.string (Pos.to_absolute pos))
                    (text node)
                    (Tast_env.print_ty
                       env
                       (Typing_defs.mk
                          (Typing_reason.Rnone, Typing_defs.get_node ty)));
                  None
              end
            | _ -> None
          end
        | Typing_defs.DeclTy _ -> None))

let get_patches ctx file =
  let nast = Ast_provider.get_ast ~full:true ctx file in
  let tast =
    (* We don't need an accurate list of typing errors, so we can skip TAST
       checks. *)
    Typing_toplevel.nast_to_tast
      ~do_tast_checks:false
      ctx
      (Naming.program ctx nast)
  in
  let type_map =
    Tast_type_collector.collect_types
      ctx
      tast.Tast_with_dynamic.under_normal_assumptions
  in
  let source_text = Full_fidelity_source_text.from_file file in
  let positioned_tree = PositionedTree.make source_text in
  let root =
    Full_fidelity_editable_positioned_syntax.from_positioned_syntax
      (PositionedTree.root positioned_tree)
  in
  let get_lambda_expression_patches node =
    let get_lambda_parameter_patches node =
      let patch =
        Option.Monad_infix.(
          match syntax node with
          | Token _ ->
            get_first_suggested_type_as_string file type_map node
            >>= fun type_str ->
            position_exclusive file node >>| fun pos ->
            ServerRenameTypes.Replace
              ServerRenameTypes.
                {
                  pos = Pos.to_absolute pos;
                  text = Printf.sprintf "(%s %s)" type_str (text node);
                }
          | ListItem { list_item; _ } -> begin
            match syntax list_item with
            | ParameterDeclaration _ ->
              get_first_suggested_type_as_string file type_map list_item
              >>= fun type_str ->
              position file list_item >>| fun pos ->
              ServerRenameTypes.Insert
                ServerRenameTypes.
                  { pos = Pos.to_absolute pos; text = type_str ^ " " }
            | _ -> None
          end
          | _ -> None)
      in
      Option.to_list patch
    in
    match syntax node with
    | LambdaExpression { lambda_signature; _ } -> begin
      match syntax lambda_signature with
      | Token _ -> get_lambda_parameter_patches lambda_signature
      | LambdaSignature { lambda_parameters; _ } ->
        List.concat_map
          (syntax_node_to_list lambda_parameters)
          ~f:get_lambda_parameter_patches
      | _ -> []
    end
    | _ -> []
  in
  let (patches, _) =
    Rewriter.aggregating_rewrite_post
      (fun node patches ->
        (get_lambda_expression_patches node @ patches, Rewriter.Result.Keep))
      root
      []
  in
  patches
