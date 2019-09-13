(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
module Syntax = Full_fidelity_editable_positioned_syntax
module Rewriter = Full_fidelity_rewriter.WithSyntax (Syntax)
module PositionedTree =
  Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax)
open Syntax

(* retrieve a type from a type_hint *)
let type_of_type_hint env (ty, _) = Tast_expand.expand_ty env ty

let parameter_type_collector =
  object
    inherit [_] Tast_visitor.reduce

    method zero = Pos.AbsolutePosMap.empty

    method plus = Pos.AbsolutePosMap.union ~combine:(fun _ a b -> Some (a @ b))

    method! on_fun_param env fun_param =
      Pos.AbsolutePosMap.singleton
        (Pos.to_absolute fun_param.Aast.param_pos)
        [
          ( env,
            Typing_defs.LoclTy
              (type_of_type_hint env fun_param.Aast.param_type_hint) );
        ]

    (* Deactivate the codemod for methods *)
    method! on_method_ _ _ = Pos.AbsolutePosMap.empty
  end

let collect_types tast =
  Errors.ignore_ (fun () -> parameter_type_collector#go tast)

let is_not_acceptable ty =
  let finder =
    object (this)
      inherit [_] Type_visitor.locl_type_visitor

      method! on_tprim acc _ =
        function
        | Aast.Tvoid
        | Aast.Tresource
        | Aast.Tnoreturn ->
          true
        | _ -> acc

      (* We consider both dynamic and nothing to be non acceptable as
      they are "too narrow" and imprecise *)
      method! on_tdynamic _ _ = true

      (* mixed, even though we could infer it and add it, might lead to further
      problems and doesn't gives us a lot of information. More conceptually
      adding mixed annotations says "it is fine to add mixed types everywhere"
      which is not really fine. *)
      method! on_toption acc _ ty =
        match ty with
        | (_, Typing_defs.Tnonnull) -> true
        | _ -> this#on_type acc ty

      (* For now we deactivate class with type parameters as we have issues
      inferring a "good enough" type for these in certain cases *)
      method! on_tclass _ _ _ _ args = List.length args <> 0
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
    position file node
    >>= fun pos ->
    Tast_type_collector.get_from_pos_map (Pos.to_absolute pos) type_map
    >>= fun tys ->
    List.find_map tys ~f:(fun (env, phase_ty) ->
        match phase_ty with
        | Typing_defs.LoclTy ty ->
          let (env, ty) = Tast_env.fold_unresolved env ty in
          let (env, ty) = Tast_env.expand_type env ty in
          begin
            match ty with
            | (_, ty_) ->
              begin
                match print_ty ty with
                | Some type_str -> Some type_str
                | None ->
                  Hh_logger.log
                    "%s failed to rewrite lambda parameter %s: the suggested type %s is non-denotable"
                    (Pos.string (Pos.to_absolute pos))
                    (text node)
                    (Tast_env.print_ty env (Typing_reason.Rnone, ty_));
                  None
              end
          end
        | Typing_defs.DeclTy _ -> None))

let get_patches tcopt file =
  let nast = Ast_provider.get_ast ~full:true file in
  let tcopt =
    TypecheckerOptions.set_infer_missing
      tcopt
      TypecheckerOptions.InferMissing.Infer_params
  in
  let tast = Typing.nast_to_tast tcopt (Naming.program nast) in
  let type_map = collect_types tast in
  let source_text = Full_fidelity_source_text.from_file file in
  let positioned_tree = PositionedTree.make source_text in
  let root =
    Full_fidelity_editable_positioned_syntax.from_positioned_syntax
      (PositionedTree.root positioned_tree)
  in
  let get_patches node =
    Option.Monad_infix.(
      match syntax node with
      | FunctionDeclarationHeader
          { function_parameter_list = { syntax = SyntaxList lst; _ }; _ } ->
        List.concat_map lst ~f:(fun n ->
            let opt =
              match syntax n with
              | ListItem
                  {
                    list_item =
                      {
                        syntax = ParameterDeclaration { parameter_type; _ };
                        _;
                      } as list_item;
                    _;
                  }
                when is_missing parameter_type ->
                get_first_suggested_type_as_string file type_map list_item
                >>= fun type_str ->
                position file list_item
                >>| fun pos ->
                ServerRefactorTypes.Insert
                  ServerRefactorTypes.
                    {
                      pos = Pos.to_absolute pos;
                      text = "<<__Soft>> " ^ type_str ^ " ";
                    }
              | _ -> None
            in
            Option.to_list opt)
      | _ -> [])
  in
  let (patches, _) =
    Rewriter.aggregating_rewrite_post
      (fun node patches ->
        let a = get_patches node @ patches in
        (a, Rewriter.Result.Keep))
      root
      []
  in
  patches
