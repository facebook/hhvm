(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

type dep_path_acc = Typing_deps.Dep.t list

type result_node = {
  dep: Typing_deps.Dep.t;
  paths: Relative_path.Set.t;
}

type result = result_node list option

(** Find a path from [current] to [dest] using a depth-first search. Only the
first result is returned.

A "path" is always zero or more extends-dependency edges followed a single
typing-dependency edge. Extends-dependencies are transitively traversed,
while typing-dependencies don't need to be transitively traversed since they
always end in a terminal node.
*)
let rec search
    ~(dep_path_acc : dep_path_acc)
    ~(seen_acc : Typing_deps.DepSet.t)
    ~(current : Typing_deps.Dep.t)
    ~(dest : Typing_deps.Dep.t) : dep_path_acc option =
  let current_direct_deps =
    current |> Typing_deps.DepSet.singleton |> Typing_deps.add_typing_deps
  in
  if Typing_deps.DepSet.mem current_direct_deps dest then
    Some (dest :: dep_path_acc)
  else if not (Typing_deps.Dep.is_class current) then
    (* There are no further extends deps to follow for this node. *)
    None
  else
    let dep_path_acc = current :: dep_path_acc in
    let extends_deps =
      current
      |> Typing_deps.Dep.extends_of_class
      |> Typing_deps.DepSet.singleton
      |> Typing_deps.add_typing_deps
      |> Typing_deps.DepSet.elements
    in
    List.fold_until
      extends_deps
      ~init:seen_acc
      ~f:(fun seen_acc extends_dep ->
        let seen_acc = Typing_deps.DepSet.add seen_acc extends_dep in
        let result =
          search ~dep_path_acc ~seen_acc ~current:extends_dep ~dest
        in
        match result with
        | Some result -> Container.Continue_or_stop.Stop (Some result)
        | None -> Container.Continue_or_stop.Continue seen_acc)
      ~finish:(fun _seen_acc -> None)

let go ~(source : Typing_deps.Dep.t) ~(dest : Typing_deps.Dep.t) : result =
  search
    ~dep_path_acc:[]
    ~seen_acc:Typing_deps.(DepSet.make ())
    ~current:source
    ~dest
  |> Option.map ~f:(fun dep_path ->
         List.rev_map dep_path ~f:(fun dep ->
             let paths =
               dep
               |> Typing_deps.DepSet.singleton
               |> Typing_deps.Files.get_files
             in
             { dep; paths }))

let result_to_json (result : result) : Hh_json.json =
  match result with
  | None -> Hh_json.JSON_Null
  | Some dep_path ->
    Hh_json.JSON_Array
      (List.map dep_path ~f:(fun { dep; paths } ->
           Hh_json.JSON_Object
             [
               ( "hash",
                 Hh_json.JSON_String (Typing_deps.Dep.to_debug_string dep) );
               ( "paths",
                 Hh_json.JSON_Array
                   ( paths
                   |> Relative_path.Set.elements
                   |> List.map ~f:(fun path ->
                          Hh_json.JSON_String (Relative_path.to_absolute path))
                   ) );
             ]))
