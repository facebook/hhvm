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
 *  1) a marshalled ocaml blob representing the shallow decls in said file
 *  2) a json blob representing the "fan in" or in other words the decls
 *     needed to typecheck said file
 *)

open Hh_prelude

let get_shallow_decls_filename (filename : string) : string = filename ^ ".bin"

let save_contents (output_filename : string) (contents : 'a) : unit =
  let chan = Stdlib.open_out_bin output_filename in
  Marshal.to_channel chan contents [];
  Stdlib.close_out chan

let get_classes_from_file (file : Relative_path.t) (ctx : Provider_context.t) :
    SSet.t =
  (match
     Direct_decl_utils.direct_decl_parse_and_cache ~decl_hash:true ctx file
   with
  | None -> []
  | Some (decls, _, _) ->
    List.filter_map
      ~f:(fun (name, decl) ->
        match decl with
        | Shallow_decl_defs.Class _ -> Some name
        | _ -> None)
      decls)
  |> SSet.of_list

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

let dump_fan_in_deps
    (ctx : Provider_context.t) (dir : string) (path : Relative_path.t) : unit =
  let file_name = Relative_path.suffix path in
  let recorded_decls = Caml.Hashtbl.create 10 in
  Typing_deps.add_dependency_callback
    "hh_single collect decls"
    (fun _ dependency ->
      Caml.Hashtbl.replace
        recorded_decls
        (Typing_deps.Dep.extract_name dependency)
        true);
  let (ctx, entry) = Provider_context.add_entry_if_missing ~ctx ~path in
  Hh_logger.log "typechecking %s" file_name;
  let _ = Tast_provider.compute_tast_and_errors_unquarantined ~ctx ~entry in
  let symbol_to_file name =
    let name = "\\" ^ name in
    match Naming_provider.get_class_path ctx name with
    | Some path ->
      (match Relative_path.prefix path with
      | Relative_path.Root -> Some (Relative_path.suffix path)
      | _ -> None)
    | _ ->
      Hh_logger.log "Couldn't find filename for symbol %s" name;
      None
  in
  let typecheck_decl_deps =
    Caml.Hashtbl.to_seq recorded_decls
    |> Sequence.of_seq
    |> Sequence.to_list
    |> List.filter_map ~f:(fun (classname, _) -> symbol_to_file classname)
    |> List.map ~f:(fun path -> Hh_json.string_ path)
    |> HashSet.of_list
    |> HashSet.to_list
  in
  let typecheck_decl_deps =
    Hh_json.(
      json_to_string ~pretty:true
      @@ JSON_Object [("decl_deps", JSON_Array typecheck_decl_deps)])
  in
  let typecheck_deps_dir = Filename.concat dir "typecheck_deps" in
  let file = file_name |> Filename.concat typecheck_deps_dir in
  Sys_utils.mkdir_p (Filename.dirname file);
  Disk.write_file ~file ~contents:typecheck_decl_deps;
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
      dump_fan_in_deps ctx dir path;
      dump_shallow_decls ctx genv dir path)
    changed_files;
  ()
