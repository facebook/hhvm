(**
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
    SymbolOccurrence.is_constructor (fst result) in
  let result_is_class result =
    SymbolOccurrence.is_class (fst result)  in
  let has_class = List.exists results ~f:result_is_class in
  let has_constructor = List.exists results ~f:result_is_constructor in
  if has_class && has_constructor
  then List.filter results ~f:result_is_constructor
  else results

let make_hover_doc_block ~basic_only file occurrence def_opt =
  match def_opt with
  | Some def ->
    let base_class_name = SymbolOccurrence.enclosing_class occurrence in
    ServerDocblockAt.go_comments_for_symbol ~def ~base_class_name ~file ~basic_only
    |> Option.to_list
  | None -> []

let make_hover_return_type env_and_ty occurrence =
  let open SymbolOccurrence in
  let open Typing_defs in
  match occurrence, env_and_ty with
  | { type_ = Function | Method _; _ }, Some (env, (_, Tfun ft)) ->
    [Printf.sprintf "Return type: `%s`" (Tast_env.print_ty env ft.ft_ret)]
  | _ -> []

let make_hover_full_name env_and_ty occurrence def_opt =
  let open SymbolOccurrence in
  let open Typing_defs in
  match occurrence, env_and_ty with
  | { type_ = Method _; _ }, _
  | { type_ = Property _ | ClassConst _; _ }, Some (_, (_, Tfun _)) ->
    let name = match def_opt with
      | Some def -> def.SymbolDefinition.full_name
      | None -> occurrence.name
    in
    [Printf.sprintf "Full name: `%s`" (Utils.strip_ns name)]
  | _ -> []

let make_hover_info env_and_ty file (occurrence, def_opt) ~basic_only =
  let open SymbolOccurrence in
  let open Typing_defs in
  let snippet = match occurrence, env_and_ty with
    | { name; _ }, None -> Utils.strip_ns name
    | { type_ = Method (classname, name); _ }, Some (env, ty)
      when name = Naming_special_names.Members.__construct ->
        let snippet_opt =
          let open Option.Monad_infix in
          Decl_provider.get_class classname
          >>= fun c -> fst (Decl_provider.Class.construct c)
          >>| fun elt ->
            let ty = Lazy.force_val elt.ce_type in
            Tast_env.print_ty_with_identity env ty occurrence def_opt
        in
        begin match snippet_opt with
        | Some s -> s
        | None -> Tast_env.print_ty_with_identity env ty occurrence def_opt
        end
    | occurrence, Some (env, ty) -> Tast_env.print_ty_with_identity env ty occurrence def_opt
  in
  let addendum = List.concat [
    make_hover_doc_block file occurrence def_opt ~basic_only;
    make_hover_return_type env_and_ty occurrence;
    make_hover_full_name env_and_ty occurrence def_opt;
  ]
  in
  HoverService.{ snippet; addendum; pos = Some occurrence.SymbolOccurrence.pos }

let go_ctx
    ~(ctx: ServerIdeContext.t)
    ~(entry: ServerIdeContext.entry)
    ~(line: int)
    ~(char: int)
    ~(basic_only: bool)
    : HoverService.result =
  let tast = ServerIdeContext.get_tast entry in
  let identities =
    ServerIdentifyFunction.go_ctx
      ~ctx
      ~entry
      ~line
      ~char in
  let env_and_ty = ServerInferType.type_at_pos tast line char
    |> Option.map ~f:(fun (env, ty) -> (env, Tast_expand.expand_ty env ty)) in
  (* There are legitimate cases where we expect to have no identities returned,
     so just format the type. *)
  match identities with
  | [] ->
    begin match env_and_ty with
    | Some (env, ty) ->
      [{ snippet = Tast_env.print_ty env ty; addendum = []; pos = None }]
    | None -> []
    end
  | identities ->
    identities
    |> filter_class_and_constructor
    |> List.map ~f:(fun (occurrence, def_opt) ->
      let path = def_opt
        |> Option.map ~f:(fun def -> def.SymbolDefinition.pos)
        |> Option.map ~f:Pos.filename
        |> Option.value ~default:(ServerIdeContext.get_path entry)
      in
      let file_input = ServerIdeContext.get_file_input ~ctx ~path in
      make_hover_info env_and_ty file_input (occurrence, def_opt) ~basic_only
    )
    |> List.remove_consecutive_duplicates ~equal:(=)
