(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open ServerEnv

module SearchServiceRunner = struct
  type t = Relative_path.t * SearchUtils.info

  (* Chosen so that multiworker takes about ~2.5 seconds *)
  let chunk_size genv = genv.local_config.ServerLocalConfig.search_chunk_size

  let queue = Queue.create ()

  (* Pops the first num_files from the queue *)
  let update_search
      (ctx : Provider_context.t) (sienv : SearchUtils.si_env) num_files :
      SearchUtils.si_env =
    let t = Unix.gettimeofday () in
    let rec iter acc n =
      if n <= 0 || Queue.is_empty queue then
        acc
      else
        let x = Queue.dequeue_exn queue in
        iter (x :: acc) (n - 1)
    in
    let fast = iter [] num_files in
    let sienv = SymbolIndex.update_files ~ctx ~sienv ~paths:fast in

    if List.length fast > 0 then (
      let str =
        Printf.sprintf
          "Updated search index for symbols in %d files:"
          (List.length fast)
      in
      ignore (Hh_logger.log_duration str t);
      if Queue.is_empty queue then Hh_logger.log "Done updating search index"
    );
    sienv

  (* Completely clears the queue *)
  let run_completely (ctx : Provider_context.t) (sienv : SearchUtils.si_env) :
      SearchUtils.si_env =
    update_search ctx sienv (Queue.length queue)

  let run genv (ctx : Provider_context.t) (sienv : SearchUtils.si_env) :
      SearchUtils.si_env =
    if Option.is_none (ServerArgs.ai_mode genv.options) then
      let size =
        if chunk_size genv = 0 then
          Queue.length queue
        else
          chunk_size genv
      in
      update_search ctx sienv size
    else
      sienv

  let internal_ssr_update
      (fn : Relative_path.t)
      (info : SearchUtils.info)
      ~(source : SearchUtils.file_source) =
    Queue.enqueue queue (fn, info, source)

  (* Return true if it's best to run all queue items in one go,
   * false if we should run small chunks every few seconds *)
  let should_run_completely
      (genv : ServerEnv.genv) (_ : SearchUtils.search_provider) : bool =
    chunk_size genv = 0 && Option.is_none (ServerArgs.ai_mode genv.options)

  let update_fileinfo_map
      (fast : Naming_table.t) ~(source : SearchUtils.file_source) : unit =
    let i = ref 0 in
    Naming_table.iter fast ~f:(fun fn info ->
        internal_ssr_update fn (SearchUtils.Full info) source;
        i := !i + 1)
end
