(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module List = Hh_core.List

(* An alias for the errors type that we marshal to and unmarshal from the saved state *)
type saved_state_errors = (Errors.phase * Relative_path.Set.t) list

let get_errors_filename (filename: string) : string = filename ^ ".err"

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

(* Gets a set of file paths out of saved state errors *)
let fold_error_files (errors_in_phases: saved_state_errors) : Relative_path.Set.t =
  List.fold
    ~init:Relative_path.Set.empty
    ~f:(fun acc (_phase, error_files) -> Relative_path.Set.union acc error_files)
    errors_in_phases

(* Given saved state errors, produces 2 sets of file paths: one for those that
    occurred in the given list of phases, and another one for all the rest.
    The 'tf' prefix is in keeping with partition_tf, where it's supposed to
    remind you which list matches the predicate (not actually a predicate in this case),
    and which doesn't. *)
let partition_error_files_tf
  (errors_in_phases: saved_state_errors)
  (phases: Errors.phase list): (Relative_path.Set.t * Relative_path.Set.t) =

  let (errors_in_phases_t, errors_in_phases_f) =
    List.partition_tf errors_in_phases ~f:(fun (phase, _error_files) -> List.mem phases phase) in

  ((fold_error_files errors_in_phases_t), (fold_error_files errors_in_phases_f))

(* Loads the file info and the errors, if any. *)
let load_saved_state
    (saved_state_filename: string) : (FileInfo.saved_state_info * saved_state_errors) =
  let chan = open_in_bin saved_state_filename in
  let (old_saved: FileInfo.saved_state_info) =
    Marshal.from_channel chan in
  Sys_utils.close_in_no_fail saved_state_filename chan;

  let errors_filename = get_errors_filename saved_state_filename in
  let (old_errors: saved_state_errors) = if not (Sys.file_exists errors_filename) then [] else
    Marshal.from_channel (open_in_bin errors_filename) in
  (old_saved, old_errors)

(* Writes some OCaml object to a file with the given filename. *)
let dump_contents
    (output_filename: string)
    (contents: 'a) : unit =
  let chan = Sys_utils.open_out_bin_no_fail output_filename in
  Marshal.to_channel chan contents [];
  Sys_utils.close_out_no_fail output_filename chan

(* Dumps the file info and the errors, if any. *)
let dump_saved_state
    (output_filename: string)
    (files_info: FileInfo.t Relative_path.Map.t)
    (errors: Errors.t) : unit =
  let (files_info_saved: FileInfo.saved_state_info) = FileInfo.info_to_saved files_info in
  dump_contents output_filename files_info_saved;

  (* Let's not write empty error files. *)
  if Errors.is_empty errors then () else

  let errors_in_phases =
    List.map
      ~f:(fun phase -> (phase, Errors.get_failed_files errors phase))
      [ Errors.Parsing; Errors.Decl; Errors.Naming; Errors.Typing ] in
  dump_contents (get_errors_filename output_filename) errors_in_phases

let update_save_state
    ~(file_info_on_disk: bool)
    (files_info: FileInfo.t Relative_path.Map.t)
    (errors: Errors.t)
    (output_filename: string) : int =
  let t = Unix.gettimeofday () in
  let db_name = output_filename ^ ".sql" in
  if not (Disk.file_exists db_name) then
    failwith "Given existing save state SQL file missing";
  dump_saved_state output_filename files_info errors;
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
    (errors: Errors.t)
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
    dump_saved_state output_filename files_info errors;
    let () = if file_info_on_disk then begin
      Hh_logger.log "Saving file info (naming table) into a SQLite table.\n";
      (save_all_file_info_sqlite db_name files_info : unit)
    end in
    let dep_table_edges_added =
      SharedMem.save_dep_table_sqlite db_name Build_id.build_revision in
    let _ : float = Hh_logger.log_duration "Saving saved state took" t in
    dep_table_edges_added
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
    update_save_state ~file_info_on_disk files_info errors output_filename

let get_in_memory_dep_table_entry_count () : (int, string) result =
  Utils.try_with_stack (fun () ->
    SharedMem.get_in_memory_dep_table_entry_count ())
  |> Core_result.map_error ~f:(fun (exn, _stack) -> Printexc.to_string exn)

(* If successful, returns the # of edges from the dependency table that were written. *)
(* TODO: write some other stats, e.g., the number of names, the number of errors, etc. *)
let go
    ~(file_info_on_disk: bool)
    (files_info: FileInfo.t Relative_path.Map.t)
    (errors: Errors.t)
    (output_filename: string) : (int, string) result =
  Utils.try_with_stack (fun () ->
    save_state ~file_info_on_disk files_info errors output_filename)
  |> Core_result.map_error ~f:(fun (exn, _stack) -> Printexc.to_string exn)
