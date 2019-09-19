(*
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

let query_class_methods (class_name : string) (method_query : string) :
    SearchUtils.result =
  Option.Monad_infix.(
    let method_query = String.lowercase method_query in
    let matches_query method_name =
      if String.length method_query > 0 then
        let method_name = String.lowercase method_name in
        String_utils.is_substring method_query method_name
      else
        true
    in
    get_class_definition_file class_name
    >>= (fun file -> Ast_provider.find_class_in_file file class_name)
    >>| (fun class_ -> class_.Aast.c_methods)
    >>| List.filter_map ~f:(fun m ->
            let (pos, name) = m.Aast.m_name in
            if matches_query name then
              Some
                SearchUtils.
                  {
                    name;
                    pos = Pos.to_absolute pos;
                    result_type = SearchUtils.SI_ClassMethod;
                  }
            else
              None)
    |> Option.value ~default:[])
