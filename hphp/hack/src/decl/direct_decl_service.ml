(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type direct_decl_mode =
  | Normal
  | Cached
  | Zoncolan

let parse
    (ctx : Provider_context.t)
    ~(trace : bool)
    ~(decl_mode : direct_decl_mode)
    (acc : FileInfo.t Relative_path.Map.t)
    (fn : Relative_path.t) : FileInfo.t Relative_path.Map.t =
  if not (FindUtils.path_filter fn) then
    acc
  else
    let start_parse_time = Unix.gettimeofday () in
    let parsed_file_opt =
      match decl_mode with
      | Normal -> Direct_decl_utils.direct_decl_parse ctx fn
      | Cached -> Direct_decl_utils.direct_decl_parse_and_cache ctx fn
      | Zoncolan -> Direct_decl_utils.direct_decl_parse_zoncolan ctx fn
    in
    match parsed_file_opt with
    | None -> acc
    | Some parsed_file ->
      let end_parse_time = Unix.gettimeofday () in
      let fileinfo = Direct_decl_utils.decls_to_fileinfo fn parsed_file in
      if trace then
        Hh_logger.log
          "[%.1fms] %s - %s"
          ((end_parse_time -. start_parse_time) *. 1000.0)
          (Relative_path.suffix fn)
          (FileInfo.to_string fileinfo);
      Relative_path.Map.add acc ~key:fn ~data:fileinfo

let go
    (ctx : Provider_context.t)
    ~(trace : bool)
    ~(decl_mode : direct_decl_mode)
    ?(worker_call : MultiWorker.call_wrapper = MultiWorker.wrapper)
    (workers : MultiWorker.worker list option)
    ~(get_next : Relative_path.t list MultiWorker.Hh_bucket.next) :
    FileInfo.t Relative_path.Map.t =
  worker_call.MultiWorker.f
    workers
    ~job:(fun init -> List.fold ~init ~f:(parse ctx ~trace ~decl_mode))
    ~neutral:Relative_path.Map.empty
    ~merge:Relative_path.Map.union
    ~next:get_next
