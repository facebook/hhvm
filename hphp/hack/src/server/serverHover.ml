(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open HoverService

(** When we get a Class occurrence and a Method occurrence, that means that the
user is hovering over an invocation of the constructor, and would therefore only
want to see information about the constructor, rather than getting both the
class and constructor back in the hover. *)
let filter_class_and_constructor results =
  let result_is_constructor result =
    SymbolOccurrence.is_constructor (fst result)
  in
  let result_is_class result = SymbolOccurrence.is_class (fst result) in
  let has_class = List.exists results ~f:result_is_class in
  let has_constructor = List.exists results ~f:result_is_constructor in
  if has_class && has_constructor then
    List.filter results ~f:result_is_constructor
  else
    results

let make_hover_doc_block file occurrence def_opt =
  match def_opt with
  | Some def ->
    let base_class_name = SymbolOccurrence.enclosing_class occurrence in
    ServerDocblockAt.go_comments_for_symbol ~def ~base_class_name ~file
    |> Option.to_list
  | None -> []

let make_hover_return_type env_and_ty occurrence =
  SymbolOccurrence.(
    Typing_defs.(
      match (occurrence, env_and_ty) with
      | ({ type_ = Function | Method _; _ }, Some (env, (_, Tfun ft))) ->
        [
          Printf.sprintf
            "Return type: `%s`"
            (Tast_env.print_ty env ft.ft_ret.et_type);
        ]
      | _ -> []))

let make_hover_full_name env_and_ty occurrence def_opt =
  SymbolOccurrence.(
    Typing_defs.(
      match (occurrence, env_and_ty) with
      | ({ type_ = Method _; _ }, _)
      | ({ type_ = Property _ | ClassConst _; _ }, Some (_, (_, Tfun _))) ->
        let name =
          match def_opt with
          | Some def -> def.SymbolDefinition.full_name
          | None -> occurrence.name
        in
        [Printf.sprintf "Full name: `%s`" (Utils.strip_ns name)]
      | _ -> []))

let make_hover_info env_and_ty file (occurrence, def_opt) =
  SymbolOccurrence.(
    Typing_defs.(
      let snippet =
        match (occurrence, env_and_ty) with
        | ({ name; _ }, None) -> Utils.strip_ns name
        | ({ type_ = Method (classname, name); _ }, Some (env, ty))
          when name = Naming_special_names.Members.__construct ->
          let snippet_opt =
            Option.Monad_infix.(
              Decl_provider.get_class classname
              >>= fun c ->
              fst (Decl_provider.Class.construct c)
              >>| fun elt ->
              let ty = Lazy.force_val elt.ce_type in
              Tast_env.print_ty_with_identity
                env
                (DeclTy ty)
                occurrence
                def_opt)
          in
          begin
            match snippet_opt with
            | Some s -> s
            | None ->
              Tast_env.print_ty_with_identity
                env
                (LoclTy ty)
                occurrence
                def_opt
          end
        | (occurrence, Some (env, ty)) ->
          Tast_env.print_ty_with_identity env (LoclTy ty) occurrence def_opt
      in
      let addendum =
        List.concat
          [
            make_hover_doc_block file occurrence def_opt;
            make_hover_return_type env_and_ty occurrence;
            make_hover_full_name env_and_ty occurrence def_opt;
          ]
      in
      HoverService.
        { snippet; addendum; pos = Some occurrence.SymbolOccurrence.pos }))

let go_ctx
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(line : int)
    ~(column : int) : HoverService.result =
  let identities = ServerIdentifyFunction.go_ctx ~ctx ~entry ~line ~column in
  let env_and_ty =
    ServerInferType.type_at_pos
      (Provider_utils.compute_tast ~ctx ~entry)
      line
      column
    |> Option.map ~f:(fun (env, ty) -> (env, Tast_expand.expand_ty env ty))
  in
  (* There are legitimate cases where we expect to have no identities returned,
     so just format the type. *)
  match identities with
  | [] ->
    begin
      match env_and_ty with
      | Some (env, ty) ->
        [{ snippet = Tast_env.print_ty env ty; addendum = []; pos = None }]
      | None -> []
    end
  | identities ->
    identities
    |> filter_class_and_constructor
    |> List.map ~f:(fun (occurrence, def_opt) ->
           let path =
             def_opt
             |> Option.map ~f:(fun def -> def.SymbolDefinition.pos)
             |> Option.map ~f:Pos.filename
             |> Option.value ~default:entry.Provider_context.path
           in
           let file_input = Provider_context.get_file_input ~ctx ~path in
           make_hover_info env_and_ty file_input (occurrence, def_opt))
    |> List.remove_consecutive_duplicates ~equal:( = )
