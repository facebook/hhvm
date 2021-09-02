(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

(*
 * This module is used in saved state jobs to create two primary artifacts for
 * each changed file in a mergebase commit
 *  1) a marshalled ocaml blob representing the decls in said file
 *  2) a json blob representing the "fan in" or in other words the decls
 *     needed to typecheck said file
 *)

open Hh_prelude

let get_shallow_decls_filename (filename : string) : string =
  filename ^ ".shallow.bin"

let get_folded_decls_filename (filename : string) : string =
  filename ^ ".folded.bin"

let save_contents (output_filename : string) (contents : 'a) : unit =
  let chan = Stdlib.open_out_bin output_filename in
  Marshal.to_channel chan contents [];
  Stdlib.close_out chan

let get_classes_from_file (file : Relative_path.t) (ctx : Provider_context.t) :
    SSet.t =
  (match
     Direct_decl_utils.direct_decl_parse_and_cache
       ~file_decl_hash:true
       ~symbol_decl_hashes:false
       ctx
       file
   with
  | None -> []
  | Some (decls, _, _, _) ->
    List.filter_map
      ~f:(fun (name, decl) ->
        match decl with
        | Shallow_decl_defs.Class _ -> Some name
        | _ -> None)
      decls)
  |> SSet.of_list

let dump_folded_decls
    (ctx : Provider_context.t) (dir : string) (path : Relative_path.t) : unit =
  let file_name = Relative_path.suffix path in
  let classes_in_file = get_classes_from_file path ctx in
  let folded_decls_in_file =
    Decl_export.collect_legacy_decls ctx classes_in_file
  in
  let folded_decls_dir = Filename.concat dir "folded_decls" in
  let file = file_name |> Filename.concat folded_decls_dir in
  Sys_utils.mkdir_p (Filename.dirname file);
  save_contents (get_folded_decls_filename file) folded_decls_in_file;
  ()

let dump_shallow_decls
    (ctx : Provider_context.t)
    (genv : ServerEnv.genv)
    (dir : string)
    (path : Relative_path.t) : unit =
  let file_name = Relative_path.suffix path in
  let classes_in_file = get_classes_from_file path ctx in
  let shallow_decls_in_file =
    Decl_export.collect_shallow_decls ctx genv.ServerEnv.workers classes_in_file
  in
  let shallow_decls_dir = Filename.concat dir "shallow_decls" in
  let file = file_name |> Filename.concat shallow_decls_dir in
  Sys_utils.mkdir_p (Filename.dirname file);
  save_contents (get_shallow_decls_filename file) shallow_decls_in_file;
  ()

let go (env : ServerEnv.env) (genv : ServerEnv.genv) (dir : string) : unit =
  let ctx = Provider_utils.ctx_from_server_env env in
  (* TODO: Make this more robust. I should be able to get the root from somewhere... *)
  let root = Wwwroot.get None in
  let mergebase_rev =
    match Future.get @@ Hg.current_mergebase_hg_rev (Path.to_string root) with
    | Ok hash -> hash
    | Error _ -> failwith "Exception getting the current mergebase revision"
  in
  let dir = Filename.concat dir mergebase_rev in
  let as_relative_paths future =
    Future.continue_with future @@ fun relative_paths ->
    List.map relative_paths ~f:(fun suffix -> Relative_path.from_root ~suffix)
  in
  let changed_files_future =
    Hg.files_changed_in_rev (Hg.Hg_rev mergebase_rev) (Path.to_string root)
    |> as_relative_paths
  in
  let changed_files =
    match Future.get changed_files_future with
    | Ok files -> files
    | Error e -> failwith (Printf.sprintf "%s" (Future.error_to_string e))
  in
  let changed_files =
    List.filter_map
      ~f:(fun path ->
        if Filename.check_suffix (Relative_path.suffix path) ".php" then
          Some path
        else
          None)
      changed_files
  in
  List.iter
    ~f:(fun path ->
      dump_shallow_decls ctx genv dir path;
      dump_folded_decls ctx dir path)
    changed_files;
  ()
