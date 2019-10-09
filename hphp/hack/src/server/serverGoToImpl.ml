(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open ServerEnv
open Reordered_argument_collections
open ServerCommandTypes.Find_refs
open ServerCommandTypes.Done_or_retry

let find_positions_of_classes
    (acc : (string * Pos.t) list) (child_classes : string list) :
    (string * Pos.t) list =
  acc
  @ List.map child_classes ~f:(fun child_class ->
        match Naming_table.Types.get_pos child_class with
        | None ->
          failwith ("Could not find definition of child class: " ^ child_class)
        | Some (FileInfo.Full pos, _type) -> (child_class, pos)
        | Some (FileInfo.File (FileInfo.Class, path), _type) ->
          (match Ast_provider.find_class_in_file path child_class with
          | None ->
            failwith
              (Printf.sprintf
                 "Could not find class %s in %s"
                 child_class
                 (Relative_path.to_absolute path))
          | Some { Aast.c_span; _ } -> (child_class, c_span))
        | Some
            FileInfo.(File ((Fun | RecordDef | Typedef | Const), _path), _type)
          ->
          failwith
            (Printf.sprintf
               "Information for class %s was returned as not a class"
               child_class))

let parallel_find_positions_of_classes
    (child_classes : string list) (workers : MultiWorker.worker list option) :
    (string * Pos.t) list =
  MultiWorker.call
    workers
    ~job:find_positions_of_classes
    ~neutral:[]
    ~merge:List.append
    ~next:(MultiWorker.next workers child_classes)

let search_class class_name genv env =
  let class_name = ServerFindRefs.add_ns class_name in
  ServerFindRefs.handle_prechecked_files
    genv
    env
    Typing_deps.Dep.(make (Class class_name))
  @@ fun () ->
  let files =
    FindRefsService.get_dependent_files
      genv.ServerEnv.workers
      (SSet.singleton class_name)
  in
  let child_classes =
    FindRefsService.find_child_classes class_name env.naming_table files
    |> SSet.elements
  in
  if List.length child_classes < 10 then
    find_positions_of_classes [] child_classes
  else
    parallel_find_positions_of_classes child_classes genv.workers

let go action genv env =
  match action with
  | Class class_name -> search_class class_name genv env
  | Member _
  | Function _
  | Record _
  | GConst _
  | LocalVar _ ->
    (env, Done [])
