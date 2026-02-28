(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let query_class_methods
    (ctx : Provider_context.t) (class_name : string) (method_query : string) :
    SearchUtils.result =
  Option.Monad_infix.(
    let method_query = String.lowercase method_query in
    let matches_query method_name =
      if String.length method_query > 0 then
        let method_name = String.lowercase method_name in
        String.is_substring ~substring:method_query method_name
      else
        true
    in
    Naming_provider.get_class_path ctx class_name
    >>= (fun file ->
          Ast_provider.find_class_in_file ctx file class_name ~full:false)
    >>| (fun class_ -> class_.Aast.c_methods)
    >>| List.filter_map ~f:(fun m ->
            let (pos, name) = m.Aast.m_name in
            if matches_query name then
              Some
                SearchUtils.
                  {
                    name;
                    pos = Pos.to_absolute pos;
                    result_type = FileInfo.SI_ClassMethod;
                  }
            else
              None)
    |> Option.value ~default:[])
