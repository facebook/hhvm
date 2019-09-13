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

let is_not_acceptable ty =
  let finder =
    object (this)
      inherit [_] Type_visitor.locl_type_visitor

      method! on_tprim acc _ prim = prim = Aast.Tresource || acc

      (* We consider both dynamic and mixed to be unacceptable as
      they are "too broad" and imprecise *)
      method! on_tdynamic _ _ = true

      method! on_toption acc _ ty =
        match ty with
        | (_, Typing_defs.Tnonnull) -> true
        | _ -> this#on_type acc ty
    end
  in
  finder#on_type false ty

let print_ty ty =
  if is_not_acceptable ty then
    None
  else
    CodemodTypePrinter.print ty

(* Module which represents the collection of all inferred returned types.
It indexes those types with the span of the associated function (or method)
declaration *)
module Collector = struct
  type t = (Aast.pos * (Typing_env_types.env * Tast.ty)) array

  (* retrieve a type from a type_hint *)
  let type_of_type_hint env (ty, _) = (Pos.none, Tast_expand.expand_ty env ty)

  (* reduce visitor used to traverse a tast and collect any inferred hint
    annotation. *)
  let declaration_visitor =
    object
      inherit [_] Tast_visitor.reduce

      method zero = (0, [])

      method plus (len1, list1) (len2, list2) =
        if len1 < len2 then
          (len1 + len2, list1 @ list2)
        else
          (len1 + len2, list2 @ list1)

      method! on_fun_ (env : Tast_env.env) fun_ =
        let ty =
          snd
          @@ type_of_type_hint
               (Tast_env.restore_fun_env env fun_)
               fun_.Aast.f_ret
        in
        (1, [(fun_.Aast.f_span, (Tast_env.tast_env_as_typing_env env, ty))])

      method! on_method_ (env : Tast_env.env) method_ =
        let ty =
          snd
          @@ type_of_type_hint
               (Tast_env.restore_method_env env method_)
               method_.Aast.m_ret
        in
        let name = snd method_.Aast.m_name in
        let is_reserved = name = Naming_special_names.Members.__construct in
        if is_reserved then
          (0, [])
        else
          ( 1,
            [(method_.Aast.m_span, (Tast_env.tast_env_as_typing_env env, ty))]
          )
    end

  (* Build a collector object from a tast *)
  let make tast : t =
    Errors.ignore_ (fun () -> declaration_visitor#go tast)
    |> snd
    |> List.sort ~compare:(fun (p1, _) (p2, _) -> Pos.compare p1 p2)
    |> List.to_array

  let ( >>| ) x y = Option.map ~f:y x

  (* Compare disjunct positions.
    Return 0 if x is inside y, < 0 if x is before y and
    otherwise > 0. Behavior undefined for overlapping positions *)
  let compare_nonoverlap x y =
    let r = Pervasives.compare (Pos.filename x) (Pos.filename y) in
    if r <> 0 then
      r
    else
      let (xstart, xend) = Pos.info_raw x in
      let (ystart, yend) = Pos.info_raw y in
      if xend < ystart then
        xend - ystart
      else if yend < xstart then
        xstart - yend
      else
        0

  (* Given a syntax node, returned a string representing the corresponding
    inferred return type. Note that internally this uses the position of the
    node and will return the type in the collection whose position includes
    the position of the node. As we can't have nested function declarations
    in hack, we are sure that the span of the declarations are disjoints.

    Returns a string option where the string always represents a denotable
    type. If the type isn't denotable it will be logged.*)
  let get (collector : t) file node : string option =
    Option.Monad_infix.(
      match position file node with
      | None -> None
      | Some pos ->
        Array.binary_search
          ~compare:(fun (includes, _) pos -> -compare_nonoverlap pos includes)
          collector
          `First_equal_to
          pos
        >>| Array.get collector
        >>| snd
        >>= fun (env, ty) ->
        (match print_ty ty with
        | None ->
          Hh_logger.log
            "[infer return]: type %s non denotable in %s at position %s"
            (Typing_print.full env ty)
            (text node)
            (Pos.string (Pos.to_absolute pos));
          None
        | s -> s))
end

let get_patches tcopt file =
  let nast = Ast_provider.get_ast ~full:true file in
  let tcopt =
    TypecheckerOptions.set_infer_missing
      tcopt
      TypecheckerOptions.InferMissing.Infer_return
  in
  let tast = Typing.nast_to_tast tcopt (Naming.program nast) in
  let ret_collector = Collector.make tast in
  let source_text = Full_fidelity_source_text.from_file file in
  let positioned_tree = PositionedTree.make source_text in
  let root =
    Full_fidelity_editable_positioned_syntax.from_positioned_syntax
      (PositionedTree.root positioned_tree)
  in
  let get_patches node =
    match syntax node with
    | FunctionDeclarationHeader ({ function_type; _ } as fdh)
      when is_missing function_type ->
      Option.Monad_infix.(
        let patch =
          Collector.get ret_collector file fdh.function_name
          >>= fun type_str ->
          position_exclusive file function_type
          >>| fun pos ->
          ServerRefactorTypes.Insert
            ServerRefactorTypes.
              {
                pos = Pos.to_absolute pos;
                text = Printf.sprintf ": <<__Soft>> %s " type_str;
              }
        in
        Option.to_list patch)
    | _ -> []
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
