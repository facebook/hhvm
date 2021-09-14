(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open ServerCheckUtils
open SearchServiceRunner
open ServerEnv
open Reordered_argument_collections
module SLC = ServerLocalConfig

module CheckKind = struct
  type t =
    | Lazy
    | Full

  let to_string = function
    | Full -> "Full_check"
    | Lazy -> "Lazy_check"

  let is_full_check = function
    | Full -> true
    | Lazy -> false
end

module CheckStats = struct
  type t = {
    reparse_count: int;
    total_rechecked_count: int;
    time_first_result: seconds_since_epoch option;
  }

  (** Update field [time_first_result] if given timestamp is the
      first sent result. *)
  let record_result_sent_ts stats new_result_sent_ts =
    {
      stats with
      time_first_result =
        Option.first_some stats.time_first_result new_result_sent_ts;
    }
end

let shallow_decl_enabled (ctx : Provider_context.t) =
  TypecheckerOptions.shallow_class_decl (Provider_context.get_tcopt ctx)

let use_direct_decl_parser (ctx : Provider_context.t) =
  TypecheckerOptions.use_direct_decl_parser (Provider_context.get_tcopt ctx)

(*****************************************************************************)
(* Debugging *)
(*****************************************************************************)

let print_defs prefix defs =
  List.iter defs ~f:(fun (_, fname) -> Printf.printf "  %s %s\n" prefix fname)

let print_fast_pos fast_pos =
  SMap.iter fast_pos ~f:(fun x (funs, classes) ->
      Printf.printf "File: %s\n" x;
      print_defs "Fun" funs;
      print_defs "Class" classes);
  Printf.printf "\n";
  Out_channel.flush stdout;
  ()

let print_fast fast =
  SMap.iter fast ~f:(fun x (funs, classes) ->
      Printf.printf "File: %s\n" x;
      SSet.iter funs ~f:(Printf.printf "  Fun %s\n");
      SSet.iter classes ~f:(Printf.printf "  Class %s\n"));
  Printf.printf "\n";
  Out_channel.flush stdout;
  ()

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

let add_old_decls old_naming_table fast =
  Relative_path.Map.fold
    fast
    ~f:
      begin
        fun filename info_names acc ->
        match Naming_table.get_file_info old_naming_table filename with
        | None -> acc
        | Some old_info ->
          let old_info_names = FileInfo.simplify old_info in
          let info_names = FileInfo.merge_names old_info_names info_names in
          Relative_path.Map.add acc ~key:filename ~data:info_names
      end
    ~init:fast

(*****************************************************************************)
(* Removes the names that were defined in the files *)
(*****************************************************************************)

let remove_decls env fast_parsed =
  Relative_path.Map.iter fast_parsed ~f:(fun fn _ ->
      match Naming_table.get_file_info env.naming_table fn with
      | None -> ()
      | Some
          {
            FileInfo.funs;
            classes;
            record_defs;
            typedefs;
            consts;
            file_mode = _;
            comments = _;
            hash = _;
          } ->
        (* we use [snd] to strip away positions *)
        let snd (_, x, _) = x in
        Naming_global.remove_decls
          ~backend:(Provider_backend.get ())
          ~funs:(List.map funs ~f:snd)
          ~classes:(List.map classes ~f:snd)
          ~record_defs:(List.map record_defs ~f:snd)
          ~typedefs:(List.map typedefs ~f:snd)
          ~consts:(List.map consts ~f:snd))

(* If the only things that would change about file analysis are positions,
 * we're not going to recheck it, and positions in its error list might
 * become stale. Look if any of those positions refer to files that have
 * actually changed and add them to files to recheck. *)
let get_files_with_stale_errors
    ~(* Set of files that were reparsed (so their ASTs and positions
      * in them could have changed. *)
    reparsed
    ~(* A subset of files which errors we want to update, or None if we want
      * to update entire error list. *)
    filter
    ~(* Consider errors only coming from those phases *)
    phases
    ~(* Current global error list *)
    errors
    ~ctx =
  let fold =
    match filter with
    | None ->
      fun phase init f ->
        (* Looking at global files *)
        Errors.fold_errors errors ~phase ~init ~f:(fun source error acc ->
            f source error acc)
    | Some files ->
      fun phase init f ->
        (* Looking only at subset of files *)
        Relative_path.Set.fold files ~init ~f:(fun file acc ->
            Errors.fold_errors_in
              errors
              ~file
              ~phase
              ~init:acc
              ~f:(fun error acc -> f file error acc))
  in
  List.fold phases ~init:Relative_path.Set.empty ~f:(fun acc phase ->
      fold phase acc (fun source error acc ->
          if
            List.exists (Errors.to_list_ error) ~f:(fun e ->
                Relative_path.Set.mem
                  reparsed
                  (fst e |> Naming_provider.resolve_position ctx |> Pos.filename))
          then
            Relative_path.Set.add acc source
          else
            acc))

(*****************************************************************************)
(* Parses the set of modified files *)
(*****************************************************************************)

let push_errors env ~rechecked ~phase new_errors =
  if TypecheckerOptions.stream_errors env.tcopt then
    let (diagnostic_pusher, time_errors_pushed) =
      Diagnostic_pusher.push_new_errors
        env.diagnostic_pusher
        ~rechecked
        new_errors
        ~phase
    in
    let env = { env with diagnostic_pusher } in
    (env, time_errors_pushed)
  else
    (env, None)

let erase_errors env = push_errors env Errors.empty

let push_and_accumulate_errors :
    env * Errors.t ->
    rechecked:Relative_path.Set.t ->
    Errors.t ->
    phase:Errors.phase ->
    env * Errors.t * seconds_since_epoch option =
 fun (env, errors_acc) ~rechecked new_errors ~phase ->
  let (env, time_errors_pushed) =
    push_errors env new_errors ~rechecked ~phase
  in
  let errors =
    Errors.incremental_update ~old:errors_acc ~new_:new_errors ~rechecked phase
  in
  (env, errors, time_errors_pushed)

(** Remove files which failed parsing from [fast] files and
    discard any previous errors they had in [omitted_phases] *)
let wont_do_failed_parsing
    fast ~stop_at_errors ~omitted_phases env failed_parsing =
  if stop_at_errors then
    let (env, time_first_erased) =
      List.fold
        omitted_phases
        ~init:(env, None)
        ~f:(fun (env, time_first_erased) phase ->
          let (env, time_erased) =
            erase_errors env ~rechecked:failed_parsing ~phase
          in
          let time_first_erased =
            Option.first_some time_first_erased time_erased
          in
          (env, time_first_erased))
    in
    let fast =
      Relative_path.Set.filter fast ~f:(fun k ->
          not
          @@ Relative_path.(
               Set.mem failed_parsing k && Set.mem env.editor_open_files k))
    in
    (env, fast, time_first_erased)
  else
    (env, fast, None)

let parsing genv env to_check ~stop_at_errors profiling =
  let (ide_files, disk_files) =
    Relative_path.Set.partition
      (Relative_path.Set.mem env.editor_open_files)
      to_check
  in
  File_provider.remove_batch disk_files;
  Ast_provider.remove_batch disk_files;
  Fixme_provider.remove_batch disk_files;

  if stop_at_errors then (
    File_provider.local_changes_push_sharedmem_stack ();
    Ast_provider.local_changes_push_sharedmem_stack ();
    Fixme_provider.local_changes_push_sharedmem_stack ()
  );

  (* Do not remove ide files from file heap *)
  Ast_provider.remove_batch ide_files;
  Fixme_provider.remove_batch ide_files;

  let env =
    {
      env with
      local_symbol_table =
        SymbolIndexCore.remove_files
          ~sienv:env.local_symbol_table
          ~paths:to_check;
    }
  in
  SharedMem.GC.collect `gentle;
  let max_size =
    if genv.local_config.ServerLocalConfig.small_buckets_for_dirty_names then
      Some 1
    else
      None
  in
  let get_next =
    MultiWorker.next
      genv.workers
      (Relative_path.Set.elements disk_files)
      ?max_size
  in
  let ctx = Provider_utils.ctx_from_server_env env in
  let (fast, errors, failed_parsing) =
    CgroupProfiler.collect_cgroup_stats ~profiling ~stage:"parsing" @@ fun () ->
    Parsing_service.go ctx genv.workers ide_files ~get_next env.popt ~trace:true
  in

  SearchServiceRunner.update_fileinfo_map
    (Naming_table.create fast)
    ~source:SearchUtils.TypeChecker;

  (* During integration tests, we want to pretend that search is run
     synchronously *)
  let ctx = Provider_utils.ctx_from_server_env env in
  let env =
    {
      env with
      local_symbol_table =
        (let sie = env.local_symbol_table in
         if
           SearchServiceRunner.should_run_completely
             genv
             sie.SearchUtils.sie_provider
         then
           SearchServiceRunner.run_completely ctx sie
         else
           sie);
    }
  in

  if stop_at_errors then (
    File_provider.local_changes_commit_batch ide_files;
    Ast_provider.local_changes_commit_batch ide_files;
    Fixme_provider.local_changes_commit_batch ide_files;
    Ast_provider.local_changes_commit_batch disk_files;
    Fixme_provider.local_changes_commit_batch disk_files;

    File_provider.local_changes_pop_sharedmem_stack ();
    Ast_provider.local_changes_pop_sharedmem_stack ();
    Fixme_provider.local_changes_pop_sharedmem_stack ();

    (env, fast, errors, failed_parsing)
  ) else
    (env, fast, errors, failed_parsing)

(*****************************************************************************)
(* At any given point in time, we want to know what each file defines.
 * The datastructure that maintains this information is called file_info.
 * This code updates the file information.
 *)
(*****************************************************************************)

let update_naming_table env fast_parsed profiling =
  let naming_table =
    CgroupProfiler.collect_cgroup_stats ~profiling ~stage:"update_deps"
    @@ fun () ->
    let ctx = Provider_utils.ctx_from_server_env env in
    let deps_mode = Provider_context.get_deps_mode ctx in
    Relative_path.Map.iter fast_parsed ~f:(fun file fi ->
        let old = Naming_table.get_file_info env.naming_table file in
        Typing_deps.Files.update_file deps_mode file fi ~old);
    Naming_table.update_many env.naming_table fast_parsed
  in
  naming_table

(*****************************************************************************)
(* Defining the global naming environment.
 * Defines an environment with the names of all the globals (classes/funs).
 *)
(*****************************************************************************)

let declare_names env fast_parsed =
  remove_decls env fast_parsed;
  let ctx = Provider_utils.ctx_from_server_env env in
  let (errorl, failed_naming) =
    Relative_path.Map.fold
      fast_parsed
      ~f:
        begin
          fun k v (errorl, failed) ->
          let (errorl', failed') =
            Naming_global.ndecl_file_error_if_already_bound ctx k v
          in
          let errorl = Errors.merge errorl' errorl in
          let failed = Relative_path.Set.union failed' failed in
          (errorl, failed)
        end
      ~init:(Errors.empty, Relative_path.Set.empty)
  in
  let fast = Naming_table.to_fast (Naming_table.create fast_parsed) in
  (errorl, failed_naming, fast)

let diff_set_and_map_keys set map =
  Relative_path.Map.fold map ~init:set ~f:(fun k _ acc ->
      Relative_path.Set.remove acc k)

let union_set_and_map_keys set map =
  Relative_path.Map.fold map ~init:set ~f:(fun k _ acc ->
      Relative_path.Set.add acc k)

let get_interrupt_config genv env =
  MultiThreadedCall.{ handlers = env.interrupt_handlers genv; env }

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
  val get_files_to_parse : ServerEnv.env -> Relative_path.Set.t * bool

  (* files to parse, should we stop if there are parsing errors *)

  val get_defs_to_redecl :
    reparsed:Relative_path.Set.t ->
    env:ServerEnv.env ->
    ctx:Provider_context.t ->
    Relative_path.Set.t

  (* Returns a tuple: files to redecl now, files to redecl later *)
  val get_defs_to_redecl_phase2 :
    ServerEnv.genv ->
    decl_defs:Naming_table.fast ->
    naming_table:Naming_table.t ->
    to_redecl_phase2:Relative_path.Set.t ->
    env:ServerEnv.env ->
    Naming_table.fast * Naming_table.fast

  val get_to_recheck2_approximation :
    to_redecl_phase2_deps:Typing_deps.DepSet.t ->
    env:ServerEnv.env ->
    ctx:Provider_context.t ->
    Relative_path.Set.t

  (* Which files to typecheck, based on results of declaration phase *)
  val get_defs_to_recheck :
    reparsed:Relative_path.Set.t ->
    phase_2_decl_defs:Naming_table.fast ->
    to_recheck:Relative_path.Set.t ->
    env:ServerEnv.env ->
    ctx:Provider_context.t ->
    enable_type_check_filter_files:bool ->
    Relative_path.Set.t * Relative_path.Set.t

  (* Update the global state based on resuts of parsing, naming and decl *)
  val get_env_after_decl :
    old_env:ServerEnv.env ->
    naming_table:Naming_table.t ->
    failed_naming:Relative_path.Set.t ->
    ServerEnv.env

  (* Update the global state based on resuts of typing *)
  val get_env_after_typing :
    old_env:ServerEnv.env ->
    errorl:Errors.t ->
    needs_phase2_redecl:Relative_path.Set.t ->
    needs_recheck:Relative_path.Set.t ->
    diag_subscribe:Diagnostic_subscription.t option ->
    ServerEnv.env

  val is_full : bool
end

module FullCheckKind : CheckKindType = struct
  let get_files_to_parse env =
    let files_to_parse =
      Relative_path.Set.(env.ide_needs_parsing |> union env.disk_needs_parsing)
    in
    (files_to_parse, false)

  let get_defs_to_redecl ~reparsed ~(env : env) ~ctx =
    (* Besides the files that actually changed, we want to also redeclare
     * those that have decl errors referring to files that were
     * reparsed, since positions in those errors can be now stale *)
    get_files_with_stale_errors
      ~reparsed
      ~filter:None
      ~phases:[Errors.Decl]
      ~errors:env.errorl
      ~ctx

  let get_defs_to_redecl_phase2
      genv ~decl_defs ~naming_table ~to_redecl_phase2 ~env =
    let fast = extend_fast genv decl_defs naming_table to_redecl_phase2 in
    (* Add decl fanout that was delayed by previous lazy checks to phase 2 *)
    let fast = extend_fast genv fast naming_table env.needs_phase2_redecl in
    (fast, Relative_path.Map.empty)

  let get_to_recheck2_approximation ~to_redecl_phase2_deps:_ ~env:_ ~ctx:_ =
    (* Full check is computing to_recheck2 set accurately, so there is no need
     * to approximate anything *)
    Relative_path.Set.empty

  let get_defs_to_recheck
      ~reparsed
      ~phase_2_decl_defs
      ~to_recheck
      ~env
      ~ctx
      ~enable_type_check_filter_files =
    (* If the user has enabled a custom file filter, we want to only
     * type check files that pass the filter *)
    let to_recheck =
      if enable_type_check_filter_files then
        ServerCheckUtils.user_filter_type_check_files
          ~to_recheck
          ~reparsed
          ~is_ide_file:(Relative_path.Set.mem env.editor_open_files)
      else
        to_recheck
    in
    (* Besides the files that actually changed, we want to also recheck
     * those that have typing errors referring to files that were
     * reparsed, since positions in those errors can be now stale.
     *)
    let stale_errors =
      get_files_with_stale_errors
        ~reparsed
        ~filter:None
        ~phases:[Errors.Decl; Errors.Typing]
        ~errors:env.errorl
        ~ctx
    in
    let to_recheck = Relative_path.Set.union stale_errors to_recheck in
    let to_recheck = Relative_path.Set.union env.needs_recheck to_recheck in
    let to_recheck =
      Relative_path.Set.union
        (Relative_path.Set.of_list (Relative_path.Map.keys phase_2_decl_defs))
        to_recheck
    in
    let to_recheck =
      if Relative_path.Set.is_empty env.remote_execution_files then
        to_recheck
      else
        env.remote_execution_files
    in
    (to_recheck, Relative_path.Set.empty)

  let get_env_after_decl ~old_env ~naming_table ~failed_naming =
    {
      old_env with
      naming_table;
      failed_naming;
      ide_needs_parsing = Relative_path.Set.empty;
      disk_needs_parsing = Relative_path.Set.empty;
    }

  let get_env_after_typing
      ~old_env ~errorl ~needs_phase2_redecl:_ ~needs_recheck ~diag_subscribe =
    let (full_check_status, remote) =
      if Relative_path.Set.is_empty needs_recheck then
        (Full_check_done, false)
      else
        (old_env.full_check_status, old_env.remote)
    in
    let why_needed_full_init =
      match old_env.init_env.why_needed_full_init with
      | Some why_needed_full_init
        when not (is_full_check_done full_check_status) ->
        Some why_needed_full_init
      | _ -> None
    in
    {
      old_env with
      errorl;
      needs_phase2_redecl = Relative_path.Set.empty;
      ServerEnv.remote_execution_files = Relative_path.Set.empty;
      needs_recheck;
      full_check_status;
      remote;
      init_env = { old_env.init_env with why_needed_full_init };
      diag_subscribe;
    }

  let is_full = true
end

module LazyCheckKind : CheckKindType = struct
  let get_files_to_parse env = (env.ide_needs_parsing, true)

  let ide_diagnosed_files env =
    match env.diag_subscribe with
    | Some ds -> Diagnostic_subscription.diagnosed_files ds
    | None -> Relative_path.Set.empty

  let is_ide_file env x =
    Relative_path.Set.mem (ide_diagnosed_files env) x
    || Relative_path.Set.mem env.editor_open_files x

  let get_defs_to_redecl ~reparsed ~env ~ctx =
    (* Same as FullCheckKind.get_defs_to_redecl, but we limit returned set only
     * to files that are relevant to IDE *)
    get_files_with_stale_errors
      ~reparsed
      ~filter:(Some (ide_diagnosed_files env))
      ~phases:[Errors.Decl]
      ~errors:env.errorl
      ~ctx

  let get_defs_to_redecl_phase2
      genv ~decl_defs ~naming_table ~to_redecl_phase2 ~env =
    (* Do phase2 only for IDE files, delay the fanout until next full check *)
    let (to_redecl_phase2_now, to_redecl_phase2_later) =
      Relative_path.Set.partition (is_ide_file env) to_redecl_phase2
    in
    ( extend_fast genv decl_defs naming_table to_redecl_phase2_now,
      extend_fast genv decl_defs naming_table to_redecl_phase2_later )

  let get_related_files ctx dep =
    let deps_mode = Provider_context.get_deps_mode ctx in
    Typing_deps.(
      add_typing_deps deps_mode (DepSet.singleton deps_mode dep)
      |> Files.get_files)

  let get_to_recheck2_approximation ~to_redecl_phase2_deps ~env ~ctx =
    (* We didn't do the full fan-out from to_redecl_phase2_deps, so the
     * to_recheck2 set might not be complete. We would recompute it during next
     * full check, but if it contains files open in editor, we would like to
     * recheck them sooner than that. We approximate it by taking all the
     * possible dependencies of dependencies and preemptively rechecking them
     * if they are open in the editor *)
    if Typing_deps.DepSet.cardinal to_redecl_phase2_deps > 1000 then
      (* inspecting tons of dependencies would take more time that just
         * rechecking all relevant files. *)
      Relative_path.Set.union env.editor_open_files (ide_diagnosed_files env)
    else
      Typing_deps.DepSet.fold
        to_redecl_phase2_deps
        ~init:Relative_path.Set.empty
        ~f:(fun x acc -> Relative_path.Set.union acc @@ get_related_files ctx x)
      |> Relative_path.Set.filter ~f:(is_ide_file env)

  let get_defs_to_recheck
      ~reparsed
      ~phase_2_decl_defs
      ~to_recheck
      ~env
      ~ctx
      ~enable_type_check_filter_files =
    (* If the user has enabled a custom file filter, we want to only
     * type check files that pass the filter. As such, we don't want
     * to add unwanted files to the "type check later"-queue *)
    let to_recheck =
      if enable_type_check_filter_files then
        ServerCheckUtils.user_filter_type_check_files
          ~to_recheck
          ~reparsed
          ~is_ide_file:(is_ide_file env)
      else
        to_recheck
    in
    (* Same as FullCheckKind.get_defs_to_recheck, but we limit returned set only
     * to files that are relevant to IDE *)
    let stale_errors =
      get_files_with_stale_errors
        ~ctx
        ~reparsed
        ~filter:(Some (ide_diagnosed_files env))
        ~phases:[Errors.Decl; Errors.Typing]
        ~errors:env.errorl
    in
    let to_recheck = Relative_path.Set.union to_recheck stale_errors in
    let (to_recheck_now, to_recheck_later) =
      Relative_path.Set.partition (is_ide_file env) to_recheck
    in
    let to_recheck_now =
      Relative_path.Set.union
        (Relative_path.Set.of_list (Relative_path.Map.keys phase_2_decl_defs))
        to_recheck_now
    in
    if Relative_path.Set.is_empty env.remote_execution_files then
      (to_recheck_now, to_recheck_later)
    else
      (env.remote_execution_files, Relative_path.Set.empty)

  let get_env_after_decl ~old_env ~naming_table ~failed_naming =
    {
      old_env with
      naming_table;
      failed_naming;
      ide_needs_parsing = Relative_path.Set.empty;
    }

  let get_env_after_typing
      ~old_env ~errorl ~needs_phase2_redecl ~needs_recheck ~diag_subscribe =
    (* If it was started, it's still started, otherwise it needs starting *)
    let full_check_status =
      match old_env.full_check_status with
      | Full_check_started -> Full_check_started
      | _ -> Full_check_needed
    in
    {
      old_env with
      errorl;
      ide_needs_parsing = Relative_path.Set.empty;
      ServerEnv.remote_execution_files = Relative_path.Set.empty;
      needs_phase2_redecl;
      needs_recheck;
      full_check_status;
      diag_subscribe;
    }

  let is_full = false
end

module Make : functor (_ : CheckKindType) -> sig
  val type_check_core :
    ServerEnv.genv ->
    ServerEnv.env ->
    float ->
    CgroupProfiler.Profiling.t ->
    ServerEnv.env * CheckStats.t * Telemetry.t
end =
functor
  (CheckKind : CheckKindType)
  ->
  struct
    let get_defs fast =
      Relative_path.Map.fold
        fast
        ~f:
          begin
            fun _ names1 names2 ->
            FileInfo.merge_names names1 names2
          end
        ~init:FileInfo.empty_names

    let get_oldified_defs env =
      Relative_path.Set.fold
        env.needs_phase2_redecl
        ~f:
          begin
            fun path acc ->
            match Naming_table.get_file_info env.naming_table path with
            | None -> acc
            | Some names -> FileInfo.(merge_names (simplify names) acc)
          end
        ~init:FileInfo.empty_names

    let get_classes naming_table path =
      match Naming_table.get_file_info naming_table path with
      | None -> SSet.empty
      | Some info ->
        List.fold
          info.FileInfo.classes
          ~init:SSet.empty
          ~f:(fun acc (_, cid, _) -> SSet.add acc cid)

    let clear_failed_parsing env errors failed_parsing =
      (* In most cases, set of files processed in a phase is a superset
       * of files from previous phase - i.e if we run decl on file A, we'll also
       * run its typing.
       * In few cases we might choose not to run further stages for files that
       * failed parsing (see ~stop_at_errors). We need to manually clear out
       * error lists for those files. *)
      Relative_path.Set.fold
        failed_parsing
        ~init:(env, errors)
        ~f:(fun path acc ->
          let path = Relative_path.Set.singleton path in
          List.fold_left
            Errors.[Naming; Decl; Typing]
            ~init:acc
            ~f:(fun acc phase ->
              let (env, errors, _) =
                push_and_accumulate_errors
                  acc
                  ~rechecked:path
                  Errors.empty
                  ~phase
              in
              (env, errors)))

    type parsing_result = {
      parse_errors: Errors.t;
      failed_parsing: Relative_path.Set.t;
      fast_parsed: FileInfo.t Relative_path.Map.t;
      time_errors_pushed: seconds_since_epoch option;
    }

    let do_parsing
        (genv : genv)
        (env : env)
        ~(errors : Errors.t)
        ~(files_to_parse : Relative_path.Set.t)
        ~(stop_at_errors : bool)
        ~(profiling : CgroupProfiler.Profiling.t) :
        ServerEnv.env * parsing_result =
      let (env, fast_parsed, errorl, failed_parsing) =
        parsing genv env files_to_parse ~stop_at_errors profiling
      in
      let (env, errors, time_errors_pushed) =
        push_and_accumulate_errors
          (env, errors)
          errorl
          ~rechecked:files_to_parse
          ~phase:Errors.Parsing
      in
      let (env, errors) = clear_failed_parsing env errors failed_parsing in
      ( env,
        {
          parse_errors = errors;
          failed_parsing;
          fast_parsed;
          time_errors_pushed;
        } )

    type naming_result = {
      errors_after_naming: Errors.t;
      failed_naming: Relative_path.Set.t;
      fast: Naming_table.fast;
      time_errors_pushed: seconds_since_epoch option;
    }

    let do_naming
        (genv : genv)
        (env : env)
        (ctx : Provider_context.t)
        ~(errors : Errors.t)
        ~(fast_parsed : FileInfo.t Relative_path.Map.t)
        ~(naming_table : Naming_table.t)
        ~(files_to_parse : Relative_path.Set.t)
        ~(profiling : CgroupProfiler.Profiling.t) : naming_result =
      CgroupProfiler.collect_cgroup_stats ~profiling ~stage:"naming"
      @@ fun () ->
      let (errorl', failed_naming, fast) = declare_names env fast_parsed in
      let (env, errors, time_errors_pushed) =
        push_and_accumulate_errors
          (env, errors)
          errorl'
          ~rechecked:
            (fast |> Relative_path.Map.keys |> Relative_path.Set.of_list)
          ~phase:Errors.Naming
      in
      (* failed_naming can be a superset of keys in fast - see comment in
       * Naming_global.ndecl_file *)
      let fast = extend_fast genv fast naming_table failed_naming in
      (* COMPUTES WHAT MUST BE REDECLARED  *)
      let failed_decl =
        CheckKind.get_defs_to_redecl ~reparsed:files_to_parse ~env ~ctx
      in
      let fast = extend_fast genv fast naming_table failed_decl in
      let fast = add_old_decls env.naming_table fast in
      { errors_after_naming = errors; failed_naming; fast; time_errors_pushed }

    type redecl_phase1_result = {
      changed: Typing_deps.DepSet.t;
      oldified_defs: FileInfo.names;
      to_recheck1: Relative_path.Set.t;
      to_recheck1_deps: Typing_deps.DepSet.t;
      to_redecl_phase2_deps: Typing_deps.DepSet.t;
      old_decl_missing_count: int;
    }

    let do_redecl_phase1
        (genv : genv)
        (env : env)
        ~(fast : FileInfo.names Relative_path.Map.t)
        ~(naming_table : Naming_table.t)
        ~(oldified_defs : FileInfo.names)
        ~(profiling : CgroupProfiler.Profiling.t) : redecl_phase1_result =
      let bucket_size = genv.local_config.SLC.type_decl_bucket_size in
      let defs_to_redecl = get_defs fast in
      let ctx = Provider_utils.ctx_from_server_env env in
      let {
        Decl_redecl_service.errors = _;
        changed;
        to_redecl = to_redecl_phase2_deps;
        to_recheck = to_recheck1_deps;
        old_decl_missing_count;
      } =
        CgroupProfiler.collect_cgroup_stats ~profiling ~stage:"redecl phase 1"
        @@ fun () ->
        Decl_redecl_service.redo_type_decl
          ~bucket_size
          ctx
          genv.workers
          (get_classes naming_table)
          ~previously_oldified_defs:oldified_defs
          ~defs:fast
      in
      (* Things that were redeclared are no longer in old heap, so we substract
       * defs_to_redecl from oldified_defs *)
      let oldified_defs =
        snd @@ Decl_utils.split_defs oldified_defs defs_to_redecl
      in
      let to_recheck1 = Typing_deps.Files.get_files to_recheck1_deps in
      {
        changed;
        oldified_defs;
        to_recheck1;
        to_recheck1_deps;
        to_redecl_phase2_deps;
        old_decl_missing_count;
      }

    type redecl_phase2_result = {
      errors_after_phase2: Errors.t;
      needs_phase2_redecl: Relative_path.Set.t;
      to_recheck2: Relative_path.Set.t;
      to_recheck2_deps: Typing_deps.DepSet.t;
          (** Note that the intuitive property
            [get_files to_recheck2_deps == to_recheck2] does NOT hold. That's
            because in a lazy check, we might add files open in the IDE *)
      time_errors_pushed: seconds_since_epoch option;
      old_decl_missing_count: int;
    }

    let do_redecl_phase2
        (genv : genv)
        (env : env)
        ~(errors : Errors.t)
        ~(fast_redecl_phase2_now : FileInfo.names Relative_path.Map.t)
        ~(naming_table : Naming_table.t)
        ~(lazy_decl_later : FileInfo.names Relative_path.Map.t)
        ~(oldified_defs : FileInfo.names)
        ~(to_redecl_phase2_deps : Typing_deps.DepSet.t)
        ~(profiling : CgroupProfiler.Profiling.t) : redecl_phase2_result =
      let ctx = Provider_utils.ctx_from_server_env env in
      let bucket_size = genv.local_config.SLC.type_decl_bucket_size in
      let defs_to_oldify = get_defs lazy_decl_later in
      Decl_redecl_service.oldify_type_decl
        ctx
        ~bucket_size
        genv.workers
        (get_classes naming_table)
        ~previously_oldified_defs:oldified_defs
        ~defs:defs_to_oldify;
      let oldified_defs = FileInfo.merge_names oldified_defs defs_to_oldify in
      let {
        Decl_redecl_service.errors = errorl';
        changed = _;
        to_redecl = _;
        to_recheck = to_recheck2_deps;
        old_decl_missing_count;
      } =
        CgroupProfiler.collect_cgroup_stats ~profiling ~stage:"redecl phase 2"
        @@ fun () ->
        Decl_redecl_service.redo_type_decl
          ~bucket_size
          ctx
          genv.workers
          (get_classes naming_table)
          ~previously_oldified_defs:oldified_defs
          ~defs:fast_redecl_phase2_now
      in
      let (env, errors, time_errors_pushed) =
        push_and_accumulate_errors
          (env, errors)
          errorl'
          ~rechecked:
            (fast_redecl_phase2_now
            |> Relative_path.Map.keys
            |> Relative_path.Set.of_list)
          ~phase:Errors.Decl
      in
      let needs_phase2_redecl =
        diff_set_and_map_keys
          (* Redeclaration delayed before and now. *)
          (union_set_and_map_keys env.needs_phase2_redecl lazy_decl_later)
          (* Redeclarations completed now. *)
          fast_redecl_phase2_now
      in
      let to_recheck2 = Typing_deps.Files.get_files to_recheck2_deps in
      let to_recheck2 =
        Relative_path.Set.union
          to_recheck2
          (CheckKind.get_to_recheck2_approximation
             ~to_redecl_phase2_deps
             ~env
             ~ctx)
      in
      {
        errors_after_phase2 = errors;
        needs_phase2_redecl;
        to_recheck2;
        to_recheck2_deps;
        time_errors_pushed;
        old_decl_missing_count;
      }

    (** Merge the results of the two redecl phases. *)
    let merge_redecl_results
        ~(fast : FileInfo.names Relative_path.Map.t)
        ~(fast_redecl_phase2_now : FileInfo.names Relative_path.Map.t)
        ~(to_recheck1 : Relative_path.Set.t)
        ~(to_recheck1_deps : Typing_deps.DepSet.t)
        ~(to_recheck2 : Relative_path.Set.t)
        ~(to_recheck2_deps : Typing_deps.DepSet.t)
        ~(to_redecl_phase2 : Relative_path.Set.t)
        ~(to_redecl_phase2_deps : Typing_deps.DepSet.t) :
        Naming_table.fast * Relative_path.Set.t * Typing_deps.DepSet.t lazy_t =
      let fast = Relative_path.Map.union fast fast_redecl_phase2_now in
      (* I want to make sure the way we compute to_recheck in terms of files
       * vs. in terms of toplevel symbol hashes does not diverge. Therefore,
       * instead of duplicating the unioning logic, I put the logic in a
       * polymorphic helper function *)
      let calc_to_recheck
          (type a)
          ~(union : a -> a -> a)
          ~(to_recheck1 : a)
          ~(to_recheck2 : a)
          ~(to_redecl_phase2 : a) =
        let to_recheck = union to_recheck1 to_recheck2 in
        let to_recheck = union to_recheck to_redecl_phase2 in
        to_recheck
      in
      let to_recheck =
        calc_to_recheck
          ~union:Relative_path.Set.union
          ~to_recheck1
          ~to_recheck2
          ~to_redecl_phase2
      in
      let to_recheck_deps =
        lazy
          (calc_to_recheck
             ~union:Typing_deps.DepSet.union
             ~to_recheck1:to_recheck1_deps
             ~to_recheck2:to_recheck2_deps
             ~to_redecl_phase2:to_redecl_phase2_deps)
      in
      (fast, to_recheck, to_recheck_deps)

    type type_checking_result = {
      env: ServerEnv.env;
      diag_subscribe: Diagnostic_subscription.t option;
      errors: Errors.t;
      telemetry: Telemetry.t;
      adhoc_profiling: Adhoc_profiler.CallTree.t;
      files_checked: Relative_path.Set.t;
      full_check_done: bool;
      needs_recheck: Relative_path.Set.t;
      total_rechecked_count: int;
      time_first_typing_error: seconds option;
    }

    let do_type_checking
        (genv : genv)
        (env : env)
        (capture_snapshot : ServerRecheckCapture.snapshot)
        ~(errors : Errors.t)
        ~(files_to_check : Relative_path.Set.t)
        ~(files_to_parse : Relative_path.Set.t)
        ~(lazy_check_later : Relative_path.Set.t)
        ~(profiling : CgroupProfiler.Profiling.t) : type_checking_result =
      let telemetry = Telemetry.create () in
      if Relative_path.(Set.mem files_to_check default) then
        Hh_logger.log "WARNING: rechecking defintion in a dummy file";
      let dynamic_view_files =
        if ServerDynamicView.dynamic_view_on () then
          env.editor_open_files
        else
          Relative_path.Set.empty
      in
      let interrupt = get_interrupt_config genv env in
      let memory_cap =
        genv.local_config.ServerLocalConfig.max_typechecker_worker_memory_mb
      in
      let longlived_workers =
        genv.local_config.ServerLocalConfig.longlived_workers
      in

      let ( errorl',
            telemetry,
            adhoc_profiling,
            env,
            cancelled,
            time_first_typing_error ) =
        let ctx = Provider_utils.ctx_from_server_env env in
        CgroupProfiler.collect_cgroup_stats ~profiling ~stage:"type check"
        @@ fun () ->
        let ( ( env,
                {
                  Typing_check_service.errors = errorl;
                  delegate_state;
                  telemetry;
                  adhoc_profiling;
                  diagnostic_pusher =
                    (diagnostic_pusher, time_first_typing_error);
                } ),
              cancelled ) =
          Typing_check_service.go_with_interrupt
            ~diagnostic_pusher:env.ServerEnv.diagnostic_pusher
            ctx
            genv.workers
            env.typing_service.delegate_state
            telemetry
            dynamic_view_files
            (files_to_check |> Relative_path.Set.elements)
            ~interrupt
            ~memory_cap
            ~longlived_workers
            ~remote_execution:env.ServerEnv.remote_execution
            ~check_info:(get_check_info genv env)
            ~profiling
        in
        let env =
          {
            env with
            diagnostic_pusher =
              Option.value diagnostic_pusher ~default:env.diagnostic_pusher;
            typing_service = { env.typing_service with delegate_state };
          }
        in
        ( errorl,
          telemetry,
          adhoc_profiling,
          env,
          cancelled,
          time_first_typing_error )
      in

      (* Add new things that need to be rechecked *)
      let needs_recheck =
        Relative_path.Set.union env.needs_recheck lazy_check_later
      in
      (* Remove things that were cancelled from things we started rechecking... *)
      let (files_to_check, needs_recheck) =
        List.fold
          cancelled
          ~init:(files_to_check, needs_recheck)
          ~f:(fun (files_to_check, needs_recheck) path ->
            ( Relative_path.Set.remove files_to_check path,
              Relative_path.Set.add needs_recheck path ))
      in
      (* ... leaving only things that we actually checked, and which can be
       * removed from needs_recheck *)
      let needs_recheck = Relative_path.Set.diff needs_recheck files_to_check in
      let needs_recheck =
        if Relative_path.Set.is_empty env.remote_execution_files then
          needs_recheck
        else
          (* We never need to recheck in RE single mode *)
          Relative_path.Set.empty
      in
      let errors =
        Errors.(
          incremental_update
            ~old:errors
            ~new_:errorl'
            ~rechecked:files_to_check
            Typing)
      in
      let (env, _future) : ServerEnv.env * string Future.t option =
        ServerRecheckCapture.update_after_recheck
          genv
          env
          capture_snapshot
          ~changed_files:files_to_parse
          ~cancelled_files:(Relative_path.Set.of_list cancelled)
          ~rechecked_files:files_to_check
          ~recheck_errors:errorl'
          ~all_errors:errors
      in
      let full_check_done =
        CheckKind.is_full && Relative_path.Set.is_empty needs_recheck
      in
      let diag_subscribe =
        Option.map env.diag_subscribe ~f:(fun x ->
            Diagnostic_subscription.update
              x
              ~priority_files:env.editor_open_files
              ~global_errors:errors
              ~full_check_done)
      in

      let total_rechecked_count = Relative_path.Set.cardinal files_to_check in
      {
        env;
        diag_subscribe;
        errors;
        telemetry;
        adhoc_profiling;
        files_checked = files_to_check;
        full_check_done;
        needs_recheck;
        total_rechecked_count;
        time_first_typing_error;
      }

    let type_check_core genv env start_time profiling =
      let t = Unix.gettimeofday () in
      (* `start_time` is when the recheck_loop started and includes preliminaries like
       * reading about file-change notifications and communicating with client.
       * We record all our telemetry uniformally with respect to this start.
       * `t` is legacy, used for ad-hoc duration reporting within this function.
       * For the following, env.int_env.why_needed_full_init is set to Some by
       * ServerLazyInit, and we include it here, and then it's subsequently
       * set to None at the end of this method by the call to [get_env_after_typing].
       * Thus, if it's present here, it means the typecheck we're about to do is
       * the initial one of a lazy init. *)
      let telemetry =
        Telemetry.create ()
        |> Telemetry.object_opt
             ~key:"init"
             ~value:env.ServerEnv.init_env.ServerEnv.why_needed_full_init
      in
      let time_first_error = None in
      let env =
        if CheckKind.is_full then
          { env with full_check_status = Full_check_started }
        else
          env
      in
      let env =
        if Relative_path.Set.is_empty env.remote_execution_files then
          env
        else
          (* We always clear past errors in RE mode *)
          { env with errorl = Errors.empty }
      in
      (* Files in env.needs_decl contain declarations which were not finished.
       * They were only oldified, but we didn't run phase2 redeclarations for them
       * which would compute new versions, compare them with old ones and remove
       * the old ones. We'll use oldified_defs sets to track what is in the old
       * heap as we progress with redeclaration *)
      let oldified_defs = get_oldified_defs env in
      let (files_to_parse, stop_at_errors) = CheckKind.get_files_to_parse env in
      (* We need to do naming phase for files that failed naming in a previous cycle.
       * "Failed_naming" comes from duplicate name errors; the idea is that a change
       * deletes one of the duplicates, well, this change should cause us to re-parse
       * and re-process the remaining duplicate so it can be properly recorded in the
       * forward and reverse naming tables (even though the remaining duplicate
       * didn't itself change). *)
      if not (Relative_path.Set.is_empty env.failed_naming) then
        Hh_logger.log
          "Also reparsing these files with failed naming: %s"
          (Relative_path.Set.elements env.failed_naming
          |> List.map ~f:Relative_path.suffix
          |> String.concat ~sep:" ");
      let files_to_parse =
        Relative_path.Set.union files_to_parse env.failed_naming
      in

      let reparse_count = Relative_path.Set.cardinal files_to_parse in
      if reparse_count = 1 then
        files_to_parse
        |> Relative_path.Set.choose
        |> Relative_path.to_absolute
        |> Hh_logger.log "Processing changes to 1 file: %s"
      else
        Hh_logger.log "Processing changes to %d files" reparse_count;

      let telemetry =
        if CheckKind.is_full then (
          let redecl_count =
            Relative_path.Set.cardinal env.needs_phase2_redecl
          in
          let check_count = Relative_path.Set.cardinal env.needs_recheck in
          Hh_logger.log
            "Processing deferred type decl for %d file(s), deferred typechecking for %d file(s)"
            redecl_count
            check_count;
          telemetry
          |> Telemetry.int_ ~key:"redecl_count" ~value:redecl_count
          |> Telemetry.int_ ~key:"check_count" ~value:check_count
        ) else
          telemetry
      in

      (* PARSING ***************************************************************)
      ServerProgress.send_progress
        ~include_in_logs:false
        "parsing %d files"
        reparse_count;
      let logstring = Printf.sprintf "Parsing %d files" reparse_count in
      Hh_logger.log "Begin %s" logstring;

      (* Parse all changed files. This clears the file contents cache prior
          to parsing. *)
      let parse_t = Unix.gettimeofday () in
      let telemetry =
        Telemetry.duration telemetry ~key:"parse_start" ~start_time
      in
      let errors = env.errorl in
      let ( env,
            {
              parse_errors = errors;
              failed_parsing;
              fast_parsed;
              time_errors_pushed;
            } ) =
        do_parsing genv env ~errors ~files_to_parse ~stop_at_errors ~profiling
      in
      let time_first_error =
        Option.first_some time_first_error time_errors_pushed
      in

      (* DIRECT DECL PARSER *****************)
      (* We should use the direct decl parser results for the naming phase.
         The direct decl parser behaviour is different from the legacy parser:
         parser errors in the body might be ignored! As such, if we use the
         legacy parser result to update the naming table, we might erroneously
         remove entries! When typechecking files using that entry, erroneous
         "Name unbound" errors will be reported. Those are unrecoverable,
         because decl diffing won't catch any changes (the direct decl parser
         returns the same result with or without parsing errors), and no fanout
         will be computed.

         TODO(hverr, jakebailey): It would be great if we could only do a
         direct-decl-parse (rather than doing a direct-decl-parse pass after the
         AST-parse pass). If we removed the AST-parse pass, it would probably be
         best to remove the failed_parsing tracking too (since the direct decl
         parser won't emit the complete set of parse errors). Then we'd attempt
         to typecheck those files, and we could make sure that we emit parsing
         errors during the typechecking step.*)
      let ctx = Provider_utils.ctx_from_server_env env in
      let fast_parsed =
        if
          TypecheckerOptions.use_direct_decl_in_tc_loop
            (Provider_context.get_tcopt ctx)
          && use_direct_decl_parser ctx
        then
          let get_next =
            MultiWorker.next
              genv.workers
              (Relative_path.Set.elements files_to_parse)
          in
          Direct_decl_service.go ctx genv.workers get_next
        else
          fast_parsed
      in

      let hs = SharedMem.SMTelemetry.heap_size () in
      let telemetry =
        telemetry
        |> Telemetry.duration ~key:"parse_end" ~start_time
        |> Telemetry.int_ ~key:"parse_end_heap_size" ~value:hs
        |> Telemetry.int_ ~key:"parse_count" ~value:reparse_count
      in
      HackEventLogger.parsing_end_for_typecheck t hs ~parsed_count:reparse_count;
      let t = Hh_logger.log_duration logstring t in
      Hh_logger.log "Heap size: %d" hs;

      (* UPDATE NAMING TABLES **************************************************)
      let logstring = "Updating deps" in
      Hh_logger.log "Begin %s" logstring;
      let telemetry =
        Telemetry.duration telemetry ~key:"naming_update_start" ~start_time
      in

      (* Update the naming_table, which is a map from filename to the names of
         toplevel symbols declared in that file. Also, update Typing_deps' table,
         which is a map from toplevel symbol hash (Dep.t) to filename. *)
      let naming_table = update_naming_table env fast_parsed profiling in
      HackEventLogger.updating_deps_end
        ~count:reparse_count
        ~desc:"serverTypeCheck"
        ~start_t:t;
      let t = Hh_logger.log_duration logstring t in
      let telemetry =
        Telemetry.duration telemetry ~key:"naming_update_end" ~start_time
      in

      (* NAMING ****************************************************************)
      ServerProgress.send_progress
        ~include_in_logs:false
        "resolving symbol references";
      let logstring = "Naming" in
      Hh_logger.log "Begin %s" logstring;
      let telemetry =
        Telemetry.duration telemetry ~key:"naming_start" ~start_time
      in

      let deptable_unlocked =
        Typing_deps.allow_dependency_table_reads env.deps_mode true
      in
      (* Run Naming_global, updating the reverse naming table (which maps the names
         of toplevel symbols to the files in which they were declared) in shared
         memory. Does not run Naming itself (which converts an AST to a NAST by
         assigning unique identifiers to locals, among other things). The Naming
         module is something of a historical artifact and is slated for removal,
         but for now, it is run immediately before typechecking. *)
      let {
        errors_after_naming = errors;
        failed_naming;
        fast;
        time_errors_pushed;
      } =
        do_naming
          genv
          env
          ctx
          ~errors
          ~fast_parsed
          ~naming_table
          ~files_to_parse
          ~profiling
      in
      let time_first_error =
        Option.first_some time_first_error time_errors_pushed
      in

      let heap_size = SharedMem.SMTelemetry.heap_size () in
      Hh_logger.log "Heap size: %d" heap_size;
      HackEventLogger.naming_end ~count:reparse_count t heap_size;
      let t = Hh_logger.log_duration logstring t in
      let telemetry =
        telemetry
        |> Telemetry.duration ~key:"naming_end" ~start_time
        |> Telemetry.int_ ~key:"naming_end_heap_size" ~value:heap_size
      in

      (* REDECL PHASE 1 ********************************************************)
      ServerProgress.send_progress ~include_in_logs:false "determining changes";
      let count = Relative_path.Map.cardinal fast in
      let logstring =
        Printf.sprintf "Type declaration (phase 1) for %d files" count
      in
      Hh_logger.log "Begin %s" logstring;
      Hh_logger.log
        "(Recomputing type declarations in changed files and determining immediate typechecking fanout)";
      let telemetry =
        telemetry
        |> Telemetry.duration ~key:"redecl1_start" ~start_time
        |> Telemetry.int_ ~key:"redecl1_file_count" ~value:count
      in

      (* Do phase 1 of redeclaration. Here we compare the old and new versions of
         the declarations defined in all changed files, and collect the set of
         files which need to be re-typechecked as a consequence of those changes,
         as well as the set of files whose folded class declarations must be
         recomputed as a consequence of those changes (in phase 2).

         When shallow_class_decl is enabled, there is no need to do phase 2--the
         only source of class information needing recomputing is linearizations.
         These are invalidated by Decl_redecl_service.redo_type_decl in phase 1,
         and are lazily recomputed as needed. *)
      let {
        changed;
        oldified_defs;
        to_recheck1;
        to_recheck1_deps;
        to_redecl_phase2_deps;
        old_decl_missing_count;
      } =
        do_redecl_phase1 genv env ~fast ~naming_table ~oldified_defs ~profiling
      in
      let telemetry =
        telemetry
        |> Telemetry.int_
             ~key:"phase_1_old_decl_missing_count"
             ~value:old_decl_missing_count
      in

      let to_redecl_phase2 =
        Typing_deps.Files.get_files to_redecl_phase2_deps
      in
      let hs = SharedMem.SMTelemetry.heap_size () in
      HackEventLogger.first_redecl_end t hs;
      let t = Hh_logger.log_duration logstring t in
      Hh_logger.log "Heap size: %d" hs;
      let telemetry =
        telemetry
        |> Telemetry.duration ~key:"redecl1_end" ~start_time
        |> Telemetry.int_ ~key:"redecl1_end_heap_size" ~value:hs
      in

      ServerRevisionTracker.decl_changed
        genv.ServerEnv.local_config
        (Relative_path.Set.cardinal to_redecl_phase2);
      let telemetry =
        Telemetry.duration
          telemetry
          ~key:"revtrack1_decl_changed_end"
          ~start_time
      in

      (* REDECL PHASE 2 ********************************************************)

      (* For a full typecheck, we want to redeclare everything that needs
         redeclaration (either because it was invalidated in phase 1, or because
         it was invalidated by a previous lazy check). For a lazy check, we only
         want to redeclare files open in the IDE, leaving everything else to be
         lazily redeclared later. In either case, there's no need to attempt to
         redeclare definitions in files with parse errors.

         When shallow_class_decl is enabled, there is no need to do phase 2. *)
      let telemetry =
        Telemetry.duration telemetry ~key:"redecl2_now_start" ~start_time
      in
      let (fast_redecl_phase2_now, lazy_decl_later) =
        if shallow_decl_enabled ctx then
          (Relative_path.Map.empty, Relative_path.Map.empty)
        else
          CheckKind.get_defs_to_redecl_phase2
            genv
            ~decl_defs:fast
            ~naming_table
            ~to_redecl_phase2
            ~env
      in
      let count = Relative_path.Map.cardinal fast_redecl_phase2_now in
      let telemetry =
        Telemetry.duration telemetry ~key:"redecl2_now_end" ~start_time
      in
      ServerProgress.send_progress
        ~include_in_logs:false
        "evaluating type declarations of %d files"
        count;
      let logstring =
        Printf.sprintf "Type declaration (phase 2) for %d files" count
      in
      Hh_logger.log "Begin %s" logstring;
      let telemetry =
        telemetry
        |> Telemetry.duration ~key:"redecl2_start" ~start_time
        |> Telemetry.int_ ~key:"redecl2_count_now" ~value:count
        |> Telemetry.int_
             ~key:"redecl2_count_later"
             ~value:(Relative_path.Map.cardinal lazy_decl_later)
        |> Telemetry.bool_ ~key:"shallow" ~value:(shallow_decl_enabled ctx)
        |> Telemetry.bool_
             ~key:"direct_decl"
             ~value:(use_direct_decl_parser ctx)
        |> Telemetry.bool_
             ~key:"ss_64bit"
             ~value:
               (Typing_deps_mode.is_64bit @@ Provider_context.get_deps_mode ctx)
      in

      if not (shallow_decl_enabled ctx) then (
        Hh_logger.log
          "(Recomputing type declarations for descendants of changed classes and determining full typechecking fanout)";
        Hh_logger.log
          "Invalidating (but not recomputing) declarations in %d files"
          (Relative_path.Map.cardinal lazy_decl_later)
      );

      (* Redeclare the set of files whose folded class decls needed to be
         recomputed as a result of phase 1. Collect the set of files which need to
         be re-typechecked because of changes between the old and new
         declarations. We need not collect a set of files to redeclare (again)
         because our to_redecl set from phase 1 included the transitive children
         of changed classes.

         When shallow_class_decl is enabled, there is no need to do phase 2. *)
      let {
        errors_after_phase2 = errors;
        needs_phase2_redecl;
        to_recheck2;
        to_recheck2_deps;
        time_errors_pushed;
        old_decl_missing_count;
      } =
        if shallow_decl_enabled ctx then
          {
            errors_after_phase2 = errors;
            needs_phase2_redecl = Relative_path.Set.empty;
            to_recheck2 = Relative_path.Set.empty;
            to_recheck2_deps = Typing_deps.DepSet.make env.deps_mode;
            time_errors_pushed = None;
            old_decl_missing_count = 0;
          }
        else
          do_redecl_phase2
            genv
            env
            ~errors
            ~fast_redecl_phase2_now
            ~naming_table
            ~lazy_decl_later
            ~oldified_defs
            ~to_redecl_phase2_deps
            ~profiling
      in
      let telemetry =
        telemetry
        |> Telemetry.int_
             ~key:"phase_2_old_decl_missing_count"
             ~value:old_decl_missing_count
      in
      let telemetry =
        Telemetry.duration telemetry ~key:"redecl2_end" ~start_time
      in
      let time_first_error =
        Option.first_some time_first_error time_errors_pushed
      in

      let (fast, to_recheck, to_recheck_deps) =
        merge_redecl_results
          ~fast
          ~fast_redecl_phase2_now
          ~to_recheck1
          ~to_recheck1_deps
          ~to_recheck2
          ~to_recheck2_deps
          ~to_redecl_phase2
          ~to_redecl_phase2_deps
      in
      let hs = SharedMem.SMTelemetry.heap_size () in
      HackEventLogger.second_redecl_end t hs;
      let t = Hh_logger.log_duration logstring t in
      Hh_logger.log "Heap size: %d" hs;
      let telemetry =
        telemetry
        |> Telemetry.duration ~key:"redecl2_end2_merge" ~start_time
        |> Telemetry.int_ ~key:"redecl2_end2_heap_size" ~value:hs
      in
      ServerRevisionTracker.typing_changed
        genv.local_config
        (Relative_path.Set.cardinal to_recheck);
      let telemetry =
        Telemetry.duration
          telemetry
          ~key:"revtrack2_typing_changed_end"
          ~start_time
      in

      (* we use lazy here to avoid expensive string generation when logging
       * is not enabled *)
      Hh_logger.log_lazy ~category:"fanout_information"
      @@ lazy
           Hh_json.(
             json_to_string
             @@ JSON_Object
                  [
                    ("tag", string_ "incremental_fanout");
                    ( "hashes",
                      array_
                        string_
                        Typing_deps.(
                          List.map ~f:Dep.to_hex_string
                          @@ DepSet.elements (Lazy.force to_recheck_deps)) );
                  ]);

      let env =
        CheckKind.get_env_after_decl ~old_env:env ~naming_table ~failed_naming
      in
      (* HANDLE PRECHECKED FILES AFTER LOCAL CHANGES ***************************)
      Hh_logger.log "Begin evaluating prechecked changes";
      let telemetry =
        Telemetry.duration telemetry ~key:"prechecked1_start" ~start_time
      in

      let (env, prechecked1_telemetry) =
        ServerPrecheckedFiles.update_after_local_changes
          genv
          env
          changed
          ~start_time
      in

      let t = Hh_logger.log_duration "Evaluating prechecked changes" t in
      let telemetry =
        telemetry
        |> Telemetry.duration ~key:"prechecked1_end" ~start_time
        |> Telemetry.object_ ~key:"prechecked1" ~value:prechecked1_telemetry
        |> Telemetry.int_
             ~key:"prechecked1_changed"
             ~value:(Typing_deps.DepSet.cardinal changed)
      in

      let (_ : bool) =
        Typing_deps.allow_dependency_table_reads env.deps_mode deptable_unlocked
      in

      (* Checking this before starting typechecking because we want to attribtue
       * big rechecks to rebases, even when restarting is disabled *)
      if
        genv.local_config.ServerLocalConfig.hg_aware_recheck_restart_threshold
        = 0
      then
        ServerRevisionTracker.check_blocking ();
      let telemetry =
        Telemetry.duration
          telemetry
          ~key:"revtrack3_check_blocking_end"
          ~start_time
      in

      (* TYPE CHECKING *********************************************************)
      let type_check_start_t = Unix.gettimeofday () in

      (* For a full check, typecheck everything which may be affected by the
         changes. For a lazy check, typecheck only the affected files which are
         open in the IDE, leaving other affected files to be lazily checked later.
         In either case, don't attempt to typecheck files with parse errors. *)
      let (files_to_check, lazy_check_later) =
        CheckKind.get_defs_to_recheck
          ~reparsed:files_to_parse
          ~phase_2_decl_defs:fast
          ~to_recheck
          ~env
          ~ctx
          ~enable_type_check_filter_files:
            genv.ServerEnv.local_config
              .ServerLocalConfig.enable_type_check_filter_files
      in
      let env =
        ServerRemoteUtils.start_delegate_if_needed
          env
          genv
          (Relative_path.Set.cardinal files_to_check)
          errors
      in
      let (env, files_to_check, time_erased_errors) =
        wont_do_failed_parsing
          files_to_check
          ~stop_at_errors
          ~omitted_phases:[Errors.Typing]
          env
          failed_parsing
      in
      let time_first_error =
        Option.first_some time_first_error time_erased_errors
      in
      let to_recheck_count = Relative_path.Set.cardinal files_to_check in
      (* The intent of capturing the snapshot here is to increase the likelihood
          of the state-on-disk being the same as what the parser saw *)
      let (env, capture_snapshot) =
        ServerRecheckCapture.update_before_recheck
          genv
          env
          ~to_recheck_count
          ~changed_files:files_to_parse
          ~parse_t
      in
      ServerProgress.send_progress
        ~include_in_logs:false
        "typechecking %d files"
        to_recheck_count;
      let logstring = Printf.sprintf "typechecking %d files" in
      Hh_logger.log "Begin %s" (logstring to_recheck_count);

      ServerCheckpoint.process_updates files_to_check;

      let telemetry =
        Telemetry.duration telemetry ~key:"typecheck_start" ~start_time
      in

      (* Typecheck all of the files we determined might need rechecking as a
         consequence of the changes (or, in a lazy check, the subset of those
         files which are open in an IDE buffer). *)
      let {
        env;
        diag_subscribe;
        errors;
        telemetry = typecheck_telemetry;
        adhoc_profiling;
        files_checked;
        full_check_done = _;
        needs_recheck;
        total_rechecked_count;
        time_first_typing_error;
      } =
        do_type_checking
          genv
          env
          capture_snapshot
          ~errors
          ~files_to_check
          ~files_to_parse
          ~lazy_check_later
          ~profiling
      in
      let time_first_error =
        Option.first_some time_first_error time_first_typing_error
      in

      let heap_size = SharedMem.SMTelemetry.heap_size () in
      Hh_logger.log "Heap size: %d" heap_size;

      let logstring =
        Printf.sprintf "Typechecked %d files" total_rechecked_count
      in
      let t = Hh_logger.log_duration logstring t in
      Hh_logger.log "Total: %f\n%!" (t -. start_time);
      let telemetry =
        telemetry
        |> Telemetry.duration ~key:"typecheck_end" ~start_time
        |> Telemetry.object_ ~key:"typecheck" ~value:typecheck_telemetry
        |> Telemetry.object_
             ~key:"hash"
             ~value:(ServerUtils.log_and_get_sharedmem_load_telemetry ())
        |> Telemetry.int_opt
             ~key:"depgraph_delta_num_edges"
             ~value:
               (Typing_deps.Telemetry.depgraph_delta_num_edges
                  (Provider_context.get_deps_mode ctx))
        |> Telemetry.int_ ~key:"typecheck_heap_size" ~value:heap_size
        |> Telemetry.int_
             ~key:"typecheck_to_recheck_count"
             ~value:to_recheck_count
        |> Telemetry.int_
             ~key:"typecheck_total_rechecked_count"
             ~value:total_rechecked_count
        |> Telemetry.int_
             ~key:"typecheck_files_checked"
             ~value:(Relative_path.Set.cardinal files_checked)
        |> Telemetry.int_
             ~key:"typecheck_lazy_check_later_count"
             ~value:(Relative_path.Set.cardinal lazy_check_later)
        |> Telemetry.int_opt
             ~key:"typecheck_mem_cap"
             ~value:
               genv.local_config
                 .ServerLocalConfig.max_typechecker_worker_memory_mb
        |> Telemetry.int_opt
             ~key:"typecheck_defer_decl_threshold"
             ~value:
               genv.local_config
                 .ServerLocalConfig.defer_class_declaration_threshold
        |> Telemetry.int_opt
             ~key:"typecheck_defer_mem_threshold"
             ~value:
               genv.local_config
                 .ServerLocalConfig.defer_class_memory_mb_threshold
        |> Telemetry.bool_
             ~key:"enable_type_check_filter_files"
             ~value:
               genv.local_config
                 .ServerLocalConfig.enable_type_check_filter_files
        |> Telemetry.bool_
             ~key:"typecheck_longlived_workers"
             ~value:genv.local_config.ServerLocalConfig.longlived_workers
      in

      (* INVALIDATE FILES (EXPERIMENTAL TYPES IN CODEGEN) **********************)
      ServerInvalidateUnits.go genv ctx files_checked fast_parsed naming_table;

      let telemetry =
        Telemetry.duration telemetry ~key:"invalidate_end" ~start_time
      in

      (* WRAP-UP ***************************************************************)
      let env =
        CheckKind.get_env_after_typing
          ~old_env:env
          ~errorl:errors
          ~needs_phase2_redecl
          ~needs_recheck
          ~diag_subscribe
      in

      (* STATS LOGGING *********************************************************)
      if SharedMem.SMTelemetry.hh_log_level () > 0 then begin
        Measure.print_stats ();
        Measure.print_distributions ()
      end;
      let telemetry =
        if SharedMem.SMTelemetry.hh_log_level () > 0 then
          Telemetry.object_
            telemetry
            ~key:"shmem"
            ~value:(SharedMem.SMTelemetry.get_telemetry ())
        else
          telemetry
      in

      let telemetry =
        telemetry
        |> Telemetry.object_
             ~key:"errors"
             ~value:(Errors.as_telemetry env.errorl)
        |> Telemetry.object_
             ~key:"repo_states"
             ~value:(Watchman.RepoStates.get_as_telemetry ())
      in

      (* HANDLE PRECHECKED FILES AFTER RECHECK *********************************)
      let telemetry =
        Telemetry.duration telemetry ~key:"prechecked2_start" ~start_time
      in
      let deptable_unlocked =
        Typing_deps.allow_dependency_table_reads env.deps_mode true
      in
      let (env, prechecked2_telemetry) =
        ServerPrecheckedFiles.update_after_recheck
          genv
          env
          files_checked
          ~start_time
      in
      let (_ : bool) =
        Typing_deps.allow_dependency_table_reads env.deps_mode deptable_unlocked
      in
      let telemetry =
        telemetry
        |> Telemetry.duration ~key:"prechecked2_end" ~start_time
        |> Telemetry.object_ ~key:"prechecked2" ~value:prechecked2_telemetry
      in

      (* We might have completed a full check, which might mean that a rebase was
       * successfully processed. *)
      ServerRevisionTracker.check_non_blocking env;
      let telemetry =
        Telemetry.duration
          telemetry
          ~key:"revtrack4_check_non_blocking_end"
          ~start_time
      in

      let env =
        {
          env with
          typing_service =
            {
              delegate_state =
                Typing_service_delegate.stop env.typing_service.delegate_state;
              enabled = false;
            };
        }
      in
      let telemetry =
        Telemetry.duration telemetry ~key:"stop_typing_service" ~start_time
      in

      (* CAUTION! Lots of alerts/dashboards depend on this event, particularly start_t  *)
      HackEventLogger.type_check_end
        (Option.some_if CheckKind.is_full telemetry)
        ~heap_size
        ~started_count:to_recheck_count
        ~count:total_rechecked_count
        ~experiments:genv.local_config.ServerLocalConfig.experiments
        ~desc:"serverTypeCheck"
        ~start_t:type_check_start_t
        ~adhoc_profiling:(Adhoc_profiler.CallTree.to_string adhoc_profiling);

      ( env,
        {
          CheckStats.reparse_count;
          total_rechecked_count;
          time_first_result = time_first_error;
        },
        telemetry )
  end

module FC = Make (FullCheckKind)
module LC = Make (LazyCheckKind)

let type_check_unsafe genv env kind start_time profiling =
  let check_kind = CheckKind.to_string kind in
  let telemetry =
    Telemetry.create ()
    |> Telemetry.string_ ~key:"kind" ~value:check_kind
    |> Telemetry.duration ~key:"start" ~start_time
  in
  (match kind with
  | CheckKind.Lazy -> HackEventLogger.set_lazy_incremental ()
  | CheckKind.Full -> ());

  (* CAUTION! Lots of alerts/dashboards depend on the exact string of check_kind *)
  HackEventLogger.with_check_kind check_kind @@ fun () ->
  Hh_logger.log "******************************************";
  match kind with
  | CheckKind.Lazy ->
    Hh_logger.log
      "Check kind: will check only those files already open in IDE or with reported errors ('%s')"
      check_kind;
    let (_ : seconds option) =
      ServerBusyStatus.send ServerCommandTypes.Doing_local_typecheck
    in
    let telemetry =
      Telemetry.duration telemetry ~key:"core_start" ~start_time
    in
    let (env, stats, core_telemetry) =
      LC.type_check_core genv env start_time profiling
    in
    let telemetry =
      telemetry
      |> Telemetry.duration ~key:"core_end" ~start_time
      |> Telemetry.object_ ~key:"core" ~value:core_telemetry
    in
    let t_sent_done =
      ServerBusyStatus.send ServerCommandTypes.Done_local_typecheck
    in
    let stats = CheckStats.record_result_sent_ts stats t_sent_done in
    let telemetry = Telemetry.duration telemetry ~key:"sent_done" ~start_time in
    (env, stats, telemetry)
  | CheckKind.Full ->
    Hh_logger.log
      "Check kind: will bring hh_server to consistency with code changes, by checking whatever fanout is needed ('%s')"
      check_kind;
    let (_ : seconds option) =
      ServerBusyStatus.send
        (ServerCommandTypes.Doing_global_typecheck
           (global_typecheck_kind genv env))
    in
    let telemetry =
      Telemetry.duration telemetry ~key:"core_start" ~start_time
    in

    let (env, stats, core_telemetry) =
      FC.type_check_core genv env start_time profiling
    in

    let telemetry =
      telemetry
      |> Telemetry.duration ~key:"core_end" ~start_time
      |> Telemetry.object_ ~key:"core" ~value:core_telemetry
    in
    let t_sent_done =
      if is_full_check_done env.full_check_status then
        let (is_truncated, shown) =
          match env.ServerEnv.diag_subscribe with
          | None -> (None, 0)
          | Some ds -> Diagnostic_subscription.get_pushed_error_length ds
        in
        let total = Option.value is_truncated ~default:shown in
        let msg = ServerCommandTypes.Done_global_typecheck { shown; total } in
        ServerBusyStatus.send msg
      else
        None
    in
    let stats = CheckStats.record_result_sent_ts stats t_sent_done in
    let telemetry =
      telemetry
      |> Telemetry.duration ~key:"sent_done" ~start_time
      |> Telemetry.bool_
           ~key:"diag_subscribe"
           ~value:(Option.is_some env.ServerEnv.diag_subscribe)
      (* If an IDE client is connected [env.diag_subscribe], let's log telemetry about
         this fact. This way we can look at all recheck telemetry and determine how often
         it's done with an IDE connected, hence presumably how long users wait in the IDE. *)
    in
    (env, stats, telemetry)

let type_check :
    genv ->
    env ->
    CheckKind.t ->
    seconds ->
    CgroupProfiler.Profiling.t ->
    env * CheckStats.t * Telemetry.t =
 fun genv env kind start_time profiling ->
  ServerUtils.with_exit_on_exception @@ fun () ->
  let type_check_result =
    type_check_unsafe genv env kind start_time profiling
  in
  type_check_result
