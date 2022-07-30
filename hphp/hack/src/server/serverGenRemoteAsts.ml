(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let save_contents (file : string) (contents : 'a) : unit =
  Sys_utils.mkdir_p (Filename.dirname file);
  let chan = Stdlib.open_out_bin file in
  Marshal.to_channel chan contents [];
  Stdlib.close_out chan

let go
    (env : ServerEnv.env)
    (genv : ServerEnv.genv)
    (workers : MultiWorker.worker list option)
    (dir : string) : unit =
  let ctx = Provider_utils.ctx_from_server_env env in
  let root = Wwwroot.get None in
  let mergebase_rev =
    match Future.get @@ Hg.current_mergebase_hg_rev (Path.to_string root) with
    | Ok hash -> hash
    | Error _ -> failwith "Exception getting the current mergebase revision"
  in
  let dir = Filename.concat dir mergebase_rev in
  let asts_file_name = "asts.bin" in
  let local_file = dir ^ "/" ^ asts_file_name in

  let get_next =
    ServerUtils.make_next
      ~hhi_filter:(fun _ -> true)
      ~indexer:(genv.ServerEnv.indexer FindUtils.file_filter)
      ~extra_roots:(ServerConfig.extra_paths genv.ServerEnv.config)
  in

  let job (acc : (string * string) list) (fnl : Relative_path.t list) :
      (string * string) list =
    List.fold_left
      ~init:acc
      ~f:(fun acc fn ->
        let ast = Ast_provider.get_ast ~full:true ctx fn in
        let ast_hash = Nast.generate_ast_decl_hash ast in
        (OpaqueDigest.to_hex ast_hash, Marshal.to_string ast []) :: acc)
      fnl
  in

  let results =
    MultiWorker.call workers ~job ~neutral:[] ~merge:List.append ~next:get_next
  in

  Hh_logger.log "Pased %d files into ASTs" (List.length results);
  save_contents local_file results;
  Hh_logger.log "Saved to local file %s" local_file;

  ()
