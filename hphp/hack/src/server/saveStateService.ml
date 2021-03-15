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

(* Gets a set of file paths out of saved state errors *)
let fold_error_files (errors_in_phases : saved_state_errors) :
    Relative_path.Set.t =
  List.fold
    ~init:Relative_path.Set.empty
    ~f:(fun acc (_phase, error_files) ->
      Relative_path.Set.union acc error_files)
    errors_in_phases

(* Given saved state errors, produces 2 sets of file paths: one for those that
    occurred in the given list of phases, and another one for all the rest.
    The 'tf' prefix is in keeping with partition_tf, where it's supposed to
    remind you which list matches the predicate (not actually a predicate in this case),
    and which doesn't. *)
let partition_error_files_tf
    (errors_in_phases : saved_state_errors) (phases : Errors.phase list) :
    Relative_path.Set.t * Relative_path.Set.t =
  let (errors_in_phases_t, errors_in_phases_f) =
    List.partition_tf errors_in_phases ~f:(fun (phase, _error_files) ->
        List.mem ~equal:Errors.equal_phase phases phase)
  in
  (fold_error_files errors_in_phases_t, fold_error_files errors_in_phases_f)

let load_class_decls
    ~(shallow_decls : bool) ~(hot_decls_paths : State_loader.hot_decls_paths) :
    unit =
  let start_t = Unix.gettimeofday () in
  Hh_logger.log "Begin loading class declarations";
  let State_loader.{ legacy_hot_decls_path; shallow_hot_decls_path } =
    hot_decls_paths
  in
  try
    let (filename, num_classes) =
      if shallow_decls then
        ( shallow_hot_decls_path,
          load_contents_unsafe shallow_hot_decls_path
          |> Decl_export.restore_shallow_decls )
      else
        ( legacy_hot_decls_path,
          load_contents_unsafe legacy_hot_decls_path
          |> Decl_export.restore_legacy_decls )
    in
    let msg =
      Printf.sprintf "Loaded %d class declarations from %s" num_classes filename
    in
    HackEventLogger.load_decls_end start_t;
    ignore @@ Hh_logger.log_duration msg start_t
  with exn ->
    let stack = Printexc.get_backtrace () in
    HackEventLogger.load_decls_failure exn stack;
    Hh_logger.exc exn ~stack ~prefix:"Failed to load class declarations: "

(* Loads the file info and the errors, if any. *)
let load_saved_state
    ~(load_decls : bool)
    ~(shallow_decls : bool)
    ~(naming_table_fallback_path : string option)
    ~(naming_table_path : string)
    ~(hot_decls_paths : State_loader.hot_decls_paths)
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
      let chan = In_channel.create ~binary:true naming_table_path in
      let (old_saved : Naming_table.saved_state_info) =
        Marshal.from_channel chan
      in
      Sys_utils.close_in_no_fail naming_table_path chan;
      Naming_table.from_saved old_saved
  in
  let (old_errors : saved_state_errors) =
    if not (Sys.file_exists errors_path) then
      []
    else
      Marshal.from_channel (In_channel.create ~binary:true errors_path)
  in
  if load_decls then load_class_decls ~shallow_decls ~hot_decls_paths;

  (old_naming_table, old_errors)

let get_hot_classes_filename () =
  let prefix = Relative_path.(path_of_prefix Root) in
  let ( / ) = Filename.concat in
  prefix / "hack" / "hh_hot_classes.json"

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

let dump_class_decls genv env ~base_filename =
  let ctx = Provider_utils.ctx_from_server_env env in
  let start_t = Unix.gettimeofday () in
  let hot_classes_filename = get_hot_classes_filename () in
  Hh_logger.log
    "Begin saving class declarations to %s based on %s"
    base_filename
    hot_classes_filename;
  try
    let classes = get_hot_classes hot_classes_filename in
    let t1 = Unix.gettimeofday () in
    let legacy_decls = Decl_export.collect_legacy_decls ctx classes in
    let t2 = Unix.gettimeofday () in
    let shallow_decls =
      Decl_export.collect_shallow_decls ctx genv.ServerEnv.workers classes
    in
    let t3 = Unix.gettimeofday () in
    save_contents (get_legacy_decls_filename base_filename) legacy_decls;
    save_contents (get_shallow_decls_filename base_filename) shallow_decls;
    let t4 = Unix.gettimeofday () in
    let telemetry =
      Telemetry.create ()
      |> Telemetry.int_ ~key:"count" ~value:(SSet.cardinal classes)
      |> Telemetry.float_ ~key:"load_classnames" ~value:(t1 -. start_t)
      |> Telemetry.float_ ~key:"collect_legacy" ~value:(t2 -. t1)
      |> Telemetry.float_ ~key:"collect_shallow" ~value:(t3 -. t2)
      |> Telemetry.float_ ~key:"save" ~value:(t4 -. t3)
    in
    HackEventLogger.save_decls_end start_t telemetry;
    Hh_logger.log "Saved class declarations: %s" (Telemetry.to_string telemetry)
  with e ->
    let e = Exception.wrap e in
    HackEventLogger.save_decls_failure e;
    Hh_logger.error
      "Failed to save class declarations:\n%s"
      (Exception.to_string e)

(** Dumps the naming-table (a saveable form of FileInfo), and errors if any,
and hot class decls. *)
let dump_naming_errors_decls
    ~(save_decls : bool)
    (genv : ServerEnv.genv)
    (env : ServerEnv.env)
    (output_filename : string)
    (naming_table : Naming_table.t)
    (errors : Errors.t) : unit =
  Hh_logger.log "Marshalling the naming table...";
  let (naming_table_saved : Naming_table.saved_state_info) =
    Naming_table.to_saved naming_table
  in
  save_contents output_filename naming_table_saved;

  if not (Sys.file_exists output_filename) then
    failwith
      (Printf.sprintf "Did not find file infos file '%s'" output_filename)
  else
    Hh_logger.log "Saved file infos to '%s'" output_filename;

  (* Let's not write empty error files. *)
  ( if Errors.is_empty errors then
    ()
  else
    let errors_in_phases =
      List.map
        ~f:(fun phase -> (phase, Errors.get_failed_files errors phase))
        [Errors.Parsing; Errors.Decl; Errors.Naming; Errors.Typing]
    in
    save_contents (get_errors_filename output_filename) errors_in_phases );

  if save_decls then dump_class_decls genv env ~base_filename:output_filename

let dump_dep_graph_32bit ~db_name ~replace_state_after_saving =
  let t = Unix.gettimeofday () in
  match SharedMem.loaded_dep_table_filename () with
  | None ->
    let dep_table_edges_added =
      SharedMem.save_dep_table_sqlite
        db_name
        Build_id.build_revision
        ~replace_state_after_saving
    in
    let (_ : float) = Hh_logger.log_duration "Saving saved state took" t in
    { dep_table_edges_added }
  | Some old_table_filename ->
    (* If server is running from a loaded saved state, its in-memory
     * tracked depdnencies are incomplete - most of the actual dependencies
     * are in the SQL table. We need to copy that file and update it with
     * the in-memory edges. *)
    let t = Unix.gettimeofday () in
    FileUtil.cp [old_table_filename] db_name;
    let (_ : float) =
      Hh_logger.log_duration "Made disk copy of loaded saved state. Took" t
    in
    if not (RealDisk.file_exists db_name) then
      failwith
        "Existing save state SQL file missing; disk copy must have failed";
    let dep_table_edges_added =
      SharedMem.update_dep_table_sqlite
        db_name
        Build_id.build_revision
        replace_state_after_saving
    in
    let (_ : float) = Hh_logger.log_duration "Updating saved state took" t in
    { dep_table_edges_added }

let saved_state_info_file_name ~base_file_name = base_file_name ^ "_info.json"

let saved_state_build_revision_write ~(base_file_name : string) : unit =
  let info_file = saved_state_info_file_name ~base_file_name in
  let open Hh_json in
  Out_channel.with_file info_file ~f:(fun fh ->
      json_to_output fh
      @@ JSON_Object [("build_revision", string_ Build_id.build_revision)])

let saved_state_build_revision_read ~(base_file_name : string) : string =
  let info_file = saved_state_info_file_name ~base_file_name in
  let contents = Sys_utils.cat info_file in
  let json = Some (Hh_json.json_of_string contents) in
  let build_revision = Hh_json_helpers.Jget.string_exn json "build_revision" in
  build_revision

let dump_dep_graph_64bit
    ~mode ~db_name ~incremental_info_file ~replace_state_after_saving =
  let t = Unix.gettimeofday () in
  let base_dep_graph =
    match mode with
    | Typing_deps_mode.SQLiteMode -> None
    | Typing_deps_mode.CustomMode base_dep_graph -> base_dep_graph
    | Typing_deps_mode.SaveCustomMode { graph; _ } -> graph
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
      ~build_revision:Build_id.build_revision
      ~reset_state_after_saving:replace_state_after_saving
  in
  let (_ : float) = Hh_logger.log_duration "Writing discovered edges took" t in
  { dep_table_edges_added }

(** Saves the saved state to the given path. Returns number of dependency
* edges dumped into the database. *)
let save_state
    ~(save_decls : bool)
    (genv : ServerEnv.genv)
    (env : ServerEnv.env)
    (output_filename : string)
    ~(replace_state_after_saving : bool) : save_state_result =
  let () = Sys_utils.mkdir_p (Filename.dirname output_filename) in
  let db_name =
    match env.ServerEnv.deps_mode with
    | Typing_deps_mode.SQLiteMode -> output_filename ^ ".sql"
    | Typing_deps_mode.CustomMode _
    | Typing_deps_mode.SaveCustomMode _ ->
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
    dump_naming_errors_decls
      ~save_decls
      genv
      env
      output_filename
      naming_table
      errors;
    Hh_logger.log_duration "Saving saved-state naming/errors/decls took" t
  in
  match env.ServerEnv.deps_mode with
  | Typing_deps_mode.SQLiteMode ->
    dump_dep_graph_32bit ~db_name ~replace_state_after_saving
  | Typing_deps_mode.CustomMode _ ->
    let incremental_info_file = output_filename ^ "_incremental_info.json" in
    saved_state_build_revision_write ~base_file_name:output_filename;
    dump_dep_graph_64bit
      ~mode:env.ServerEnv.deps_mode
      ~db_name
      ~incremental_info_file
      ~replace_state_after_saving
  | Typing_deps_mode.SaveCustomMode { graph = _; new_edges_dir } ->
    saved_state_build_revision_write ~base_file_name:output_filename;
    Hh_logger.warn
      "saveStateService: not saving 64-bit dep graph edges to disk, because they are already in %s"
      new_edges_dir;
    { dep_table_edges_added = 0 }

let get_in_memory_dep_table_entry_count () : (int, string) result =
  Utils.try_with_stack (fun () ->
      SharedMem.get_in_memory_dep_table_entry_count ())
  |> Result.map_error ~f:(fun (exn, _stack) -> Exn.to_string exn)

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
  |> Result.map_error ~f:(fun (exn, _stack) -> Exn.to_string exn)

(* If successful, returns the # of edges from the dependency table that were written. *)
(* TODO: write some other stats, e.g., the number of names, the number of errors, etc. *)
let go
    ~(save_decls : bool)
    (genv : ServerEnv.genv)
    (env : ServerEnv.env)
    (output_filename : string)
    ~(replace_state_after_saving : bool) : (save_state_result, string) result =
  Utils.try_with_stack (fun () ->
      save_state
        ~save_decls
        genv
        env
        output_filename
        ~replace_state_after_saving)
  |> Result.map_error ~f:(fun (exn, _stack) -> Exn.to_string exn)
