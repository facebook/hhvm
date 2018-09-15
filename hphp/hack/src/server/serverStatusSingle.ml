(**
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_core
open ServerCommandTypes

let go fn tcopt =
  let contents, path = match fn with
    | FileName file_name ->
      let path = Relative_path.create_detect_prefix file_name in
      File_heap.get_contents path, path
    | FileContent content -> Some content, Relative_path.default
  in
  match contents with
  | None -> []
  | Some x ->
    ServerIdeUtils.get_errors path x tcopt |>
    Errors.get_sorted_error_list |>
    List.map ~f:Errors.to_absolute
