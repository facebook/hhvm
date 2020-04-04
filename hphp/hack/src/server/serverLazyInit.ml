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

open Core_kernel
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

let lock_and_load_deptable
    (fn : string) ~(ignore_hh_version : bool) ~(fail_if_missing : bool) : unit =
  if String.length fn = 0 && not fail_if_missing then
    Hh_logger.log "The dependency file was not specified - ignoring"
  else
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

(* download_and_load_state_exn does these things:
 * mk_state_future which synchronously downloads ss and kicks of async dirty query
 * lock_and_load_deptable
 * load_saved_state
 * synchronously wait 200s for the async dirty query to finish
 *)
let download_and_load_state_exn
    ~(use_canary : bool)
    ~(target : ServerMonitorUtils.target_saved_state option)
    ~(genv : ServerEnv.genv)
    ~(ctx : Provider_context.t)
    ~(root : Path.t) : (loaded_info, load_state_error) result =
  ServerMonitorUtils.(
    let saved_state_handle =
      match target with
      | None -> None
      | Some
          {
            saved_state_everstore_handle;
            target_global_rev;
            watchman_mergebase;
          } ->
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
    let naming_table_saved_state =
      if genv.local_config.ServerLocalConfig.enable_naming_table_fallback then (
        Hh_logger.log "Starting naming table download.";
        Some
          (State_loader_futures.load
             ~repo:root
             ~ignore_hh_version
             ~saved_state_type:Saved_state_loader.Naming_table)
      ) else
        None
    in
    let state_future :
        (State_loader.native_load_result, State_loader.error) result =
      State_loader.mk_state_future
        ~config:genv.local_config.SLC.state_loader_timeouts
        ~use_canary
        ?saved_state_handle
        ~config_hash:(ServerConfig.config_hash genv.config)
        root
        ~ignore_hh_version
        ~ignore_hhconfig
        ~use_prechecked_files
    in
    match state_future with
    | Error error -> Error (Load_state_loader_failure error)
    | Ok result ->
      let (downloaded_naming_table_path, dirty_naming_files) =
        match naming_table_saved_state with
        | None -> (None, [])
        | Some future ->
          begin
            match State_loader_futures.wait_for_finish future with
            | Ok (naming_table_info, changed_files) ->
              let (_ : float) =
                Hh_logger.log_duration
                  "Finished downloading naming table."
                  (Future.start_t future)
              in
              let path =
                naming_table_info
                  .Saved_state_loader.Naming_table_saved_state_info
                   .naming_table_path
              in
              let changed_files =
                List.map
                  ~f:(fun path ->
                    Relative_path.create_detect_prefix (Path.to_string path))
                  changed_files
              in
              (Some (Path.to_string path), changed_files)
            | Error err ->
              Hh_logger.warn
                "Failed to download naming table saved state: %s"
                err;
              (None, [])
          end
      in
      let ignore_hh_version =
        ServerArgs.ignore_hh_version genv.ServerEnv.options
      in
      let fail_if_missing = not genv.local_config.SLC.can_skip_deptable in
      lock_and_load_deptable
        result.State_loader.deptable_fn
        ~ignore_hh_version
        ~fail_if_missing;
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
        let list_to_set x =
          List.map x Relative_path.from_root |> Relative_path.set_of_list
        in
        let dirty_naming_files = Relative_path.Set.of_list dirty_naming_files in
        let dirty_master_files = list_to_set dirty_master_files in
        let dirty_local_files = list_to_set dirty_local_files in
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
          }))

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
  lock_and_load_deptable deptable_fn ~ignore_hh_version ~fail_if_missing;
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
  && ServerArgs.ai_mode genv.options = None
  && (not (ServerArgs.check_mode genv.options))
  && ServerArgs.save_filename genv.options = None
  && ServerArgs.save_with_spec genv.options = None

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
  let deps = add_deps_of_sset (fun n -> Dep.Fun n) n_funs DepSet.empty in
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
let get_files_to_recheck
    (genv : ServerEnv.genv)
    (env : ServerEnv.env)
    (old_naming_table : Naming_table.t)
    (new_fast : FileInfo.names Relative_path.Map.t)
    (dirty_fast : FileInfo.names Relative_path.Map.t)
    (files_to_redeclare : Relative_path.Set.t) : Relative_path.Set.t =
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
  Typing_deps.get_files deps

(* We start of with a list of files that have changed since the state was
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
    (dirty_master_files : Relative_path.Set.t)
    (dirty_local_files : Relative_path.Set.t)
    (similar_files : Relative_path.Set.t)
    (t : float) : ServerEnv.env * float =
  let dirty_files =
    Relative_path.Set.union dirty_master_files dirty_local_files
  in
  let start_t = Unix.gettimeofday () in
  let dirty_fast = get_dirty_fast old_naming_table new_fast dirty_files in
  let names s =
    Relative_path.Map.fold
      dirty_fast
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
  let master_deps = names dirty_master_files |> names_to_deps in
  let local_deps = names dirty_local_files |> names_to_deps in
  (* Include similar_files in the dirty_fast used to determine which loaded
     declarations to oldify. This is necessary because the positions of
     declarations may have changed, which affects error messages and FIXMEs. *)
  let get_files_to_recheck =
    get_files_to_recheck genv env old_naming_table new_fast
    @@ extend_fast genv dirty_fast env.naming_table similar_files
  in
  let (env, to_recheck) =
    if use_prechecked_files genv then
      (* Start with dirty files and fan-out of local changes only *)
      let to_recheck =
        if genv.local_config.SLC.load_decls_from_saved_state then
          get_files_to_recheck dirty_local_files
        else
          let deps = Typing_deps.add_all_deps local_deps in
          Typing_deps.get_files deps
      in
      ( ServerPrecheckedFiles.set
          env
          (Initial_typechecking
             {
               rechecked_files = Relative_path.Set.empty;
               dirty_local_deps = local_deps;
               dirty_master_deps = master_deps;
               clean_local_deps = Typing_deps.DepSet.empty;
             }),
        to_recheck )
    else
      (* Start with full fan-out immediately *)
      let to_recheck =
        if genv.local_config.SLC.load_decls_from_saved_state then
          get_files_to_recheck dirty_files
        else
          let deps = Typing_deps.DepSet.union master_deps local_deps in
          let deps = Typing_deps.add_all_deps deps in
          Typing_deps.get_files deps
      in
      (env, to_recheck)
  in
  (* We still need to typecheck files whose declarations did not change *)
  let to_recheck = Relative_path.Set.union to_recheck similar_files in
  let fast = extend_fast genv dirty_fast env.naming_table to_recheck in
  let files_to_check = Relative_path.Map.keys fast in
  let env = { env with changed_files = dirty_files } in
  let result = type_check genv env files_to_check t in
  HackEventLogger.type_check_dirty
    ~start_t
    ~dirty_count:(Relative_path.Set.cardinal dirty_files)
    ~recheck_count:(Relative_path.Set.cardinal to_recheck);
  Hh_logger.log
    "ServerInit type_check_dirty count: %d. recheck count: %d"
    (Relative_path.Set.cardinal dirty_files)
    (Relative_path.Set.cardinal to_recheck);
  result

let get_updates_exn ~(genv : ServerEnv.genv) ~(root : Path.t) :
    Relative_path.Set.t =
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
      ~repo:(Path.make Relative_path.(path_of_prefix Root))
      ~ignore_hh_version
      ~saved_state_type:Saved_state_loader.Naming_table
  in
  match State_loader_futures.wait_for_finish loader_future with
  | Ok (info, fnl) ->
    let { Saved_state_loader.Naming_table_saved_state_info.naming_table_path } =
      info
    in
    let ctx = Provider_utils.ctx_from_server_env env in
    let naming_table_path = Path.to_string naming_table_path in
    let naming_table = Naming_table.load_from_sqlite ctx naming_table_path in
    let fnl =
      List.map fnl ~f:(fun path ->
          Relative_path.from_root (Path.to_string path))
    in
    let (env, t) =
      initialize_naming_table
        ~fnl:(Some fnl)
        ~do_naming:true
        "full initialization (with loaded naming table)"
        genv
        env
    in
    let t =
      naming_from_saved_state
        ctx
        env.naming_table
        (Relative_path.set_of_list fnl)
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
  let fast = Naming_table.to_fast env.naming_table in
  let failed_parsing = Errors.get_failed_files env.errorl Errors.Parsing in
  let fast =
    Relative_path.Set.fold
      failed_parsing
      ~f:(fun x m -> Relative_path.Map.remove m x)
      ~init:fast
  in
  let files =
    Relative_path.Map.fold fast ~init:[] ~f:(fun path _ acc ->
        match Naming_table.get_file_info env.naming_table path with
        | None -> acc
        | Some _ -> path :: acc)
  in
  (* ensuring we are writing to fresh files *)
  let dir_exists = (try Sys.is_directory out_dir with _ -> false) in
  if dir_exists then
    failwith "JSON Write Directory Exists"
  else
    Sys_utils.mkdir_p out_dir;

  Hh_logger.log "Writing JSON to: %s" out_dir;

  let ctx = Provider_utils.ctx_from_server_env env in
  let root_path = env.swriteopt.symbol_write_root_path in
  let hhi_path = env.swriteopt.symbol_write_hhi_path in
  Typing_symbol_info_writer.go genv.workers ctx out_dir root_path hhi_path files;

  (env, t)

(* If we fail to load a saved state, fall back to typechecking everything *)
let full_init (genv : ServerEnv.genv) (env : ServerEnv.env) :
    ServerEnv.env * float =
  let is_check_mode = ServerArgs.check_mode genv.options in
  let (env, t) =
    if
      genv.ServerEnv.local_config.SLC.remote_type_check
        .SLC.RemoteTypeCheck.load_naming_table_on_full_init
      = false
    then
      initialize_naming_table ~do_naming:true "full initialization" genv env
    else
      load_naming_table genv env
  in
  let env =
    let remote_enabled =
      genv.ServerEnv.local_config.SLC.remote_type_check
        .SLC.RemoteTypeCheck.enabled
    in
    if remote_enabled && is_check_mode then
      start_typing_delegate genv env
    else
      env
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
  type_check genv env (Relative_path.Map.keys fast) t

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
  Bad_files.check dirty_local_files;
  Bad_files.check changed_while_parsing;

  let (decl_and_typing_error_files, naming_and_parsing_error_files) =
    SaveStateService.partition_error_files_tf
      old_errors
      [Errors.Decl; Errors.Typing]
  in
  let (_old_parsing_phase, old_parsing_error_files) =
    match
      List.find old_errors ~f:(fun (phase, _files) -> phase = Errors.Parsing)
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
  let (similar_master_files, dirty_master_files) =
    partition_similar dirty_master_files
  in
  let (similar_local_files, dirty_local_files) =
    partition_similar dirty_local_files
  in
  let similar_files =
    Relative_path.Set.union similar_master_files similar_local_files
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
    dirty_master_files
    dirty_local_files
    similar_files
    t

let saved_state_init
    ~(load_state_approach : load_state_approach)
    (genv : ServerEnv.genv)
    (env : ServerEnv.env)
    (root : Path.t) :
    ( (ServerEnv.env * float) * (loaded_info * Relative_path.Set.t),
      load_state_error )
    result =
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
        Timeout.with_timeout ~timeout ~do_ ~on_timeout:(fun () ->
            Error Load_state_timeout)
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
