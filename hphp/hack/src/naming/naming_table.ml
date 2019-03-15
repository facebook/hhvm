(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = FileInfo.t Relative_path.Map.t
type fast = FileInfo.names Relative_path.Map.t
type saved_state_info = FileInfo.saved Relative_path.Map.t

let combine a b = Relative_path.Map.union a b
let create a = a
let empty = Relative_path.Map.empty
let filter = Relative_path.Map.filter
let fold = Relative_path.Map.fold
let get_files = Relative_path.Map.keys
let get_file_info = Relative_path.Map.get
let get_file_info_unsafe = Relative_path.Map.find_unsafe
let has_file = Relative_path.Map.mem
let iter = Relative_path.Map.iter
let update a key data = Relative_path.Map.add a ~key ~data


let from_saved saved =
  Relative_path.Map.fold saved ~init:Relative_path.Map.empty ~f:(fun fn saved acc ->
    let file_info = FileInfo.from_saved fn saved in
    Relative_path.Map.add acc fn file_info
  )

let to_saved a =
  Relative_path.Map.map a FileInfo.to_saved

let to_fast a =
  Relative_path.Map.map a FileInfo.simplify

let saved_to_fast saved =
  Relative_path.Map.map saved FileInfo.saved_to_names
