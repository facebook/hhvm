(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let get_class id =
  match Naming_table.Types.get_pos id with
  | None
  | Some (_, Naming_table.TTypedef) -> None
  | Some (pos, Naming_table.TClass) ->
    let fn = FileInfo.get_pos_filename pos in
    match Parser_heap.find_class_in_file fn id with
    | None -> None
    | Some class_ ->
      Some (Naming.class_ class_)

let get_fun id =
  match Naming_table.Funs.get_pos id with
  | None -> None
  | Some pos ->
    let fn = FileInfo.get_pos_filename pos in
    match Parser_heap.find_fun_in_file fn id with
    | None -> None
    | Some fun_ ->
      Some (Naming.fun_ fun_)
