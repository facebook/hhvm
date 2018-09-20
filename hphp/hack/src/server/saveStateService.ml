(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module List = Hh_core.List

(* Experimental: save the naming table ("file info") into the same SQLite
    database that we save the dependency table into. The table's name is NAME_INFO *)
let save_all_file_info_sqlite
    (db_name: string)
    (files_info: FileInfo.t Relative_path.Map.t) : unit =
  begin
    SharedMem.save_file_info_init db_name;
    Relative_path.Map.iter files_info (
      fun path file_info ->
        let funs = file_info.FileInfo.funs in
        let classes = file_info.FileInfo.classes in
        let typedefs = file_info.FileInfo.typedefs in
        let consts = file_info.FileInfo.consts in

        (* NOTE: uncompressing the path here seems to have a significant negative effect
            on the size of the naming table in SQLite. This needs more work: TODO. *)
        let path = Relative_path.S.to_string path in
        begin
          List.iter funs ~f:(fun (_pos, name) ->
            SharedMem.save_file_info_sqlite ~hash:(
              Naming_heap.heap_string_of_key `FunPos name
            ) ~name:name `FuncK path
          );
          List.iter classes ~f:(fun (_pos, name) ->
            SharedMem.save_file_info_sqlite ~hash:(
              Naming_heap.heap_string_of_key `TypeId name
            ) ~name:name `ClassK path
          );
          List.iter typedefs ~f:(fun (_pos, name) ->
            SharedMem.save_file_info_sqlite ~hash:(
              Naming_heap.heap_string_of_key `TypeId name
            ) ~name:name `ClassK path
          );
          List.iter consts ~f:(fun (_pos, name) ->
            SharedMem.save_file_info_sqlite ~hash:(
              Naming_heap.heap_string_of_key `ConstPos name
            ) ~name:name `ConstantK path
          )
        end
    );
    SharedMem.save_file_info_free ()
  end

let dump_filesinfo
    (output_filename: string)
    (files_info: FileInfo.t Relative_path.Map.t) : unit =
  let chan = Sys_utils.open_out_no_fail output_filename in
  let (saved: FileInfo.saved_state_info) = FileInfo.info_to_saved files_info in
  Marshal.to_channel chan saved [];
  Sys_utils.close_out_no_fail output_filename chan

let update_save_state
    ~(file_info_on_disk: bool)
    (files_info: FileInfo.t Relative_path.Map.t)
    (output_filename: string) : int =
  let t = Unix.gettimeofday () in
  let db_name = output_filename ^ ".sql" in
  if not (Disk.file_exists db_name) then
    failwith "Given existing save state SQL file missing";
  dump_filesinfo output_filename files_info;
  let () = if file_info_on_disk then begin
    failwith "incrementally updating file info on disk not yet implemented"
  end else begin
    Hh_logger.log "skip writing file info to sqlite table"
  end in
  let edges_added = SharedMem.update_dep_table_sqlite db_name Build_id.build_revision in
  ignore @@ Hh_logger.log_duration "Updating saved state took" t;
  edges_added

(** Saves the saved state to the given path. Returns number of dependency
* edges dumped into the database. *)
let save_state
    ~(file_info_on_disk: bool)
    (files_info: FileInfo.t Relative_path.Map.t)
    (output_filename: string) : int =
  let () = Sys_utils.mkdir_p (Filename.dirname output_filename) in
  let db_name = output_filename ^ ".sql" in
  let () = if Sys.file_exists output_filename then
             failwith (Printf.sprintf "Cowardly refusing to overwrite '%s'." output_filename)
           else () in
  let () = if Sys.file_exists db_name then
             failwith (Printf.sprintf "Cowardly refusing to overwrite '%s'." db_name)
           else () in
  match SharedMem.loaded_dep_table_filename () with
  | None ->
    let t = Unix.gettimeofday () in
    dump_filesinfo output_filename files_info;
    let () = if file_info_on_disk then begin
      Hh_logger.log "do save all file info sqlite\n";
      (save_all_file_info_sqlite db_name files_info : unit)
    end else begin
      Hh_logger.log "skip save all file info sqlite\n"
    end in
    let edges_added = SharedMem.save_dep_table_sqlite db_name Build_id.build_revision in
    let _ : float = Hh_logger.log_duration "Saving saved state took" t in
    edges_added
  | Some old_table_filename ->
    (** If server is running from a loaded saved state, it's in-memory
     * tracked depdnencies are incomplete - most of the actual dependencies
     * are in the SQL table. We need to copy that file and update it with
     * the in-memory edges. *)
    let t = Unix.gettimeofday () in
    let content = Sys_utils.cat old_table_filename in
    let () = Sys_utils.mkdir_p (Filename.dirname output_filename) in
    let () = Sys_utils.write_file ~file:db_name content in
    let _ : float = Hh_logger.log_duration "Made disk copy of loaded saved state. Took" t in
    update_save_state ~file_info_on_disk files_info output_filename

let get_in_memory_dep_table_entry_count () : (int, string) result =
  Core_result.try_with (fun () ->
    SharedMem.get_in_memory_dep_table_entry_count ())
  |> Core_result.map_error ~f:Printexc.to_string

(* If successful, returns the # of edges from the dependency table that were written. *)
(* TODO: write some other stats, e.g., the number of names, the number of errors, etc. *)
let go
    ~(file_info_on_disk: bool)
    (files_info: FileInfo.t Relative_path.Map.t)
    (output_filename: string) : (int, string) result =
  Core_result.try_with (fun () ->
    save_state ~file_info_on_disk files_info output_filename)
  |> Core_result.map_error ~f:Printexc.to_string
