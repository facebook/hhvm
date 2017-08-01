(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core
open ServerEnv
open ServerCheckUtils
open Reordered_argument_collections
open Utils
open String_utils
open SearchServiceRunner

open Result.Export
open Result.Monad_infix

module DepSet = Typing_deps.DepSet
module Dep = Typing_deps.Dep
module SLC = ServerLocalConfig
module LSC = LoadScriptConfig

exception No_loader
exception Loader_timeout of string

type load_mini_approach =
  | Load_mini_script of Path.t
  | Precomputed of ServerArgs.mini_state_target

(** Docs are in .mli *)
type init_result =
  | Mini_load of int option
  | Mini_load_failed of string

module ServerInitCommon = struct

  let lock_and_load_deptable fn =
    (* The sql deptable must be loaded in the master process *)
    try
      (* Take a lock on the info file for the sql *)
      LoadScriptUtils.lock_saved_state fn;
      let read_deptable_time =
        SharedMem.load_dep_table_sqlite fn
      in
      Hh_logger.log
        "Reading the dependency file took (sec): %d" read_deptable_time;
      HackEventLogger.load_deptable_end read_deptable_time;
    with SharedMem.Sql_assertion_failure 11 as e -> (* SQL_corrupt *)
      LoadScriptUtils.delete_corrupted_saved_state fn;
      raise e


  (* Return all the files that we need to typecheck *)
  let make_next_files genv : Relative_path.t MultiWorker.nextlist =
    let next_files_root = compose
      (List.map ~f:(Relative_path.(create Root)))
      (genv.indexer ServerEnv.file_filter) in
    let hhi_root = Hhi.get_hhi_root () in
    let hhi_filter = FindUtils.is_php in
    let next_files_hhi = compose
      (List.map ~f:(Relative_path.(create Hhi)))
      (Find.make_next_files
         ~name:"hhi" ~filter:hhi_filter hhi_root) in
    fun () ->
      let next = match next_files_hhi () with
      | [] -> next_files_root ()
      | x -> x
      in
      Bucket.of_list next

  let read_json_line ic =
    let output = input_line ic in
    try Hh_json.json_of_string output
    with Hh_json.Syntax_error _ as e ->
      Hh_logger.log "Failed to parse JSON: %s" output;
      raise e

  let check_json_obj_error kv =
    match List.Assoc.find kv "error" with
    | Some (Hh_json.JSON_String s) -> failwith s
    | _ -> ()

  (* Expected output from script:
   * Two lines of JSON.
   * The first line indicates the path to the state file plus some metadata
   * The second line is a list of the files that have changed since the state
   * was built
   *)
  let load_state root cmd (_ic, oc) =
    try
      let load_script_log_file = ServerFiles.load_log root in
      let cmd =
        Printf.sprintf
          "%s %s %s %s"
          (Filename.quote (Path.to_string cmd))
          (Filename.quote (Path.to_string root))
          (Filename.quote (Build_id.build_revision))
          (Filename.quote load_script_log_file) in
      Hh_logger.log "Running load_mini script: %s\n%!" cmd;
      let ic = Unix.open_process_in cmd in
      let json = read_json_line ic in
      let kv = Hh_json.get_object_exn json in
      check_json_obj_error kv;
      let state_fn = Hh_json.get_string_exn @@ List.Assoc.find_exn kv "state" in
      let corresponding_base_revision = Hh_json.get_string_exn @@
        List.Assoc.find_exn kv "corresponding_base_revision" in
      let is_cached =
        Hh_json.get_bool_exn @@ List.Assoc.find_exn kv "is_cached" in
      let deptable_fn =
        Hh_json.get_string_exn @@ List.Assoc.find_exn kv "deptable" in
      let end_time = Unix.gettimeofday () in
      let open Hh_json.Access in
      let state_distance = (return json) >>=
        get_number_int "distance" |>
        to_option
      in
      Daemon.to_channel oc
        @@ Ok (`Fst (
          state_fn,
          corresponding_base_revision,
          is_cached,
          end_time,
          deptable_fn,
          state_distance));
      let json = read_json_line ic in
      assert (Unix.close_process_in ic = Unix.WEXITED 0);
      let kv = Hh_json.get_object_exn json in
      check_json_obj_error kv;
      let to_recheck =
        Hh_json.get_array_exn @@ List.Assoc.find_exn kv "changes" in
      let to_recheck = List.map to_recheck Hh_json.get_string_exn in
      Daemon.to_channel oc @@ Ok (`Snd to_recheck)
    with e ->
      Hh_logger.exc ~prefix:"Failed to load state: " e;
      Daemon.to_channel oc @@ Error e

  let with_loader_timeout timeout stage f =
    Result.join @@ Result.try_with @@ fun () ->
    Timeout.with_timeout ~timeout ~do_:(fun _ -> f ())
      ~on_timeout:(fun _ -> raise @@ Loader_timeout stage)

  (* This generator-like function first runs the load script to download state.
   * It then waits for the load script to send it the list of files that
   * have changed since the state was downloaded.
   *)
  let mk_state_future root cmd =
    let start_time = Unix.gettimeofday () in
    Result.try_with @@ fun () ->
    let log_file =
      Sys_utils.make_link_of_timestamped (ServerFiles.load_log root) in
    let log_fd = Daemon.fd_of_path log_file in
    let {Daemon.channels = (ic, _oc); pid} as daemon =
      Daemon.fork (log_fd, log_fd) (load_state root) cmd
    (** The first generator in the future, which gets the results from the
     * process. *)
    in fun () ->
    try
      Daemon.from_channel ic >>= function
      | `Snd _ -> assert false
      | `Fst (
        fn,
        corresponding_base_revision,
        is_cached,
        end_time,
        deptable_fn,
        state_distance) ->
        lock_and_load_deptable deptable_fn;
        HackEventLogger.load_mini_worker_end ~is_cached start_time end_time;
        let time_taken = end_time -. start_time in
        Hh_logger.log "Loading mini-state took %.2fs" time_taken;
        let t = Unix.gettimeofday () in
        (** The second future, which fetches the dirty files. *)
        let get_dirty_files = fun () -> begin
        Daemon.from_channel ic >>= function
          | `Fst _ -> assert false
          | `Snd dirty_files ->
            let _, status = Unix.waitpid [] pid in
            assert (status = Unix.WEXITED 0);
            let chan = open_in fn in
            let old_saved = Marshal.from_channel chan in
            let dirty_files =
            List.map dirty_files Relative_path.from_root in
            HackEventLogger.vcs_changed_files_end t (List.length dirty_files);
            let _ = Hh_logger.log_duration "Finding changed files" t in
            Result.Ok (
              fn,
              corresponding_base_revision,
              Relative_path.set_of_list dirty_files,
              old_saved,
              state_distance)
        end in
        Result.Ok get_dirty_files
    with e ->
      (* We have failed to load the saved state in the allotted time. Kill
       * the daemon so it doesn't write to shared memory while the type-decl
       * / type-check phases are running. The kill may fail if e.g. the
       * daemon exited just after the timeout but before the kill signal goes
       * through *)
      (try Daemon.kill daemon with e -> Hh_logger.exc e);
      raise e

  let invoke_approach root approach = match approach with
   | Load_mini_script cmd ->
     mk_state_future root cmd
   | Precomputed { ServerArgs.saved_state_fn;
     corresponding_base_revision; deptable_fn; changes } ->
     lock_and_load_deptable deptable_fn;
     let changes = Relative_path.set_of_list changes in
     let chan = open_in saved_state_fn in
     let old_saved = Marshal.from_channel chan in
     let get_dirty_files = (fun () -> Result.Ok (
       saved_state_fn,
       corresponding_base_revision,
       changes,
       old_saved,
       None
     )) in
     Result.try_with (fun () -> fun () -> Result.Ok get_dirty_files)

  let is_check_mode options =
    ServerArgs.check_mode options &&
    ServerArgs.convert options = None &&
    (* Note: we need to run update_files to get an accurate saved state *)
    ServerArgs.save_filename options = None

  let indexing genv =
    let t = Unix.gettimeofday () in
    let get_next = make_next_files genv in
    HackEventLogger.indexing_end t;
    let t = Hh_logger.log_duration "Indexing" t in
    get_next, t

  let parsing ~lazy_parse genv env ~get_next t =
    let quick = lazy_parse in
    let files_info, errorl, failed =
      Parsing_service.go
        ~quick
        genv.workers
        Relative_path.Set.empty
        ~get_next
        env.popt in
    let files_info = Relative_path.Map.union files_info env.files_info in
    let hs = SharedMem.heap_size () in
    Hh_logger.log "Heap size: %d" hs;
    Stats.(stats.init_parsing_heap_size <- hs);
    (* TODO: log a count of the number of files parsed... 0 is a placeholder *)
    HackEventLogger.parsing_end t hs  ~parsed_count:0;
    let env = { env with
      files_info;
      errorl = Errors.merge errorl env.errorl;
      failed_parsing = Relative_path.Set.union env.failed_parsing failed;
    } in
    env, (Hh_logger.log_duration "Parsing" t)

  let update_files genv files_info t =
    if is_check_mode genv.options then t else begin
      Typing_deps.update_files files_info;
      HackEventLogger.updating_deps_end t;
      Hh_logger.log_duration "Updating deps" t
    end

  let naming env t =
    let env =
      Relative_path.Map.fold env.files_info ~f:begin fun k v env ->
        let errorl, failed = NamingGlobal.ndecl_file env.tcopt k v in
        { env with
          errorl = Errors.merge errorl env.errorl;
          failed_naming = Relative_path.Set.union env.failed_naming failed;
        }
      end ~init:env
    in
    let hs = SharedMem.heap_size () in
    Hh_logger.log "Heap size: %d" hs;
    HackEventLogger.global_naming_end t;
    env, (Hh_logger.log_duration "Naming" t)

  let type_decl genv env fast t =
    let bucket_size = genv.local_config.SLC.type_decl_bucket_size in
    let errorl, failed_decl =
      Decl_service.go ~bucket_size genv.workers env.tcopt fast in
    let hs = SharedMem.heap_size () in
    Hh_logger.log "Heap size: %d" hs;
    Stats.(stats.init_heap_size <- hs);
    HackEventLogger.type_decl_end t;
    let t = Hh_logger.log_duration "Type-decl" t in
    let env = {
      env with
      errorl = Errors.merge errorl env.errorl;
      failed_decl;
    } in
    env, t

  let type_check genv env fast t =
    if ServerArgs.ai_mode genv.options = None
    then begin
      let count = Relative_path.Map.cardinal fast in
      let errorl, err_info =
        Typing_check_service.go genv.workers env.tcopt fast in
      let { Decl_service.
        errs = failed;
        lazy_decl_errs = lazy_decl_failed;
      } = err_info in
      let hs = SharedMem.heap_size () in
      Hh_logger.log "Heap size: %d" hs;
      HackEventLogger.type_check_end count t;
      let env = { env with
        errorl = Errors.merge errorl env.errorl;
        failed_decl = Relative_path.Set.union env.failed_decl lazy_decl_failed;
        failed_check = failed;
      } in
      env, (Hh_logger.log_duration "Type-check" t)
    end else env, t

  let get_dirty_fast old_fast fast dirty =
    Relative_path.Set.fold dirty ~f:begin fun fn acc ->
      let dirty_fast = Relative_path.Map.get fast fn in
      let dirty_old_fast = Relative_path.Map.get old_fast fn in
      let fast = Option.merge dirty_old_fast dirty_fast FileInfo.merge_names in
      match fast with
      | Some fast -> Relative_path.Map.add acc ~key:fn ~data:fast
      | None -> acc
    end ~init:Relative_path.Map.empty

  let get_all_deps {FileInfo.n_funs; n_classes; n_types; n_consts} =
    let add_deps_of_sset dep_ctor sset depset =
      SSet.fold sset ~init:depset ~f:begin fun n acc ->
        let dep = dep_ctor n in
        let deps = Typing_deps.get_bazooka dep in
        DepSet.union deps acc
      end
    in
    let deps = add_deps_of_sset (fun n -> Dep.Fun n) n_funs DepSet.empty in
    let deps = add_deps_of_sset (fun n -> Dep.FunName n) n_funs deps in
    let deps = add_deps_of_sset (fun n -> Dep.Class n) n_classes deps in
    let deps = add_deps_of_sset (fun n -> Dep.Class n) n_types deps in
    let deps = add_deps_of_sset (fun n -> Dep.GConst n) n_consts deps in
    let deps = add_deps_of_sset (fun n -> Dep.GConstName n) n_consts deps in
    (* We need to type check all classes that have extend dependencies on the
     * classes that have changed
     *)
    let deps =
      SSet.fold ~f:begin fun class_name acc ->
        let hash = Typing_deps.Dep.make (Dep.Class class_name) in
        Decl_compare.get_extend_deps hash acc
      end n_classes ~init:deps
    in
    deps

  (* We start of with a list of files that have changed since the state was
   * saved (dirty_files), and two maps of the class / function declarations
   * -- one made when the state was saved (old_fast) and one made for the
   * current files in the repository (fast). We grab the declarations from both
   * , to account for both the declaratons that were deleted and those that
   * are newly created. Then we use the deptable to figure out the files
   * that referred to them. Finally we recheck the lot. *)
  let type_check_dirty genv env old_fast fast dirty_files t =
    let start_time = Unix.gettimeofday () in
    let fast = get_dirty_fast old_fast fast dirty_files in
    let names = Relative_path.Map.fold fast ~f:begin fun _k v acc ->
      FileInfo.merge_names v acc
    end ~init:FileInfo.empty_names in
    let deps = get_all_deps names in
    let to_recheck = Typing_deps.get_files deps in
    let fast = extend_fast fast env.files_info to_recheck in
    let result = type_check genv env fast t in
    HackEventLogger.type_check_dirty start_time
      (Relative_path.Set.cardinal dirty_files);
    Hh_logger.log "ServerInit type_check_dirty count: %d"
      (Relative_path.Set.cardinal dirty_files);
    result

  let get_build_targets env =
    let untracked, tracked = BuildMain.get_live_targets env in
    let untracked =
      List.map untracked Relative_path.from_root in
    let tracked =
      List.map tracked Relative_path.from_root in
    Relative_path.set_of_list untracked, Relative_path.set_of_list tracked

  let get_state_future genv root state_future timeout =
    let state = state_future
    >>= with_loader_timeout timeout "wait_for_changes"
    >>= fun (
      saved_state_fn,
      corresponding_base_revision,
      dirty_files,
      old_saved,
      state_distance
    ) ->
    genv.wait_until_ready ();
    let root = Path.to_string root in
    let updates = genv.notifier_async () in
    let open ServerNotifierTypes in
    let updates = match updates with
      | Notifier_state_enter (name, _) ->
        (** We ignore the returned debut port result. This is unfortunate but
         * harmless (since we should be using write_opt everywhere and it is
         * crash-resilient and handles the Option for us anyway).
         *
         * We can't easily use the returned result and set the env.debug_port
         * without making it a mutable reference (gross), and we can't return
         * a new genv in this function because we're in the Error/Result
         * monad for the state. *)
        let _ = Debug_port.write_opt
        (Debug_event.Fresh_vcs_state name) genv.debug_port in
        SSet.empty
      | Notifier_state_leave _
      | Notifier_unavailable -> SSet.empty
      | Notifier_synchronous_changes updates
      | Notifier_async_changes updates -> updates in
    let updates = SSet.filter updates (fun p ->
      string_starts_with p root && ServerEnv.file_filter p) in
    let changed_while_parsing = Relative_path.(relativize_set Root updates) in
    Ok (saved_state_fn,
      corresponding_base_revision,
      dirty_files,
      changed_while_parsing,
      old_saved,
      state_distance)
    in
    state
end

type saved_state_fn = string
type corresponding_base_rev = string
(** Newer versions of load script also output the distance of the
 * saved state's revision to the node's merge base. *)
type state_distance = int option

type state_result =
 (saved_state_fn * corresponding_base_rev * Relative_path.Set.t
   * Relative_path.Set.t * FileInfo.saved_state_info * state_distance, exn)
 Result.t

(* Laziness *)
type lazy_level = Off | Decl | Parse | Init

module type InitKind = sig
  val init :
    load_mini_approach:(load_mini_approach, exn) Result.t ->
    ServerEnv.genv ->
    lazy_level ->
    ServerEnv.env ->
    Path.t ->
    (ServerEnv.env * float) * state_result
end

(* Eager Initialization:
* hh_server can initialize either by typechecking the entire project (aka
* starting from a "fresh state") or by loading from a saved state and
* typechecking what has changed.
*
* If we start from a fresh state, we run the following phases:
*
*   Parsing -> Naming -> Type-decl(skipped if lazy_decl)-> Type-check
*
* If we are loading a state, we do
*
*   Run load script and parsing concurrently -> Naming -> Type-decl
*
* Then we typecheck only the files that have changed since the state was
* saved.
*
* This is done in fairly similar manner to the incremental update
* code in ServerTypeCheck. The key difference is that incremental mode
* can compare the files that it has just parsed with their old versions,
* thereby (in theory) recomputing the least amount possible. OTOH,
* ServerInit only has the latest version of each file, so it has to make
* the most conservative estimate about what to recheck.
*)
module ServerEagerInit : InitKind = struct
  open ServerInitCommon
  let init ~load_mini_approach genv lazy_level env root =
    (* Spawn this first so that it can run in the background while parsing is
     * going on. The script can fail in a variety of ways, but the resolution
     * is always the same -- we fall back to rechecking everything. Running it
     * in the Result monad provides a convenient way to locate the error
     * handling code in one place. *)
    let state_future =
     load_mini_approach >>= invoke_approach root in
    let get_next, t = indexing genv in
    let lazy_parse = lazy_level = Parse in
    let env, t = parsing ~lazy_parse genv env ~get_next t in
    SearchServiceRunner.update_fileinfo_map env.files_info;

    let timeout = genv.local_config.SLC.load_mini_script_timeout in
    let state_future = state_future >>=
      with_loader_timeout timeout "wait_for_state"
    in

    let t = update_files genv env.files_info t in
    let env, t = naming env t in
    let fast = FileInfo.simplify_fast env.files_info in
    let fast = Relative_path.Set.fold env.failed_parsing
      ~f:(fun x m -> Relative_path.Map.remove m x) ~init:fast in
    let env, t =
      if lazy_level <> Off then env, t
      else type_decl genv env fast t in

    let state = get_state_future genv root state_future timeout in
    match state with
    | Ok (
      saved_state_fn,
      corresponding_base_revision,
      dirty_files,
      changed_while_parsing,
      old_saved,
      _state_distance) ->
      let old_fast = FileInfo.saved_to_fast old_saved in
      (* During eager init, we don't need to worry about tracked targets since
         they we end up parsing everything anyways
      *)
      let build_targets, _ = get_build_targets env in
      Hh_logger.log "Successfully loaded mini-state";
      let global_state = ServerGlobalState.save () in
      let loaded_event = Debug_event.Loaded_saved_state ({
        Debug_event.filename = saved_state_fn;
        corresponding_base_revision;
        dirty_files;
        changed_while_parsing;
        build_targets;
      }, global_state) in
      let () = Printf.eprintf "Sending Loaded_saved_state debug event\n%!" in
      let _ = Debug_port.write_opt loaded_event genv.debug_port in
      (* Build targets are untracked by version control, so we must always
       * recheck them. While we could query hg / git for the untracked files,
       * it's much slower. *)
      let dirty_files =
        Relative_path.Set.union dirty_files build_targets in
      (* If a file has changed while we were parsing, we may have parsed the
       * new version, so we must treat it as possibly creating new type
       * errors. *)
      let dirty_files =
        Relative_path.Set.union dirty_files changed_while_parsing in
      (* But we still want to keep it in the set of things that need to be
       * reparsed in the next round of incremental updates. *)
      let env = { env with
        failed_parsing =
          Relative_path.Set.union env.failed_parsing changed_while_parsing;
      } in
      type_check_dirty genv env old_fast fast dirty_files t, state
    | Error err ->
      (* Fall back to type-checking everything *)
      SharedMem.cleanup_sqlite ();
      if err <> No_loader then begin
        HackEventLogger.load_mini_exn err;
        Hh_logger.exc ~prefix:"Could not load mini state: " err;
      end;
      type_check genv env fast t, state
end

(* Lazy Initialization:
 * During Lazy initialization, hh_server tries to do as little work as possible.
 * If we load from saved state, our steps are:
 * Load from saved state -> Parse dirty files -> Naming -> Dirty Typecheck
 * Otherwise, we fall back to the original with lazy decl and parse turned on:
 * Full Parsing -> Naming -> Full Typecheck
 *)
module ServerLazyInit : InitKind = struct
  open ServerInitCommon

  (* Run naming from a fast generated from saved state.
   * No errors are generated because we assume the fast is directly from
   * a clean state.
   *)
  let naming_with_fast fast t =
    Relative_path.Map.iter fast ~f:begin fun k info ->
    let { FileInfo.n_classes=classes;
         n_types=typedefs;
         n_funs=funs;
         n_consts=consts} = info in
    NamingGlobal.ndecl_file_fast k ~funs ~classes ~typedefs ~consts
    end;
    HackEventLogger.fast_naming_end t;
    let hs = SharedMem.heap_size () in
    Hh_logger.log "Heap size: %d" hs;
    (Hh_logger.log_duration "Naming fast" t)

  (*
   * In eager initialization, this is done at the parsing step with
   * parsing hooks. During lazy init, need to do it manually from the fast
   * instead since we aren't parsing the codebase.
   *)
  let update_search saved t =
    (* Only look at Hack files *)
    let fast = FileInfo.saved_to_hack_files saved in
    (* Filter out non php files *)
    let fast = Relative_path.Map.filter fast
      ~f:(fun s _ ->
          let fn = (Relative_path.to_absolute s) in
          not (FilesToIgnore.should_ignore fn)
          && FindUtils.is_php fn) in

    Relative_path.Map.iter fast
    ~f: (fun fn names ->
      SearchServiceRunner.update (fn, (SearchServiceRunner.Fast names));
    );
    Hh_logger.log_duration "Loading search indices" t


  let init ~load_mini_approach genv lazy_level env root =
    assert (lazy_level = Init);
    let state_future =
      load_mini_approach >>= invoke_approach root in

    let timeout = genv.local_config.SLC.load_mini_script_timeout in
    let state_future = state_future >>= fun f ->
      with_loader_timeout timeout "wait_for_state" f
    in

    let state = get_state_future genv root state_future timeout in

    match state with
    | Ok (
      saved_state_fn, corresponding_base_revision,
      dirty_files, changed_while_parsing, old_saved, _state_distance) ->
      let build_targets, tracked_targets = get_build_targets env in
      Hh_logger.log "Successfully loaded mini-state";
      let global_state = ServerGlobalState.save () in
      let loaded_event = Debug_event.Loaded_saved_state ({
        Debug_event.filename = saved_state_fn;
        corresponding_base_revision;
        dirty_files;
        changed_while_parsing;
        build_targets;
      }, global_state) in
      Hh_logger.log "Sending Loaded_saved_state debug event\n";
      let _ = Debug_port.write_opt loaded_event genv.debug_port in
      let t = Unix.gettimeofday () in
      (* Build targets are untracked by version control, so we must always
       * recheck them. While we could query hg / git for the untracked files,
       * it's much slower. *)
      let dirty_files =
        Relative_path.Set.union dirty_files build_targets in
      let dirty_files =
        Relative_path.Set.union dirty_files changed_while_parsing in
      (*
        Tracked targets are build files that are tracked by version control.
        We don't need to typecheck them, but we do need to parse them to load
        them into memory, since arc rebuild deletes them before running.
        This avoids build step dependencies and file_heap_stale errors crashing
        the server when build fails and the deleted files aren't properly
        regenerated.
      *)
      let parsing_files =
        Relative_path.Set.union dirty_files tracked_targets in
      let parsing_files_list = Relative_path.Set.elements parsing_files in
      let old_fast = FileInfo.saved_to_fast old_saved in

      (* Get only the hack files for global naming *)
      let old_hack_files = FileInfo.saved_to_hack_files old_saved in
      let old_info = FileInfo.saved_to_info old_saved in
      (* Parse dirty files only *)
      let next = MultiWorker.next genv.workers parsing_files_list in
      let env, t = parsing genv env ~lazy_parse:true ~get_next:next t in
      SearchServiceRunner.update_fileinfo_map env.files_info;

      let t = update_files genv env.files_info t in
      (* Name all the files from the old fast (except the new ones we parsed) *)
      let old_hack_names = Relative_path.Map.filter old_hack_files (fun k _v ->
          not (Relative_path.Set.mem parsing_files k)
        ) in

      let t = naming_with_fast old_hack_names t in
      (* Do global naming on all dirty files *)
      let env, t = naming env t in

      (* Add all files from fast to the files_info object *)
      let fast = FileInfo.simplify_fast env.files_info in
      let fast = Relative_path.Set.fold env.failed_parsing
        ~f:(fun x m -> Relative_path.Map.remove m x) ~init:fast in

      let env = { env with
        failed_parsing =
          Relative_path.Set.union env.failed_parsing changed_while_parsing;
      } in
      let env = { env with
        files_info=Relative_path.Map.union env.files_info old_info;
      } in
      (* Update the fileinfo object's dependencies now that we have full fast *)
      let t = update_files genv env.files_info t in

      let t = update_search old_saved t in
      type_check_dirty genv env old_fast fast dirty_files t, state
    | Error err ->
      (* Fall back to type-checking everything *)
      SharedMem.cleanup_sqlite ();
      if err <> No_loader then begin
        HackEventLogger.load_mini_exn err;
        Hh_logger.exc ~prefix:"Could not load mini state: " err;
      end;
      let get_next, t = indexing genv in
      let env, t = parsing ~lazy_parse:true genv env ~get_next t in
      SearchServiceRunner.update_fileinfo_map env.files_info;
      let t = update_files genv env.files_info t in
      let env, t = naming env t in
      let fast = FileInfo.simplify_fast env.files_info in
      let fast = Relative_path.Set.fold env.failed_parsing
        ~f:(fun x m -> Relative_path.Map.remove m x) ~init:fast in
      type_check genv env fast t, state
end


module SIC = ServerInitCommon

let ai_check genv files_info env t =
  match ServerArgs.ai_mode genv.options with
  | Some ai_opt ->
    let all_passed = List.for_all
      [env.failed_parsing; env.failed_decl; env.failed_check;]
      (fun m -> Relative_path.Set.is_empty m) in
    if not all_passed then begin
      Hh_logger.log "Cannot run AI because of errors in source";
      env, t
    end
    else begin
      let check_mode = ServerArgs.check_mode genv.options in
      let errorl, failed = Ai.go
          Typing_check_utils.check_defs genv.workers files_info
          env.tcopt ai_opt check_mode in
      let env = { env with
                  errorl = Errors.merge errorl env.errorl;
                  failed_check = Relative_path.Set.union failed env.failed_check;
                } in
      env, (Hh_logger.log_duration "Ai" t)
    end
  | None -> env, t

let run_search genv t =
  if SearchServiceRunner.should_run_completely genv
  then begin
    (* The duration is already logged by SearchServiceRunner *)
    SearchServiceRunner.run_completely genv;
    HackEventLogger.update_search_end t
  end
  else ()

let save_state env fn =
  let t = Unix.gettimeofday () in
  if not (Errors.is_empty env.errorl)
  then failwith "--save-mini only works if there are no type errors!";
  let chan = Sys_utils.open_out_no_fail fn in
  let saved = FileInfo.info_to_saved env.files_info in
  Marshal.to_channel chan saved [];
  Sys_utils.close_out_no_fail fn chan;
  let sqlite_save_t = SharedMem.save_dep_table_sqlite (fn^".sql") in
  Hh_logger.log "Saving deptable using sqlite took(seconds): %d" sqlite_save_t;
  ignore @@ Hh_logger.log_duration "Saving" t


let get_lazy_level genv =
  let lazy_decl = Option.is_none (ServerArgs.ai_mode genv.options) in
  let lazy_parse = genv.local_config.SLC.lazy_parse in
  let lazy_initialize = genv.local_config.SLC.lazy_init in
  match lazy_decl, lazy_parse, lazy_initialize with
  | true, false, false -> Decl
  | true, true, false -> Parse
  | true, true, true -> Init
  | _ -> Off


(* entry point *)
let init ?load_mini_approach genv =
  let lazy_lev = get_lazy_level genv in
  let load_mini_approach = Result.of_option load_mini_approach
    ~error:No_loader in
  let env = ServerEnvBuild.make_env genv.config in
  let root = ServerArgs.root genv.options in
  let (env, t), state =
    if lazy_lev = Init then
      ServerLazyInit.init ~load_mini_approach genv lazy_lev env root
    else
      ServerEagerInit.init ~load_mini_approach genv lazy_lev env root
  in
  let env, t = ai_check genv env.files_info env t in
  run_search genv t;
  SharedMem.init_done ();
  ServerUtils.print_hash_stats ();
  let result = match state with
    | Result.Ok (
      _saved_state_fn,
      _corresponding_base_revision,
      _dirty_files,
      _changed_while_parsing,
      _old_saved,
      state_distance) ->
        Mini_load state_distance
    | Result.Error e ->
      Mini_load_failed (Printexc.to_string e)
  in
  env, result
