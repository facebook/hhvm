(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open SymbolInfoServiceUtils

(* This module dumps all the symbol info(like fun-calls) in input files *)

let parallel_helper workers filename_l tcopt =
  MultiWorker.call
    workers
    ~job:(helper tcopt)
    ~neutral:[]
    ~merge:List.rev_append
    ~next:(MultiWorker.next workers filename_l)

(* Entry Point *)
let go workers file_list env =
  let filename_l =
    file_list
    |> List.filter ~f:FindUtils.file_filter
    |> List.map ~f:(Relative_path.create Relative_path.Root)
  in
  let ctx = Provider_utils.ctx_from_server_env env in
  let raw_result =
    if List.length filename_l < 10 then
      helper ctx [] filename_l
    else
      parallel_helper workers filename_l ctx
  in
  format_result raw_result
