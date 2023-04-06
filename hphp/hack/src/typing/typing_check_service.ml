(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Hack_bucket = Bucket
open Hh_prelude
open Option.Monad_infix
module Bucket = Hack_bucket
open Typing_service_types
open Typing_check_job

(*
####

The type checking service receives a list of files and their symbols as input and
distributes the work to worker processes.

A worker process's input is a subset of files. At the end of its work,
it returns back to master the following progress report:
  - completed: a list of computations it completed
  - remaining: a list of computations which it didn't end up performing. For example,
    it may not have been able to get through the whole list of computations because
    it exceeded its worker memory cap
  - deferred: a list of computations which were discovered to be necessary to be
    performed before some of the computations in the input subset

The deferred list needs further explanation, so read below.

####

Here's how we actually plumb through the workitems and their individual results, via MultiWorker:
* We have mutable state [ref files_to_process], a list of filenames still to be processed
    for the entire typecheck.
* Datatype [progress] represents an input batch of work for a worker to do,
    and also an output indication of how it dispatched that batch
* Datatype [typing_result] represents the accumulation of telemetry, errors, deps results from workers.
    Each individual worker is given an empty [typing_result] as input,
    and merges its own results into that empty input to give a per-worker output,
    and then we have a separate accumulator and we merge each per-worker's output
    into that accumulator.
* next : () -> progress
    This mutates [files_to_process] by removing a bucket of filenames,
    and returns a degenerate progress {remaining=bucket; completed=[]; deferred=[]}
    which is basically just the bucket of work to be done by the job.
* neutral : typing_result
    This value is just the empty typing_result {errors=Empty; deps=Empty; telemetry=Empty}
* job : typing_result -> progress -> (typing_result', progress')
    MultiWorker will invoke this job. For input,
    it provides a copy of the degenerate [typing_result] that it got from [neutral], and
    it provides the degenerate [progress] i.e. just the bucket of work that it got from [next]
    The behavior of our job is to take items out of progress.remaining and typecheck them.
    It repeats this process until it either runs out of items or its heap grows too big.
    It returns a new progress' {remaining'; completed; deferred} to show the items still
    remaining, the ones it completed, and the ones it had to defer (see below).
    It returns a new typing_result' {errors; deps; telemetry} by merging its own
    newly discovered errors, deps, telemetry onto the (degenerate i.e. empty)
    typing_result it was given as input.
* merge :  (typing_result * progress) (accumulator : typing_result) -> typing_result
    The initial value of accumulator is the same [neutral] that was given to each job.
    After each job, MultiWorker calls [merge] to merge the results of that job
    into the accumulator.
    Our merge function looks at the progress {remaining;completed;deferred} that
    came out of the job, and mutates [files_to_process] by sticking back "remaining+deferred" into it.
    It then merges the typing_result {errors;deps;telemetry} that came out of the job
    with those in its accumulator.

The type signatures for MultiWorker look like they'd allow a variety of implementations,
e.g. having just a single accumulator that starts at "neutral" and feeds one by one into
each job. But we don't do that.

####

The normal computation kind for the type checking service is to type check all symbols in a file.
The type checker would always be able to find a given symbol it needs to perform this job
if all the declarations were computed for all the files in the repo before type checking begins.

However, such eager declaration does not occur in the following scenarios:
  - When the server initialization strategy is Lazy: not having a declaration phase was
      the original meaning of lazy init
  - If the server initialized from a saved state: most decls would not be available because
      saved states only consist of one or more of the following: naming table, dependency
      graph, errors, and hot decls (a small subset of all possible decls)

The decl heap module, when asked for a decl that is not yet in memory, can declare the decl's
file lazily. Declaring a file means parsing the file and extracting the symbols declared in it,
hence the term 'decl'.

This lazy declaration strategy works reasonably well for most cases, but there's a case where
it works poorly: if a particular file that is getting type checked requires a large number
of symbols which have not yet been declared, the unlucky worker ends up serially declaring
all the files that contain those symbols. In some cases, we observe type checking times on
the order of minutes for a single file.

Therefore, to account for the fact that we don't have the overall eager declaration phase
in some initialization scenarios, the decl heap module is now capable of refusing to declare
a file and instead adding the file to a list of deferments.

The type checker worker is then able to return that list and the file which was being type
checked when deferments occured back to the master process. The master process is then
able to distribute the declaration of the files and the (re)checking of the original
file to all of its workers, thus achieving parallelism and better type checking times overall.

The deferment of declarations is adjustable: it works by using a threshold value that dictates
the maximum number of lazy declarations allowed per file. If the threshold is not set, then
there is no limit, and no declarations would be deferred. If the threshold is set at 1,
then all declarations would be deferred.

The idea behind deferring declarations is similar to the 2nd of the 4 build systems considered in
the following paper (Make, Excel's calc engine, Shake, and Bazel):
    https://www.microsoft.com/en-us/research/uploads/prod/2018/03/build-systems-final.pdf

The paper refers to this approach as "restarting", and further suggests that recording and reusing
the chain of jobs could be used to minimize the number of restarts.
 *)

module Delegate = Typing_service_delegate

type seconds_since_epoch = float

let neutral : unit -> typing_result = Typing_service_types.make_typing_result

let should_enable_deferring (file : check_file_workitem) =
  not file.was_already_deferred

type process_file_results = {
  file_errors: Errors.t;
  deferred_decls: Deferred_decl.deferment list;
}

let scrape_class_names (ast : Nast.program) : SSet.t =
  let names = ref SSet.empty in
  let visitor =
    object
      (* It would look less clumsy to use Aast.reduce, but would use set union which has higher complexity. *)
      inherit [_] Aast.iter

      method! on_class_name _ (_p, id) = names := SSet.add id !names
    end
  in
  visitor#on_program () ast;
  !names

let process_file
    (ctx : Provider_context.t)
    (file : check_file_workitem)
    ~(log_errors : bool)
    ~(decl_cap_mb : int option) : process_file_results =
  let fn = file.path in
  let (file_errors, ast) = Ast_provider.get_ast_with_error ~full:true ctx fn in
  if not (Errors.is_empty file_errors) then
    { file_errors; deferred_decls = [] }
  else
    let opts = Provider_context.get_tcopt ctx in
    let (funs, classes, typedefs, gconsts, modules) = Nast.get_defs ast in
    let ctx = Provider_context.map_tcopt ctx ~f:(fun _tcopt -> opts) in
    let ignore_check_typedef opts fn name =
      ignore (check_typedef opts fn name)
    in
    let ignore_check_const opts fn name = ignore (check_const opts fn name) in
    let ignore_check_module opts fn name = ignore (check_module opts fn name) in
    try
      let result =
        Deferred_decl.with_deferred_decls
          ~enable:(should_enable_deferring file)
          ~declaration_threshold_opt:
            (TypecheckerOptions.defer_class_declaration_threshold opts)
          ~memory_mb_threshold_opt:decl_cap_mb
        @@ fun () ->
        Errors.do_with_context ~drop_fixmed:false fn Errors.Typing @@ fun () ->
        let (fun_tasts, fun_global_tvenvs) =
          List.map funs ~f:FileInfo.id_name
          |> List.filter_map ~f:(type_fun ctx fn)
          |> List.unzip
        in
        let fun_tasts = List.concat fun_tasts in
        let (class_tasts, class_global_tvenvs) =
          List.map classes ~f:FileInfo.id_name
          |> List.filter_map ~f:(type_class ctx fn)
          |> List.unzip
        in
        let class_global_tvenvs = List.concat class_global_tvenvs in
        List.map typedefs ~f:FileInfo.id_name
        |> List.iter ~f:(ignore_check_typedef ctx fn);
        List.map gconsts ~f:FileInfo.id_name
        |> List.iter ~f:(ignore_check_const ctx fn);
        List.map modules ~f:FileInfo.id_name
        |> List.iter ~f:(ignore_check_module ctx fn);
        (fun_tasts @ class_tasts, fun_global_tvenvs @ class_global_tvenvs)
      in
      match result with
      | Ok (file_errors, (tasts, global_tvenvs)) ->
        if TypecheckerOptions.global_inference opts then
          Typing_global_inference.StateSubConstraintGraphs.build_and_save
            ctx
            tasts
            global_tvenvs;
        if log_errors then
          List.iter (Errors.get_error_list file_errors) ~f:(fun error ->
              let { User_error.claim; code; _ } = error in
              let (pos, msg) = claim in
              let (l1, l2, c1, c2) = Pos.info_pos_extended pos in
              Hh_logger.log
                "%s(%d:%d-%d:%d) [%d] %s"
                (Relative_path.suffix fn)
                l1
                c1
                l2
                c2
                code
                msg);
        { file_errors; deferred_decls = [] }
      | Error () ->
        let deferred_decls =
          Errors.ignore_ (fun () -> Naming.program ctx ast)
          |> scrape_class_names
          |> SSet.elements
          |> List.filter_map ~f:(fun class_name ->
                 Naming_provider.get_class_path ctx class_name >>| fun fn ->
                 (fn, class_name))
        in
        { file_errors = Errors.empty; deferred_decls }
    with
    | WorkerCancel.Worker_should_exit as exn ->
      (* Cancellation requests must be re-raised *)
      let e = Exception.wrap exn in
      Exception.reraise e
    | exn ->
      let e = Exception.wrap exn in
      prerr_endline ("Exception on file " ^ Relative_path.S.to_string fn);
      Exception.reraise e

module ProcessFilesTally = struct
  (** Counters for the [check_file_workitem] of each sort being processed *)
  type t = {
    decls: int;  (** how many [Declare] items we performed *)
    prefetches: int;  (** how many [Prefetch] items we performed *)
    checks_done: int;  (** how many [Check] items we typechecked *)
    checks_deferred: int;  (** how many [Check] items we deferred to later *)
    decls_deferred: int;  (** how many [Declare] items we added for later *)
    exceeded_cap_count: int;  (** how many times we exceeded the memory cap *)
  }

  let empty =
    {
      decls = 0;
      prefetches = 0;
      checks_done = 0;
      checks_deferred = 0;
      decls_deferred = 0;
      exceeded_cap_count = 0;
    }

  let incr_decls tally = { tally with decls = tally.decls + 1 }

  let record_caps ~workitem_ends_under_cap tally =
    if workitem_ends_under_cap then
      tally
    else
      { tally with exceeded_cap_count = tally.exceeded_cap_count + 1 }

  let incr_prefetches tally = { tally with prefetches = tally.prefetches + 1 }

  let incr_checks tally deferred_decls =
    if List.is_empty deferred_decls then
      { tally with checks_done = tally.checks_done + 1 }
    else
      {
        tally with
        checks_deferred = tally.checks_deferred + 1;
        decls_deferred = tally.decls_deferred + List.length deferred_decls;
      }

  let count tally =
    tally.checks_done + tally.checks_deferred + tally.decls + tally.prefetches

  let get_telemetry tally =
    Telemetry.create ()
    |> Telemetry.int_ ~key:"decls" ~value:tally.decls
    |> Telemetry.int_ ~key:"prefetches" ~value:tally.prefetches
    |> Telemetry.int_ ~key:"checks_done" ~value:tally.checks_done
    |> Telemetry.int_ ~key:"checks_deferred" ~value:tally.checks_deferred
    |> Telemetry.int_ ~key:"decls_deferred" ~value:tally.decls_deferred
    |> Telemetry.int_ ~key:"exceeded_cap_count" ~value:tally.exceeded_cap_count
end

let get_stats ~include_slightly_costly_stats tally :
    HackEventLogger.ProfileTypeCheck.stats =
  let telemetry =
    Counters.get_counters ()
    |> Telemetry.object_
         ~key:"tally"
         ~value:(ProcessFilesTally.get_telemetry tally)
  in
  HackEventLogger.ProfileTypeCheck.get_stats
    ~include_current_process:true
    ~include_slightly_costly_stats
    ~shmem_heap_size:(SharedMem.SMTelemetry.heap_size ())
    telemetry

external hh_malloc_trim : unit -> unit = "hh_malloc_trim"

let process_one_workitem
    ~ctx
    ~check_info
    ~batch_info
    ~memory_cap
    ~longlived_workers
    ~error_count_at_start_of_batch
    ~progress
    ~errors
    ~stats
    ~tally =
  let decl_cap_mb =
    if check_info.use_max_typechecker_worker_memory_for_decl_deferral then
      memory_cap
    else
      None
  in
  let workitem_cap_mb = Option.value memory_cap ~default:Int.max_value in
  let type_check_twice =
    check_info.per_file_profiling
      .HackEventLogger.PerFileProfilingConfig.profile_type_check_twice
  in
  let fn = List.hd_exn progress.remaining in

  let (file, decl, mid_stats, file_errors, deferred, tally) =
    match fn with
    | Check file ->
      (* We'll show at least the first five errors in the project. Maybe more,
         if this file has more in it, or if other concurrent workers race to print
         the first five errors before us. *)
      let log_errors =
        check_info.log_errors
        && error_count_at_start_of_batch + Errors.count errors < 5
      in
      let result = process_file ctx file ~decl_cap_mb ~log_errors in
      let mid_stats =
        if type_check_twice then
          Some (get_stats ~include_slightly_costly_stats:false tally)
        else
          None
      in
      begin
        if type_check_twice then
          let _ignored = process_file ctx file ~decl_cap_mb in
          ()
      end;
      let tally = ProcessFilesTally.incr_checks tally result.deferred_decls in
      let deferred =
        if List.is_empty result.deferred_decls then
          []
        else
          List.map result.deferred_decls ~f:(fun fn -> Declare fn)
          @ [Check { file with was_already_deferred = true }]
      in
      (Some file, None, mid_stats, result.file_errors, deferred, tally)
    | Declare (_path, class_name) ->
      let (_ : Decl_provider.class_decl option) =
        Decl_provider.get_class ctx class_name
      in
      ( None,
        Some class_name,
        None,
        Errors.empty,
        [],
        ProcessFilesTally.incr_decls tally )
    | Prefetch paths ->
      Vfs.prefetch paths;
      ( None,
        None,
        None,
        Errors.empty,
        [],
        ProcessFilesTally.incr_prefetches tally )
  in
  let errors = Errors.merge file_errors errors in
  let workitem_ends_under_cap = Gc_utils.get_heap_size () <= workitem_cap_mb in
  let final_stats =
    get_stats
      ~include_slightly_costly_stats:
        ((not longlived_workers) && not workitem_ends_under_cap)
      tally
  in
  let (workitem_end_stats, workitem_end_second_stats) =
    match mid_stats with
    | None -> (final_stats, None)
    | Some mid_stats -> (mid_stats, Some final_stats)
  in

  let (fns, check_fns, errors) = (List.tl_exn progress.remaining, [], errors) in

  (* If the major heap has exceeded the bounds, we (1) first try and bring the size back down
     by flushing the parser cache and doing a major GC; (2) if this fails, we decline to typecheck
     the remaining files.
     We use [quick_stat] instead of [stat] in Gc_utils.get_heap_size in order to avoid walking the major heap,
     and we don't change the minor heap because it's small and fixed-size.
     This test is performed after we've processed at least one item, to ensure we make at least some progress. *)
  let tally = ProcessFilesTally.record_caps ~workitem_ends_under_cap tally in
  let workitem_ends_under_cap =
    if workitem_ends_under_cap || not longlived_workers then
      workitem_ends_under_cap
    else begin
      SharedMem.invalidate_local_caches ();
      HackEventLogger.flush ();
      Gc.compact ();
      hh_malloc_trim ();
      Gc_utils.get_heap_size () <= workitem_cap_mb
    end
  in

  HackEventLogger.ProfileTypeCheck.process_workitem
    ~batch_info
    ~workitem_index:(ProcessFilesTally.count tally)
    ~file:(Option.map file ~f:(fun file -> file.path))
    ~file_was_already_deferred:
      (Option.map file ~f:(fun file -> file.was_already_deferred))
    ~decl
    ~error_code:(Errors.choose_code_opt file_errors)
    ~workitem_ends_under_cap
    ~workitem_start_stats:stats
    ~workitem_end_stats
    ~workitem_end_second_stats;

  let progress =
    {
      completed = (fn :: check_fns) @ progress.completed;
      remaining = fns;
      deferred = List.concat [deferred; progress.deferred];
    }
  in

  (progress, errors, tally, final_stats, workitem_ends_under_cap)

let process_workitems
    (ctx : Provider_context.t)
    ({ errors; dep_edges; telemetry } : typing_result)
    (progress : typing_progress)
    ~(memory_cap : int option)
    ~(longlived_workers : bool)
    ~(check_info : check_info)
    ~(worker_id : string)
    ~(batch_number : int)
    ~(error_count_at_start_of_batch : int)
    ~(typecheck_info : HackEventLogger.ProfileTypeCheck.typecheck_info) :
    typing_result * typing_progress =
  Decl_counters.set_mode
    check_info.per_file_profiling
      .HackEventLogger.PerFileProfilingConfig.profile_decling;
  let _prev_counters_state = Counters.reset () in
  let batch_info =
    HackEventLogger.ProfileTypeCheck.get_batch_info
      ~typecheck_info
      ~worker_id
      ~batch_number
      ~batch_size:(List.length progress.remaining)
      ~start_batch_stats:
        (get_stats ~include_slightly_costly_stats:true ProcessFilesTally.empty)
  in

  if not longlived_workers then SharedMem.invalidate_local_caches ();
  File_provider.local_changes_push_sharedmem_stack ();
  Ast_provider.local_changes_push_sharedmem_stack ();
  (* Let's curry up the function now so it's easier to invoke later in the loop. *)
  let process_one_workitem =
    process_one_workitem
      ~ctx
      ~check_info
      ~batch_info
      ~error_count_at_start_of_batch
      ~memory_cap
      ~longlived_workers
  in

  let rec process_workitems_loop ~progress ~errors ~stats ~tally =
    match progress.remaining with
    | [] -> (progress, errors)
    | _ ->
      let (progress, errors, tally, stats, file_ends_under_cap) =
        process_one_workitem ~progress ~errors ~stats ~tally
      in
      if file_ends_under_cap then
        process_workitems_loop ~progress ~errors ~stats ~tally
      else
        (progress, errors)
  in

  (* Process as many files as we can, and merge in their errors *)
  let (progress, errors) =
    process_workitems_loop
      ~progress
      ~errors
      ~tally:ProcessFilesTally.empty
      ~stats:
        (get_stats ~include_slightly_costly_stats:true ProcessFilesTally.empty)
  in

  (* Update edges *)
  let new_dep_edges =
    Typing_deps.flush_ideps_batch (Provider_context.get_deps_mode ctx)
  in
  let dep_edges = Typing_deps.merge_dep_edges dep_edges new_dep_edges in
  if
    Provider_context.get_tcopt ctx
    |> TypecheckerOptions.record_fine_grained_dependencies
  then
    Typing_pessimisation_deps.finalize (Provider_context.get_deps_mode ctx);

  (* Gather up our various forms of telemetry... *)
  let end_heap_mb = Gc.((quick_stat ()).Stat.heap_words) * 8 / 1024 / 1024 in
  let this_batch_telemetry =
    Telemetry.create ()
    |> Telemetry.object_ ~key:"operations" ~value:(Counters.get_counters ())
    |> Telemetry.int_ ~key:"end_heap_mb_sum" ~value:end_heap_mb
    |> Telemetry.int_ ~key:"batch_count" ~value:1
  in
  let telemetry = Telemetry.add telemetry this_batch_telemetry in

  TypingLogger.flush_buffers ();
  Ast_provider.local_changes_pop_sharedmem_stack ();
  File_provider.local_changes_pop_sharedmem_stack ();
  ({ errors; dep_edges; telemetry }, progress)

let load_and_process_workitems
    (ctx : Provider_context.t)
    (typing_result : typing_result)
    (progress : typing_progress)
    ~(memory_cap : int option)
    ~(longlived_workers : bool)
    ~(check_info : check_info)
    ~(worker_id : string)
    ~(batch_number : int)
    ~(error_count_at_start_of_batch : int)
    ~(typecheck_info : HackEventLogger.ProfileTypeCheck.typecheck_info) :
    typing_result * typing_progress =
  Option.iter check_info.memtrace_dir ~f:(fun temp_dir ->
      let file = Caml.Filename.temp_file ~temp_dir "memtrace.worker." ".ctf" in
      Daemon.start_memtracing file);
  (* When the type-checking worker receives SIGUSR1, display a position which
     corresponds approximately with the function/expression being checked. *)
  Sys_utils.set_signal
    Sys.sigusr1
    (Sys.Signal_handle Typing.debug_print_last_pos);
  process_workitems
    ctx
    typing_result
    progress
    ~memory_cap
    ~longlived_workers
    ~check_info
    ~worker_id
    ~batch_number
    ~error_count_at_start_of_batch
    ~typecheck_info

(*****************************************************************************)
(* Let's go! That's where the action is *)
(*****************************************************************************)

let possibly_push_new_errors_to_lsp_client :
    progress:Typing_service_types.typing_progress ->
    Errors.t ->
    Diagnostic_pusher.t option ->
    Diagnostic_pusher.t option * seconds_since_epoch option =
 fun ~progress new_errors diag ->
  match diag with
  | None -> (None, None)
  | Some diag ->
    let rechecked =
      progress.completed
      |> List.filter_map ~f:(function
             | Check { path; was_already_deferred = _ } -> Some path
             | Declare _
             | Prefetch _ ->
               None)
      |> Relative_path.Set.of_list
    in
    let (diag, time_errors_pushed) =
      Diagnostic_pusher.push_new_errors
        diag
        ~rechecked
        new_errors
        ~phase:Errors.Typing
    in
    (Some diag, time_errors_pushed)

(** Merge the results from multiple workers.

    We don't really care about which files are left unchecked since we use
    (gasp) mutation to track that, so combine the errors but always return an
    empty list for the list of unchecked files. *)
let merge
    ~(should_prefetch_deferred_files : bool)
    ~(batch_counts_by_worker_id : int SMap.t ref)
    ~(errors_so_far : int ref)
    ~(check_info : check_info)
    (delegate_state : Delegate.state ref)
    (workitems_to_process : workitem BigList.t ref)
    (workitems_initial_count : int)
    (workitems_in_progress : workitem Hash_set.t)
    (files_checked_count : int ref)
    (diagnostic_pusher : Diagnostic_pusher.t option ref)
    (time_first_error : seconds_since_epoch option ref)
    ( (worker_id : string),
      (produced_by_job : typing_result),
      ({ kind = progress_kind; progress : typing_progress } : job_progress) )
    (acc : typing_result) : typing_result =
  (* Update batch count *)
  begin
    match progress_kind with
    | Progress ->
      let prev_batch_count =
        SMap.find_opt worker_id !batch_counts_by_worker_id
        |> Option.value ~default:0
      in
      batch_counts_by_worker_id :=
        SMap.add worker_id (prev_batch_count + 1) !batch_counts_by_worker_id
    | DelegateProgress _ -> ()
    | SimpleDelegateProgress _ -> ()
  end;

  (* And error count *)
  errors_so_far := !errors_so_far + Errors.count produced_by_job.errors;

  (* Merge in remote-worker results *)
  begin
    match progress_kind with
    | Progress -> ()
    | SimpleDelegateProgress _ -> ()
    | DelegateProgress _ ->
      delegate_state :=
        Delegate.merge !delegate_state produced_by_job.errors progress
  end;

  workitems_to_process :=
    BigList.append progress.remaining !workitems_to_process;

  (* Let's also prepend the deferred files! *)
  workitems_to_process := BigList.append progress.deferred !workitems_to_process;

  (* Prefetch the deferred files, if necessary *)
  workitems_to_process :=
    if should_prefetch_deferred_files && List.length progress.deferred > 10 then
      let files_to_prefetch =
        List.fold progress.deferred ~init:[] ~f:(fun acc computation ->
            match computation with
            | Declare (path, _) -> path :: acc
            | _ -> acc)
      in
      BigList.cons (Prefetch files_to_prefetch) !workitems_to_process
    else
      !workitems_to_process;

  (* If workers can steal work from each other, then it's possible that
     some of the files that the current worker completed checking have already
     been removed from the in-progress set. Thus, we should keep track of
     how many type check computations we actually remove from the in-progress
     set. Note that we also skip counting Declare and Prefetch computations,
     since they are not relevant for computing how many files we've type
     checked. *)
  let completed_check_count =
    List.fold
      ~init:0
      ~f:(fun acc workitem ->
        match Hash_set.Poly.strict_remove workitems_in_progress workitem with
        | Ok () -> begin
          match workitem with
          | Check _ -> acc + 1
          | _ -> acc
        end
        | _ -> acc)
      progress.completed
  in

  (* Deferred type check computations should be subtracted from completed
     in order to produce an accurate count because they we requeued them, yet
     they were also included in the completed list.
  *)
  let is_check workitem =
    match workitem with
    | Check _ -> true
    | _ -> false
  in
  let deferred_check_count = List.count ~f:is_check progress.deferred in
  let completed_check_count = completed_check_count - deferred_check_count in

  files_checked_count := !files_checked_count + completed_check_count;
  ServerProgress.write_percentage
    ~operation:"typechecking"
    ~done_count:!files_checked_count
    ~total_count:workitems_initial_count
    ~unit:"files"
    ~extra:(Typing_service_delegate.get_progress !delegate_state);

  (* Handle errors paradigm (3) - push updates to errors-file as soon as their batch is finished *)
  if check_info.log_errors then
    ServerProgress.ErrorsWrite.report produced_by_job.errors;
  (* Handle errors paradigm (2) - push updates to lsp as well *)
  let (diag_pusher, time_errors_pushed) =
    possibly_push_new_errors_to_lsp_client
      ~progress
      produced_by_job.errors
      !diagnostic_pusher
  in
  diagnostic_pusher := diag_pusher;
  time_first_error := Option.first_some !time_first_error time_errors_pushed;

  Typing_deps.register_discovered_dep_edges produced_by_job.dep_edges;
  Typing_deps.register_discovered_dep_edges acc.dep_edges;

  let produced_by_job =
    { produced_by_job with dep_edges = Typing_deps.dep_edges_make () }
  in
  let acc = { acc with dep_edges = Typing_deps.dep_edges_make () } in

  accumulate_job_output produced_by_job acc

let next
    (workers : MultiWorker.worker list option)
    (delegate_state : Delegate.state ref)
    (workitems_to_process : workitem BigList.t ref)
    (workitems_in_progress : workitem Hash_set.Poly.t)
    (workitems_processed_count : int ref)
    (remote_payloads : remote_computation_payload list ref)
    (record : Measure.record)
    (telemetry : Telemetry.t) : unit -> job_progress Bucket.bucket =
  let num_workers =
    match workers with
    | Some w -> List.length w
    | None -> 1
  in
  let return_bucket_job (kind : progress_kind) ~current_bucket ~remaining_jobs =
    (* Update our shared mutable state, because hey: it's not like we're
       writing OCaml or anything. *)
    workitems_to_process := remaining_jobs;
    List.iter ~f:(Hash_set.Poly.add workitems_in_progress) current_bucket;
    Bucket.Job
      {
        kind;
        progress = { completed = []; remaining = current_bucket; deferred = [] };
      }
  in
  fun () ->
    Measure.time ~record "time" @@ fun () ->
    let workitems_to_process_length = BigList.length !workitems_to_process in
    let controller_started = Delegate.controller_started !delegate_state in
    let delegate_job =
      (*
          This is the "reduce" part of the mapreduce paradigm. We activate this when workitems_to_check is empty,
          or in other words the local typechecker is done with its work. We'll try and download all the remote
          worker outputs in once go. For any payloads that aren't available we'll simply stop waiting on the
          remote worker and have the local worker "steal" the work.
        *)
      let remote_workitems_to_process_length =
        List.fold ~init:0 !remote_payloads ~f:(fun acc payload ->
            acc + BigList.length payload.payload)
      in
      let ( remaining_local_workitems_to_process,
            controller,
            remaining_payloads,
            job,
            _telemetry ) =
        Typing_service_delegate.collect
          ~telemetry
          !delegate_state
          !workitems_to_process
          workitems_to_process_length
          !remote_payloads
      in
      (* Update the total workitems_processed_count after remote workers
         have made progress, so we can update the progress bar with the
         correct number of files typechecked.
      *)
      (if List.length !remote_payloads > List.length remaining_payloads then
        let remaining_remote_workitems_to_process =
          List.fold ~init:0 remaining_payloads ~f:(fun acc payload ->
              acc + BigList.length payload.payload)
        in
        let local_processed_count =
          !workitems_processed_count
          - BigList.length remaining_local_workitems_to_process
        in
        let remote_processed_count =
          remote_workitems_to_process_length
          - remaining_remote_workitems_to_process
        in
        workitems_processed_count :=
          local_processed_count + remote_processed_count);

      workitems_to_process := remaining_local_workitems_to_process;
      delegate_state := controller;
      remote_payloads := remaining_payloads;

      job
    in

    let (state, delegate_job) = (!delegate_state, delegate_job) in
    delegate_state := state;

    (* If a delegate job is returned, then that means that it should be done
       by the next MultiWorker worker (the one for whom we're creating a job
       in this function). If delegate job is None, then the regular (local
       type checking) logic applies. *)
    match delegate_job with
    | Some { current_bucket; remaining_jobs; job } ->
      if controller_started then
        return_bucket_job
          (SimpleDelegateProgress job)
          ~current_bucket
          ~remaining_jobs
      else
        return_bucket_job (DelegateProgress job) ~current_bucket ~remaining_jobs
    | None ->
      (* WARNING: the following List.length is costly - for a full init, files_to_process starts
         out as the size of the entire repo, and we're traversing the entire list. *)
      (match workitems_to_process_length with
      | 0 when Hash_set.Poly.is_empty workitems_in_progress -> Bucket.Done
      | 0 -> Bucket.Wait
      | _ ->
        let jobs = !workitems_to_process in
        begin
          match num_workers with
          (* When num_workers is zero, the execution mode is delegate-only, so we give an empty bucket to MultiWorker for execution. *)
          | 0 ->
            return_bucket_job Progress ~current_bucket:[] ~remaining_jobs:jobs
          | _ ->
            let bucket_size =
              Bucket.calculate_bucket_size
                ~num_jobs:workitems_to_process_length
                ~num_workers
                ()
            in
            let (current_bucket, remaining_jobs) =
              BigList.split_n jobs bucket_size
            in
            return_bucket_job Progress ~current_bucket ~remaining_jobs
        end)

let on_cancelled
    (next : unit -> 'a Bucket.bucket)
    (files_to_process : 'b Hash_set.Poly.elt BigList.t ref)
    (files_in_progress : 'b Hash_set.Poly.t) : unit -> 'a list =
 fun () ->
  (* The size of [files_to_process] is bounded only by repo size, but
      [files_in_progress] is capped at [(worker count) * (max bucket size)]. *)
  files_to_process :=
    BigList.append (Hash_set.Poly.to_list files_in_progress) !files_to_process;
  let rec add_next acc =
    match next () with
    | Bucket.Job j -> add_next (j :: acc)
    | Bucket.Wait
    | Bucket.Done ->
      acc
  in
  add_next []

let process_with_hh_distc
    ~(interrupt : 'a MultiWorker.interrupt_config) ~(check_info : check_info) :
    typing_result =
  (* TODO: Plumb extra --config name=value args through to spawn() *)
  (* TODO: Poll interrupts *)
  let _ = interrupt in
  (* TODO: the following is a bug! I'm not exactly sure how to get root here... *)
  let root = Wwwroot.interpret_command_line_root_parameter [] in
  (* We don't want to use with_tempdir because we need to keep the folder around
     for subseqent typechecks that will read the dep graph in the folder *)
  let ss_dir = Tempfile.mkdtemp ~skip_mocking:false in
  let dg_path = Path.(to_string @@ concat ss_dir "hh_mini_saved_state.hhdg") in
  let handle =
    Hh_distc_ffi.spawn (Path.to_string root) (Path.to_string ss_dir) dg_path
    |> Result.ok_or_failwith
  in
  let fd = Hh_distc_ffi.get_fd handle in
  let rec drain_events (done_count, total_count) =
    match Hh_distc_ffi.recv handle |> Result.ok_or_failwith with
    | Some (Hh_distc_types.Errors errors) ->
      if check_info.log_errors then ServerProgress.ErrorsWrite.report errors;
      drain_events (done_count, total_count)
    | Some (Hh_distc_types.TypingStart total_count) ->
      drain_events (done_count, total_count)
    | Some (Hh_distc_types.TypingProgress n) ->
      let done_count = done_count + n in
      drain_events (done_count, total_count)
    | None -> (done_count, total_count)
  in
  let buf = Bytes.create 1 in
  let select_timeout = -1.0 in
  let rec event_loop (done_count, total_count) =
    (* hh_distc sends a byte each time new events are ready. *)
    (* TODO add fd to interrupts or something? *)
    let (_ready, _, _) = Unix.select [fd] [] [] select_timeout in
    (* TODO only read from fd if it was ready *)
    let n = Unix.recv fd buf 0 1 [] in
    match n with
    | 0 ->
      ServerProgress.write "hh_distc done";
      Hh_distc_ffi.join handle
    | _ ->
      let (done_count, total_count) = drain_events (done_count, total_count) in
      ServerProgress.write_percentage
        ~operation:"hh_distc checking"
        ~done_count
        ~total_count
        ~unit:"files"
        ~extra:None;
      event_loop (done_count, total_count)
  in
  ServerProgress.write "hh_distc running";
  match event_loop (0, 0) with
  | Ok errors ->
    (* TODO: Clear in memory deps. Doesn't effect correctness but can cause larger fanouts *)
    Typing_deps.replace (Typing_deps_mode.InMemoryMode (Some dg_path));
    {
      errors;
      dep_edges = Typing_deps.dep_edges_make ();
      telemetry = Telemetry.create ();
    }
  | Error msg ->
    Hh_logger.log "Error with hh_distc: %s" msg;
    failwith msg

(**
  `next` and `merge` both run in the master process and update mutable
  state in order to track work in progress and work remaining.
  `job` runs in each worker and does not have access to this mutable state.
 *)
let process_in_parallel
    ?diagnostic_pusher
    (ctx : Provider_context.t)
    (workers : MultiWorker.worker list option)
    (delegate_state : Delegate.state)
    (telemetry : Telemetry.t)
    (workitems : workitem BigList.t)
    ~(interrupt : 'a MultiWorker.interrupt_config)
    ~(memory_cap : int option)
    ~(longlived_workers : bool)
    ~(check_info : check_info)
    ~(typecheck_info : HackEventLogger.ProfileTypeCheck.typecheck_info) :
    typing_result
    * Delegate.state
    * Telemetry.t
    * _
    * Relative_path.t list
    * (Diagnostic_pusher.t option * seconds_since_epoch option) =
  let record = Measure.create () in
  (* [record] is used by [next] *)
  let delegate_state = ref delegate_state in
  let workitems_to_process = ref workitems in
  let workitems_in_progress = Hash_set.Poly.create () in
  let workitems_processed_count = ref 0 in
  let workitems_initial_count = BigList.length workitems in
  let remote_payloads = ref [] in
  let diagnostic_pusher = ref diagnostic_pusher in
  let time_first_error = ref None in
  let errors_so_far = ref 0 in
  let controller_started = Delegate.controller_started !delegate_state in
  let batch_counts_by_worker_id = ref SMap.empty in

  let (telemetry, telemetry_start_t) : Telemetry.t * float option =
    if controller_started then (
      Hh_logger.log "Dispatch hulk lite initial payloads";
      let workitems_to_process_length = BigList.length !workitems_to_process in
      let ( payloads,
            workitems,
            controller,
            (dispatch_telemetry, hulk_dispatch_start_t) ) =
        Typing_service_delegate.dispatch
          !delegate_state
          !workitems_to_process
          workitems_to_process_length
      in
      remote_payloads := payloads;
      workitems_to_process := workitems;
      delegate_state := controller;
      (dispatch_telemetry, Some hulk_dispatch_start_t)
    ) else
      (telemetry, None)
  in

  let next =
    next
      workers
      delegate_state
      workitems_to_process
      workitems_in_progress
      workitems_processed_count
      remote_payloads
      record
      telemetry
  in
  let should_prefetch_deferred_files =
    Vfs.is_vfs ()
    && TypecheckerOptions.prefetch_deferred_files
         (Provider_context.get_tcopt ctx)
  in
  (* The [job] lambda is marshalled, sent to the worker process, unmarshalled there, and executed.
     It is marshalled immediately before being executed. *)
  let job (typing_result : typing_result) (progress : job_progress) :
      string * typing_result * job_progress =
    let worker_id = Option.value ~default:"main" (Hh_logger.get_id ()) in
    let (typing_result, computation_progress) =
      match progress.kind with
      | Progress ->
        load_and_process_workitems
          ctx
          ~memory_cap
          ~longlived_workers
          ~check_info
          ~typecheck_info
          ~worker_id
          ~error_count_at_start_of_batch:!errors_so_far
          ~batch_number:
            (SMap.find_opt worker_id !batch_counts_by_worker_id
            |> Option.value ~default:0)
          typing_result
          progress.progress
      | DelegateProgress job -> Delegate.process job
      | SimpleDelegateProgress job -> Delegate.process job
    in
    (worker_id, typing_result, { progress with progress = computation_progress })
  in
  let (typing_result, env, cancelled_results) =
    MultiWorker.call_with_interrupt
      workers
      ~job
      ~neutral:(neutral ())
      ~merge:
        (merge
           ~should_prefetch_deferred_files
           ~batch_counts_by_worker_id
           ~errors_so_far
           ~check_info
           delegate_state
           workitems_to_process
           workitems_initial_count
           workitems_in_progress
           workitems_processed_count
           diagnostic_pusher
           time_first_error)
      ~next
      ~on_cancelled:
        (on_cancelled next workitems_to_process workitems_in_progress)
      ~interrupt
  in
  let paths_of (cancelled_results : job_progress list) : Relative_path.t list =
    let paths_of (cancelled_progress : job_progress) =
      let cancelled_computations = cancelled_progress.progress.remaining in
      let paths_of paths (cancelled_workitem : workitem) =
        match cancelled_workitem with
        | Check { path; _ } -> path :: paths
        | _ -> paths
      in
      List.fold cancelled_computations ~init:[] ~f:paths_of
    in
    List.concat (List.map cancelled_results ~f:paths_of)
  in
  let _ =
    if controller_started then
      HackEventLogger.hulk_type_check_end
        telemetry
        workitems_initial_count
        ~start_t:
          (Option.value_exn
             telemetry_start_t
             ~message:"Unexpected missing telemetry start time for Hulk Lite")
    else
      ()
  in

  (* We want to ensure controller state is reset for the recheck *)
  delegate_state := Typing_service_delegate.stop !delegate_state;

  ( typing_result,
    !delegate_state,
    telemetry,
    env,
    paths_of cancelled_results,
    (!diagnostic_pusher, !time_first_error) )

let process_sequentially
    ?diagnostic_pusher ctx fnl ~longlived_workers ~check_info ~typecheck_info :
    typing_result * (Diagnostic_pusher.t option * seconds_since_epoch option) =
  let progress =
    { completed = []; remaining = BigList.as_list fnl; deferred = [] }
  in
  (* Since we're running sequentially here, we don't want 'process_files' to activate its own memtracing: *)
  let check_info = { check_info with memtrace_dir = None } in
  let (typing_result, progress) =
    process_workitems
      ctx
      (neutral ())
      progress
      ~memory_cap:None
      ~longlived_workers
      ~error_count_at_start_of_batch:0
      ~check_info
      ~worker_id:"master"
      ~batch_number:(-1)
      ~typecheck_info
  in
  let push_result =
    possibly_push_new_errors_to_lsp_client
      ~progress
      typing_result.errors
      diagnostic_pusher
  in
  (typing_result, push_result)

type 'a job_result = 'a * Relative_path.t list

module type Mocking_sig = sig
  val with_test_mocking :
    (* real job payload, that we can modify... *)
    workitem BigList.t ->
    ((* ... before passing it to the real job executor... *)
     workitem BigList.t ->
    'a job_result) ->
    (* ... which output we can also modify. *)
    'a job_result
end

module NoMocking = struct
  let with_test_mocking fnl f = f fnl
end

module TestMocking = struct
  let cancelled = ref Relative_path.Set.empty

  let set_is_cancelled x = cancelled := Relative_path.Set.add !cancelled x

  let is_cancelled x = Relative_path.Set.mem !cancelled x

  let with_test_mocking fnl f =
    let (mock_cancelled, fnl) =
      List.partition_map (BigList.as_list fnl) ~f:(fun computation ->
          match computation with
          | Check { path; _ } ->
            if is_cancelled path then
              First path
            else
              Second computation
          | _ -> Second computation)
    in
    (* Only cancel once to avoid infinite loops *)
    cancelled := Relative_path.Set.empty;
    let (res, cancelled) = f (BigList.create fnl) in
    (res, mock_cancelled @ cancelled)
end

module Mocking =
  (val if Injector_config.use_test_stubbing then
         (module TestMocking : Mocking_sig)
       else
         (module NoMocking : Mocking_sig))

let should_process_sequentially
    (opts : TypecheckerOptions.t) (workitems : workitem BigList.t) : bool =
  (* If decls can be deferred, then we should process in parallel, since
     we are likely to have more computations than there are files to type check. *)
  let defer_threshold =
    TypecheckerOptions.defer_class_declaration_threshold opts
  in
  let parallel_threshold =
    TypecheckerOptions.parallel_type_checking_threshold opts
  in
  match (defer_threshold, BigList.length workitems) with
  | (None, file_count) when file_count < parallel_threshold -> true
  | _ -> false

type result = {
  errors: Errors.t;
  delegate_state: Delegate.state;
  telemetry: Telemetry.t;
  diagnostic_pusher: Diagnostic_pusher.t option * seconds_since_epoch option;
}

let go_with_interrupt
    ?(diagnostic_pusher : Diagnostic_pusher.t option)
    (ctx : Provider_context.t)
    (workers : MultiWorker.worker list option)
    (delegate_state : Delegate.state)
    (telemetry : Telemetry.t)
    (fnl : Relative_path.t list)
    ~(interrupt : 'a MultiWorker.interrupt_config)
    ~(memory_cap : int option)
    ~(longlived_workers : bool)
    ~(use_hh_distc_instead_of_hulk : bool)
    ~(check_info : check_info) : (_ * result) job_result =
  let typecheck_info =
    HackEventLogger.ProfileTypeCheck.get_typecheck_info
      ~init_id:check_info.init_id
      ~check_reason:check_info.check_reason
      ~recheck_id:check_info.recheck_id
      ~start_hh_stats:(CgroupProfiler.get_initial_stats ())
      ~start_typecheck_stats:
        (HackEventLogger.ProfileTypeCheck.get_stats
           ~include_current_process:false
           ~include_slightly_costly_stats:true
           ~shmem_heap_size:(SharedMem.SMTelemetry.heap_size ())
           (Telemetry.create ()))
      ~config:check_info.per_file_profiling
  in
  let opts = Provider_context.get_tcopt ctx in
  let sample_rate = TypecheckerOptions.typecheck_sample_rate opts in
  let fnl = BigList.create fnl in
  ServerProgress.write "typechecking %d files" (BigList.length fnl);
  let fnl =
    if Float.(sample_rate >= 1.0) then
      fnl
    else
      let result =
        BigList.filter
          ~f:(fun x ->
            Float.(
              float (Base.String.hash (Relative_path.suffix x) mod 1000000)
              <= sample_rate *. 1000000.0))
          fnl
      in
      Hh_logger.log
        "Sampling %f percent of files: %d out of %d"
        sample_rate
        (BigList.length result)
        (BigList.length fnl);
      result
  in
  let fnl =
    BigList.map fnl ~f:(fun path ->
        Check { path; was_already_deferred = false })
  in
  Mocking.with_test_mocking fnl @@ fun fnl ->
  let ( typing_result,
        delegate_state,
        telemetry,
        env,
        cancelled_fnl,
        diagnostic_pusher ) =
    if BigList.length fnl > 100_000 && use_hh_distc_instead_of_hulk then
      let typing_result = process_with_hh_distc ~interrupt ~check_info in
      ( typing_result,
        delegate_state,
        telemetry,
        interrupt.MultiThreadedCall.env,
        [],
        (None, None) )
    else if should_process_sequentially opts fnl then begin
      Hh_logger.log "Type checking service will process files sequentially";
      let (typing_result, diagnostic_pusher) =
        process_sequentially
          ?diagnostic_pusher
          ctx
          fnl
          ~longlived_workers
          ~check_info
          ~typecheck_info
      in
      ( typing_result,
        delegate_state,
        telemetry,
        interrupt.MultiThreadedCall.env,
        [],
        diagnostic_pusher )
    end else begin
      Hh_logger.log "Type checking service will process files in parallel";
      let num_workers = TypecheckerOptions.num_local_workers opts in
      let workers =
        match (workers, num_workers) with
        | (Some workers, Some num_local_workers) ->
          let (workers, _) = List.split_n workers num_local_workers in
          Some workers
        | (None, _)
        | (_, None) ->
          workers
      in
      process_in_parallel
        ?diagnostic_pusher
        ctx
        workers
        delegate_state
        telemetry
        fnl
        ~interrupt
        ~memory_cap
        ~longlived_workers
        ~check_info
        ~typecheck_info
    end
  in
  let { errors; dep_edges; telemetry = typing_telemetry } = typing_result in
  Typing_deps.register_discovered_dep_edges dep_edges;

  let telemetry =
    telemetry |> Telemetry.object_ ~key:"profiling_info" ~value:typing_telemetry
  in
  ( (env, { errors; delegate_state; telemetry; diagnostic_pusher }),
    cancelled_fnl )

let go
    (ctx : Provider_context.t)
    (workers : MultiWorker.worker list option)
    (delegate_state : Delegate.state)
    (telemetry : Telemetry.t)
    (fnl : Relative_path.t list)
    ~(memory_cap : int option)
    ~(longlived_workers : bool)
    ~(use_hh_distc_instead_of_hulk : bool)
    ~(check_info : check_info) : result =
  let interrupt = MultiThreadedCall.no_interrupt () in
  let (((), result), cancelled) =
    go_with_interrupt
      ?diagnostic_pusher:None
      ctx
      workers
      delegate_state
      telemetry
      fnl
      ~interrupt
      ~memory_cap
      ~longlived_workers
      ~use_hh_distc_instead_of_hulk
      ~check_info
  in
  assert (List.is_empty cancelled);
  result
