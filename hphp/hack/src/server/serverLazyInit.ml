(*
 * Copyright (c) 2018, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* Lazy Initialization:

   During Lazy initialization, hh_server tries to do as little work as possible.
   The init_from_saved_state behavior is this:

     Load from saved state -> Parse dirty files -> Naming -> Dirty Typecheck

   The full_init behavior is this: (similar to eager init, but with lazy decl)

     Full Parsing -> Naming -> Full Typecheck (with lazy decl)
*)

(* module Hack_bucket = Bucket *)
open Hh_prelude

(* module Bucket = Hack_bucket *)
open GlobalOptions
open Result.Export
open Reordered_argument_collections
open SearchServiceRunner
open ServerCheckUtils
open ServerEnv
open ServerInitCommon
open ServerInitTypes
open String_utils
module SLC = ServerLocalConfig

type deptable =
  | SQLiteDeptable of string
  | CustomDeptable of string

let deptable_with_filename ~(is_64bit : bool) (fn : string) : deptable =
  if is_64bit then
    CustomDeptable fn
  else
    SQLiteDeptable fn

let lock_and_load_deptable
    ~(base_file_name : string)
    ~(deptable : deptable)
    ~(ignore_hh_version : bool)
    ~(fail_if_missing : bool) : unit =
  match deptable with
  | SQLiteDeptable fn ->
    if String.length fn = 0 && not fail_if_missing then
      Hh_logger.log "The dependency file was not specified - ignoring"
    else begin
      (* The SQLite deptable must be loaded in the master process *)

      (* Take a lock on the info file for the SQLite *)
      try
        LoadScriptUtils.lock_saved_state fn;
        let start_t = Unix.gettimeofday () in
        SharedMem.load_dep_table_sqlite fn ignore_hh_version;
        let (_t : float) =
          Hh_logger.log_duration "Did read the dependency file (sec)" start_t
        in
        HackEventLogger.load_deptable_end start_t
      with
      | (SharedMem.Sql_assertion_failure 11 | SharedMem.Sql_assertion_failure 14)
        as e ->
        (* SQL_corrupt *)
        let stack = Caml.Printexc.get_raw_backtrace () in
        LoadScriptUtils.delete_corrupted_saved_state fn;
        Caml.Printexc.raise_with_backtrace e stack
    end
  | CustomDeptable fn ->
    let () =
      if not ignore_hh_version then
        let build_revision =
          SaveStateService.saved_state_build_revision_read ~base_file_name
        in
        if not (String.equal build_revision Build_id.build_revision) then
          raise
          @@ Failure
               (Printf.sprintf
                  ( "Saved-state build mismatch, this saved-state was built "
                  ^^ " for version '%s', but we expected '%s'" )
                  build_revision
                  Build_id.build_revision)
    in
    (* The new dependency graph is threaded through function calls
     * instead of stored in a global *)
    Hh_logger.log "Custom dependency graph will be loaded lazily from %s" fn

let merge_saved_state_futures
    (genv : genv)
    (ctx : Provider_context.t)
    (dependency_table_saved_state_future :
      ( ( Saved_state_loader.Naming_and_dep_table_info.main_artifacts,
          Saved_state_loader.Naming_and_dep_table_info.additional_info )
        Saved_state_loader.load_result,
        ServerInitTypes.load_state_error )
      result
      Future.t)
    (naming_table_saved_state_future :
      ( ( Saved_state_loader.Naming_table_info.main_artifacts,
          Saved_state_loader.Naming_table_info.additional_info )
        Saved_state_loader.load_result
        option,
        string )
      result
      Future.t) : (loaded_info, load_state_error) result =
  let t = Unix.gettimeofday () in

  let merge dependency_table_saved_state_result naming_table_saved_state_result
      =
    match dependency_table_saved_state_result with
    | Error error ->
      Hh_logger.log
        "Unhandled Future.error from state loader: %s"
        (Future.error_to_string error);
      let e = Exception.wrap_unraised (Future.error_to_exn error) in
      let exn = Exception.to_exn e in
      let stack = Utils.Callstack (Exception.get_backtrace_string e) in
      Error (Load_state_unhandled_exception { exn; stack })
    | Ok (Error error) -> Error error
    | Ok (Ok deptable_result) ->
      let ( downloaded_naming_table_path,
            naming_table_manifold_path,
            dirty_naming_files ) =
        match naming_table_saved_state_result with
        | Ok (Ok None) -> (None, None, [])
        | Ok
            (Ok
              (Some
                {
                  Saved_state_loader.main_artifacts;
                  additional_info = ();
                  changed_files;
                  manifold_path;
                  corresponding_rev = _;
                  mergebase_rev = _;
                  is_cached = _;
                })) ->
          let (_ : float) =
            Hh_logger.log_duration
              "Finished downloading naming table and dependency graph."
              t
          in
          let path =
            main_artifacts
              .Saved_state_loader.Naming_table_info.naming_table_path
          in
          (Some (Path.to_string path), Some manifold_path, changed_files)
        | Ok (Error err) ->
          Hh_logger.warn "Failed to download naming table saved state: %s" err;
          (None, None, [])
        | Error error ->
          Hh_logger.warn
            "Failed to download the naming table saved state: %s"
            (Future.error_to_string error);
          (None, None, [])
      in
      let ignore_hh_version =
        ServerArgs.ignore_hh_version genv.ServerEnv.options
      in
      let fail_if_missing = not genv.local_config.SLC.can_skip_deptable in
      let {
        Saved_state_loader.main_artifacts;
        additional_info;
        changed_files = _;
        manifold_path = _;
        corresponding_rev;
        mergebase_rev;
        is_cached = _;
      } =
        deptable_result
      in
      let {
        Saved_state_loader.Naming_and_dep_table_info.naming_table_path =
          deptable_naming_table_blob_path;
        dep_table_path;
        legacy_hot_decls_path;
        shallow_hot_decls_path;
        errors_path;
      } =
        main_artifacts
      in
      let {
        Saved_state_loader.Naming_and_dep_table_info.mergebase_global_rev;
        dep_table_is_64bit;
        dirty_files_promise;
      } =
        additional_info
      in
      let deptable =
        deptable_with_filename
          ~is_64bit:dep_table_is_64bit
          (Path.to_string dep_table_path)
      in
      lock_and_load_deptable
        ~base_file_name:(Path.to_string deptable_naming_table_blob_path)
        ~deptable
        ~ignore_hh_version
        ~fail_if_missing;
      let load_decls = genv.local_config.SLC.load_decls_from_saved_state in
      let shallow_decls = genv.local_config.SLC.shallow_class_decl in
      let naming_table_fallback_path =
        get_naming_table_fallback_path genv downloaded_naming_table_path
      in
      let (old_naming_table, old_errors) =
        SaveStateService.load_saved_state
          ctx
          ~naming_table_path:(Path.to_string deptable_naming_table_blob_path)
          ~naming_table_fallback_path
          ~load_decls
          ~shallow_decls
          ~legacy_hot_decls_path:(Path.to_string legacy_hot_decls_path)
          ~shallow_hot_decls_path:(Path.to_string shallow_hot_decls_path)
          ~errors_path:(Path.to_string errors_path)
      in
      let t = Unix.time () in
      (match dirty_files_promise |> Future.get ~timeout:200 with
      | Error error -> Error (Load_state_dirty_files_failure error)
      | Ok
          {
            Saved_state_loader.Naming_and_dep_table_info.master_changes =
              dirty_master_files;
            local_changes = dirty_local_files;
          } ->
        let () = HackEventLogger.state_loader_dirty_files t in
        let dirty_naming_files = Relative_path.Set.of_list dirty_naming_files in
        let dirty_master_files = dirty_master_files in
        let dirty_local_files = dirty_local_files in
        Ok
          {
            naming_table_fn = Path.to_string deptable_naming_table_blob_path;
            deptable_fn = Path.to_string dep_table_path;
            deptable_is_64bit = dep_table_is_64bit;
            naming_table_fallback_fn = naming_table_fallback_path;
            corresponding_rev = Hg.Hg_rev corresponding_rev;
            mergebase_rev = mergebase_global_rev;
            mergebase = Future.of_value (Some mergebase_rev);
            dirty_naming_files;
            dirty_master_files;
            dirty_local_files;
            old_naming_table;
            old_errors;
            state_distance = None;
            naming_table_manifold_path;
          })
  in
  let merge left right = Ok (merge left right) in
  let future =
    Future.merge
      dependency_table_saved_state_future
      naming_table_saved_state_future
      merge
  in
  (* We don't call Future.get on the merged future until it's ready because
      the implementation of Future.get blocks on the first future until it's
      ready, then moves on to the second future, but the second future, since
      it's composed of bound continuations, will not be making as much progress
      on its own in this case. *)
  let rec wait_until_ready future =
    if not (Future.is_ready future) then begin
      Sys_utils.sleep ~seconds:0.04;
      wait_until_ready future
    end else
      match Future.get future ~timeout:0 with
      | Ok result -> result
      | Error error -> failwith (Future.error_to_string error)
  in
  wait_until_ready future

let download_and_load_state_exn
    ~(genv : ServerEnv.genv) ~(ctx : Provider_context.t) ~(root : Path.t) :
    (loaded_info, load_state_error) result =
  let ignore_hh_version = ServerArgs.ignore_hh_version genv.options in
  (* TODO(hverr): Support the ignore_hhconfig flag, how to do this with Watchman? *)
  let _ignore_hhconfig = ServerArgs.saved_state_ignore_hhconfig genv.options in
  let naming_table_saved_state_future =
    if genv.local_config.ServerLocalConfig.enable_naming_table_fallback then begin
      Hh_logger.log "Starting naming table download.";
      let loader_future =
        State_loader_futures.load
          ~watchman_opts:
            Saved_state_loader.Watchman_options.{ root; sockname = None }
          ~ignore_hh_version
          ~saved_state_type:Saved_state_loader.Naming_table
        |> Future.with_timeout ~timeout:60
      in
      Future.continue_and_map_err loader_future @@ fun result ->
      match result with
      | Ok (Ok load_state) -> Ok (Some load_state)
      | Ok (Error e) -> Error (Saved_state_loader.long_user_message_of_error e)
      | Error e -> Error (Future.error_to_string e)
    end else
      Future.of_value (Ok None)
  in
  let dependency_table_saved_state_future :
      ( ( Saved_state_loader.Naming_and_dep_table_info.main_artifacts,
          Saved_state_loader.Naming_and_dep_table_info.additional_info )
        Saved_state_loader.load_result,
        ServerInitTypes.load_state_error )
      result
      Future.t =
    Hh_logger.log "Downloading dependency graph from DevX infra";
    let loader_future =
      State_loader_futures.load
        ~watchman_opts:
          Saved_state_loader.Watchman_options.{ root; sockname = None }
        ~ignore_hh_version
        ~saved_state_type:
          (Saved_state_loader.Naming_and_dep_table
             {
               is_64bit =
                 genv.local_config.ServerLocalConfig.load_state_natively_64bit;
             })
      |> Future.with_timeout ~timeout:60
    in
    let loader_future =
      Future.continue_with loader_future @@ function
      | Error e -> Error (Load_state_saved_state_loader_failure e)
      | Ok v -> Ok v
    in
    loader_future
  in
  merge_saved_state_futures
    genv
    ctx
    dependency_table_saved_state_future
    naming_table_saved_state_future

let use_precomputed_state_exn
    (genv : ServerEnv.genv)
    (ctx : Provider_context.t)
    (info : ServerArgs.saved_state_target_info)
    (profiling : CgroupProfiler.Profiling.t) : loaded_info =
  let {
    ServerArgs.naming_table_path;
    corresponding_base_revision;
    deptable_fn;
    deptable_is_64bit;
    changes;
    naming_changes;
    prechecked_changes;
  } =
    info
  in
  let ignore_hh_version = ServerArgs.ignore_hh_version genv.ServerEnv.options in
  let fail_if_missing = not genv.local_config.SLC.can_skip_deptable in
  let deptable =
    deptable_with_filename ~is_64bit:deptable_is_64bit deptable_fn
  in
  CgroupProfiler.collect_cgroup_stats ~profiling ~stage:"load deptable"
  @@ fun () ->
  lock_and_load_deptable
    ~base_file_name:naming_table_path
    ~deptable
    ~ignore_hh_version
    ~fail_if_missing;
  let changes = Relative_path.set_of_list changes in
  let naming_changes = Relative_path.set_of_list naming_changes in
  let prechecked_changes = Relative_path.set_of_list prechecked_changes in
  let load_decls = genv.local_config.SLC.load_decls_from_saved_state in
  let shallow_decls = genv.local_config.SLC.shallow_class_decl in
  let naming_table_fallback_path = get_naming_table_fallback_path genv None in
  let legacy_hot_decls_path =
    ServerArgs.legacy_hot_decls_path_for_target_info info
  in
  let shallow_hot_decls_path =
    ServerArgs.shallow_hot_decls_path_for_target_info info
  in
  let errors_path = ServerArgs.errors_path_for_target_info info in
  let (old_naming_table, old_errors) =
    CgroupProfiler.collect_cgroup_stats ~profiling ~stage:"load saved state"
    @@ fun () ->
    SaveStateService.load_saved_state
      ctx
      ~naming_table_path
      ~naming_table_fallback_path
      ~load_decls
      ~shallow_decls
      ~legacy_hot_decls_path
      ~shallow_hot_decls_path
      ~errors_path
  in
  {
    naming_table_fn = naming_table_path;
    deptable_fn;
    deptable_is_64bit;
    naming_table_fallback_fn = naming_table_fallback_path;
    corresponding_rev =
      Hg.Global_rev (int_of_string corresponding_base_revision);
    mergebase_rev = None;
    mergebase = Future.of_value None;
    dirty_naming_files = naming_changes;
    dirty_master_files = prechecked_changes;
    dirty_local_files = changes;
    old_naming_table;
    old_errors;
    state_distance = None;
    naming_table_manifold_path = None;
  }

(** This function ensures the naming-table is ready for us to do "naming" on the dirty files
("parsing_files"), i.e. check all the symbols in them, add them to the naming table, and report
errors if any of the dirty files had duplicate names.
*)
let naming_from_saved_state
    (ctx : Provider_context.t)
    (old_naming_table : Naming_table.t)
    (parsing_files : Relative_path.Set.t)
    (naming_table_fallback_fn : string option)
    (t : float)
    ~(profiling : CgroupProfiler.Profiling.t) : float =
  CgroupProfiler.collect_cgroup_stats
    ~profiling
    ~stage:"naming from saved state"
  @@ fun () ->
  begin
    match naming_table_fallback_fn with
    | Some _ ->
      (* Set the SQLite fallback path for the reverse naming table, then block out all entries in
      any dirty files to make sure we properly handle file deletes. *)
      Relative_path.Set.iter parsing_files ~f:(fun k ->
          match Naming_table.get_file_info old_naming_table k with
          | None ->
            (* If we can't find the file in [old_naming_table] we don't consider that an error, since
             * it could be a new file that was added. *)
            ()
          | Some v ->
            let backend = Provider_context.get_backend ctx in
            Naming_provider.remove_type_batch
              backend
              (v.FileInfo.classes |> List.map ~f:snd);
            Naming_provider.remove_type_batch
              backend
              (v.FileInfo.typedefs |> List.map ~f:snd);
            Naming_provider.remove_type_batch
              backend
              (v.FileInfo.record_defs |> List.map ~f:snd);
            Naming_provider.remove_fun_batch
              backend
              (v.FileInfo.funs |> List.map ~f:snd);
            Naming_provider.remove_const_batch
              backend
              (v.FileInfo.consts |> List.map ~f:snd))
    | None ->
      (* Name all the files from the old naming-table (except the new ones we parsed since
    they'll be named by our caller, next). We assume the old naming-table came from a clean
    state, which is why we skip checking for "already bound" conditions. *)
      let old_hack_names =
        Naming_table.filter old_naming_table ~f:(fun k _v ->
            not (Relative_path.Set.mem parsing_files k))
      in
      Naming_table.fold old_hack_names ~init:() ~f:(fun k info () ->
          Naming_global.ndecl_file_skip_if_already_bound ctx k info);
      hh_log_heap ()
  end;
  HackEventLogger.naming_from_saved_state_end t;
  Hh_logger.log_duration "NAMING_FROM_SAVED_STATE_END" t

(* Prechecked files are gated with a flag and not supported in AI/check/saving
 * of saved state modes. *)
let use_prechecked_files (genv : ServerEnv.genv) : bool =
  ServerPrecheckedFiles.should_use genv.options genv.local_config
  && Option.is_none (ServerArgs.ai_mode genv.options)
  && (not (ServerArgs.check_mode genv.options))
  && Option.is_none (ServerArgs.save_filename genv.options)
  && Option.is_none (ServerArgs.save_with_spec genv.options)

let get_dirty_fast
    (old_naming_table : Naming_table.t)
    (fast : FileInfo.names Relative_path.Map.t)
    (dirty : Relative_path.Set.t) : FileInfo.names Relative_path.Map.t =
  Relative_path.Set.fold
    dirty
    ~f:
      begin
        fun fn acc ->
        let dirty_fast = Relative_path.Map.find_opt fast fn in
        let dirty_old_fast =
          Naming_table.get_file_info old_naming_table fn
          |> Option.map ~f:FileInfo.simplify
        in
        let fast =
          Option.merge dirty_old_fast dirty_fast ~f:FileInfo.merge_names
        in
        match fast with
        | Some fast -> Relative_path.Map.add acc ~key:fn ~data:fast
        | None -> acc
      end
    ~init:Relative_path.Map.empty

let names_to_deps (deps_mode : Typing_deps_mode.t) (names : FileInfo.names) :
    Typing_deps.DepSet.t =
  let open Typing_deps in
  let { FileInfo.n_funs; n_classes; n_record_defs; n_types; n_consts } =
    names
  in
  let add_deps_of_sset dep_ctor sset depset =
    SSet.fold sset ~init:depset ~f:(fun n acc ->
        DepSet.add acc (Dep.make (hash_mode deps_mode) (dep_ctor n)))
  in
  let deps =
    add_deps_of_sset (fun n -> Dep.Fun n) n_funs (DepSet.make deps_mode)
  in
  let deps = add_deps_of_sset (fun n -> Dep.Type n) n_classes deps in
  let deps = add_deps_of_sset (fun n -> Dep.Type n) n_record_defs deps in
  let deps = add_deps_of_sset (fun n -> Dep.Type n) n_types deps in
  let deps = add_deps_of_sset (fun n -> Dep.GConst n) n_consts deps in
  let deps = add_deps_of_sset (fun n -> Dep.GConstName n) n_consts deps in
  deps

(** Compare declarations loaded from the saved state to declarations based on
    the current versions of dirty files. This lets us check a smaller set of
    files than the set we'd check if old declarations were not available.
    To be used only when load_decls_from_saved_state is enabled. *)
let get_files_to_undecl_and_recheck
    (genv : ServerEnv.genv)
    (env : ServerEnv.env)
    (old_naming_table : Naming_table.t)
    (new_fast : FileInfo.names Relative_path.Map.t)
    (dirty_fast : FileInfo.names Relative_path.Map.t)
    (files_to_redeclare : Relative_path.Set.t) :
    Relative_path.Set.t * Relative_path.Set.t =
  let bucket_size = genv.local_config.SLC.type_decl_bucket_size in
  let fast =
    Relative_path.Set.fold
      files_to_redeclare
      ~init:Relative_path.Map.empty
      ~f:(fun path acc ->
        match Relative_path.Map.find_opt dirty_fast path with
        | Some info -> Relative_path.Map.add acc ~key:path ~data:info
        | None -> acc)
  in
  let get_classes path =
    let old_names =
      Naming_table.get_file_info old_naming_table path
      |> Option.map ~f:FileInfo.simplify
    in
    let new_names = Relative_path.Map.find_opt new_fast path in
    let classes_from_names x = x.FileInfo.n_classes in
    let old_classes = Option.map old_names ~f:classes_from_names in
    let new_classes = Option.map new_names ~f:classes_from_names in
    Option.merge old_classes new_classes ~f:SSet.union
    |> Option.value ~default:SSet.empty
  in
  let dirty_names =
    Relative_path.Map.fold dirty_fast ~init:FileInfo.empty_names ~f:(fun _ ->
        FileInfo.merge_names)
  in
  let ctx = Provider_utils.ctx_from_server_env env in
  Decl_redecl_service.oldify_type_decl
    ctx
    ~bucket_size
    genv.workers
    get_classes
    ~previously_oldified_defs:FileInfo.empty_names
    ~defs:dirty_names;
  let { Decl_redecl_service.to_redecl; to_recheck; _ } =
    Decl_redecl_service.redo_type_decl
      ~bucket_size
      ctx
      genv.workers
      get_classes
      ~previously_oldified_defs:dirty_names
      ~defs:fast
  in
  Decl_redecl_service.remove_old_defs ctx ~bucket_size genv.workers dirty_names;
  let to_recheck_deps = Typing_deps.add_all_deps env.deps_mode to_redecl in
  let to_recheck_deps = Typing_deps.DepSet.union to_recheck_deps to_recheck in
  let files_to_undecl = Typing_deps.Files.get_files to_redecl in
  let files_to_recheck = Typing_deps.Files.get_files to_recheck_deps in
  (* we use lazy here to avoid expensive string generation when logging
       * is not enabled *)
  Hh_logger.log_lazy ~category:"fanout_information"
  @@ lazy
       Hh_json.(
         json_to_string
         @@ JSON_Object
              [
                ("tag", string_ "saved_state_init_fanout");
                ( "hashes",
                  array_
                    string_
                    Typing_deps.(
                      List.map ~f:Dep.to_hex_string
                      @@ DepSet.elements to_recheck_deps) );
              ]);

  (files_to_undecl, files_to_recheck)

(* We start off with a list of files that have changed since the state was
 * saved (dirty_files), and two maps of the class / function declarations
 * -- one made when the state was saved (old_fast) and one made for the
 * current files in the repository (new_fast). We grab the declarations from
 * both, to account for both the declaratons that were deleted and those that
 * are newly created. Then we use the deptable to figure out the files that
 * referred to them. Finally we recheck the lot.
 *
 * Args:
 *
 * genv, env : environments
 * old_fast: old file-ast from saved state
 * new_fast: newly parsed file ast
 * dirty_master_files and dirty_local_files: we need to typecheck these and,
 *    since their decl have changed, also all of their dependencies
 * similar_files: we only need to typecheck these,
 *    not their dependencies since their decl are unchanged
 * *)
let type_check_dirty
    (genv : ServerEnv.genv)
    (env : ServerEnv.env)
    (old_naming_table : Naming_table.t)
    (new_fast : FileInfo.names Relative_path.Map.t)
    ~(dirty_master_files_unchanged_hash : Relative_path.Set.t)
    ~(dirty_master_files_changed_hash : Relative_path.Set.t)
    ~(dirty_local_files_unchanged_hash : Relative_path.Set.t)
    ~(dirty_local_files_changed_hash : Relative_path.Set.t)
    (t : float)
    (profiling : CgroupProfiler.Profiling.t) : ServerEnv.env * float =
  let start_t = Unix.gettimeofday () in
  let telemetry =
    Telemetry.create ()
    |> Telemetry.float_ ~key:"start_time" ~value:start_t
    |> Telemetry.string_ ~key:"reason" ~value:"lazy_dirty_init"
  in
  let dirty_files_unchanged_hash =
    Relative_path.Set.union
      dirty_master_files_unchanged_hash
      dirty_local_files_unchanged_hash
  in
  let dirty_files_changed_hash =
    Relative_path.Set.union
      dirty_master_files_changed_hash
      dirty_local_files_changed_hash
  in
  let dirty_changed_fast =
    get_dirty_fast old_naming_table new_fast dirty_files_changed_hash
  in
  let names s =
    Relative_path.Map.fold
      dirty_changed_fast
      ~f:
        begin
          fun k v acc ->
          if Relative_path.Set.mem s k then
            FileInfo.merge_names v acc
          else
            acc
        end
      ~init:FileInfo.empty_names
  in
  let ctx = Provider_utils.ctx_from_server_env env in
  let deps_mode = Provider_context.get_deps_mode ctx in
  let master_deps =
    names dirty_master_files_changed_hash |> names_to_deps deps_mode
  in
  let local_deps =
    names dirty_local_files_changed_hash |> names_to_deps deps_mode
  in
  (* Include similar_files in the dirty_fast used to determine which loaded
     declarations to oldify. This is necessary because the positions of
     declarations may have changed, which affects error messages and FIXMEs. *)
  let get_files_to_undecl_and_recheck =
    get_files_to_undecl_and_recheck genv env old_naming_table new_fast
    @@ extend_fast
         genv
         dirty_changed_fast
         env.naming_table
         dirty_files_unchanged_hash
  in
  let (env, to_undecl, to_recheck) =
    if use_prechecked_files genv then
      (* Start with dirty files and fan-out of local changes only *)
      let (to_undecl, to_recheck) =
        if genv.local_config.SLC.load_decls_from_saved_state then
          get_files_to_undecl_and_recheck dirty_local_files_changed_hash
        else
          let deps = Typing_deps.add_all_deps env.deps_mode local_deps in
          (Relative_path.Set.empty, Typing_deps.Files.get_files deps)
      in
      ( ServerPrecheckedFiles.set
          env
          (Initial_typechecking
             {
               rechecked_files = Relative_path.Set.empty;
               dirty_local_deps = local_deps;
               dirty_master_deps = master_deps;
               clean_local_deps = Typing_deps.(DepSet.make deps_mode);
             }),
        to_undecl,
        to_recheck )
    else
      (* Start with full fan-out immediately *)
      let (to_undecl, to_recheck) =
        if genv.local_config.SLC.load_decls_from_saved_state then
          get_files_to_undecl_and_recheck dirty_files_changed_hash
        else
          let deps = Typing_deps.DepSet.union master_deps local_deps in
          let deps = Typing_deps.add_all_deps env.deps_mode deps in
          (Relative_path.Set.empty, Typing_deps.Files.get_files deps)
      in
      (env, to_undecl, to_recheck)
  in
  (* We still need to typecheck files whose declarations did not change *)
  let to_recheck =
    Relative_path.Set.union to_recheck dirty_files_unchanged_hash
  in
  let fast = extend_fast genv dirty_changed_fast env.naming_table to_recheck in
  let files_to_check = Relative_path.Map.keys fast in

  (* HACK: dump the fanout that we calculated and exit. This is for
  `hh_fanout`'s regression testing vs. `hh_server`. This can be deleted once
  we no longer worry about `hh_fanout` regressing vs. `hh_server`. Deletion
  is tracked at T65464119. *)
  if ServerArgs.dump_fanout genv.options then (
    Hh_json.json_to_multiline_output
      stdout
      (Hh_json.JSON_Object
         [
           ( "recheck_files",
             Hh_json.JSON_Array
               ( files_to_check
               |> List.map ~f:Relative_path.to_absolute
               |> List.map ~f:Hh_json.string_ ) );
         ]);
    exit 0
  ) else
    (* In case we saw that any hot decls had become invalid, we have to remove them.
  Note: we don't need to do a full "redecl" of them since their fanout has
  already been encompassed by to_recheck. *)
    let names_to_undecl =
      Relative_path.Set.fold
        to_undecl
        ~init:FileInfo.empty_names
        ~f:(fun file acc ->
          match Naming_table.get_file_info old_naming_table file with
          | None -> acc
          | Some info ->
            let names = FileInfo.simplify info in
            FileInfo.merge_names acc names)
    in
    let ctx = Provider_utils.ctx_from_server_env env in
    Decl_redecl_service.remove_defs
      ctx
      names_to_undecl
      SMap.empty
      ~collect_garbage:false;

    let env = { env with changed_files = dirty_files_changed_hash } in
    let files_to_check =
      if
        not
          genv.ServerEnv.local_config
            .ServerLocalConfig.enable_type_check_filter_files
      then
        files_to_check
      else
        Relative_path.Set.elements
        @@ ServerCheckUtils.user_filter_type_check_files
             ~to_recheck:(Relative_path.Set.of_list files_to_check)
             ~reparsed:
               (Relative_path.Set.union
                  dirty_files_unchanged_hash
                  dirty_files_changed_hash)
             ~is_ide_file:(fun _ -> false)
    in
    let files_to_check =
      if genv.ServerEnv.local_config.ServerLocalConfig.re_worker then
        []
      else
        files_to_check
    in
    let init_telemetry =
      telemetry
      |> Telemetry.int_
           ~key:"dirty_master_files_unchanged_hash"
           ~value:(Relative_path.Set.cardinal dirty_master_files_unchanged_hash)
      |> Telemetry.int_
           ~key:"dirty_master_files_changed_hash"
           ~value:(Relative_path.Set.cardinal dirty_master_files_changed_hash)
      |> Telemetry.int_
           ~key:"dirty_local_files_unchanged_hash"
           ~value:(Relative_path.Set.cardinal dirty_local_files_unchanged_hash)
      |> Telemetry.int_
           ~key:"dirty_local_files_changed_hash"
           ~value:(Relative_path.Set.cardinal dirty_local_files_changed_hash)
      |> Telemetry.int_
           ~key:"dirty_files_unchanged_hash"
           ~value:(Relative_path.Set.cardinal dirty_files_unchanged_hash)
      |> Telemetry.int_
           ~key:"dirty_files_changed_hash"
           ~value:(Relative_path.Set.cardinal dirty_files_changed_hash)
      |> Telemetry.int_
           ~key:"to_recheck"
           ~value:(Relative_path.Set.cardinal to_recheck)
    in
    let result =
      type_check
        genv
        env
        files_to_check
        init_telemetry
        t
        ~profile_label:"type check dirty files"
        ~profiling
    in
    HackEventLogger.type_check_dirty
      ~start_t
      ~dirty_count:(Relative_path.Set.cardinal dirty_files_changed_hash)
      ~recheck_count:(Relative_path.Set.cardinal to_recheck);
    Hh_logger.log
      "ServerInit type_check_dirty count: %d. recheck count: %d"
      (Relative_path.Set.cardinal dirty_files_changed_hash)
      (Relative_path.Set.cardinal to_recheck);
    result

let get_updates_exn ~(genv : ServerEnv.genv) ~(root : Path.t) :
    Relative_path.Set.t =
  let start_t = Unix.gettimeofday () in
  Hh_logger.log "Getting files changed while parsing...";
  let files_changed_while_parsing =
    ServerNotifierTypes.(
      genv.wait_until_ready ();
      match genv.notifier_async () with
      | Notifier_state_enter _
      | Notifier_state_leave _
      | Notifier_unavailable ->
        Relative_path.Set.empty
      | Notifier_synchronous_changes updates
      | Notifier_async_changes updates ->
        let root = Path.to_string root in
        let filter p = string_starts_with p root && FindUtils.file_filter p in
        SSet.filter updates ~f:filter
        |> Relative_path.relativize_set Relative_path.Root)
  in
  ignore
    ( Hh_logger.log_duration
        "Finished getting files changed while parsing"
        start_t
      : float );
  HackEventLogger.changed_while_parsing_end start_t;
  files_changed_while_parsing

let initialize_naming_table
    (progress_message : string)
    ?(fnl : Relative_path.t list option = None)
    ?(do_naming : bool = false)
    (genv : ServerEnv.genv)
    (env : ServerEnv.env)
    (profiling : CgroupProfiler.Profiling.t) : ServerEnv.env * float =
  SharedMem.cleanup_sqlite ();
  ServerProgress.send_progress "%s" progress_message;
  let (get_next, count, t) =
    match fnl with
    | Some fnl ->
      ( MultiWorker.next genv.workers fnl,
        Some (List.length fnl),
        Unix.gettimeofday () )
    | None ->
      let (get_next, t) = indexing genv in
      (get_next, None, t)
  in
  (* The full_fidelity_parser currently works better in both memory and time
     with a full parse rather than parsing decl asts and then parsing full ones *)
  let lazy_parse = not genv.local_config.SLC.use_full_fidelity_parser in
  (* full init - too many files to trace all of them *)
  let trace = false in
  let (env, t) =
    parsing
      ~lazy_parse
      genv
      env
      ~get_next
      ?count
      t
      ~trace
      ~profile_label:"parsing"
      ~profiling
  in
  if not do_naming then
    (env, t)
  else
    let ctx = Provider_utils.ctx_from_server_env env in
    let t =
      update_files
        genv
        env.naming_table
        ctx
        t
        ~profile_label:"update file deps"
        ~profiling
    in
    naming env t ~profile_label:"naming" ~profiling

let write_symbol_info_init
    (genv : ServerEnv.genv)
    (env : ServerEnv.env)
    (profiling : CgroupProfiler.Profiling.t) : ServerEnv.env * float =
  let (env, t) =
    initialize_naming_table
      "write symbol info initialization"
      genv
      env
      profiling
  in
  let ctx = Provider_utils.ctx_from_server_env env in
  let t =
    update_files
      genv
      env.naming_table
      ctx
      t
      ~profile_label:"update file deps"
      ~profiling
  in
  let (env, t) = naming env t ~profile_label:"naming" ~profiling in
  let index_paths = env.swriteopt.symbol_write_index_paths in
  let index_paths_file = env.swriteopt.symbol_write_index_paths_file in
  let files =
    if List.length index_paths > 0 || Option.is_some index_paths_file then
      List.concat
        [
          Option.value_map index_paths_file ~default:[] ~f:In_channel.read_lines
          |> List.map ~f:Relative_path.storage_of_string;
          index_paths
          |> List.filter ~f:Sys.file_exists
          |> List.map ~f:(fun path -> Relative_path.from_root ~suffix:path);
        ]
    else
      let fast = Naming_table.to_fast env.naming_table in
      let failed_parsing = Errors.get_failed_files env.errorl Errors.Parsing in
      let fast =
        Relative_path.Set.fold
          failed_parsing
          ~f:(fun x m -> Relative_path.Map.remove m x)
          ~init:fast
      in
      let exclude_hhi = not env.swriteopt.symbol_write_include_hhi in
      let ignore_paths = env.swriteopt.symbol_write_ignore_paths in
      Relative_path.Map.fold fast ~init:[] ~f:(fun path _ acc ->
          match Naming_table.get_file_info env.naming_table path with
          | None -> acc
          | Some _ ->
            if
              (exclude_hhi && Relative_path.is_hhi (Relative_path.prefix path))
              || List.exists ignore_paths ~f:(fun ignore ->
                     String.equal (Relative_path.S.to_string path) ignore)
            then
              acc
            else
              path :: acc)
  in
  match env.swriteopt.symbol_write_index_paths_file_output with
  | Some output ->
    List.map ~f:Relative_path.storage_to_string files
    |> Out_channel.write_lines output;
    (env, t)
  | None ->
    let out_dir =
      match ServerArgs.write_symbol_info genv.options with
      | None -> failwith "No write directory specified for --write-symbol-info"
      | Some s -> s
    in
    (* Ensure we are writing to fresh files *)
    let is_invalid =
      try
        if not (Sys.is_directory out_dir) then
          true
        else
          Array.length (Sys.readdir out_dir) > 0
      with _ ->
        Sys_utils.mkdir_p out_dir;
        false
    in
    if is_invalid then failwith "JSON write directory is invalid or non-empty";

    Hh_logger.log "Writing JSON to: %s" out_dir;

    let ctx = Provider_utils.ctx_from_server_env env in
    let root_path = env.swriteopt.symbol_write_root_path in
    let hhi_path = env.swriteopt.symbol_write_hhi_path in
    (* TODO(milliechen): log memory for this step *)
    Symbol_info_writer.go genv.workers ctx out_dir root_path hhi_path files;

    (env, t)

(* If we fail to load a saved state, fall back to typechecking everything *)
let full_init
    (genv : ServerEnv.genv)
    (env : ServerEnv.env)
    (profiling : CgroupProfiler.Profiling.t) : ServerEnv.env * float =
  let init_telemetry =
    Telemetry.create ()
    |> Telemetry.float_ ~key:"start_time" ~value:(Unix.gettimeofday ())
    |> Telemetry.string_ ~key:"reason" ~value:"lazy_full_init"
  in
  let is_check_mode = ServerArgs.check_mode genv.options in
  let run () =
    let (env, t) =
      initialize_naming_table
        ~do_naming:true
        "full initialization"
        genv
        env
        profiling
    in
    if not is_check_mode then
      SearchServiceRunner.update_fileinfo_map
        env.naming_table
        ~source:SearchUtils.Init;
    let fast = Naming_table.to_fast env.naming_table in
    let failed_parsing = Errors.get_failed_files env.errorl Errors.Parsing in
    let fast =
      Relative_path.Set.fold
        failed_parsing
        ~f:(fun x m -> Relative_path.Map.remove m x)
        ~init:fast
    in
    let fnl = Relative_path.Map.keys fast in
    let env =
      if is_check_mode then
        start_delegate_if_needed env genv (List.length fnl) env.errorl
      else
        env
    in
    type_check
      genv
      env
      fnl
      init_telemetry
      t
      ~profile_label:"type check"
      ~profiling
  in
  let run_experiment () =
    let ctx = Provider_utils.ctx_from_server_env env in
    let t_full_init = Unix.gettimeofday () in
    let fast = Direct_decl_service.go ctx genv.workers (fst (indexing genv)) in
    let t = Hh_logger.log_duration "parsing decl" t_full_init in
    let naming_table = Naming_table.update_many env.naming_table fast in
    let t = Hh_logger.log_duration "updating naming table" t in
    let env = { env with naming_table } in
    let t =
      update_files
        genv
        env.naming_table
        ctx
        t
        ~profile_label:"update files"
        ~profiling
    in
    let (env, t) = naming env t ~profile_label:"naming" ~profiling in
    let fnl = Relative_path.Map.keys fast in
    if not is_check_mode then
      SearchServiceRunner.update_fileinfo_map
        env.naming_table
        ~source:SearchUtils.Init;
    let type_check_result =
      type_check
        genv
        env
        fnl
        init_telemetry
        t
        ~profile_label:"type check"
        ~profiling
    in
    Hh_logger.log_duration "full init" t_full_init |> ignore;
    type_check_result
  in
  if
    GlobalOptions.tco_use_direct_decl_parser
      (ServerConfig.parser_options genv.config)
  then (
    Hh_logger.log "full init experiment";
    run_experiment ()
  ) else (
    Hh_logger.log "full init";
    run ()
  )

let parse_only_init
    (genv : ServerEnv.genv)
    (env : ServerEnv.env)
    (profiling : CgroupProfiler.Profiling.t) : ServerEnv.env * float =
  initialize_naming_table "parse-only initialization" genv env profiling

let get_mergebase (mergebase_future : Hg.hg_rev option Future.t) :
    Hg.hg_rev option =
  match Future.get mergebase_future with
  | Ok mergebase ->
    let () =
      match mergebase with
      | Some mergebase -> Hh_logger.log "Got mergebase hash: %s" mergebase
      | None -> Hh_logger.log "No mergebase hash"
    in
    mergebase
  | Error error ->
    Hh_logger.log
      "Getting mergebase hash failed: %s"
      (Future.error_to_string error);
    None

let post_saved_state_initialization
    ~(genv : ServerEnv.genv)
    ~(env : ServerEnv.env)
    ~(state_result : loaded_info * Relative_path.Set.t)
    (profiling : CgroupProfiler.Profiling.t) : ServerEnv.env * float =
  let ((loaded_info : ServerInitTypes.loaded_info), changed_while_parsing) =
    state_result
  in
  let trace = genv.local_config.SLC.trace_parsing in
  let {
    naming_table_fallback_fn;
    dirty_naming_files;
    dirty_local_files;
    dirty_master_files;
    old_naming_table;
    mergebase_rev;
    mergebase;
    old_errors;
    deptable_fn;
    deptable_is_64bit;
    naming_table_fn = _;
    corresponding_rev = _;
    state_distance = _;
    naming_table_manifold_path;
  } =
    loaded_info
  in
  if genv.local_config.SLC.hg_aware then
    if ServerArgs.is_using_precomputed_saved_state genv.options then begin
      HackEventLogger.tried_to_be_hg_aware_with_precomputed_saved_state_warning
        ();
      Hh_logger.log
        "Warning: disabling restart on rebase (server was started with precomputed saved-state)"
    end else
      Option.iter mergebase_rev ~f:ServerRevisionTracker.initialize;
  let env =
    {
      env with
      init_env =
        {
          env.init_env with
          mergebase = get_mergebase mergebase;
          naming_table_manifold_path;
        };
      deps_mode =
        ( if deptable_is_64bit then
          match ServerArgs.save_64bit genv.options with
          | Some new_edges_dir ->
            Typing_deps_mode.SaveCustomMode
              { graph = Some deptable_fn; new_edges_dir }
          | None -> Typing_deps_mode.CustomMode (Some deptable_fn)
        else
          Typing_deps_mode.SQLiteMode );
    }
  in

  let (decl_and_typing_error_files, naming_and_parsing_error_files) =
    SaveStateService.partition_error_files_tf
      old_errors
      [Errors.Decl; Errors.Typing]
  in
  let (_old_parsing_phase, old_parsing_error_files) =
    match
      List.find old_errors ~f:(fun (phase, _files) ->
          match phase with
          | Errors.Parsing -> true
          | _ -> false)
    with
    | Some (a, b) -> (a, b)
    | None -> (Errors.Parsing, Relative_path.Set.empty)
  in
  Hh_logger.log
    "Number of files with Decl and Typing errors: %d"
    (Relative_path.Set.cardinal decl_and_typing_error_files);

  Hh_logger.log
    "Number of files with Naming and Parsing errors: %d"
    (Relative_path.Set.cardinal naming_and_parsing_error_files);

  let (decl_and_typing_error_files, naming_and_parsing_error_files) =
    SaveStateService.partition_error_files_tf
      old_errors
      [Errors.Decl; Errors.Typing]
  in
  (* Parse and name all dirty files uniformly *)
  let dirty_files =
    List.fold
      ~init:Relative_path.Set.empty
      ~f:Relative_path.Set.union
      [
        naming_and_parsing_error_files;
        dirty_naming_files;
        dirty_master_files;
        dirty_local_files;
      ]
  in
  let t = Unix.gettimeofday () in
  let dirty_files = Relative_path.Set.union dirty_files changed_while_parsing in
  let parsing_files =
    Relative_path.Set.filter dirty_files ~f:FindUtils.path_filter
  in
  ( CgroupProfiler.collect_cgroup_stats ~stage:"remove fixmes" ~profiling
  @@ fun () -> Fixme_provider.remove_batch parsing_files );
  let parsing_files_list = Relative_path.Set.elements parsing_files in
  (* Parse dirty files only *)
  let max_size =
    if genv.local_config.ServerLocalConfig.small_buckets_for_dirty_names then
      Some 1
    else
      None
  in
  let next = MultiWorker.next genv.workers parsing_files_list ?max_size in
  let (env, t) =
    parsing
      genv
      env
      ~lazy_parse:true
      ~get_next:next
      ~count:(List.length parsing_files_list)
      t
      ~trace
      ~profile_label:"parse dirty files"
      ~profiling
  in
  SearchServiceRunner.update_fileinfo_map
    env.naming_table
    ~source:SearchUtils.TypeChecker;
  let ctx = Provider_utils.ctx_from_server_env env in
  let t =
    update_files
      genv
      env.naming_table
      ctx
      t
      ~profile_label:"update file deps"
      ~profiling
  in
  let t =
    naming_from_saved_state
      ctx
      old_naming_table
      parsing_files
      naming_table_fallback_fn
      t
      ~profiling
  in
  (* Do global naming on all dirty files *)
  let (env, t) = naming env t ~profile_label:"naming dirty files" ~profiling in

  (* Add all files from fast to the files_info object *)
  let fast = Naming_table.to_fast env.naming_table in
  let failed_parsing = Errors.get_failed_files env.errorl Errors.Parsing in
  let fast =
    Relative_path.Set.fold
      failed_parsing
      ~f:(fun x m -> Relative_path.Map.remove m x)
      ~init:fast
  in
  let env =
    {
      env with
      disk_needs_parsing =
        Relative_path.Set.union env.disk_needs_parsing changed_while_parsing;
    }
  in
  (* Separate the dirty files from the files whose decl only changed *)
  (* Here, for each dirty file, we compare its hash to the one saved
     in the saved state. If the hashes are the same, then the declarations
     on the file have not changed and we only need to retypecheck that file,
     not all of its dependencies.
     We call these files "similar" to their previous versions. *)
  let partition_similar dirty_files =
    Relative_path.Set.partition
      (fun f ->
        let info1 = Naming_table.get_file_info old_naming_table f in
        let info2 = Naming_table.get_file_info env.naming_table f in
        match (info1, info2) with
        | (Some x, Some y) ->
          (match (x.FileInfo.hash, y.FileInfo.hash) with
          | (Some x, Some y) -> Int64.equal x y
          | _ -> false)
        | _ -> false)
      dirty_files
  in
  let (dirty_master_files_unchanged_hash, dirty_master_files_changed_hash) =
    partition_similar dirty_master_files
  in
  let (dirty_local_files_unchanged_hash, dirty_local_files_changed_hash) =
    partition_similar dirty_local_files
  in
  let env =
    {
      env with
      naming_table = Naming_table.combine old_naming_table env.naming_table;
      (* The only reason old_parsing_error_files are added to disk_needs_parsing
                 here is because of an issue that seems to be already tracked in T30786759 *)
      disk_needs_parsing = old_parsing_error_files;
      needs_recheck =
        Relative_path.Set.union env.needs_recheck decl_and_typing_error_files;
    }
  in
  (* Update the fileinfo object's dependencies now that we have full fast *)
  let t =
    update_files
      genv
      env.naming_table
      ctx
      t
      ~profile_label:"update files again"
      ~profiling
  in
  type_check_dirty
    genv
    env
    old_naming_table
    fast
    ~dirty_master_files_unchanged_hash
    ~dirty_master_files_changed_hash
    ~dirty_local_files_unchanged_hash
    ~dirty_local_files_changed_hash
    t
    profiling

let saved_state_init
    ~(load_state_approach : load_state_approach)
    (genv : ServerEnv.genv)
    (env : ServerEnv.env)
    (root : Path.t)
    (profiling : CgroupProfiler.Profiling.t) :
    ( (ServerEnv.env * float) * (loaded_info * Relative_path.Set.t),
      load_state_error )
    result =
  let t = Unix.gettimeofday () in
  let attempt_fix = genv.local_config.SLC.attempt_fix_credentials in
  let () =
    match Security.check_credentials ~attempt_fix with
    | Ok success ->
      HackEventLogger.credentials_check_end
        (Printf.sprintf "saved_state_init: %s" (Security.show_success success))
        t
    | Error error ->
      let kind = Security.to_error_kind_string error in
      let message = Security.to_error_message_string error in
      Hh_logger.log "Error kind: %s\nError message: %s" kind message;
      HackEventLogger.credentials_check_failure
        (Printf.sprintf "saved_state_init: [%s]" kind)
        t
  in

  ServerProgress.send_progress "loading saved state";

  let ctx = Provider_utils.ctx_from_server_env env in
  (* A historical quirk: we allowed the timeout once while downloading+loading *)
  (* saved-state, and then once again while waiting to get dirty files from hg *)
  let timeout = 2 * genv.local_config.SLC.load_state_script_timeout in
  (* following function will be run under the timeout *)
  let do_ (_id : Timeout.t) : (loaded_info, load_state_error) result =
    let state_result =
      CgroupProfiler.collect_cgroup_stats ~profiling ~stage:"load saved state"
      @@ fun () ->
      match load_state_approach with
      | Precomputed info ->
        Ok (use_precomputed_state_exn genv ctx info profiling)
      | Load_state_natively -> download_and_load_state_exn ~genv ~ctx ~root
    in
    state_result
  in
  let t = Unix.gettimeofday () in
  let state_result =
    try
      match
        Timeout.with_timeout
          ~timeout
          ~do_
          ~on_timeout:(fun (_ : Timeout.timings) -> Error Load_state_timeout)
      with
      | Error error -> Error error
      | Ok loaded_info ->
        let changed_while_parsing = get_updates_exn ~genv ~root in
        Ok (loaded_info, changed_while_parsing)
    with exn ->
      let stack = Utils.Callstack (Printexc.get_backtrace ()) in
      Error (Load_state_unhandled_exception { exn; stack })
  in
  HackEventLogger.saved_state_download_and_load_done
    ~load_state_approach:(show_load_state_approach load_state_approach)
    ~success:(Result.is_ok state_result)
    ~state_result:
      (match state_result with
      | Error _ -> None
      | Ok (i, _) -> Some (show_loaded_info i))
    ~load_state_natively_64bit:
      genv.local_config.ServerLocalConfig.load_state_natively_64bit
    t;
  match state_result with
  | Error err -> Error err
  | Ok state_result ->
    ServerProgress.send_progress "loading saved state succeeded";
    let (env, t) =
      post_saved_state_initialization ~state_result ~env ~genv profiling
    in
    Ok ((env, t), state_result)
