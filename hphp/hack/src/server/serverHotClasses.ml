(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let go
    (workers : MultiWorker.worker list option)
    (env : ServerEnv.env)
    (threshold : int) : string =
  let ctx = Provider_utils.ctx_from_server_env env in
  let deps_mode = Provider_context.get_deps_mode ctx in
  let naming_table = env.ServerEnv.naming_table in
  let start_t = Unix.gettimeofday () in
  Hh_logger.log "Generating hot classes file...";
  let classes_in_repo =
    Naming_table.fold naming_table ~init:[] ~f:(fun path info acc ->
        match Relative_path.prefix path with
        | Relative_path.Root ->
          List.fold info.FileInfo.classes ~init:acc ~f:(fun acc (_, x) ->
              x :: acc)
        | _ -> acc)
  in
  let add_hot_classes acc cids =
    List.fold cids ~init:acc ~f:(fun acc cid ->
        let open Typing_deps in
        let deps =
          DepSet.singleton deps_mode
          @@ Dep.make (hash_mode deps_mode) (Dep.Type cid)
        in
        let deps = Typing_deps.add_all_deps deps_mode deps in
        if DepSet.cardinal deps > threshold then
          cid :: acc
        else
          acc)
  in
  let classes_to_save =
    MultiWorker.call
      workers
      ~job:add_hot_classes
      ~neutral:[]
      ~merge:List.rev_append
      ~next:(MultiWorker.next workers classes_in_repo)
  in
  let classes_to_save =
    classes_to_save
    |> List.sort ~compare:String.compare
    |> List.remove_consecutive_duplicates ~equal:String.equal
  in
  let result =
    Hh_json.(
      let comment = List.map ServerHotClassesDescription.comment string_ in
      let classes = List.map classes_to_save string_ in
      json_to_string ~pretty:true
      @@ JSON_Object
           [
             ("comment", JSON_Array comment);
             ("threshold", int_ threshold);
             ("classes", JSON_Array classes);
           ])
  in
  let result = ServerHotClassesDescription.postprocess (result ^ "\n") in
  let (_ : float) =
    Hh_logger.log_duration "Done generating hot classes file" start_t
  in
  result
