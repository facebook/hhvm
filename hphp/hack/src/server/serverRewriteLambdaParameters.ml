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
module Rewriter = Full_fidelity_rewriter.WithSyntax(Syntax)
module PositionedTree = Full_fidelity_syntax_tree
  .WithSyntax(Full_fidelity_positioned_syntax)

open Syntax

module LambdaParameterType : sig
  val print : Typing_defs.locl Typing_defs.ty -> string option
end = struct
  open Typing_defs

  exception Non_denotable

  let print_tprim =
    let open Nast in
    function
    | Tbool -> "bool"
    | Tint -> "int"
    | Tfloat -> "float"
    | Tnum -> "num"
    | Tstring -> "string"
    | Tarraykey -> "arraykey"
    | Tnull | Tvoid | Tresource | Tnoreturn -> raise Non_denotable

  let rec print_ty_exn ty =
    match snd ty with
    | Tprim p -> print_tprim p
    | Tany | Terr | Tvar _ | Tabstract ((AKdependent _ | AKgeneric _), _)
    | Tanon _ | Tunresolved _ | Tobject | Tarraykind (AKany | AKempty) ->
      raise Non_denotable
    | Tnonnull -> "nonnull"
    | Tdynamic -> "dynamic"
    | Tabstract (AKenum name, _) -> Utils.strip_ns name
    | Toption (_, Tnonnull) -> "mixed"
    | Toption ty -> "?" ^ print_ty_exn ty
    | Tfun ft ->
      let params = List.map ft.ft_params ~f:print_fun_param_exn in
      let params = match ft.ft_arity with
        | Fstandard _ -> params
        | Fellipsis _ -> raise Non_denotable
        | Fvariadic (_, p) -> params @ [print_ty_exn p.fp_type ^ "..."] in
      Printf.sprintf "(function(%s): %s)"
        (String.concat ~sep:", " params) (print_ty_exn ft.ft_ret)
    | Ttuple tyl ->
      "(" ^ print_tyl_exn tyl ^ ")"
    | Tshape (fields_known, fdm) ->
      let fields =
        List.map (Nast.ShapeMap.elements fdm) ~f:print_shape_field_exn in
      let fields = match fields_known with
        | FieldsFullyKnown -> fields
        | FieldsPartiallyKnown _ -> fields @ ["..."] in
      Printf.sprintf "shape(%s)" (String.concat ~sep:", " fields)
    | Tabstract (AKnewtype (name, []), _) -> Utils.strip_ns name
    | Tabstract (AKnewtype (name, tyl), _) ->
      Utils.strip_ns name ^ "<" ^ print_tyl_exn tyl ^ ">"
    | Tclass ((_, name), _, []) -> Utils.strip_ns name
    | Tclass ((_, name), _, tyl) ->
      Utils.strip_ns name ^ "<" ^ print_tyl_exn tyl ^ ">"
    | Tarraykind (AKvarray ty) ->
      Printf.sprintf "varray<%s>" (print_ty_exn ty)
    | Tarraykind (AKvarray_or_darray ty) ->
      Printf.sprintf "varray_or_darray<%s>" (print_ty_exn ty)
    | Tarraykind (AKvec ty) ->
      Printf.sprintf "array<%s>" (print_ty_exn ty)
    | Tarraykind (AKdarray (ty1, ty2)) ->
      Printf.sprintf "darray<%s, %s>" (print_ty_exn ty1) (print_ty_exn ty2)
    | Tarraykind (AKmap (ty1, ty2)) ->
      Printf.sprintf "array<%s, %s>" (print_ty_exn ty1) (print_ty_exn ty2)

  and print_tyl_exn tyl =
    String.concat ~sep:", " (List.map tyl ~f:print_ty_exn)

  and print_fun_param_exn param =
    match param.fp_kind with
    | FPinout -> "inout " ^ print_ty_exn param.fp_type
    | _ -> print_ty_exn param.fp_type

  and print_shape_field_exn (name, {sft_optional; sft_ty; _}) =
    Printf.sprintf "%s%s => %s"
      (if sft_optional then "?" else "")
      (print_shape_field_name name)
      (print_ty_exn sft_ty)

  and print_shape_field_name name =
    let s = Typing_env.get_shape_field_name name in
    match name with
    | Ast.SFlit_str _ -> "'" ^ s ^ "'"
    | _ -> s

  let print ty = try Some (print_ty_exn ty) with Non_denotable -> None
end

let get_first_suggested_type_as_string file type_map node =
  let open Option.Monad_infix in
  position file node >>= fun pos ->
  Tast_type_collector.get_from_pos_map
    (Pos.to_absolute pos) type_map >>= fun tys ->
  List.find_map tys ~f:(fun (env, phase_ty) ->
    match phase_ty with
    | Typing_defs.LoclTy ty ->
      let env, ty = Tast_env.fold_unresolved env ty in
      let env, ty = Tast_env.expand_type env ty in
      begin match ty with
      | Typing_reason.Rsolve_fail _, ty_ ->
        begin match LambdaParameterType.print ty with
        | Some type_str -> Some type_str
        | None ->
          Hh_logger.log "%s failed to rewrite lambda parameter %s: the suggested type %s is non-denotable"
            (Pos.string (Pos.to_absolute pos))
            (text node)
            (Tast_env.print_ty env (Typing_reason.Rnone, ty_));
          None
        end
      | _ -> None
      end
    | Typing_defs.DeclTy _ -> None)

let get_patches tcopt file =
  let ast = Parser_heap.get_from_parser_heap ~full:true file in
  let tast = Typing.nast_to_tast tcopt (Naming.program ast) in
  let type_map = Tast_type_collector.collect_types tast in
  let source_text = Full_fidelity_source_text.from_file file in
  let positioned_tree = PositionedTree.make source_text in
  let root = Full_fidelity_editable_positioned_syntax.from_positioned_syntax
    (PositionedTree.root positioned_tree) in
  let get_lambda_expression_patches node =
    let get_lambda_parameter_patches node =
      let patch =
        let open Option.Monad_infix in
        match syntax node with
        | Token _ ->
          get_first_suggested_type_as_string file type_map node >>= fun type_str ->
          position_exclusive file node >>| fun pos ->
          ServerRefactorTypes.Replace ServerRefactorTypes.{
            pos = Pos.to_absolute pos;
            text = Printf.sprintf "(%s %s)" type_str (text node);
          }
        | ListItem {list_item; _} ->
          begin match syntax list_item with
          | ParameterDeclaration _ ->
            get_first_suggested_type_as_string file type_map list_item >>= fun type_str ->
            position file list_item >>| fun pos ->
            ServerRefactorTypes.Insert ServerRefactorTypes.{
              pos = Pos.to_absolute pos;
              text = type_str ^ " ";
            }
          | _ -> None
          end
        | _ -> None in
      Option.to_list patch in
    match syntax node with
    | LambdaExpression {lambda_signature; _} ->
      begin match syntax lambda_signature with
      | Token _ ->
        get_lambda_parameter_patches lambda_signature
      | LambdaSignature {lambda_parameters; _} ->
        List.concat_map (syntax_node_to_list lambda_parameters)
          ~f:get_lambda_parameter_patches
      | _ -> []
      end
    | _ -> [] in
  let patches, _ = Rewriter.aggregating_rewrite_post
     (fun node patches ->
       get_lambda_expression_patches node @ patches, Rewriter.Result.Keep)
     root [] in
  patches
