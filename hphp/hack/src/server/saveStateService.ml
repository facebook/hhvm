(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open SaveStateServiceTypes

let get_errors_filename (filename : string) : string = filename ^ ".err"

let get_errors_filename_json (filename : string) : string =
  filename ^ ".err.json"

let get_legacy_decls_filename (filename : string) : string = filename ^ ".decls"

let get_shallow_decls_filename (filename : string) : string =
  filename ^ ".shallowdecls"

(* Writes some OCaml object to a file with the given filename. *)
let save_contents (output_filename : string) (contents : 'a) : unit =
  let chan = Stdlib.open_out_bin output_filename in
  Marshal.to_channel chan contents [];
  Stdlib.close_out chan

(* If the contents doesn't contain the value of the expected type, the result
   is undefined behavior. We may crash, or we may continue with a bogus value. *)
let load_contents_unsafe (input_filename : string) : 'a =
  let ic = Stdlib.open_in_bin input_filename in
  let contents = Marshal.from_channel ic in
  Stdlib.close_in ic;
  contents

(* Loads the file info and the errors, if any. *)
let load_saved_state_exn
    ~(naming_table_fallback_path : string option)
    ~(errors_path : string)
    (ctx : Provider_context.t) : Naming_table.t * saved_state_errors =
  let old_naming_table =
    match naming_table_fallback_path with
    | Some nt_path ->
      let naming_table_exists = Sys.file_exists nt_path in
      Hh_logger.log
        "Checked if the old naming table file exists (%b)"
        naming_table_exists;
      if not naming_table_exists then
        failwith
          (Printf.sprintf
             "Naming table file does not exist on disk: '%s'"
             nt_path);
      Naming_table.load_from_sqlite ctx nt_path
    | None ->
      (*
        If we reach here, then there was neither a SQLite naming table
        in the saved state directory, nor was a path provided via
        genv.local_config.naming_table_path.

        ServerCheckUtils.get_naming_table_fallback_path will return None,
        in which case we raise a failure, since the OCaml-marshalled
        naming table no longer exists.
      *)
      HackEventLogger.naming_table_sqlite_missing ();
      let str =
        "No naming table path found, either from saved state directory or local config"
      in
      Hh_logger.log "%s" str;
      failwith str
  in
  let (old_errors : saved_state_errors) =
    if not (Sys.file_exists errors_path) then
      Relative_path.Set.empty
    else
      Marshal.from_channel (In_channel.create ~binary:true errors_path)
  in
  (old_naming_table, old_errors)

let get_hot_classes (filename : string) : SSet.t =
  if not (Disk.file_exists filename) then (
    Hh_logger.log "Hot classes file '%s' was not found" filename;
    SSet.empty
  ) else
    Disk.cat filename
    |> Hh_json.json_of_string
    |> Hh_json.get_object_exn
    |> List.find_exn ~f:(fun (k, _) -> String.equal k "classes")
    |> snd
    |> Hh_json.get_array_exn
    |> List.map ~f:Hh_json.get_string_exn
    |> SSet.of_list

(** Dumps the naming-table (a saveable form of FileInfo), and errors if any,
and hot class decls. *)
let dump_naming_and_errors
    (output_filename : string)
    (naming_table : Naming_table.t)
    (errors : Errors.t) : unit =
  let naming_sql_filename = output_filename ^ "_naming.sql" in
  let (save_result : Naming_sqlite.save_result) =
    Naming_table.save naming_table naming_sql_filename
  in
  Hh_logger.log
    "Inserted symbols into the naming table:\n%s"
    (Naming_sqlite.show_save_result save_result);
  Hh_logger.log
    "Finished saving naming table with %d errors."
    (List.length save_result.Naming_sqlite.errors);

  if List.length save_result.Naming_sqlite.errors > 0 then
    Exit.exit Exit_status.Sql_assertion_failure;

  assert (Sys.file_exists naming_sql_filename);
  Hh_logger.log "Saved naming table sqlite to '%s'" naming_sql_filename;
  (* Let's not write empty error files. *)
  (if Errors.is_empty errors then
    ()
  else
    let error_files : saved_state_errors = Errors.get_failed_files errors in
    save_contents (get_errors_filename output_filename) error_files);
  ()

(** Sorts and dumps the error relative paths in JSON format.
 * An empty JSON list will be dumped if there are no errors.*)
let dump_errors_json (output_filename : string) (errors : Errors.t) : unit =
  let error_files = Errors.get_failed_files errors in
  let errors_json =
    Hh_json.(
      JSON_Array
        (List.rev
           (Relative_path.Set.fold
              ~init:[]
              ~f:(fun relative_path acc ->
                JSON_String (Relative_path.suffix relative_path) :: acc)
              error_files)))
  in
  let chan = Stdlib.open_out (get_errors_filename_json output_filename) in
  Hh_json.json_to_output chan errors_json;
  Stdlib.close_out chan

let saved_state_info_file_name ~base_file_name = base_file_name ^ "_info.json"

let saved_state_build_revision_write ~(base_file_name : string) : unit =
  let info_file = saved_state_info_file_name ~base_file_name in
  let open Hh_json in
  Out_channel.with_file info_file ~f:(fun fh ->
      json_to_output fh
      @@ JSON_Object [("build_revision", string_ Build_id.build_revision)])

let saved_state_build_revision_read ~(base_file_name : string) : string =
  let info_file = saved_state_info_file_name ~base_file_name in
  let contents = RealDisk.cat info_file in
  let json = Some (Hh_json.json_of_string contents) in
  let build_revision = Hh_json_helpers.Jget.string_exn json "build_revision" in
  build_revision

let dump_dep_graph_64bit ~mode ~db_name ~incremental_info_file =
  let t = Unix.gettimeofday () in
  let base_dep_graph =
    match mode with
    | Typing_deps_mode.InMemoryMode base_dep_graph -> base_dep_graph
    | Typing_deps_mode.SaveToDiskMode { graph; _ } -> graph
  in
  let () =
    let open Hh_json in
    Out_channel.with_file incremental_info_file ~f:(fun fh ->
        json_to_output fh
        @@ JSON_Object [("base_dep_graph", opt_string_to_json base_dep_graph)])
  in
  let dep_table_edges_added =
    Typing_deps.save_discovered_edges
      mode
      ~dest:db_name
      ~reset_state_after_saving:false
  in
  let (_ : float) = Hh_logger.log_duration "Writing discovered edges took" t in
  { dep_table_edges_added }

(** Saves the saved state to the given path. Returns number of dependency
* edges dumped into the database. *)
let save_state (env : ServerEnv.env) (output_filename : string) :
    save_state_result =
  let () = Sys_utils.mkdir_p (Filename.dirname output_filename) in
  let db_name =
    match env.ServerEnv.deps_mode with
    | Typing_deps_mode.InMemoryMode _
    | Typing_deps_mode.SaveToDiskMode _ ->
      output_filename ^ "_64bit_dep_graph.delta"
  in
  let () =
    if Sys.file_exists output_filename then
      failwith
        (Printf.sprintf "Cowardly refusing to overwrite '%s'." output_filename)
    else
      ()
  in
  let () =
    if Sys.file_exists db_name then
      failwith (Printf.sprintf "Cowardly refusing to overwrite '%s'." db_name)
    else
      ()
  in
  let (_ : float) =
    let naming_table = env.ServerEnv.naming_table in
    let errors = env.ServerEnv.errorl in
    let t = Unix.gettimeofday () in
    dump_naming_and_errors output_filename naming_table errors;
    Hh_logger.log_duration "Saving saved-state naming+errors took" t
  in
  match env.ServerEnv.deps_mode with
  | Typing_deps_mode.InMemoryMode _ ->
    let incremental_info_file = output_filename ^ "_incremental_info.json" in
    dump_errors_json output_filename env.ServerEnv.errorl;
    saved_state_build_revision_write ~base_file_name:output_filename;
    dump_dep_graph_64bit
      ~mode:env.ServerEnv.deps_mode
      ~db_name
      ~incremental_info_file
  | Typing_deps_mode.SaveToDiskMode
      { graph = _; new_edges_dir; human_readable_dep_map_dir } ->
    dump_errors_json output_filename env.ServerEnv.errorl;
    saved_state_build_revision_write ~base_file_name:output_filename;
    Hh_logger.warn
      "saveStateService: not saving 64-bit dep graph edges to disk, because they are already in %s"
      new_edges_dir;
    (match human_readable_dep_map_dir with
    | None -> ()
    | Some dir ->
      Hh_logger.warn "saveStateService: human readable dep map dir: %s" dir);
    { dep_table_edges_added = 0 }

let go_naming (naming_table : Naming_table.t) (output_filename : string) :
    (save_naming_result, string) result =
  Utils.try_with_stack (fun () ->
      let save_result = Naming_table.save naming_table output_filename in
      Hh_logger.log
        "Inserted symbols into the naming table:\n%s"
        (Naming_sqlite.show_save_result save_result);

      if List.length save_result.Naming_sqlite.errors > 0 then begin
        Sys_utils.rm_dir_tree output_filename;
        failwith "Naming table state had errors - deleting output file!"
      end else
        {
          nt_files_added = save_result.Naming_sqlite.files_added;
          nt_symbols_added = save_result.Naming_sqlite.symbols_added;
        })
  |> Result.map_error ~f:(fun e -> Exception.get_ctor_string e)

(* If successful, returns the # of edges from the dependency table that were written. *)
(* TODO: write some other stats, e.g., the number of names, the number of errors, etc. *)
let go (env : ServerEnv.env) (output_filename : string) :
    (save_state_result, string) result =
  Utils.try_with_stack (fun () -> save_state env output_filename)
  |> Result.map_error ~f:(fun e -> Exception.get_ctor_string e)
