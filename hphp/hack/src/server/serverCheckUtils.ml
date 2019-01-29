(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let extend_fast fast naming_table additional_files =
  Relative_path.Set.fold additional_files ~init:fast ~f:begin fun x acc ->
    match Relative_path.Map.get fast x with
    | None ->
        (try
           let info = Naming_table.get_file_info_unsafe naming_table x in
           let info_names = FileInfo.simplify info in
           Relative_path.Map.add acc ~key:x ~data:info_names
         with Not_found ->
           acc)
    | Some _ -> acc
  end
