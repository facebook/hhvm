(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let get_class id =
  match Naming_table.Types.get_filename_and_kind id with
  | None
  | Some (_, Naming_table.TTypedef) ->
    None
  | Some (_, Naming_table.TRecordDef) -> None
  | Some (fn, Naming_table.TClass) ->
    (match Ast_provider.find_class_in_file fn id with
    | None -> None
    | Some class_ -> Some (Errors.ignore_ (fun () -> Naming.class_ class_)))

let get_fun id =
  match Naming_table.Funs.get_filename id with
  | None -> None
  | Some fn ->
    (match Ast_provider.find_fun_in_file fn id with
    | None -> None
    | Some fun_ -> Some (Naming.fun_ fun_))
