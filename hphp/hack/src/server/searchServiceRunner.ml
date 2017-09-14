(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


open Core
open ServerEnv

module SearchServiceRunner  = struct
  type info =
    | Full of FileInfo.t
    | Fast of FileInfo.names

  type t = (HackSearchService.SS.Trie.SearchUpdates.key * info)

  (* Chosen so that multiworker takes about ~2.5 seconds *)
  let chunk_size genv =
    genv.local_config.ServerLocalConfig.search_chunk_size

  (* Update the search service for this file *)
  let update_single (fn, info) =
    match info with
    | Full infos ->
      HackSearchService.WorkerApi.update_from_fileinfo fn infos
    | Fast names ->
      HackSearchService.WorkerApi.update_from_fast fn names

  let queue = Queue.create ()

  (* Pops the first num_files from the queue *)
  let update_search genv num_files =
    let t = Unix.gettimeofday() in
    let rec iter acc n =
      if n <= 0 || Queue.is_empty queue then acc
      else
        let x = Queue.pop queue in
        iter (x::acc) (n-1) in
    let fast = iter [] num_files in
    let update_single_search _ fast_list =
      List.iter fast_list update_single in

    (* If there aren't enough files just do it on one thread *)
    (if (List.length fast) < 100
      then update_single_search () fast
      else
        let next_fast_files =
          MultiWorker.next genv.workers fast in
        MultiWorker.call
            genv.workers
            ~job:update_single_search
            ~neutral:()
            ~merge:(fun _ _ -> ())
            ~next:next_fast_files);

    HackSearchService.MasterApi.update_search_index
      ~fuzzy:!HackSearchService.fuzzy (List.map fast fst);
    if (List.length fast > 0) then
    let str = Printf.sprintf "Updating %d search files:" (List.length fast) in
    ignore(Hh_logger.log_duration (str) t)
    else ()

  (* Completely clears the queue *)
  let run_completely genv =
    update_search genv (Queue.length queue)

  let run genv () =
    if ServerArgs.ai_mode genv.options = None then begin
      let size = if chunk_size genv = 0
        then Queue.length queue
        else chunk_size genv
      in
      update_search genv size
    end
    else ()

  let update x = Queue.push x queue

  let update_full fn ast = Queue.push (fn, Full ast) queue

  let should_run_completely genv =
   chunk_size genv = 0
   && ServerArgs.ai_mode genv.options = None

  let update_fileinfo_map fast =
    Relative_path.Map.iter fast
    ~f: begin fun fn info ->
      update_full fn info
    end

end
