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

open Hh_prelude
open GlobalOptions
open Result.Export
open Reordered_argument_collections
open SearchServiceRunner
open ServerCheckUtils
open ServerEnv
open ServerInitCommon
open ServerInitTypes
open String_utils
module DepSet = Typing_deps.DepSet
module Dep = Typing_deps.Dep
module SLC = ServerLocalConfig

type deptable =
  | SQLiteDeptable of string
  | CustomDeptable of string

let deptable_with_filename (genv : genv) ~(is_64bit : bool) (fn : string) :
    deptable =
  match ServerArgs.with_dep_graph_v2 genv.options with
  | Some fn -> CustomDeptable fn
  | None ->
    if is_64bit then
      CustomDeptable fn
    else
      SQLiteDeptable fn

let lock_and_load_deptable
    (deptable : deptable) ~(ignore_hh_version : bool) ~(fail_if_missing : bool)
    : unit =
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
    Hh_logger.log "Loading custom dependency graph from %s" fn;
    (* We force load the dependency graph here early, because we
     * want to catch loading errors early, and "properly" handle them *)
    Typing_deps.(set_mode @@ CustomMode fn);
    (match Typing_deps.force_load_custom_dep_graph () with
    | Ok () -> ()
    | Error msg -> failwith msg)

let merge_saved_state_futures
    (genv : genv)
    (ctx : Provider_context.t)
    (dependency_table_saved_state_future :
      (State_loader.native_load_result, State_loader.error) result Future.t)
    (naming_table_saved_state_future :
      ( Saved_state_loader.Naming_table_info.t Saved_state_loader.load_result
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
        "Unhandled error from State_loader: %s"
        (Future.show_error error);
      let e = Exception.wrap_unraised (Future.error_to_exn error) in
      let exn = Exception.to_exn e in
      let stack = Utils.Callstack (Exception.get_backtrace_string e) in
      Error (Load_state_unhandled_exception { exn; stack })
    | Ok (Error error) -> Error (Load_state_loader_failure error)
    | Ok (Ok result) ->
      let (downloaded_naming_table_path, dirty_naming_files) =
        match naming_table_saved_state_result with
        | Ok (Ok None) -> (None, [])
        | Ok
            (Ok
              (Some { Saved_state_loader.saved_state_info; changed_files; _ }))
          ->
          let (_ : float) =
            Hh_logger.log_duration "Finished downloading naming table." t
          in
          let path =
            saved_state_info
              .Saved_state_loader.Naming_table_info.naming_table_path
          in
          (Some (Path.to_string path), changed_files)
        | Ok (Error err) ->
          Hh_logger.warn "Failed to download naming table saved state: %s" err;
          (None, [])
        | Error error ->
          Hh_logger.warn
            "Failed to download the naming table saved state: %s"
            (Future.error_to_string error);
          (None, [])
      in
      let ignore_hh_version =
        ServerArgs.ignore_hh_version genv.ServerEnv.options
      in
      let fail_if_missing = not genv.local_config.SLC.can_skip_deptable in
      let deptable =
        deptable_with_filename
          genv
          ~is_64bit:result.State_loader.deptable_is_64bit
          result.State_loader.deptable_fn
      in
      lock_and_load_deptable deptable ~ignore_hh_version ~fail_if_missing;
      let load_decls = genv.local_config.SLC.load_decls_from_saved_state in
      let naming_table_fallback_path =
        get_naming_table_fallback_path genv downloaded_naming_table_path
      in
      let (old_naming_table, old_errors) =
        SaveStateService.load_saved_state
          ctx
          result.State_loader.saved_state_fn
          ~naming_table_fallback_path
          ~load_decls
      in
      let t = Unix.time () in
      (match result.State_loader.dirty_files |> Future.get ~timeout:200 with
      | Error error -> Error (Load_state_dirty_files_failure error)
      | Ok (dirty_master_files, dirty_local_files) ->
        let () = HackEventLogger.state_loader_dirty_files t in
        let dirty_naming_files = Relative_path.Set.of_list dirty_naming_files in
        let dirty_master_files = Relative_path.Set.of_list dirty_master_files in
        let dirty_local_files = Relative_path.Set.of_list dirty_local_files in
        Ok
          {
            saved_state_fn = result.State_loader.saved_state_fn;
            deptable_fn = result.State_loader.deptable_fn;
            naming_table_fn = naming_table_fallback_path;
            corresponding_rev = result.State_loader.corresponding_rev;
            mergebase_rev = result.State_loader.mergebase_rev;
            mergebase = result.State_loader.mergebase;
            dirty_naming_files;
            dirty_master_files;
            dirty_local_files;
            old_naming_table;
            old_errors;
            state_distance = Some result.State_loader.state_distance;
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
    ~(use_canary : bool)
    ~(target : ServerMonitorUtils.target_saved_state option)
    ~(genv : ServerEnv.genv)
    ~(ctx : Provider_context.t)
    ~(root : Path.t) : (loaded_info, load_state_error) result =
  let open ServerMonitorUtils in
  let saved_state_handle =
    match target with
    | None -> None
    | Some
        { saved_state_everstore_handle; target_global_rev; watchman_mergebase }
      ->
      Some
        {
          State_loader.saved_state_everstore_handle;
          saved_state_for_rev = Hg.Global_rev target_global_rev;
          watchman_mergebase;
        }
  in
  let ignore_hh_version = ServerArgs.ignore_hh_version genv.options in
  let ignore_hhconfig = ServerArgs.saved_state_ignore_hhconfig genv.options in
  let use_prechecked_files =
    ServerPrecheckedFiles.should_use genv.options genv.local_config
  in
  let naming_table_saved_state_future =
    (* TODO(hverr): Support manifold naming table in 64-bit mode *)
    if
      ServerLocalConfig.(
        genv.local_config.enable_naming_table_fallback
        && not genv.local_config.load_state_natively_64bit)
    then begin
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
      (State_loader.native_load_result, State_loader.error) result Future.t =
    State_loader.mk_state_future
      ~config:genv.local_config.SLC.state_loader_timeouts
      ~use_canary
      ~load_64bit:genv.local_config.SLC.load_state_natively_64bit
      ?saved_state_handle
      ~config_hash:(ServerConfig.config_hash genv.config)
      root
      ~ignore_hh_version
      ~ignore_hhconfig
      ~use_prechecked_files
  in
  merge_saved_state_futures
    genv
    ctx
    dependency_table_saved_state_future
    naming_table_saved_state_future

let use_precomputed_state_exn
    (genv : ServerEnv.genv)
    (ctx : Provider_context.t)
    (info : ServerArgs.saved_state_target_info) : loaded_info =
  let {
    ServerArgs.saved_state_fn;
    corresponding_base_revision;
    deptable_fn;
    changes;
    naming_changes;
    prechecked_changes;
  } =
    info
  in
  let ignore_hh_version = ServerArgs.ignore_hh_version genv.ServerEnv.options in
  let fail_if_missing = not genv.local_config.SLC.can_skip_deptable in
  let deptable = deptable_with_filename genv ~is_64bit:false deptable_fn in
  lock_and_load_deptable deptable ~ignore_hh_version ~fail_if_missing;
  let changes = Relative_path.set_of_list changes in
  let naming_changes = Relative_path.set_of_list naming_changes in
  let prechecked_changes = Relative_path.set_of_list prechecked_changes in
  let load_decls = genv.local_config.SLC.load_decls_from_saved_state in
  let naming_table_fallback_path = get_naming_table_fallback_path genv None in
  let (old_naming_table, old_errors) =
    SaveStateService.load_saved_state
      ctx
      saved_state_fn
      ~naming_table_fallback_path
      ~load_decls
  in
  {
    saved_state_fn;
    deptable_fn;
    naming_table_fn = naming_table_fallback_path;
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
  }

(* Run naming from a fast generated from saved state.
 * No errors are generated because we assume the fast is directly from
 * a clean state.
 *)
let naming_with_fast
    (ctx : Provider_context.t)
    (fast : FileInfo.names Relative_path.Map.t)
    (t : float) : float =
  Relative_path.Map.iter fast ~f:(fun k info ->
      let {
        FileInfo.n_classes = classes;
        n_record_defs = record_defs;
        n_types = typedefs;
        n_funs = funs;
        n_consts = consts;
      } =
        info
      in
      Naming_global.ndecl_file_fast
        ctx
        k
        ~funs
        ~classes
        ~record_defs
        ~typedefs
        ~consts);
  HackEventLogger.fast_naming_end t;
  let hs = SharedMem.heap_size () in
  Hh_logger.log "Heap size: %d" hs;
  Hh_logger.log_duration "Naming fast" t

let naming_from_saved_state
    (ctx : Provider_context.t)
    (old_naming_table : Naming_table.t)
    (parsing_files : Relative_path.Set.t)
    (naming_table_fn : string option)
    (t : float) : float =
  (* If we're falling back to SQLite we don't need to explicitly do a naming
     pass, but if we're not then we do. *)
  match naming_table_fn with
  | Some _ ->
    (* Set the SQLite fallback path for the reverse naming table, then block out all entries in
      any dirty files to make sure we properly handle file deletes. *)
    Relative_path.Set.iter parsing_files (fun k ->
        match Naming_table.get_file_info old_naming_table k with
        | None ->
          (* If we can't find the file in [old_naming_table] we don't consider that an error, since
           * it could be a new file that was added. *)
          ()
        | Some v ->
          let backend = Provider_context.get_backend ctx in
          Naming_provider.remove_type_batch
            backend
            (v.FileInfo.classes |> List.map ~f:snd |> SSet.of_list);
          Naming_provider.remove_type_batch
            backend
            (v.FileInfo.typedefs |> List.map ~f:snd |> SSet.of_list);
          Naming_provider.remove_type_batch
            backend
            (v.FileInfo.record_defs |> List.map ~f:snd |> SSet.of_list);
          Naming_provider.remove_fun_batch
            backend
            (v.FileInfo.funs |> List.map ~f:snd |> SSet.of_list);
          Naming_provider.remove_const_batch
            backend
            (v.FileInfo.consts |> List.map ~f:snd |> SSet.of_list));
    Unix.gettimeofday ()
  | None ->
    (* Name all the files from the old fast (except the new ones we parsed) *)
    let old_hack_names =
      Naming_table.filter old_naming_table (fun k _v ->
          not (Relative_path.Set.mem parsing_files k))
    in
    naming_with_fast ctx (Naming_table.to_fast old_hack_names) t

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
          Option.merge dirty_old_fast dirty_fast FileInfo.merge_names
        in
        match fast with
        | Some fast -> Relative_path.Map.add acc ~key:fn ~data:fast
        | None -> acc
      end
    ~init:Relative_path.Map.empty

let names_to_deps (names : FileInfo.names) : DepSet.t =
  let { FileInfo.n_funs; n_classes; n_record_defs; n_types; n_consts } =
    names
  in
  let add_deps_of_sset dep_ctor sset depset =
    SSet.fold sset ~init:depset ~f:(fun n acc ->
        DepSet.add acc (Dep.make (dep_ctor n)))
  in
  let deps = add_deps_of_sset (fun n -> Dep.Fun n) n_funs (DepSet.make ()) in
  let deps = add_deps_of_sset (fun n -> Dep.FunName n) n_funs deps in
  let deps = add_deps_of_sset (fun n -> Dep.Class n) n_classes deps in
  let deps = add_deps_of_sset (fun n -> Dep.RecordDef n) n_record_defs deps in
  let deps = add_deps_of_sset (fun n -> Dep.Class n) n_types deps in
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
        | Some info -> Relative_path.Map.add acc path info
        | None -> acc)
  in
  let get_classes path =
    let old_names =
      Naming_table.get_file_info old_naming_table path
      |> Option.map ~f:FileInfo.simplify
    in
    let new_names = Relative_path.Map.find_opt new_fast path in
    let classes_from_names x = x.FileInfo.n_classes in
    let old_classes = Option.map old_names classes_from_names in
    let new_classes = Option.map new_names classes_from_names in
    Option.merge old_classes new_classes SSet.union
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
      ~conservative_redecl:false
      ~bucket_size
      ctx
      genv.workers
      get_classes
      ~previously_oldified_defs:dirty_names
      ~defs:fast
  in
  Decl_redecl_service.remove_old_defs ctx ~bucket_size genv.workers dirty_names;
  let deps = Typing_deps.add_all_deps to_redecl in
  let deps = Typing_deps.DepSet.union deps to_recheck in
  let files_to_undecl = Typing_deps.Files.get_files to_redecl in
  let files_to_recheck = Typing_deps.Files.get_files deps in
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
    (t : float) : ServerEnv.env * float =
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
  let master_deps = names dirty_master_files_changed_hash |> names_to_deps in
  let local_deps = names dirty_local_files_changed_hash |> names_to_deps in
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
          let deps = Typing_deps.add_all_deps local_deps in
          (Relative_path.Set.empty, Typing_deps.Files.get_files deps)
      in
      ( ServerPrecheckedFiles.set
          env
          (Initial_typechecking
             {
               rechecked_files = Relative_path.Set.empty;
               dirty_local_deps = local_deps;
               dirty_master_deps = master_deps;
               clean_local_deps = Typing_deps.(DepSet.make ());
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
          let deps = Typing_deps.add_all_deps deps in
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
    let result = type_check genv env files_to_check init_telemetry t in
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
    (env : ServerEnv.env) : ServerEnv.env * float =
  SharedMem.cleanup_sqlite ();
  ServerProgress.send_progress_to_monitor "%s" progress_message;
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
  let (env, t) = parsing ~lazy_parse genv env ~get_next ?count t ~trace in
  if not do_naming then
    (env, t)
  else
    let t = update_files genv env.naming_table t in
    naming env t

let load_naming_table (genv : ServerEnv.genv) (env : ServerEnv.env) :
    ServerEnv.env * float =
  let ignore_hh_version = ServerArgs.ignore_hh_version genv.options in
  let loader_future =
    State_loader_futures.load
      ~watchman_opts:
        Saved_state_loader.Watchman_options.
          {
            root = Path.make Relative_path.(path_of_prefix Root);
            sockname = None;
          }
      ~ignore_hh_version
      ~saved_state_type:Saved_state_loader.Naming_table
  in
  match
    State_loader_futures.wait_for_finish_with_debug_details loader_future
  with
  | Ok { Saved_state_loader.saved_state_info; changed_files; _ } ->
    let { Saved_state_loader.Naming_table_info.naming_table_path } =
      saved_state_info
    in
    let ctx = Provider_utils.ctx_from_server_env env in
    let naming_table_path = Path.to_string naming_table_path in
    let naming_table = Naming_table.load_from_sqlite ctx naming_table_path in
    let (env, t) =
      initialize_naming_table
        ~fnl:(Some changed_files)
        ~do_naming:true
        "full initialization (with loaded naming table)"
        genv
        env
    in
    let t =
      naming_from_saved_state
        ctx
        env.naming_table
        (Relative_path.set_of_list changed_files)
        (Some naming_table_path)
        t
    in
    ( {
        env with
        naming_table = Naming_table.combine naming_table env.naming_table;
      },
      t )
  | Error e ->
    Hh_logger.log "Failed to load naming table: %s" e;
    initialize_naming_table
      ~do_naming:true
      "full initialization (failed to load naming table)"
      genv
      env

let write_symbol_info_init (genv : ServerEnv.genv) (env : ServerEnv.env) :
    ServerEnv.env * float =
  let out_dir =
    match ServerArgs.write_symbol_info genv.options with
    | None -> failwith "No write directory specified for --write-symbol-info"
    | Some s -> s
  in
  let (env, t) =
    initialize_naming_table "write symbol info initialization" genv env
  in
  let t = update_files genv env.naming_table t in
  let (env, t) = naming env t in
  let index_paths = env.swriteopt.symbol_write_index_paths in
  let files =
    if List.length index_paths > 0 then
      List.map index_paths (fun path -> Relative_path.from_root ~suffix:path)
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
              || List.exists ignore_paths (fun ignore ->
                     String.equal (Relative_path.S.to_string path) ignore)
            then
              acc
            else
              path :: acc)
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
  Symbol_info_writer.go genv.workers ctx out_dir root_path hhi_path files;

  (env, t)

(* If we fail to load a saved state, fall back to typechecking everything *)
let full_init (genv : ServerEnv.genv) (env : ServerEnv.env) :
    ServerEnv.env * float =
  let init_telemetry =
    Telemetry.create ()
    |> Telemetry.float_ ~key:"start_time" ~value:(Unix.gettimeofday ())
    |> Telemetry.string_ ~key:"reason" ~value:"lazy_full_init"
  in
  let is_check_mode = ServerArgs.check_mode genv.options in
  let (env, t) =
    if
      not
        genv.ServerEnv.local_config.SLC.remote_type_check
          .SLC.RemoteTypeCheck.load_naming_table_on_full_init
    then
      initialize_naming_table ~do_naming:true "full initialization" genv env
    else
      load_naming_table genv env
  in
  if not is_check_mode then
    SearchServiceRunner.update_fileinfo_map env.naming_table SearchUtils.Init;
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
  type_check genv env fnl init_telemetry t

let parse_only_init (genv : ServerEnv.genv) (env : ServerEnv.env) :
    ServerEnv.env * float =
  initialize_naming_table "parse-only initialization" genv env

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
    ~(state_result : loaded_info * Relative_path.Set.t) : ServerEnv.env * float
    =
  let ((loaded_info : ServerInitTypes.loaded_info), changed_while_parsing) =
    state_result
  in
  let trace = genv.local_config.SLC.trace_parsing in
  let hg_aware = genv.local_config.SLC.hg_aware in
  let {
    naming_table_fn;
    dirty_naming_files;
    dirty_local_files;
    dirty_master_files;
    old_naming_table;
    mergebase_rev;
    mergebase;
    old_errors;
    _;
  } =
    loaded_info
  in
  if hg_aware then Option.iter mergebase_rev ~f:ServerRevisionTracker.initialize;
  let env =
    {
      env with
      init_env = { env.init_env with mergebase = get_mergebase mergebase };
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
  Fixme_provider.remove_batch parsing_files;
  let parsing_files_list = Relative_path.Set.elements parsing_files in
  (* Parse dirty files only *)
  let next = MultiWorker.next genv.workers parsing_files_list in
  let (env, t) =
    parsing
      genv
      env
      ~lazy_parse:true
      ~get_next:next
      ~count:(List.length parsing_files_list)
      t
      ~trace
  in
  SearchServiceRunner.update_fileinfo_map
    env.naming_table
    SearchUtils.TypeChecker;

  let t = update_files genv env.naming_table t in
  let t =
    let ctx = Provider_utils.ctx_from_server_env env in
    naming_from_saved_state ctx old_naming_table parsing_files naming_table_fn t
  in
  (* Do global naming on all dirty files *)
  let (env, t) = naming env t in
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
          | (Some x, Some y) -> OpaqueDigest.equal x y
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
  let t = update_files genv env.naming_table t in
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

let saved_state_init
    ~(load_state_approach : load_state_approach)
    (genv : ServerEnv.genv)
    (env : ServerEnv.env)
    (root : Path.t) :
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

  ServerProgress.send_progress_to_monitor "loading saved state";

  let ctx = Provider_utils.ctx_from_server_env env in
  (* A historical quirk: we allowed the timeout once while downloading+loading *)
  (* saved-state, and then once again while waiting to get dirty files from hg *)
  let timeout = 2 * genv.local_config.SLC.load_state_script_timeout in
  (* following function will be run under the timeout *)
  let do_ (_id : Timeout.t) : (loaded_info, load_state_error) result =
    match load_state_approach with
    | Precomputed info -> Ok (use_precomputed_state_exn genv ctx info)
    | Load_state_natively use_canary ->
      download_and_load_state_exn ~use_canary ~target:None ~genv ~ctx ~root
    | Load_state_natively_with_target target ->
      download_and_load_state_exn
        ~use_canary:false
        ~target:(Some target)
        ~genv
        ~ctx
        ~root
  in
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
        let changed_while_parsing = get_updates_exn genv root in
        Ok (loaded_info, changed_while_parsing)
    with exn ->
      let stack = Utils.Callstack (Printexc.get_backtrace ()) in
      Error (Load_state_unhandled_exception { exn; stack })
  in
  match state_result with
  | Error err -> Error err
  | Ok state_result ->
    ServerProgress.send_progress_to_monitor "loading saved state succeeded";
    let (env, t) = post_saved_state_initialization ~state_result ~env ~genv in
    Ok ((env, t), state_result)
