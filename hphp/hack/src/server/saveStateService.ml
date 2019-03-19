(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

(* An alias for the errors type that we marshal to and unmarshal from the saved state *)
type saved_state_errors = (Errors.phase * Relative_path.Set.t) list

let get_errors_filename (filename: string) : string = filename ^ ".err"

let get_decls_filename (filename: string) : string = filename ^ ".decls"

(* Experimental: save the naming table ("file info") into the same SQLite
    database that we save the dependency table into. The table's name is NAME_INFO *)
let save_all_file_info_sqlite
    (db_name: string)
    (naming_table: Naming_table.t) : unit =
  begin
    SharedMem.save_file_info_init db_name;
    Naming_table.iter naming_table (
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
              Naming_table.Funs.heap_string_of_key name
            ) ~name:name `FuncK path
          );
          List.iter classes ~f:(fun (_pos, name) ->
            SharedMem.save_file_info_sqlite ~hash:(
              Naming_table.Types.heap_string_of_key name
            ) ~name:name `ClassK path
          );
          List.iter typedefs ~f:(fun (_pos, name) ->
            SharedMem.save_file_info_sqlite ~hash:(
              Naming_table.Types.heap_string_of_key name
            ) ~name:name `ClassK path
          );
          List.iter consts ~f:(fun (_pos, name) ->
            SharedMem.save_file_info_sqlite ~hash:(
              Naming_table.Consts.heap_string_of_key name
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
    List.partition_tf
      errors_in_phases
      ~f:(fun (phase, _error_files) -> List.mem ~equal:(=) phases phase) in

  ((fold_error_files errors_in_phases_t), (fold_error_files errors_in_phases_f))

let load_contents_exn (input_filename: string) : 'a =
  let ic = Pervasives.open_in_bin input_filename in
  let contents = Marshal.from_channel ic in
  Pervasives.close_in ic;
  contents

let load_class_decls (input_filename: string) : unit =
  let start_t = Unix.gettimeofday () in
  Hh_logger.log "Begin loading class declarations";
  try
    Hh_logger.log "Unmarshalling class declarations from %s" input_filename;
    let decls = load_contents_exn input_filename in
    Hh_logger.log "Importing class declarations...";
    let classes = Decl_export.import_class_decls decls in
    let num_classes = SSet.cardinal classes in
    let msg = Printf.sprintf "Loaded %d class declarations" num_classes in
    ignore @@ Hh_logger.log_duration msg start_t
  with exn ->
    let stack = Printexc.get_backtrace () in
    HackEventLogger.load_decls_failure exn stack;
    Hh_logger.exc exn ~stack ~prefix:"Failed to load class declarations: "

(* Loads the file info and the errors, if any. *)
let load_saved_state
    ~(load_decls: bool)
    (saved_state_filename: string)
  : Naming_table.saved_state_info * saved_state_errors =
  let chan = In_channel.create ~binary:true saved_state_filename in
  let (old_saved: Naming_table.saved_state_info) =
    Marshal.from_channel chan in
  Sys_utils.close_in_no_fail saved_state_filename chan;

  let errors_filename = get_errors_filename saved_state_filename in
  let (old_errors: saved_state_errors) = if not (Sys.file_exists errors_filename) then [] else
    Marshal.from_channel (In_channel.create ~binary:true errors_filename) in

  if load_decls then load_class_decls (get_decls_filename saved_state_filename);

  (old_saved, old_errors)

(* Writes some OCaml object to a file with the given filename. *)
let dump_contents
    (output_filename: string)
    (contents: 'a) : unit =
  let chan = Sys_utils.open_out_bin_no_fail output_filename in
  Marshal.to_channel chan contents [];
  Sys_utils.close_out_no_fail output_filename chan

let dump_contents_exn
    (output_filename: string)
    (contents: 'a) : unit =
  let oc = Pervasives.open_out_bin output_filename in
  Marshal.to_channel oc contents [];
  Pervasives.close_out oc

let get_hot_classes_filename () =
  let prefix = Relative_path.(path_of_prefix Root) in
  let (/) = Filename.concat in
  prefix / "hack" / "hh_hot_classes.json"

let get_hot_classes (filename: string) : SSet.t =
  if not (Disk.file_exists filename)
  then begin
    Hh_logger.log "Hot classes file '%s' was not found" filename;
    SSet.empty
  end
  else begin
    Disk.cat filename
    |> Hh_json.json_of_string
    |> Hh_json.get_object_exn
    |> List.find_exn ~f:(fun (k, _) -> k = "classes")
    |> snd
    |> Hh_json.get_array_exn
    |> List.map ~f:Hh_json.get_string_exn
    |> SSet.of_list
  end

let dump_class_decls filename =
  let start_t = Unix.gettimeofday () in
  Hh_logger.log "Begin saving class declarations";
  try
    let hot_classes_filename = get_hot_classes_filename () in
    Hh_logger.log "Reading hot class names from %s" hot_classes_filename;
    let classes = get_hot_classes hot_classes_filename in
    Hh_logger.log "Exporting %d class declarations..." @@ SSet.cardinal classes;
    let decls = Decl_export.export_class_decls classes in
    Hh_logger.log "Marshalling class declarations...";
    dump_contents_exn filename decls;
    ignore @@ Hh_logger.log_duration "Saved class declarations" start_t
  with exn ->
    let stack = Printexc.get_backtrace () in
    HackEventLogger.save_decls_failure exn stack;
    Hh_logger.exc exn ~stack ~prefix:"Failed to save class declarations: "

(* Dumps the file info and the errors, if any. *)
let dump_saved_state
    ~(save_decls: bool)
    (output_filename: string)
    (naming_table: Naming_table.t)
    (errors: Errors.t) : unit =
  let (naming_table_saved: Naming_table.saved_state_info) =
    Naming_table.to_saved naming_table
  in
  dump_contents output_filename naming_table_saved;

  (* Let's not write empty error files. *)
  if Errors.is_empty errors then () else begin
    let errors_in_phases =
      List.map
        ~f:(fun phase -> (phase, Errors.get_failed_files errors phase))
        [ Errors.Parsing; Errors.Decl; Errors.Naming; Errors.Typing ] in
    dump_contents (get_errors_filename output_filename) errors_in_phases
  end;

  if save_decls then
    dump_class_decls (get_decls_filename output_filename)

let update_save_state
    ~(file_info_on_disk: bool)
    ~(save_decls: bool)
    (naming_table: Naming_table.t)
    (errors: Errors.t)
    (output_filename: string)
    (replace_state_after_saving: bool) : int =
  let t = Unix.gettimeofday () in
  let db_name = output_filename ^ ".sql" in
  if not (RealDisk.file_exists db_name) then
    failwith "Given existing save state SQL file missing";
  dump_saved_state ~save_decls output_filename naming_table errors;
  let () = if file_info_on_disk then begin
    failwith "incrementally updating file info on disk not yet implemented"
  end else begin
    Hh_logger.log "skip writing file info to sqlite table"
  end in
  let edges_added = SharedMem.update_dep_table_sqlite
    db_name
    Build_id.build_revision
    replace_state_after_saving in
  ignore @@ Hh_logger.log_duration "Updating saved state took" t;
  edges_added

(** Saves the saved state to the given path. Returns number of dependency
* edges dumped into the database. *)
let save_state
    ~(file_info_on_disk: bool)
    ~(save_decls: bool)
    (naming_table: Naming_table.t)
    (errors: Errors.t)
    (output_filename: string)
    ~(replace_state_after_saving: bool): int =
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
    dump_saved_state ~save_decls output_filename naming_table errors;
    let () = if file_info_on_disk then begin
      Hh_logger.log "Saving file info (naming table) into a SQLite table.\n";
      (save_all_file_info_sqlite db_name naming_table : unit)
    end in
    let dep_table_edges_added =
      SharedMem.save_dep_table_sqlite
        db_name
        Build_id.build_revision
        replace_state_after_saving in
    let _ : float = Hh_logger.log_duration "Saving saved state took" t in
    dep_table_edges_added
  | Some old_table_filename ->
    (** If server is running from a loaded saved state, it's in-memory
     * tracked depdnencies are incomplete - most of the actual dependencies
     * are in the SQL table. We need to copy that file and update it with
     * the in-memory edges. *)
    let t = Unix.gettimeofday () in
    let content = RealDisk.cat old_table_filename in
    let () = RealDisk.mkdir_p (Filename.dirname output_filename) in
    let () = RealDisk.write_file ~file:db_name ~contents:content in
    let _ : float = Hh_logger.log_duration "Made disk copy of loaded saved state. Took" t in
    update_save_state
      ~file_info_on_disk
      ~save_decls
      naming_table
      errors
      output_filename
      replace_state_after_saving

let get_in_memory_dep_table_entry_count () : (int, string) result =
  Utils.try_with_stack (fun () ->
    SharedMem.get_in_memory_dep_table_entry_count ())
  |> Result.map_error ~f:(fun (exn, _stack) -> Exn.to_string exn)

(* If successful, returns the # of edges from the dependency table that were written. *)
(* TODO: write some other stats, e.g., the number of names, the number of errors, etc. *)
let go
    ~(file_info_on_disk: bool)
    ~(save_decls: bool)
    (naming_table: Naming_table.t)
    (errors: Errors.t)
    (output_filename: string)
    ~(replace_state_after_saving: bool): (int, string) result =
  Utils.try_with_stack
  begin
    fun () -> save_state
      ~file_info_on_disk
      ~save_decls
      naming_table errors
      output_filename
      ~replace_state_after_saving
  end
  |> Result.map_error ~f:(fun (exn, _stack) -> Exn.to_string exn)
