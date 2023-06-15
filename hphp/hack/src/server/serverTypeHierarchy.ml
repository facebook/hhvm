(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open ServerTypeHierarchyTypes

let go_quarantined
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(line : int)
    ~(column : int) : ServerTypeHierarchyTypes.result =
  let _ast =
    Ast_provider.compute_ast ~popt:(Provider_context.get_popt ctx) ~entry
  in
  let () =
    begin
      Printf.printf "%i %i" line column
    end
  in
  []

let json_of_hierarchy_entry (entry : hierarchyEntry) =
  Hh_json.JSON_Object
    [
      ("name", Hh_json.string_ entry.name);
      ("kind", Hh_json.string_ (show_entryKind entry.kind));
      ("pos", Hh_json.string_ (Pos.string_no_file entry.pos));
      ("ancestors", Hh_json.array_ Hh_json.string_ entry.ancestors);
    ]

let json_of_results ~(results : result) =
  Hh_json.array_ json_of_hierarchy_entry results
