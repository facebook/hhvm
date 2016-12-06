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

open Result.Export
open Result.Monad_infix

module DepSet = Typing_deps.DepSet
module Dep = Typing_deps.Dep
module SLC = ServerLocalConfig
module LSC = LoadScriptConfig

exception No_loader
exception Loader_timeout of string


module ServerInitCommon = struct
  (* Return all the files that we need to typecheck *)
  let make_next_files genv : Relative_path.t MultiWorker.nextlist =
    let next_files_root = compose
      (List.map ~f:(Relative_path.(create Root)))
      (genv.indexer ServerEnv.file_filter) in
    let hhi_root = Hhi.get_hhi_root () in
    let hhi_filter = begin fun s ->
      (FindUtils.is_php s)
        (** If experimental disabled, we don't parse hhi files under
         * the experimental directory. *)
        && (TypecheckerOptions.experimental_feature_enabled
            (ServerConfig.typechecker_options genv.config)
            TypecheckerOptions.experimental_dict
          || not (FindUtils.has_ancestor s "experimental"))
    end in
    let next_files_hhi = compose
      (List.map ~f:(Relative_path.(create Hhi)))
      (Find.make_next_files
         ~name:"hhi" ~filter:hhi_filter hhi_root) in
    fun () ->
      match next_files_hhi () with
      | [] -> next_files_root ()
      | x -> x

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
  let load_state root use_sql cmd (_ic, oc) =
    try
      let load_script_log_file = ServerFiles.load_log root in
      let cmd =
        Printf.sprintf
          "%s %s %s %s %s"
          (Filename.quote (Path.to_string cmd))
          (Filename.quote (Path.to_string root))
          (Filename.quote Build_id.build_revision)
          (Filename.quote load_script_log_file)
          (Filename.quote (string_of_bool use_sql)) in
      Hh_logger.log "Running load_mini script: %s\n%!" cmd;
      let ic = Unix.open_process_in cmd in
      let json = read_json_line ic in
      let kv = Hh_json.get_object_exn json in
      check_json_obj_error kv;
      let state_fn = Hh_json.get_string_exn @@ List.Assoc.find_exn kv "state" in
      let is_cached =
        Hh_json.get_bool_exn @@ List.Assoc.find_exn kv "is_cached" in
      let deptable_fn =
        Hh_json.get_string_exn @@ List.Assoc.find_exn kv "deptable" in
      (* The sql deptable needs to be loaded in the master process, so
       * defer that to later, give dummy result for read time
       *)
      let read_deptable_time =
        if use_sql
        then 0
        else SharedMem.load_dep_table deptable_fn
      in
      let end_time = Unix.gettimeofday () in
      Daemon.to_channel oc
        @@ Ok (
        `Fst (state_fn, is_cached, end_time, deptable_fn, read_deptable_time));
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

  (* This generator-like function first runs the load script to download state
   * and loads the downloaded dependency table into shared memory. It then
   * waits for the load script to send it the list of files that have changed
   * since the state was downloaded.
   *
   * The loading of the dependency table must not run concurrently with any
   * operations that might write to the deptable. *)
  let mk_state_future root use_sql cmd =
    let start_time = Unix.gettimeofday () in
    Result.try_with @@ fun () ->
    let log_file =
      Sys_utils.make_link_of_timestamped (ServerFiles.load_log root) in
    let log_fd = Daemon.fd_of_path log_file in
    let {Daemon.channels = (ic, _oc); pid} as daemon =
      Daemon.fork (log_fd, log_fd)
                  (load_state root use_sql)
                  cmd
    (** The first generator in the future, which gets the results from the
     * process. *)
    in fun () ->
    try
      Daemon.from_channel ic >>= function
      | `Snd _ -> assert false
      | `Fst (fn, is_cached, end_time, deptable_fn, read_deptable_time) ->
        (* As promised, the sql deptable is being loaded in the master process
         * if we are in the sql mode
         *)
        let read_deptable_time =
          if use_sql
          then SharedMem.load_dep_table_sqlite deptable_fn
          else read_deptable_time
        in
        Hh_logger.log
          "Reading the dependency file took (sec): %d" read_deptable_time;
        HackEventLogger.load_deptable_end read_deptable_time;
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
            let old_modes = Marshal.from_channel chan in
            let dirty_files =
            List.map dirty_files Relative_path.(concat Root) in
            HackEventLogger.vcs_changed_files_end t;
            let _ = Hh_logger.log_duration "Finding changed files" t in
            Result.Ok (Relative_path.set_of_list dirty_files, old_modes)
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
      || not (is_check_mode genv.options)
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
    let fast = get_dirty_fast old_fast fast dirty_files in
    let names = Relative_path.Map.fold fast ~f:begin fun _k v acc ->
      FileInfo.merge_names v acc
    end ~init:FileInfo.empty_names in
    let deps = get_all_deps names in
    let to_recheck = Typing_deps.get_files deps in
    let fast = extend_fast fast env.files_info to_recheck in
    type_check genv env fast t

  let get_build_targets env =
    let targets =
      List.map (BuildMain.get_live_targets env) (Relative_path.(concat Root)) in
    Relative_path.set_of_list targets

  let get_state_future genv env root state_future timeout =
    let state = state_future
    >>= with_loader_timeout timeout "wait_for_changes"
    >>= fun (dirty_files, old_modes) ->
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
    (* Build targets are untracked by version control, so we must always
     * recheck them. While we could query hg / git for the untracked files,
     * it's much slower. *)
    let dirty_files =
      Relative_path.Set.union dirty_files (get_build_targets env) in
    Ok (dirty_files, changed_while_parsing, old_modes)
    in
    state
end

type state_result =
 (Relative_path.Set.t * Relative_path.Set.t * FileInfo.fast_with_modes, exn)
 Result.t

(* Laziness *)
type lazy_level = Off | Decl | Parse | Init

module type InitKind = sig
  val init :
    load_mini_script:(Path.t, exn) Result.t ->
    ServerEnv.genv ->
    lazy_level ->
    ServerEnv.env ->
    Path.t ->
    use_sql: bool ->
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
  let init ~load_mini_script genv lazy_level env root ~use_sql =
    (* Spawn this first so that it can run in the background while parsing is
     * going on. The script can fail in a variety of ways, but the resolution
     * is always the same -- we fall back to rechecking everything. Running it
     * in the Result monad provides a convenient way to locate the error
     * handling code in one place. *)
    let state_future =
     load_mini_script >>= mk_state_future root use_sql in
    let get_next, t = indexing genv in
    let lazy_parse = lazy_level = Parse in
    let env, t = parsing ~lazy_parse genv env ~get_next t in

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

    let state = get_state_future genv env root state_future timeout in
    match state with
    | Ok (dirty_files, changed_while_parsing, old_modes) ->
      let old_fast = FileInfo.modes_to_fast old_modes in
      Hh_logger.log "Successfully loaded mini-state";
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
    let hs = SharedMem.heap_size () in
    Hh_logger.log "Heap size: %d" hs;
    (Hh_logger.log_duration "Naming fast" t)

  (*
   * In eager initialization, this is done at the parsing step with
   * parsing hooks. During lazy init, need to do it manually from the fast
   * instead since we aren't parsing the codebase.
   *)
  let update_search genv fast t =
    let fast = Relative_path.Map.filter fast
      ~f:(fun s _ ->
          let fn = (Relative_path.to_absolute s) in
          not (FilesToIgnore.should_ignore fn)
          && FindUtils.is_php fn) in
    let update_single_search _ fast_list =
      List.iter fast_list
      (fun (fn, fast) ->
         HackSearchService.WorkerApi.update_from_fast fn fast
      ) in
    let next_fast_files =
      MultiWorker.next genv.workers (Relative_path.Map.elements fast) in
    MultiWorker.call
        genv.workers
        ~job:update_single_search
        ~neutral:()
        ~merge:(fun _ _ -> ())
        ~next:next_fast_files;
    HackSearchService.MasterApi.update_search_index
      ~fuzzy:false (Relative_path.Map.keys fast);
    Hh_logger.log_duration "Updating search indices" t



  let init ~load_mini_script genv lazy_level env root ~use_sql =
    assert (lazy_level = Init);
    let state_future =
      load_mini_script >>= mk_state_future root use_sql in

    let timeout = genv.local_config.SLC.load_mini_script_timeout in
    let state_future = state_future >>= fun f ->
      with_loader_timeout timeout "wait_for_state" f
    in

    let state = get_state_future genv env root state_future timeout in

    match state with
    | Ok (dirty_files, changed_while_parsing, old_modes) ->
      Hh_logger.log "Successfully loaded mini-state";
      let t = Unix.gettimeofday () in
      let dirty_files =
        Relative_path.Set.union dirty_files changed_while_parsing in
      let dirty_files_list = Relative_path.Set.elements dirty_files in
      let old_fast = FileInfo.modes_to_fast old_modes in
      let old_info = FileInfo.modes_to_info old_modes in
      (* Parse dirty files only *)
      let next = MultiWorker.next genv.workers dirty_files_list in
      let env, t = parsing genv env ~lazy_parse:true ~get_next:next t in
      let t = update_files genv env.files_info t in
      (* Name all the files from the old fast (except dirty) *)
      let old_fast_names = Relative_path.Map.filter old_fast (fun k _v ->
          not (Relative_path.Set.mem dirty_files k)
        ) in

      let t = naming_with_fast old_fast_names t in
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

      let t = update_search genv old_fast t in
      type_check_dirty genv env old_fast fast dirty_files t, state
    | Error err ->
      (* Fall back to type-checking everything *)
      if err <> No_loader then begin
        HackEventLogger.load_mini_exn err;
        Hh_logger.exc ~prefix:"Could not load mini state: " err;
      end;
      let get_next, t = indexing genv in
      let env, t = parsing ~lazy_parse:true genv env ~get_next t in
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
      [env.failed_parsing; env.failed_decl]
      (fun m -> Relative_path.Set.is_empty m) in
    if not all_passed then begin
      Hh_logger.log "Cannot run AI because of errors in source";
      Exit_status.exit Exit_status.CantRunAI
    end;
    let check_mode = ServerArgs.check_mode genv.options in
    let errorl, failed = Ai.go
      Typing_check_utils.check_defs genv.workers files_info
        env.tcopt ai_opt check_mode in
    let env = { env with
      errorl = Errors.merge errorl env.errorl;
      failed_check = Relative_path.Set.union failed env.failed_check;
    } in
    env, (Hh_logger.log_duration "Ai" t)
  | None -> env, t

let print_hash_stats () =
  let { SharedMem.
    used_slots;
    slots;
    nonempty_slots = _ } = SharedMem.dep_stats () in
  let load_factor = float_of_int used_slots /. float_of_int slots in
  Hh_logger.log "Dependency table load factor: %d / %d (%.02f)"
    used_slots slots load_factor;
  let { SharedMem.
    used_slots;
    slots;
    nonempty_slots } = SharedMem.hash_stats () in
  let load_factor = float_of_int used_slots /. float_of_int slots in
  Hh_logger.log
    "Hashtable load factor: %d / %d (%.02f) with %d nonempty slots"
    used_slots slots load_factor nonempty_slots;
  ()

let save_state env fn =
  let t = Unix.gettimeofday () in
  if not (Errors.is_empty env.errorl)
  then failwith "--save-mini only works if there are no type errors!";
  let chan = Sys_utils.open_out_no_fail fn in
  let modes = FileInfo.info_to_modes env.files_info in
  Marshal.to_channel chan modes [];
  Sys_utils.close_out_no_fail fn chan;
  let sqlite_save_t = SharedMem.save_dep_table_sqlite (fn^".sql") in
  let save_t = SharedMem.save_dep_table (fn^".deptable") in
  Hh_logger.log "Saving deptable using sqlite took(seconds): %d" sqlite_save_t;
  Hh_logger.log "Saving deptable without sqlite took(seconds): %d" save_t;
  ignore @@ Hh_logger.log_duration "Saving" t


let get_lazy_level genv =
  let lazy_decl = genv.local_config.SLC.lazy_decl &&
    Option.is_none (ServerArgs.ai_mode genv.options) in
  let lazy_parse = genv.local_config.SLC.lazy_parse in
  let lazy_initialize = genv.local_config.SLC.lazy_init in
  match lazy_decl, lazy_parse, lazy_initialize with
  | true, false, false -> Decl
  | true, true, false -> Parse
  | true, true, true -> Init
  | _ -> Off


(* entry point *)
let init ?load_mini_script genv =
  let lazy_lev = get_lazy_level genv in
  let load_mini_script = Result.of_option load_mini_script ~error:No_loader in
  let env = ServerEnvBuild.make_env genv.config in
  let root = ServerArgs.root genv.options in
  let use_sql = LSC.use_sql genv.local_config.SLC.load_script_config in
  let (env, t), state =
    if lazy_lev = Init then
      ServerLazyInit.init ~load_mini_script genv lazy_lev env root ~use_sql
    else
      ServerEagerInit.init ~load_mini_script genv lazy_lev env root ~use_sql in
  let env, _t = ai_check genv env.files_info env t in
  SharedMem.init_done ();
  print_hash_stats ();
  env, Result.is_ok state
