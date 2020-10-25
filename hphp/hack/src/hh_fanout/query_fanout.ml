(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type single_result = {
  hash: Typing_deps.Dep.t;
  paths: Relative_path.Set.t;
}

type result = single_result list

let go
    ~(deps_mode : Typing_deps_mode.t)
    ~(dep_hash : Typing_deps.Dep.t)
    ~(include_extends : bool) : result =
  let dep_set = Typing_deps.get_ideps_from_hash deps_mode dep_hash in
  let dep_set =
    if include_extends then
      Typing_deps.add_all_deps deps_mode dep_set
    else
      Typing_deps.add_typing_deps deps_mode dep_set
  in
  dep_set
  |> Typing_deps.DepSet.elements
  |> List.map ~f:(fun hash ->
         let paths =
           Typing_deps.Files.get_files
             (Typing_deps.DepSet.singleton deps_mode hash)
         in
         { hash; paths })

let result_to_json result : Hh_json.json =
  Hh_json.JSON_Object
    (List.map result ~f:(fun { hash; paths } ->
         ( Typing_deps.Dep.to_debug_string hash,
           Hh_json.JSON_Array
             ( paths
             |> Relative_path.Set.elements
             |> List.map ~f:(fun path ->
                    Hh_json.JSON_String (Relative_path.to_absolute path)) ) )))
