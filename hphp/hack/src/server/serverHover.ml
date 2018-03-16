(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_core
open HoverService

let symbols_at (file, line, char) tcopt =
  let contents = match file with
    | ServerUtils.FileName file_name ->
      let relative_path = Relative_path.(create Root file_name) in
      File_heap.get_contents relative_path
    | ServerUtils.FileContent content -> Some content
  in
  match contents with
  | None -> []
  | Some contents -> ServerIdentifyFunction.go contents line char tcopt

let type_at (file, line, char) tcopt files_info =
  let open Typing_defs in
  let _, tast = ServerIdeUtils.check_file_input tcopt files_info file in
  match ServerInferType.type_at_pos tast line char with
  | Some (_, (_, Tanon _) as infer_type_results1) ->
    (* The Tanon type doesn't include argument or return types, so it's
       displayed as "[fun]". To try to show something a little more useful, we
       call `returned_type_at_pos`. This will give us the function's return type
       (if it is being invoked). *)
    Some (Option.value
      (ServerInferType.returned_type_at_pos tast line char)
      ~default:infer_type_results1)
  | results -> results

let make_hover_info tcopt (file, _line, _char) env_and_ty (occurrence, def_opt) =
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
            Typing_print.full_with_identity env ty occurrence def_opt
        in
        begin match snippet_opt with
        | Some s -> s
        | None -> Typing_print.full_with_identity env ty occurrence def_opt
        end
    | occurrence, Some (env, ty) -> Typing_print.full_with_identity env ty occurrence def_opt
  in
  let addendum = [
    (match def_opt with
      | Some def ->
        begin match def.SymbolDefinition.docblock with
        | Some s -> [s]
        | None ->
          let def_file = match ((Pos.filename def.SymbolDefinition.pos), file) with
            | (p, ServerUtils.FileName fn) when p = Relative_path.default ->
              Relative_path.(create Root fn)
            | fn, _ -> fn
          in
          let def_line = (Pos.end_line def.SymbolDefinition.pos) in
          Docblock_finder.find_single_docblock ~tidy:true def_file def_line
          |> Option.to_list
        end
      | None -> []);
    (match occurrence, env_and_ty with
      | { type_ = Method _; _ }, _
      | { type_ = Property _; _ }, Some (_, (_, Tfun _))
      | { type_ = ClassConst _; _ }, Some (_, (_, Tfun _)) ->
        let name = match def_opt with
          | Some def -> def.SymbolDefinition.full_name
          | None -> occurrence.name
        in
        [Printf.sprintf "Full name: `%s`" (Utils.strip_ns name)]
      | _ -> []);
  ] |> List.concat in
  { snippet; addendum }

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
    | Some (env, ty) -> [{ snippet = Typing_print.full_strip_ns env ty; addendum = [] }]
    | None -> []
    end
  | identities ->
    identities
    |> IdentifySymbolService.filter_redundant
    |> List.map ~f:(make_hover_info tcopt (file, line, char) env_and_ty)
