(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let helper ctx acc path_list =
  let (ctx, tasts) = Server_infer_type_batch.get_tast_map ctx path_list in
  let holes =
    List.concat_map path_list ~f:(fun path ->
        let tast = Relative_path.Map.find tasts path in
        Server_collect_tast_holes.tast_holes
          ctx
          tast.Tast_with_dynamic.under_normal_assumptions
          Server_command_types.Tast_hole.Typing)
  in
  holes @ acc

let go :
    Multi_worker.worker list option ->
    string list ->
    Server_env.env ->
    Tast_holes_service.result =
 fun workers file_list env ->
  let file_list =
    file_list
    |> List.dedup_and_sort ~compare:String.compare
    |> List.map ~f:Relative_path.create_detect_prefix
  in
  let ctx = Provider_utils.ctx_from_server_env env in
  if List.length file_list < 10 then
    helper ctx [] file_list
  else
    Multi_worker.call
      workers
      ~job:(fun acc file -> helper ctx acc file)
      ~neutral:[]
      ~merge:List.rev_append
        (* constant stack space, though Base.List will call rev_append anyway past a threshold *)
      ~next:(Multi_worker.next workers file_list)
