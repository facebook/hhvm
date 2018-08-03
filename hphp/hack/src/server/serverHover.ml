(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core
open HoverService

let symbols_at (file, line, char) tcopt =
  let contents = match file with
    | ServerCommandTypes.FileName file_name ->
      let relative_path = Relative_path.(create Root file_name) in
      File_heap.get_contents relative_path
    | ServerCommandTypes.FileContent content -> Some content
  in
  match contents with
  | None -> []
  | Some contents -> ServerIdentifyFunction.go contents line char tcopt

let type_at (file, line, char) tcopt files_info =
  let tcopt = {
    tcopt with
    GlobalOptions.tco_dynamic_view = ServerDynamicView.dynamic_view_on ();
  } in
  let _, tast = ServerIdeUtils.check_file_input tcopt files_info file in
  ServerInferType.type_at_pos tast line char

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

let make_hover_doc_block tcopt file occurrence def_opt =
  match def_opt with
  | Some def ->
    let base_class_name = SymbolOccurrence.enclosing_class occurrence in
    ServerDocblockAt.go_def tcopt def ~base_class_name ~file
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

let make_hover_info tcopt env_and_ty file (occurrence, def_opt) =
  let open SymbolOccurrence in
  let open Typing_defs in
  let snippet = match occurrence, env_and_ty with
    | { name; _ }, None -> Utils.strip_ns name
    | { type_ = Method (classname, name); _ }, Some (env, ty)
      when name = Naming_special_names.Members.__construct ->
        let snippet_opt =
          let open Option.Monad_infix in
          Typing_lazy_heap.get_class tcopt classname
          >>= fun c -> fst c.tc_construct
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
    make_hover_doc_block tcopt file occurrence def_opt;
    make_hover_return_type env_and_ty occurrence;
    make_hover_full_name env_and_ty occurrence def_opt;
  ]
  in
  HoverService.{ snippet; addendum; pos = Some occurrence.SymbolOccurrence.pos }

let go env (file, line, char) =
  let position = (file, line, char) in
  let ServerEnv.{ tcopt; files_info; _ } = env in
  let identities = symbols_at position tcopt in
  let env_and_ty = type_at position tcopt files_info in
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
    |> List.map ~f:(make_hover_info tcopt env_and_ty file)
    |> List.remove_consecutive_duplicates ~equal:(=)
