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

let should_update_changed_file (path : Relative_path.t) : bool =
  Relative_path.is_root (Relative_path.prefix path)
  && FindUtils.path_filter path

module Batch = struct
  external batch_index_root_relative_paths_only :
    DeclParserOptions.t ->
    bool ->
    Path.t ->
    (Relative_path.t * string option option) list ->
    (Relative_path.t * (FileInfo.t * FileInfo.si_addendum list) option) list
    = "batch_index_hackrs_ffi_root_relative_paths_only"

  type update_result = {
    naming_table: Naming_table.t;
    sienv: SearchUtils.si_env;
    changes: FileInfo.change list;
  }

  (** For each path, direct decl parse to compute the names and positions in the file. If the file at the path doesn't exist, return [None]. *)
  let compute_file_info_batch_root_relative_paths_only
      (popt : ParserOptions.t) (paths : Relative_path.t list) :
      (Relative_path.t * (FileInfo.t * FileInfo.si_addendum list) option) list =
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
    let popt = Provider_context.get_popt ctx in
    (* Do a fast parallel decl-parse of all changes *)
    let start_time = Unix.gettimeofday () in
    (* NOTE: our batch index/file info computation expects only files relative to root. *)
    let parse_results =
      Relative_path.Set.filter changes ~f:should_update_changed_file
      |> Relative_path.Set.elements
      |> compute_file_info_batch_root_relative_paths_only popt
    in
    let changed_file_infos =
      List.map parse_results ~f:(fun (path, new_file_info_with_addendum) ->
          let old_file_info = Naming_table.get_file_info naming_table path in
          {
            FileInfo.path;
            new_file_info = Option.map new_file_info_with_addendum ~f:fst;
            old_file_info;
          })
    in
    (* update the reverse-naming-table, which is mutable storage owned by backend *)
    let t_update_reverse_nt = Unix.gettimeofday () in
    List.iter
      changed_file_infos
      ~f:(fun { FileInfo.path; new_file_info; old_file_info } ->
        Naming_provider.update
          ~backend:(Provider_context.get_backend ctx)
          ~path
          ~old_file_info
          ~new_file_info);
    (* update the forward-naming-table (file -> symbols) *)
    (* remove old, then add new *)
    let t_update_forward_nt = Unix.gettimeofday () in
    let naming_table =
      List.fold_left
        changed_file_infos
        ~init:naming_table
        ~f:(fun naming_table { FileInfo.path; old_file_info; _ } ->
          match old_file_info with
          | None -> naming_table
          | Some _ -> Naming_table.remove naming_table path)
    in
    (* update new *)
    let paths_with_new_file_info =
      List.filter_map
        changed_file_infos
        ~f:(fun { FileInfo.path; new_file_info; _ } ->
          Option.map new_file_info ~f:(fun new_file_info ->
              (path, new_file_info)))
    in
    let naming_table =
      Naming_table.update_many
        naming_table
        (paths_with_new_file_info |> Relative_path.Map.of_list)
    in
    (* update search index *)
    (* remove paths without new file info *)
    let t_si = Unix.gettimeofday () in
    let paths_without_new_file_info =
      List.filter changed_file_infos ~f:(fun { FileInfo.new_file_info; _ } ->
          Option.is_none new_file_info)
      |> List.map ~f:(fun { FileInfo.path; _ } -> path)
      |> Relative_path.Set.of_list
    in
    let sienv =
      SymbolIndexCore.remove_files ~sienv ~paths:paths_without_new_file_info
    in
    (* now update paths with new file info *)
    let get_addenda_opt (path, new_file_info_with_addenda_opt) =
      Option.map
        new_file_info_with_addenda_opt
        ~f:(fun (_new_file_info, addenda) ->
          (path, addenda, SearchUtils.TypeChecker))
    in
    let paths_with_addenda = List.filter_map parse_results ~f:get_addenda_opt in
    let sienv =
      SymbolIndexCore.update_from_addenda ~sienv ~paths_with_addenda
    in

    let end_time = Unix.gettimeofday () in
    let telemetry =
      Telemetry.create ()
      |> Telemetry.duration
           ~key:"parse_duration"
           ~start_time
           ~end_time:t_update_reverse_nt
      |> Telemetry.duration
           ~key:"update_reverse_nt_duration"
           ~start_time:t_update_reverse_nt
           ~end_time:t_update_forward_nt
      |> Telemetry.duration
           ~key:"update_forward_nt_duration"
           ~start_time:t_update_forward_nt
           ~end_time:t_si
      |> Telemetry.duration ~key:"update_si_duration" ~start_time:t_si ~end_time
    in
    log "Update_naming_tables_and_si: %s" (Telemetry.to_string telemetry);
    { naming_table; sienv; changes = changed_file_infos }
end
