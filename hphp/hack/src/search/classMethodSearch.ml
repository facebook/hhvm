(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
*)

open Core_kernel

let get_class_definition_file class_name =
  let class_def = Naming_table.Types.get_pos class_name in
  match class_def with
  | Some (pos, Naming_table.TClass) ->
    let file =
      match pos with
      | FileInfo.Full pos -> Pos.filename pos
      | FileInfo.File (_, file) -> file
    in
    Some file
  | _ -> None

let query_class_methods
    (class_name: string)
    (method_query: string): SearchUtils.result =
  let open Option.Monad_infix in
  let method_query = String.lowercase method_query in
  let matches_query method_name =
    if String.length method_query > 0 then begin
      let method_name = String.lowercase method_name in
      String_utils.is_substring method_query method_name
    end else true
  in
  get_class_definition_file class_name
  >>= (fun file -> Parser_heap.find_class_in_file file class_name)
  >>| (fun class_ -> class_.Ast.c_body)
  >>| List.filter_map ~f:begin fun class_elt ->
    match class_elt with
    | Ast.Method Ast.{m_kind; m_name = (pos, name); _}
      when matches_query name ->
      let is_static = List.mem ~equal:(=) m_kind Ast.Static in
      Some SearchUtils. {
          name;
          pos = (Pos.to_absolute pos);
          result_type = Method (is_static, class_name)
        }
    | _ -> None
  end
  |> Option.value ~default:[]
