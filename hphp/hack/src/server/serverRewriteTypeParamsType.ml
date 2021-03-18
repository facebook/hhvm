(**
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
module Phase = Typing_phase

(* retrieve a type from a type_hint *)
let type_of_type_hint env (ty, _) = Tast_expand.expand_ty env ty

open Syntax

let parameter_type_collector =
  object (self)
    inherit [_] Tast_visitor.reduce

    method zero = Pos.AbsolutePosMap.empty

    method plus = Pos.AbsolutePosMap.union ~combine:(fun _ a b -> Some (a @ b))

    (* Here we want to rewrite some type hints to a more "complete" version.
    To do that we proceed in several steps:
    1) Find type parameters with some type hints
    2) Compare the localized type hint with the the inferred hint. If they
     are the same then we didn't rewrote it so we leave the annotation as
     if. Otherwise we replace the annotations
  *)
    method! on_fun_param env fun_param =
      let tenv = Tast_env.tast_env_as_typing_env env in
      let inferred_hint =
        type_of_type_hint env fun_param.Aast.param_type_hint
      in
      let hintopt = Aast.hint_of_type_hint fun_param.Aast.param_type_hint in
      match hintopt with
      | None -> self#zero
      | Some hint ->
        let (_env, ty) =
          Phase.localize_hint_with_self tenv ~ignore_errors:true hint
        in
        if Typing_defs.equal_locl_ty inferred_hint ty then
          self#zero
        else
          Pos.AbsolutePosMap.singleton
            (Pos.to_absolute fun_param.Aast.param_pos)
            [(env, Typing_defs.LoclTy inferred_hint)]
  end

let collect_types tast =
  Errors.ignore_ (fun () -> parameter_type_collector#go tast)

let is_not_acceptable ty =
  let finder =
    object
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
          begin
            match Typing_defs.get_node ty with
            | ty_ ->
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
                       (Typing_defs.mk (Typing_reason.Rnone, ty_)));
                  None
              end
          end
        | Typing_defs.DeclTy _ -> None))

let get_patches ctx file =
  let nast = Ast_provider.get_ast ~full:true ctx file in
  let ctx =
    Provider_context.map_tcopt ctx ~f:TypecheckerOptions.set_global_inference
  in
  let tast =
    (* [Infer_params] is not implemented with a TAST check, so we can skip TAST
    checks safely. *)
    Typing_toplevel.nast_to_tast
      ~do_tast_checks:false
      ctx
      (Naming.program ctx nast)
  in
  let type_map = collect_types ctx tast in
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
                        syntax =
                          ParameterDeclaration
                            { parameter_type; parameter_name; _ };
                        _;
                      };
                    _;
                  }
                when not @@ is_missing parameter_type ->
                get_first_suggested_type_as_string file type_map parameter_name
                >>= fun type_str ->
                position file parameter_type >>| fun pos ->
                ServerRefactorTypes.Replace
                  ServerRefactorTypes.
                    {
                      pos = Pos.to_absolute (Pos.advance_one pos);
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
