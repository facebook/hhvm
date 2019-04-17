(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open ServerEnv

module SearchServiceRunner = struct
  type t = Relative_path.t * SearchUtils.info

  (* Chosen so that multiworker takes about ~2.5 seconds *)
  let chunk_size genv =
    genv.local_config.ServerLocalConfig.search_chunk_size

  let queue = Queue.create ()

  (* Pops the first num_files from the queue *)
  let update_search genv num_files =
    let t = Unix.gettimeofday() in
    let rec iter acc n =
      if n <= 0 || Queue.is_empty queue then acc
      else
        let x = Queue.dequeue_exn queue in
        iter (x::acc) (n-1) in
    let fast = iter [] num_files in

    SymbolIndex.update genv.workers fast;

    if (List.length fast > 0) then begin
      let str =
        Printf.sprintf
          "Updated search index for symbols in %d files:"
          (List.length fast)
      in
      ignore(Hh_logger.log_duration (str) t);
      if Queue.is_empty queue
      then Hh_logger.log "Done updating search index"
    end

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

  let update x = Queue.enqueue queue x

  let update_full fn ast = Queue.enqueue queue (fn, SearchUtils.Full ast)

  let should_run_completely genv =
   chunk_size genv = 0
   && ServerArgs.ai_mode genv.options = None

  let update_fileinfo_map fast =
    Naming_table.iter fast
    ~f: begin fun fn info ->
      update_full fn info
    end

end
