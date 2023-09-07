(*
 * Copyright (c) 2019, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let log s = Hh_logger.log ("[ide-incremental] " ^^ s)

let log_debug s = Hh_logger.debug ("[ide-incremental] " ^^ s)

(** Print file change *)
let log_file_info_change
    ~(old_file_info : FileInfo.t option)
    ~(new_file_info : FileInfo.t option)
    ~(path : Relative_path.t) : unit =
  let verb =
    match (old_file_info, new_file_info) with
    | (Some _, Some _) -> "updated"
    | (Some _, None) -> "deleted"
    | (None, Some _) -> "added"
    | (None, None) ->
      (* May or may not indicate a bug in either the language client or the
         language server.

         - Could happen if the language client sends spurious notifications.
         - Could happen if the editor writes files in a certain way, such as if
           they delete the file before moving a new one into place.
         - Could happen if the language server was not able to read the file,
           despite it existing on disk (e.g. due to permissions). In this case,
           we would fail to generate its [FileInfo.t] and assume that it was
           deleted. This is correct from a certain point of view.
         - Could happen due to a benign race condition where we process
           file-change notifications more slowly than they happen. If a file is
           quickly created, then deleted before we process the create event,
           we'll think it was deleted twice. This is the correct way to handle
           the race condition.
      *)
      "spuriously updated"
  in
  log_debug "File changed: %s %s" (Relative_path.to_absolute path) verb

(* TODO(T150077239): deprecate. *)

(** This fetches the new names out of the modified file. *)
let compute_fileinfo_for_path
    (popt : ParserOptions.t) (contents : string option) (path : Relative_path.t)
    : FileInfo.t option =
  match contents with
  | None -> None
  (* The file couldn't be read from disk. Assume it's been deleted or is
     otherwise inaccessible. Our caller will delete the entries from the
     naming and reverse naming table; there's nothing for us to do here. *)
  | Some contents ->
    Some
      (Direct_decl_parser.parse_and_hash_decls
         (DeclParserOptions.from_parser_options popt)
         (ParserOptions.deregister_php_stdlib popt)
         path
         contents
      |> Direct_decl_parser.decls_to_fileinfo path)

type changed_file_results = {
  naming_table: Naming_table.t;
  sienv: SearchUtils.si_env;
  old_file_info: FileInfo.t option;
  new_file_info: FileInfo.t option;
}

let should_update_changed_file (path : Relative_path.t) : bool =
  Relative_path.is_root (Relative_path.prefix path)
  && FindUtils.path_filter path

let update_naming_tables_for_changed_file
    ~(ctx : Provider_context.t)
    ~(naming_table : Naming_table.t)
    ~(sienv : SearchUtils.si_env)
    ~(path : Relative_path.t) : changed_file_results =
  if should_update_changed_file path then begin
    (* note: clientIdeDaemon uses local-memory-backed file provider. This operation converts [path] to an absolute path, then tries to cat it.*)
    let contents = File_provider.get_contents path in
    let new_file_info =
      compute_fileinfo_for_path (Provider_context.get_popt ctx) contents path
    in
    let old_file_info = Naming_table.get_file_info naming_table path in
    log_file_info_change ~old_file_info ~new_file_info ~path;
    (* update the reverse-naming-table, which is mutable storage owned by backend *)
    Naming_provider.update
      ~backend:(Provider_context.get_backend ctx)
      ~path
      ~old_file_info
      ~new_file_info;
    (* remove old FileInfo from forward-naming-table, then add new FileInfo *)
    let naming_table =
      match old_file_info with
      | None -> naming_table
      | Some _ -> Naming_table.remove naming_table path
    in
    let naming_table =
      match new_file_info with
      | None -> naming_table
      | Some new_file_info ->
        Naming_table.update naming_table path new_file_info
    in
    (* update search index *)
    let sienv =
      match new_file_info with
      | None ->
        SymbolIndexCore.remove_files
          ~sienv
          ~paths:(Relative_path.Set.singleton path)
      | Some info ->
        SymbolIndexCore.update_files
          ~ctx
          ~sienv
          ~paths:[(path, info, SearchUtils.TypeChecker)]
    in
    { naming_table; sienv; old_file_info; new_file_info }
  end else begin
    log "Ignored change to file %s" (Relative_path.to_absolute path);
    { naming_table; sienv; old_file_info = None; new_file_info = None }
  end

module Batch = struct
  external batch_index_root_relative_paths_only :
    DeclParserOptions.t ->
    bool ->
    Path.t ->
    (Relative_path.t * string option option) list ->
    (Relative_path.t * (FileInfo.t * SearchTypes.si_addendum list) option) list
    = "batch_index_hackrs_ffi_root_relative_paths_only"

  type changed_file_info = {
    path: Relative_path.t;
    old_file_info: FileInfo.t option;
    new_file_info: FileInfo.t option;
  }

  type update_result = {
    naming_table: Naming_table.t;
    sienv: SearchUtils.si_env;
    changes: changed_file_info list;
  }

  (** For each path, direct decl parse to compute the names and positions in the file. If the file at the path doesn't exist, return [None]. *)
  let compute_file_info_batch_root_relative_paths_only
      (popt : ParserOptions.t) (paths : Relative_path.t list) :
      (Relative_path.t * (FileInfo.t * SearchTypes.si_addendum list) option)
      list =
    let paths =
      if Disk.is_real_disk then
        List.map paths ~f:(fun path -> (path, None))
      else
        (* For tests which use test disk, the test disk lives in ocaml and can't be read
           from the FFI rust parallel reader which only uses std::fs::read. Therefore, in
           the test case, we'll read the files (sequentially) ourselves and give them to
           rust. *)
        List.map paths ~f:(fun path ->
            let contents =
              try Some (Disk.cat (Relative_path.to_absolute path)) with
              | Disk.No_such_file_or_directory _ -> None
            in
            (path, Some contents))
    in
    batch_index_root_relative_paths_only
      (DeclParserOptions.from_parser_options popt)
      (ParserOptions.deregister_php_stdlib popt)
      (Relative_path.path_of_prefix Relative_path.Root |> Path.make)
      paths

  let update_naming_tables_and_si
      ~(ctx : Provider_context.t)
      ~(naming_table : Naming_table.t)
      ~(sienv : SearchUtils.si_env)
      ~(changes : Relative_path.Set.t) : update_result =
    log
      "Batch change %d files, e.g. %s"
      (Relative_path.Set.cardinal changes)
      (Relative_path.Set.choose_opt changes
      |> Option.value_map ~default:"[none]" ~f:Relative_path.suffix);
    Relative_path.Set.filter changes ~f:(fun path ->
        not @@ should_update_changed_file path)
    |> Relative_path.Set.iter ~f:(fun ignored_path ->
           log
             "Ignored change to file %s"
             (Relative_path.to_absolute ignored_path));
    (* NOTE: our batch index/file info computation expects only files relative to root. *)
    let batch_index_benchmark_start = Unix.gettimeofday () in
    let index_result =
      let changes_to_update =
        Relative_path.Set.filter changes ~f:should_update_changed_file
      in
      compute_file_info_batch_root_relative_paths_only
        (Provider_context.get_popt ctx)
        (Relative_path.Set.elements changes_to_update)
    in
    log
      "Batch index completed in %f seconds"
      (Unix.gettimeofday () -. batch_index_benchmark_start);
    let changed_file_infos =
      List.map index_result ~f:(fun (path, new_file_info_with_addendum) ->
          let old_file_info = Naming_table.get_file_info naming_table path in
          {
            path;
            new_file_info = Option.map new_file_info_with_addendum ~f:fst;
            old_file_info;
          })
    in
    (* update the reverse-naming-table, which is mutable storage owned by backend *)
    let update_reverse_naming_table_benchmark_start = Unix.gettimeofday () in
    List.iter
      changed_file_infos
      ~f:(fun { path; new_file_info; old_file_info } ->
        Naming_provider.update
          ~backend:(Provider_context.get_backend ctx)
          ~path
          ~old_file_info
          ~new_file_info);
    log
      "Updated reverse naming table in %f seconds"
      (Unix.gettimeofday () -. update_reverse_naming_table_benchmark_start);
    (* update the forward-naming-table (file -> symbols) *)
    (* remove old, then add new *)
    let update_forward_naming_table_benchmark_start = Unix.gettimeofday () in
    let naming_table =
      List.fold_left
        changed_file_infos
        ~init:naming_table
        ~f:(fun naming_table { path; old_file_info; _ } ->
          match old_file_info with
          | None -> naming_table
          | Some _ -> Naming_table.remove naming_table path)
    in
    (* update new *)
    let naming_table =
      let paths_with_new_file_info =
        List.filter_map changed_file_infos ~f:(fun { path; new_file_info; _ } ->
            Option.map new_file_info ~f:(fun new_file_info ->
                (path, new_file_info)))
      in
      Naming_table.update_many
        naming_table
        (paths_with_new_file_info |> Relative_path.Map.of_list)
    in
    log
      "Updated forward naming table in %f seconds"
      (Unix.gettimeofday () -. update_forward_naming_table_benchmark_start);
    (* update search index *)
    (* remove paths without new file info *)
    let removing_search_index_files_benchmark_start = Unix.gettimeofday () in
    let sienv =
      let paths_without_new_file_info =
        List.filter changed_file_infos ~f:(fun { new_file_info; _ } ->
            Option.is_none new_file_info)
        |> List.map ~f:(fun { path; _ } -> path)
        |> Relative_path.Set.of_list
      in
      SymbolIndexCore.remove_files ~sienv ~paths:paths_without_new_file_info
    in
    log
      "Removed files from search index in %f seconds"
      (Unix.gettimeofday () -. removing_search_index_files_benchmark_start);
    (* now update paths with new file info *)
    let updating_search_index_new_files_benchmark_start =
      Unix.gettimeofday ()
    in
    let sienv =
      let get_addenda_opt (path, new_file_info_with_addenda_opt) =
        Option.map
          new_file_info_with_addenda_opt
          ~f:(fun (_new_file_info, addenda) ->
            (path, addenda, SearchUtils.TypeChecker))
      in
      let paths_with_addenda =
        List.filter_map index_result ~f:get_addenda_opt
      in
      SymbolIndexCore.update_from_addenda ~sienv ~paths_with_addenda
    in
    log
      "Update search index with new files in %f seconds"
      (Unix.gettimeofday () -. updating_search_index_new_files_benchmark_start);
    { naming_table; sienv; changes = changed_file_infos }
end
