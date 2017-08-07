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
open ServerCheckUtils
open SearchServiceRunner
open ServerEnv
open Reordered_argument_collections
open Utils

module SLC = ServerLocalConfig

type check_kind =
  (* Lazy check is a check limited to the files open in IDE. It:
   * - produces push diagnostics for those files
   * - updates their parsing / naming / decl definitions on heap
   * - updates their parsing level indexes, like HackSearchService or
   *     ServerEnv.files_info
   * - invalidates their declaration dependencies, by removing them from the
   *     heap and depending on lazy declaration to redeclare them on
   *     as-needed basis later
   * - stores the information about what it skipped doing to be finished later
   *     by Full_check
   *
   * It does not do the "full" expensive fanout:
   * - does not re-declare dependencies ("phase 2 decl")
   * - does not fan out to all typing dependencies
   * - because of that, it does not update structures depending on global state,
   *     like global error list, dependency table or the lists of files that
   *     failed parsing / declaration / checking
   *
   * Any operation that need the global state to be up to date and cannot get
   * the data that they need through lazy decl, need to be preceded by
   * Full_check. *)
  | Lazy_check
  (* Full check brings the global state of the server to consistency by
   * executing all the re-checks that lazy checks delayed. It processes the
   * disk updates and typechecks the full fanout of accumulated changes. *)
  | Full_check

(*****************************************************************************)
(* Debugging *)
(*****************************************************************************)

let print_defs prefix defs =
  List.iter defs begin fun (_, fname) ->
    Printf.printf "  %s %s\n" prefix fname;
  end

let print_fast_pos fast_pos =
  SMap.iter fast_pos begin fun x (funs, classes) ->
    Printf.printf "File: %s\n" x;
    print_defs "Fun" funs;
    print_defs "Class" classes;
  end;
  Printf.printf "\n";
  flush stdout;
  ()

let print_fast fast =
  SMap.iter fast begin fun x (funs, classes) ->
    Printf.printf "File: %s\n" x;
    SSet.iter funs (Printf.printf "  Fun %s\n");
    SSet.iter classes (Printf.printf "  Class %s\n");
  end;
  Printf.printf "\n";
  flush stdout;
  ()

let debug_print_path_set genv name set =
  ServerDebug.log genv begin fun () ->
    let open Hh_json in
    let files = Relative_path.Set.fold set ~init:[] ~f:begin fun k acc ->
      JSON_String (Relative_path.suffix k) :: acc
    end in
    JSON_Object [
      "type", JSON_String "incremental_files";
      "name", JSON_String name;
      "files", JSON_Array files;
    ]
  end

let debug_print_fast_keys genv name fast =
  ServerDebug.log genv begin fun () ->
    let open Hh_json in
    let files = Relative_path.Map.fold fast ~init:[] ~f:begin fun k _v acc ->
      JSON_String (Relative_path.suffix k) :: acc
    end in
    let decls = Relative_path.Map.fold fast ~init:[] ~f:begin fun _k v acc ->
      let {FileInfo.n_funs; n_classes; n_types; n_consts} = v in
      let prepend_json_strings decls acc =
        SSet.fold decls ~init:acc ~f:(fun n acc -> JSON_String n :: acc) in
      let acc = prepend_json_strings n_funs acc in
      let acc = prepend_json_strings n_classes acc in
      let acc = prepend_json_strings n_types acc in
      let acc = prepend_json_strings n_consts acc in
      acc
    end in
    JSON_Object [
      "type", JSON_String "incremental_files";
      "name", JSON_String name;
      "files", JSON_Array files;
      "decls", JSON_Array decls;
    ]
  end

(*****************************************************************************)
(* Given a set of Ast.id list produce a SSet.t (got rid of the positions)    *)
(*****************************************************************************)

let set_of_idl l =
  List.fold_left l ~f:(fun acc (_, x) -> SSet.add acc x) ~init:SSet.empty

(*****************************************************************************)
(* We want add all the declarations that were present in a file *before* the
 * current modification. The scenario:
 * File foo.php was defining the class A.
 * The user gets rid of class A (in foo.php)
 * In general, the type-checker determines what must be re-declared or
 * re-typechecked, by comparing the old and the new type-definitions.
 * That's why we are adding the 'old' definitions to the file.
 * In this case, the redecl phase (typing/typing_redecl_service.ml) is going
 * to compare the 'old' definition of A with the new one. It will realize that
 * the new one is missing, and go ahead and retype everything that depends
 * on A.
 * Without a call to add_old_decls, the class A wouldn't appear anywhere,
 * and we wouldn't realize that we have to re-check the types that depend
 * on A.
 *)
(*****************************************************************************)

let add_old_decls old_files_info fast =
  Relative_path.Map.fold fast ~f:begin fun filename info_names acc ->
    match Relative_path.Map.get old_files_info filename with
    | None -> acc
    | Some old_info ->
      let old_info_names = FileInfo.simplify old_info in
      let info_names = FileInfo.merge_names old_info_names info_names in
      Relative_path.Map.add acc ~key:filename ~data:info_names
  end ~init:fast

let reparse_infos files_info fast =
  Relative_path.Map.fold fast ~f:begin fun x _y acc ->
    try
      let info = Relative_path.Map.find_unsafe files_info x in
      Relative_path.Map.add acc ~key:x ~data:info
    with Not_found -> acc
  end ~init:Relative_path.Map.empty

(*****************************************************************************)
(* Removes the names that were defined in the files *)
(*****************************************************************************)

let remove_decls env fast_parsed =
  Relative_path.Map.iter fast_parsed begin fun fn _ ->
    match Relative_path.Map.get env.files_info fn with
    | None -> ()
    | Some {FileInfo.
             funs = funl;
             classes = classel;
             typedefs = typel;
             consts = constl;
             file_mode = _;
             comments = _;
           } ->
      let funs = set_of_idl funl in
      let classes = set_of_idl classel in
      let typedefs = set_of_idl typel in
      let consts = set_of_idl constl in
      NamingGlobal.remove_decls ~funs ~classes ~typedefs ~consts
  end

(*****************************************************************************)
(* Parses the set of modified files *)
(*****************************************************************************)

(* Even when we remove an IDE file that failed after parsing stage, it might
 * appear again in later stages - we need to filter it every time we extend
 * the set of files to process *)
let remove_failed_parsing fast ~stop_at_errors env failed_parsing =
  if stop_at_errors then Relative_path.Map.filter fast
    ~f:(fun k _ -> not @@ Relative_path.(Set.mem failed_parsing k &&
                                         Set.mem env.editor_open_files k))
  else fast

let parsing genv env to_check ~stop_at_errors =

  let ide_files, disk_files  =
    Relative_path.Set.partition (Relative_path.Set.mem env.editor_open_files)
      to_check in

  File_heap.FileHeap.remove_batch disk_files;
  Parser_heap.ParserHeap.remove_batch disk_files;
  Fixmes.HH_FIXMES.remove_batch disk_files;
  if stop_at_errors then begin
    File_heap.FileHeap.LocalChanges.push_stack ();
    Parser_heap.ParserHeap.LocalChanges.push_stack ();
    Fixmes.HH_FIXMES.LocalChanges.push_stack ();
  end;
  (* Do not remove ide files from file heap *)
  Parser_heap.ParserHeap.remove_batch ide_files;
  Fixmes.HH_FIXMES.remove_batch ide_files;
  HackSearchService.MasterApi.clear_shared_memory to_check;
  SharedMem.collect `gentle;
  let get_next = MultiWorker.next
    genv.workers (Relative_path.Set.elements disk_files) in
  let (fast, errors, failed_parsing) as res =
    Parsing_service.go genv.workers ide_files ~get_next env.popt in

  SearchServiceRunner.update_fileinfo_map fast;
  (* During integration tests, we want to pretend that search is run
    synchronously *)
  if SearchServiceRunner.should_run_completely genv
    then SearchServiceRunner.run_completely genv;

  if stop_at_errors then begin
    (* Revert changes and ignore results for IDE files that failed parsing *)
    let ide_failed_parsing =
      Relative_path.Set.inter failed_parsing ide_files in
    let fast =
      remove_failed_parsing fast stop_at_errors env ide_failed_parsing in
    let ide_success_parsing =
      Relative_path.Set.diff ide_files ide_failed_parsing in

    File_heap.FileHeap.LocalChanges.revert_batch failed_parsing;
    Parser_heap.ParserHeap.LocalChanges.revert_batch ide_failed_parsing;
    Fixmes.HH_FIXMES.LocalChanges.revert_batch ide_failed_parsing;

    File_heap.FileHeap.LocalChanges.commit_batch ide_success_parsing;
    Parser_heap.ParserHeap.LocalChanges.commit_batch ide_success_parsing;
    Fixmes.HH_FIXMES.LocalChanges.commit_batch ide_success_parsing;
    Parser_heap.ParserHeap.LocalChanges.commit_batch disk_files;
    Fixmes.HH_FIXMES.LocalChanges.commit_batch disk_files;

    File_heap.FileHeap.LocalChanges.pop_stack ();
    Parser_heap.ParserHeap.LocalChanges.pop_stack ();
    Fixmes.HH_FIXMES.LocalChanges.pop_stack ();
    (fast, errors, failed_parsing)
  end else res

(*****************************************************************************)
(* At any given point in time, we want to know what each file defines.
 * The datastructure that maintains this information is called file_info.
 * This code updates the file information.
 *)
(*****************************************************************************)

let update_file_info env fast_parsed =
  Typing_deps.update_files fast_parsed;
  let files_info = Relative_path.Map.union fast_parsed env.files_info in
  files_info

(*****************************************************************************)
(* Defining the global naming environment.
 * Defines an environment with the names of all the globals (classes/funs).
 *)
(*****************************************************************************)

let declare_names env fast_parsed =
  (* We need to do naming phase for files that failed naming before, even
   * if they were not re-parsed in this iteration, so we are extending
   * fast_parsed with them. *)
  let fast_parsed = Relative_path.Set.fold env.failed_naming
    ~init:fast_parsed
    ~f:begin fun k acc ->
      match Relative_path.Map.get acc k with
      | Some _ -> acc (* the file was re-parsed already *)
      | None ->
        (* The file was not re-parsed, so it's correct to look up its contents
         * in (old) env. *)
        match Relative_path.Map.get env.files_info k with
        | None -> acc (* this should not happen - failed_naming should be
                         a subset of keys in files_info *)
        | Some v -> Relative_path.Map.add acc k v
    end
  in
  remove_decls env fast_parsed;
  let errorl, failed_naming =
    Relative_path.Map.fold fast_parsed ~f:begin fun k v (errorl, failed) ->
      let errorl', failed'= NamingGlobal.ndecl_file env.tcopt k v in
      let errorl = Errors.merge errorl' errorl in
      let failed = Relative_path.Set.union failed' failed in
      errorl, failed
    end ~init:(Errors.empty, Relative_path.Set.empty) in
  let fast = FileInfo.simplify_fast fast_parsed in
  errorl, failed_naming, fast

let diff_set_and_map_keys set map =
  Relative_path.Map.fold map
    ~init:set
    ~f:(fun k _ acc  -> Relative_path.Set.remove acc k)

let union_set_and_map_keys set map =
  Relative_path.Map.fold map
    ~init:set
    ~f:(fun k _ acc  -> Relative_path.Set.add acc k)

(*****************************************************************************)
(* Function called after parsing, does nothing by default. *)
(*****************************************************************************)

let hook_after_parsing = ref None

(*****************************************************************************)
(* Where the action is! *)
(*****************************************************************************)

module type CheckKindType = sig
  (* Parsing treats files open in IDE and files coming from disk differently:
  *
  * - for IDE files, we need to look up their contents in the map in env,
  *     instead of reading from disk (duh)
  * - we parse IDE files in master process (to avoid passing env to the
  *     workers)
  * - to make the IDE more responsive, we try to shortcut the typechecking at
  *   the parsing level if there were parsing errors
  *)
  val get_files_to_parse :
    ServerEnv.env ->
    Relative_path.Set.t * bool
    (* files to parse, should we stop if there are parsing errors *)

  val get_defs_to_redecl :
     parsing_defs:FileInfo.fast ->
     files_info:FileInfo.t Relative_path.Map.t ->
     env:ServerEnv.env ->
     FileInfo.fast

  (* Returns a tuple: files to redecl now, files to redecl later *)
  val get_defs_to_redecl_phase2 :
    decl_defs:FileInfo.fast ->
    files_info:FileInfo.t Relative_path.Map.t ->
    to_redecl_phase2:Relative_path.Set.t ->
    env:ServerEnv.env ->
    FileInfo.fast * FileInfo.fast

  val get_to_recheck2_approximation :
    to_redecl_phase2_deps:Typing_deps.DepSet.t ->
    env:ServerEnv.env ->
    Relative_path.Set.t

  (* Which files to typecheck, based on results of declaration phase *)
  val get_defs_to_recheck :
    phase_2_decl_defs:FileInfo.fast ->
    files_info:FileInfo.t Relative_path.Map.t ->
    to_recheck:Relative_path.Set.t ->
    env:ServerEnv.env ->
    FileInfo.fast * Relative_path.Set.t

  (* Update the global state based on resuts of all phases *)
  val get_new_env :
    old_env:ServerEnv.env ->
    files_info:FileInfo.t Relative_path.Map.t ->
    errorl:Errors.t ->
    failed_parsing:Relative_path.Set.t ->
    failed_naming:Relative_path.Set.t ->
    failed_decl:Relative_path.Set.t ->
    failed_check:Relative_path.Set.t ->
    needs_phase2_redecl:Relative_path.Set.t ->
    lazy_check_later:Relative_path.Set.t ->
    diag_subscribe:Diagnostic_subscription.t option ->
    ServerEnv.env
end

module FullCheckKind : CheckKindType = struct
  let get_files_to_parse env =
    let files_to_parse = Relative_path.Set.(
      env.ide_needs_parsing |> union
      env.disk_needs_parsing |> union
      env.failed_parsing |> union
      (* Full_check reconstructs error list from the scratch, so it always
       * rechecks all the files that had errors (env.failed_parsing).
       * But we don't store which IDE files had errors, so let's add all of them
       * here. *)
      env.editor_open_files
    ) in
    files_to_parse, false

  let get_defs_to_redecl ~parsing_defs ~files_info ~env =
     extend_fast parsing_defs files_info env.failed_decl

  let get_defs_to_redecl_phase2 ~decl_defs ~files_info ~to_redecl_phase2 ~env =
    let fast = extend_fast decl_defs files_info to_redecl_phase2 in
    (* Add decl fanout that was delayed by previous lazy checks to phase 2 *)
    let fast = extend_fast fast files_info env.needs_phase2_redecl in
    fast, Relative_path.Map.empty

  let get_to_recheck2_approximation ~to_redecl_phase2_deps:_ ~env:_ =
    (* Full check is computing to_recheck2 set accurately, so there is no need
     * to approximate anything *)
    Relative_path.Set.empty

  let get_defs_to_recheck ~phase_2_decl_defs ~files_info ~to_recheck ~env =
    let to_recheck = Relative_path.Set.union env.failed_decl to_recheck in
    let to_recheck = Relative_path.Set.union env.failed_check to_recheck in
    let to_recheck = Relative_path.Set.union env.needs_recheck to_recheck in
    extend_fast phase_2_decl_defs files_info to_recheck, Relative_path.Set.empty

  let get_new_env
      ~old_env
      ~files_info
      ~errorl
      ~failed_parsing
      ~failed_naming
      ~failed_decl
      ~failed_check
      ~needs_phase2_redecl:_
      ~lazy_check_later:_
      ~diag_subscribe =
    {
      files_info;
      tcopt = old_env.tcopt;
      popt = old_env.popt;
      errorl = errorl;
      failed_parsing;
      failed_naming;
      failed_decl;
      failed_check;
      persistent_client = old_env.persistent_client;
      ide_idle = old_env.ide_idle;
      last_command_time = old_env.last_command_time;
      last_notifier_check_time = old_env.last_notifier_check_time;
      last_idle_job_time = old_env.last_idle_job_time;
      editor_open_files = old_env.editor_open_files;
      ide_needs_parsing = Relative_path.Set.empty;
      disk_needs_parsing = Relative_path.Set.empty;
      needs_phase2_redecl = Relative_path.Set.empty;
      needs_recheck = Relative_path.Set.empty;
      needs_full_check = false;
      diag_subscribe;
      recent_recheck_loop_stats = old_env.recent_recheck_loop_stats;
    }
end

module LazyCheckKind : CheckKindType = struct
  let get_files_to_parse env =
    (* Approximate failed_* sets for IDE files by taking all files with IDE
     * errors and treating them as if they were in failed_parsing *)
    let failed_in_ide = match env.diag_subscribe with
      | Some ds ->  Diagnostic_subscription.files_with_errors_in_ide ds
      | None -> Relative_path.Set.empty
    in

    let files_to_parse = Relative_path.Set.(
      failed_in_ide |> union
      env.ide_needs_parsing
    ) in
    (* Skip the disk updates, process the IDE updates *)
    files_to_parse, true

  let get_defs_to_redecl ~parsing_defs ~files_info:_ ~env:_ =
      (* We don't need to add env.failed_decl here because lazy check doesn't
      * try to update the global error list *)
      parsing_defs

  let get_defs_to_redecl_phase2
      ~decl_defs ~files_info ~to_redecl_phase2 ~env:_ =
    (* Do phase2 only for IDE files, delay the fanout until next full check *)
    decl_defs, extend_fast decl_defs files_info to_redecl_phase2

  let get_related_files dep =
    Typing_deps.get_ideps_from_hash dep |> Typing_deps.get_files

  let has_errors_in_ide env = match env.diag_subscribe with
    | Some ds ->  Diagnostic_subscription.file_has_errors_in_ide ds
    | None -> (fun _ -> false)

  let is_ide_file env x =
    Relative_path.Set.mem env.editor_open_files x || has_errors_in_ide env x

  let get_to_recheck2_approximation ~to_redecl_phase2_deps ~env =
    (* We didn't do the full fan-out from to_redecl_phase2_deps, so the
     * to_recheck2 set might not be complete. We would recompute it during next
     * full check, but if it contains files open in editor, we would like to
     * recheck them sooner than that. We approximate it by taking all the
     * possible dependencies of dependencies and preemptively rechecking them
     * if they are open in the editor *)
    Typing_deps.DepSet.fold to_redecl_phase2_deps
      ~init:Relative_path.Set.empty
      ~f:(fun x acc -> Relative_path.Set.union acc @@ get_related_files x)
    |> Relative_path.Set.filter ~f:(is_ide_file env)

  let get_defs_to_recheck ~phase_2_decl_defs ~files_info ~to_recheck ~env =
    let to_recheck_now, to_recheck_later =
      Relative_path.Set.partition (is_ide_file env) to_recheck in
    extend_fast phase_2_decl_defs files_info to_recheck_now, to_recheck_later

  let get_new_env
      ~old_env
      ~files_info
      ~errorl:_
      ~failed_parsing:_
      ~failed_naming
      ~failed_decl:_
      ~failed_check:_
      ~needs_phase2_redecl
      ~lazy_check_later
      ~diag_subscribe =
    let needs_recheck =
      Relative_path.Set.union old_env.needs_recheck lazy_check_later in
    { old_env with
       files_info;
       failed_naming;
       ide_needs_parsing = Relative_path.Set.empty;
       needs_phase2_redecl;
       needs_recheck;
       needs_full_check = true;
       diag_subscribe;
     }
end

module Make: functor(CheckKind:CheckKindType) -> sig
  val type_check :
    ServerEnv.genv ->
    ServerEnv.env ->
    ServerEnv.env * int * int
end = functor(CheckKind:CheckKindType) -> struct

  let get_defs fast =
    Relative_path.Map.fold fast ~f:begin fun _ names1 names2 ->
      FileInfo.merge_names names1 names2
    end ~init:FileInfo.empty_names

  let get_oldified_defs env =
    Relative_path.Set.fold env.needs_phase2_redecl ~f:begin fun path acc ->
      match Relative_path.Map.get env.files_info path with
      | None -> acc
      | Some names -> FileInfo.(merge_names (simplify names) acc)
    end ~init:FileInfo.empty_names

  let type_check genv env =
    let start_t = Unix.gettimeofday () in
    let t = start_t in
    (* Files in env.needs_decl contain declarations which were not finished.
     * They were only oldified, but we didn't run phase2 redeclarations for them
     * which would compute new versions, compare them with old ones and remove
     * the old ones. We'll use oldified_defs sets to track what is in the old
     * heap as we progress with redeclaration *)
    let oldified_defs = get_oldified_defs env in

    let files_to_parse, stop_at_errors = CheckKind.get_files_to_parse env in

    let reparse_count = Relative_path.Set.cardinal files_to_parse in
    Hh_logger.log "Files to recompute: %d" reparse_count;
    if reparse_count == 1 then
      files_to_parse |>
      Relative_path.Set.choose |>
      Relative_path.to_absolute |>
      Hh_logger.log "Filename: %s";

    (* PARSING *)

    debug_print_path_set genv "files_to_parse" files_to_parse;
    let fast_parsed, errorl, failed_parsing =
      parsing genv env files_to_parse ~stop_at_errors in
    let hs = SharedMem.heap_size () in
    Hh_logger.log "Heap size: %d" hs;
    HackEventLogger.parsing_end t hs ~parsed_count:reparse_count;
    let t = Hh_logger.log_duration "Parsing" t in

    (* UPDATE FILE INFO *)
    let old_env = env in
    let files_info = update_file_info env fast_parsed in
    HackEventLogger.updating_deps_end t;
    let t = Hh_logger.log_duration "Updating deps" t in

    (* BUILDING AUTOLOADMAP *)
    Option.iter !hook_after_parsing begin fun f ->
      f genv { env with files_info }
    end;
    HackEventLogger.parsing_hook_end t;
    let t = Hh_logger.log_duration "Parsing Hook" t in

    (* NAMING *)
    let errorl', failed_naming, fast = declare_names env fast_parsed in
    (* failed_naming can be a superset of keys in fast - see comment in
     * NamingGlobal.ndecl_file *)
    let fast = extend_fast fast files_info failed_naming in

    (* COMPUTES WHAT MUST BE REDECLARED  *)
    let fast = CheckKind.get_defs_to_redecl fast files_info env in
    let fast = add_old_decls env.files_info fast in
    let fast = remove_failed_parsing fast stop_at_errors env failed_parsing in
    let errorl = Errors.merge errorl' errorl in

    HackEventLogger.naming_end t;
    let t = Hh_logger.log_duration "Naming" t in

    let bucket_size = genv.local_config.SLC.type_decl_bucket_size in
    debug_print_fast_keys genv "to_redecl_phase1" fast;
    let defs_to_redecl = get_defs fast in
    let _, _, to_redecl_phase2_deps, to_recheck1 =
      Decl_redecl_service.redo_type_decl
        ~bucket_size genv.workers env.tcopt oldified_defs fast defs_to_redecl in

    (* Things that were redeclared are no longer in old heap, so we substract
     * defs_ro_redecl from oldified_defs *)
    let oldified_defs =
      snd @@ Decl_utils.split_defs oldified_defs defs_to_redecl in
    let to_redecl_phase2 = Typing_deps.get_files to_redecl_phase2_deps in
    let to_recheck1 = Typing_deps.get_files to_recheck1 in
    let hs = SharedMem.heap_size () in
    Hh_logger.log "Heap size: %d" hs;
    HackEventLogger.first_redecl_end t hs;
    let t = Hh_logger.log_duration "Determining changes" t in

    (* DECLARING TYPES: Phase2 *)
    let fast_redecl_phase2_now, lazy_decl_later =
      CheckKind.get_defs_to_redecl_phase2 fast files_info to_redecl_phase2 env
    in

    let fast_redecl_phase2_now = remove_failed_parsing
      fast_redecl_phase2_now stop_at_errors env failed_parsing in

    debug_print_fast_keys genv "to_redecl_phase2" fast_redecl_phase2_now;
    debug_print_fast_keys genv "lazy_decl_later" lazy_decl_later;

    let defs_to_oldify = get_defs lazy_decl_later in
    Decl_redecl_service.oldify_type_decl ~bucket_size
      genv.workers files_info oldified_defs defs_to_oldify;
    let oldified_defs = FileInfo.merge_names oldified_defs defs_to_oldify in

    let defs_to_redecl_phase2 = get_defs fast_redecl_phase2_now in
    let errorl', failed_decl, _to_redecl2, to_recheck2 =
      Decl_redecl_service.redo_type_decl ~bucket_size genv.workers
        env.tcopt oldified_defs fast_redecl_phase2_now defs_to_redecl_phase2 in

    let needs_phase2_redecl = diff_set_and_map_keys
      (* Redaclaration delayed before and now. *)
      (union_set_and_map_keys env.needs_phase2_redecl lazy_decl_later)
      (* Redeclarations completed now. *)
      fast_redecl_phase2_now
    in

    let to_recheck2 = Typing_deps.get_files to_recheck2 in
    let to_recheck2 = Relative_path.Set.union to_recheck2
      (CheckKind.get_to_recheck2_approximation to_redecl_phase2_deps env) in
    let errorl = Errors.merge errorl' errorl in

    (* DECLARING TYPES: merging results of the 2 phases *)
    let fast = Relative_path.Map.union fast fast_redecl_phase2_now in
    let to_recheck = Relative_path.Set.union to_recheck1 to_recheck2 in
    let to_recheck = Relative_path.Set.union to_recheck to_redecl_phase2 in
    let hs = SharedMem.heap_size () in
    Hh_logger.log "Heap size: %d" hs;
    HackEventLogger.second_redecl_end t hs;
    let t = Hh_logger.log_duration "Type-decl" t in

    (* TYPE CHECKING *)
    let fast, lazy_check_later =
      CheckKind.get_defs_to_recheck fast files_info to_recheck env in
    let fast = remove_failed_parsing fast stop_at_errors env failed_parsing in
    ServerCheckpoint.process_updates fast;
    debug_print_fast_keys genv "to_recheck" fast;
    debug_print_path_set genv "lazy_check_later" lazy_check_later;
    let errorl', err_info =
      Typing_check_service.go genv.workers env.tcopt fast in
    let { Decl_service.
      errs = failed_check;
      lazy_decl_errs = lazy_decl_failed;
    } = err_info in
    let failed_decl = Relative_path.Set.union failed_decl lazy_decl_failed in
    let errorl', failed_check = match ServerArgs.ai_mode genv.options with
      | None -> errorl', failed_check
      | Some ai_opt ->
        let fast_infos = reparse_infos files_info fast in
        let ae, af = Ai.go_incremental
          Typing_check_utils.check_defs
          genv.workers fast_infos env.tcopt ai_opt in
        (Errors.merge errorl' ae),
        (Relative_path.Set.union af failed_check)
    in
    let errorl = Errors.merge errorl' errorl in

    let diag_subscribe = Option.map old_env.diag_subscribe
      ~f:(fun x -> Diagnostic_subscription.update
        x env.editor_open_files fast errorl) in

    let total_rechecked_count = Relative_path.Map.cardinal fast in
    HackEventLogger.type_check_end total_rechecked_count t;
    let t = Hh_logger.log_duration "Type-check" t in

    Hh_logger.log "Total: %f\n%!" (t -. start_t);
    ServerDebug.info genv "incremental_done";

    let new_env = CheckKind.get_new_env
      old_env
      files_info
      errorl
      failed_parsing
      failed_naming
      failed_decl
      failed_check
      needs_phase2_redecl
      lazy_check_later
      diag_subscribe
    in

    new_env, reparse_count, total_rechecked_count
end

let check_kind_to_string = function
  | Full_check -> "Full_check"
  | Lazy_check -> "Lazy_check"

module FC = Make(FullCheckKind)
module LC = Make(LazyCheckKind)

let type_check genv env kind =
  (match kind with
  | Lazy_check -> HackEventLogger.set_lazy_incremental ()
  | Full_check -> ());
  let check_kind = check_kind_to_string kind in
  HackEventLogger.with_check_kind check_kind @@ begin fun () ->
    Printf.eprintf "******************************************\n";
    Hh_logger.log "Check kind: %s" check_kind;
    match kind with
    | Lazy_check ->
      ServerBusyStatus.send env ServerCommandTypes.Doing_local_typecheck;
      let res = LC.type_check genv env in
      ServerBusyStatus.send env ServerCommandTypes.Done_local_typecheck;
      res
    | Full_check ->
      ServerBusyStatus.send env ServerCommandTypes.Doing_global_typecheck;
      let res = FC.type_check genv env in
      ServerBusyStatus.send env ServerCommandTypes.Done_global_typecheck;
      res
  end

let type_check genv env kind =
  ServerUtils.with_exit_on_exception @@ fun () ->
  type_check genv env kind

(*****************************************************************************)
(* Checks that the working directory is clean *)
(*****************************************************************************)

let check genv env =
  if !debug then begin
    Printf.printf "****************************************\n";
    Printf.printf "Start Check\n";
    flush stdout;
  end;
  type_check genv env
