(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Hack_bucket = Bucket
open Hh_prelude
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

type progress = job_progress

let neutral : unit -> typing_result =
 fun () ->
  {
    errors = Errors.empty;
    dep_edges = Typing_deps.dep_edges_make ();
    telemetry = Telemetry.create ();
    jobs_finished_to_end = Measure.create ();
    jobs_finished_early = Measure.create ();
  }

let should_enable_deferring
    (opts : GlobalOptions.t) (file : check_file_computation) =
  match GlobalOptions.tco_max_times_to_defer_type_checking opts with
  | Some max_times when file.deferred_count >= max_times -> false
  | _ -> true

type process_file_results = {
  errors: Errors.t;
  deferred_decls: Deferred_decl.deferment list;
}

let process_file_remote_execution
    (_dynamic_view_files : Relative_path.Set.t)
    (ctx : Provider_context.t)
    (errors : Errors.t)
    (file : check_file_computation) : process_file_results =
  let fn = file.path in
  let deps_mode = Provider_context.get_deps_mode ctx in
  let errors' = Re.process_file fn deps_mode in
  { errors = Errors.merge errors' errors; deferred_decls = [] }

let process_file
    (dynamic_view_files : Relative_path.Set.t)
    (ctx : Provider_context.t)
    (errors : Errors.t)
    (file : check_file_computation) : process_file_results =
  let fn = file.path in
  let (errors', ast) = Ast_provider.get_ast_with_error ~full:true ctx fn in
  if not (Errors.is_empty errors') then
    { errors = Errors.merge errors' errors; deferred_decls = [] }
  else
    let opts =
      {
        (Provider_context.get_tcopt ctx) with
        GlobalOptions.tco_dynamic_view =
          Relative_path.Set.mem dynamic_view_files fn;
      }
    in
    let (funs, classes, record_defs, typedefs, gconsts) = Nast.get_defs ast in
    let ctx = Provider_context.map_tcopt ctx ~f:(fun _tcopt -> opts) in
    let ignore_type_record_def opts fn name =
      ignore (type_record_def opts fn name)
    in
    let ignore_check_typedef opts fn name =
      ignore (check_typedef opts fn name)
    in
    let ignore_check_const opts fn name = ignore (check_const opts fn name) in
    try
      let result =
        Deferred_decl.with_deferred_decls
          ~enable:(should_enable_deferring opts file)
          ~declaration_threshold_opt:
            (GlobalOptions.tco_defer_class_declaration_threshold opts)
          ~memory_mb_threshold_opt:
            (GlobalOptions.tco_defer_class_memory_mb_threshold opts)
        @@ fun () ->
        Errors.do_with_context fn Errors.Typing @@ fun () ->
        let (fun_tasts, fun_global_tvenvs) =
          List.map funs ~f:snd
          |> List.filter_map ~f:(type_fun ctx fn)
          |> List.unzip
        in
        let (class_tasts, class_global_tvenvs) =
          List.map classes ~f:snd
          |> List.filter_map ~f:(type_class ctx fn)
          |> List.unzip
        in
        let class_global_tvenvs = List.concat class_global_tvenvs in
        List.map record_defs ~f:snd
        |> List.iter ~f:(ignore_type_record_def ctx fn);
        List.map typedefs ~f:snd |> List.iter ~f:(ignore_check_typedef ctx fn);
        List.map gconsts ~f:snd |> List.iter ~f:(ignore_check_const ctx fn);
        (fun_tasts @ class_tasts, fun_global_tvenvs @ class_global_tvenvs)
      in
      match result with
      | Ok (errors', (tasts, global_tvenvs)) ->
        if GlobalOptions.tco_global_inference opts then
          Typing_global_inference.StateSubConstraintGraphs.build_and_save
            ctx
            tasts
            global_tvenvs;
        { errors = Errors.merge errors' errors; deferred_decls = [] }
      | Error deferred_decls -> { errors; deferred_decls }
    with
    | WorkerCancel.Worker_should_exit as e ->
      (* Cancellation requests must be re-raised *)
      raise e
    | e ->
      let stack = Caml.Printexc.get_raw_backtrace () in
      let () =
        prerr_endline ("Exception on file " ^ Relative_path.S.to_string fn)
      in
      Caml.Printexc.raise_with_backtrace e stack

let get_mem_telemetry () : Telemetry.t option =
  if SharedMem.hh_log_level () > 0 then
    Some
      ( Telemetry.create ()
      |> Telemetry.object_ ~key:"gc" ~value:(Telemetry.quick_gc_stat ())
      |> Telemetry.object_ ~key:"shmem" ~value:(SharedMem.get_telemetry ()) )
  else
    None

let profile_log
    ~(check_info : check_info)
    ~(start_counters : Counters.time_in_sec * Telemetry.t)
    ~(end_counters : Counters.time_in_sec * Telemetry.t)
    ~(second_run_end_counters : (Counters.time_in_sec * Telemetry.t) option)
    ~(start_heap_mb : int)
    ~(end_heap_mb : int)
    ~(file : check_file_computation)
    ~(result : process_file_results) : unit =
  let (start_time, start_counters) = start_counters in
  let (end_time, end_counters) = end_counters in
  let duration = end_time -. start_time in
  let duration_second_run =
    Option.map second_run_end_counters ~f:(fun (time, _) -> time -. end_time)
  in
  let deciding_time = Option.value duration_second_run ~default:duration in
  (* "deciding_time" is what we compare against the threshold, to see if we should log. *)
  (* We'll also log if it had been previously deferred, or if it's being deferred right now. *)
  let should_log =
    Float.(deciding_time >= check_info.profile_type_check_duration_threshold)
    || end_heap_mb - start_heap_mb
       >= check_info.profile_type_check_memory_threshold_mb
    || file.deferred_count > 0
    || not (List.is_empty result.deferred_decls)
  in
  if should_log then begin
    let profile = Telemetry.diff ~all:false ~prev:start_counters end_counters in
    let profile_second_run =
      Option.map second_run_end_counters ~f:(fun (_, counters) ->
          Telemetry.diff ~all:false ~prev:end_counters counters)
    in
    let filesize_opt =
      try Some (Relative_path.to_absolute file.path |> Unix.stat).Unix.st_size
      with Unix.Unix_error _ -> None
    in
    let deferment_telemetry =
      Telemetry.create ()
      |> Telemetry.int_ ~key:"times_checked" ~value:(file.deferred_count + 1)
      |> Telemetry.int_
           ~key:"files_to_declare"
           ~value:(List.length result.deferred_decls)
    in
    let telemetry =
      Telemetry.create ()
      |> Telemetry.int_opt ~key:"filesize" ~value:filesize_opt
      |> Telemetry.object_ ~key:"deferment" ~value:deferment_telemetry
      |> Telemetry.object_ ~key:"profile" ~value:profile
      |> Telemetry.int_ ~key:"start_heap_mb" ~value:start_heap_mb
      |> Telemetry.int_ ~key:"end_heap_mb" ~value:end_heap_mb
    in
    let telemetry =
      Option.fold
        ~init:telemetry
        profile_second_run
        ~f:(fun telemetry profile ->
          Telemetry.object_ telemetry ~key:"profile_second_run" ~value:profile)
    in
    HackEventLogger.ProfileTypeCheck.process_file
      ~recheck_id:check_info.recheck_id
      ~path:file.path
      ~telemetry;
    Hh_logger.log
      "%s [%s] %fs %dMiB->%dMiB%s"
      (Relative_path.suffix file.path)
      ( if List.is_empty result.deferred_decls then
        "type-check"
      else
        "discover-decl-deps" )
      (Option.value duration_second_run ~default:duration)
      start_heap_mb
      end_heap_mb
      ( if SharedMem.hh_log_level () > 0 then
        "\n" ^ Telemetry.to_string telemetry
      else
        "" )
  end

let read_counters () : Counters.time_in_sec * Telemetry.t =
  let typecheck_time = Counters.read_time Counters.Category.Typecheck in
  let mem_telemetry = get_mem_telemetry () in
  let operations_counters = Counters.get_counters () in
  ( typecheck_time,
    Telemetry.create ()
    |> Telemetry.object_opt ~key:"memory" ~value:mem_telemetry
    |> Telemetry.object_ ~key:"operations" ~value:operations_counters )

module ProcessFilesTally = struct
  (** Counters for the [file_computation] of each sort being processed *)
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

  let incr_caps tally =
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
end

let get_heap_size () = Gc.((quick_stat ()).Stat.heap_words) * 8 / 1024 / 1024

external hh_malloc_trim : unit -> unit = "hh_malloc_trim"

(* At start of file processing or when the memory cap is exceeded, we
 * clear all non-essential state (caches, logger) and force a GC.
 * The call to `hh_malloc_trim` ensures that memory is returned to
 * the system.
 *)
let clear_caches_and_force_gc () =
  SharedMem.invalidate_caches ();
  HackEventLogger.flush ();
  Gc.compact ();
  hh_malloc_trim ()

let process_files
    (dynamic_view_files : Relative_path.Set.t)
    (ctx : Provider_context.t)
    ({ errors; dep_edges; telemetry; jobs_finished_early; jobs_finished_to_end } :
      typing_result)
    (progress : computation_progress)
    ~(memory_cap : int option)
    ~(longlived_workers : bool)
    ~(remote_execution : bool)
    ~(check_info : check_info) : typing_result * computation_progress =
  if not longlived_workers then SharedMem.invalidate_caches ();
  File_provider.local_changes_push_sharedmem_stack ();
  Ast_provider.local_changes_push_sharedmem_stack ();

  Decl_counters.set_mode check_info.profile_decling;
  let _prev_counters_state = Counters.reset () in
  let (_start_counter_time, start_counters) = read_counters () in
  let tally = ProcessFilesTally.empty in
  let start_file_count = List.length progress.remaining in
  let start_time = Unix.gettimeofday () in

  let rec process_or_exit errors progress tally max_heap_mb =
    (* If the major heap has exceeded the bounds, we
      (1) first try and bring the size back down by flushing the parser cache and doing a major GC;
      (2) if this fails, we decline to typecheck the remaining files.
    We use [quick_stat] instead of [stat] in get_heap_size in order to avoid walking the major heap,
    and we don't change the minor heap because it's small and fixed-size.
    The start-remaining test is to make sure we make at least one file of progress
    even in case of a crazy low memory cap. *)
    let heap_mb = get_heap_size () in
    let max_heap_mb = Int.max heap_mb max_heap_mb in
    let cap = Option.value memory_cap ~default:Int.max_value in
    let over_cap =
      heap_mb > cap && start_file_count > List.length progress.remaining
    in
    let (exit_now, tally, heap_mb) =
      if over_cap then
        let new_heap_mb =
          if longlived_workers then begin
            clear_caches_and_force_gc ();
            get_heap_size ()
          end else
            heap_mb
        in
        (new_heap_mb > cap, ProcessFilesTally.incr_caps tally, new_heap_mb)
      else
        (false, tally, heap_mb)
    in
    match progress.remaining with
    | [] -> (errors, progress, tally, heap_mb, max_heap_mb)
    | _ when exit_now ->
      let cgroup_stats = CGroup.get_stats () in
      (match cgroup_stats with
      | Error _ -> ()
      | Ok { CGroup.total; _ } ->
        Measure.sample "worker_cgroup_total" (float_of_int total));
      (errors, progress, tally, heap_mb, max_heap_mb)
    | fn :: fns ->
      let (errors, deferred, tally) =
        match fn with
        | Check file ->
          let process_file () =
            if remote_execution then
              process_file_remote_execution dynamic_view_files ctx errors file
            else
              process_file dynamic_view_files ctx errors file
          in
          let result =
            if check_info.profile_log then (
              let start_counters = read_counters () in
              let start_heap_mb = heap_mb in
              let result = process_file () in
              let end_counters = read_counters () in
              let end_heap_mb = get_heap_size () in
              let (start_heap_mb, end_heap_mb, second_run_end_counters) =
                if check_info.profile_type_check_twice then
                  (* we're running this routine solely for the side effect *)
                  (* of seeing how much time+memory it takes to run. *)
                  let _ignored = process_file () in
                  (end_heap_mb, get_heap_size (), Some (read_counters ()))
                else
                  (start_heap_mb, end_heap_mb, None)
              in
              profile_log
                ~check_info
                ~start_counters
                ~end_counters
                ~second_run_end_counters
                ~start_heap_mb
                ~end_heap_mb
                ~file
                ~result;
              result
            ) else
              process_file ()
          in
          let tally =
            ProcessFilesTally.incr_checks tally result.deferred_decls
          in
          let deferred =
            if List.is_empty result.deferred_decls then
              []
            else
              List.map result.deferred_decls ~f:(fun fn -> Declare fn)
              @ [Check { file with deferred_count = file.deferred_count + 1 }]
          in
          (result.errors, deferred, tally)
        | Declare (_path, class_name) ->
          let (_ : Decl_provider.class_decl option) =
            Decl_provider.get_class ctx class_name
          in
          (errors, [], ProcessFilesTally.incr_decls tally)
        | Prefetch paths ->
          Vfs.prefetch paths;
          (errors, [], ProcessFilesTally.incr_prefetches tally)
      in
      let progress =
        {
          completed = fn :: progress.completed;
          remaining = fns;
          deferred = List.concat [deferred; progress.deferred];
        }
      in
      process_or_exit errors progress tally max_heap_mb
  in

  (* Process as many files as we can, and merge in their errors *)
  let (errors, progress, tally, final_heap_mb, max_heap_mb) =
    process_or_exit errors progress tally 0
  in

  (* Update edges *)
  let new_dep_edges =
    Typing_deps.flush_ideps_batch (Provider_context.get_deps_mode ctx)
  in
  let dep_edges = Typing_deps.merge_dep_edges dep_edges new_dep_edges in

  (* Gather up our various forms of telemetry... *)
  let (_end_counter_time, end_counters) = read_counters () in
  (* Note: the 'add' operation (performed here, and also later in case of
  MultiWorker.merge) will strip all non-numbers from telemetry. *)
  let telemetry =
    Telemetry.add
      telemetry
      (Telemetry.diff
         ~all:false
         ~suffix_keys:false
         end_counters
         ~prev:start_counters)
  in
  let processed_file_count =
    start_file_count - List.length progress.remaining
  in
  let processed_file_fraction =
    float_of_int processed_file_count /. float_of_int start_file_count
  in
  let record =
    if List.is_empty progress.remaining then
      jobs_finished_to_end
    else
      jobs_finished_early
  in
  let open ProcessFilesTally in
  Measure.sample ~record "seconds" (Unix.gettimeofday () -. start_time);
  Measure.sample ~record "final_heap_mb" (float_of_int final_heap_mb);
  Measure.sample ~record "files" (float_of_int processed_file_count);
  Measure.sample ~record "files_fraction" processed_file_fraction;
  Measure.sample ~record "decls" (float_of_int tally.decls);
  Measure.sample ~record "prefetches" (float_of_int tally.prefetches);
  Measure.sample ~record "checks_done" (float_of_int tally.checks_done);
  Measure.sample ~record "checks_deferred" (float_of_int tally.checks_deferred);
  Measure.sample ~record "decls_deferred" (float_of_int tally.decls_deferred);
  Measure.sample
    ~record
    "exceeded_cap_count"
    (float_of_int tally.exceeded_cap_count);
  Measure.sample ~record "max_heap_mb" (float_of_int max_heap_mb);

  TypingLogger.flush_buffers ();
  Ast_provider.local_changes_pop_sharedmem_stack ();
  File_provider.local_changes_pop_sharedmem_stack ();
  ( { errors; dep_edges; telemetry; jobs_finished_early; jobs_finished_to_end },
    progress )

let load_and_process_files
    (ctx : Provider_context.t)
    (dynamic_view_files : Relative_path.Set.t)
    (typing_result : typing_result)
    (progress : computation_progress)
    ~(memory_cap : int option)
    ~(longlived_workers : bool)
    ~(remote_execution : bool)
    ~(check_info : check_info) : typing_result * computation_progress =
  (* When the type-checking worker receives SIGUSR1, display a position which
     corresponds approximately with the function/expression being checked. *)
  Sys_utils.set_signal
    Sys.sigusr1
    (Sys.Signal_handle Typing.debug_print_last_pos);
  process_files
    dynamic_view_files
    ctx
    typing_result
    progress
    ~memory_cap
    ~longlived_workers
    ~remote_execution
    ~check_info

(*****************************************************************************)
(* Let's go! That's where the action is *)
(*****************************************************************************)

(** Merge the results from multiple workers.

    We don't really care about which files are left unchecked since we use
    (gasp) mutation to track that, so combine the errors but always return an
    empty list for the list of unchecked files. *)
let merge
    ~(should_prefetch_deferred_files : bool)
    (delegate_state : Delegate.state ref)
    (files_to_process : file_computation BigList.t ref)
    (files_initial_count : int)
    (files_in_progress : file_computation Hash_set.t)
    (files_checked_count : int ref)
    ((produced_by_job : typing_result), (progress : progress))
    (acc : typing_result) : typing_result =
  let () =
    match progress.kind with
    | Progress -> ()
    | DelegateProgress _ ->
      delegate_state :=
        Delegate.merge !delegate_state produced_by_job.errors progress.progress
  in
  let progress = progress.progress in

  files_to_process := BigList.append progress.remaining !files_to_process;

  (* Let's also prepend the deferred files! *)
  files_to_process := BigList.append progress.deferred !files_to_process;

  (* Prefetch the deferred files, if necessary *)
  files_to_process :=
    if should_prefetch_deferred_files && List.length progress.deferred > 10 then
      let files_to_prefetch =
        List.fold progress.deferred ~init:[] ~f:(fun acc computation ->
            match computation with
            | Declare (path, _) -> path :: acc
            | _ -> acc)
      in
      BigList.cons (Prefetch files_to_prefetch) !files_to_process
    else
      !files_to_process;

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
      ~f:(fun acc computation ->
        match Hash_set.Poly.strict_remove files_in_progress computation with
        | Ok () ->
          begin
            match computation with
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
  let is_check file =
    match file with
    | Check _ -> true
    | _ -> false
  in
  let deferred_check_count = List.count ~f:is_check progress.deferred in
  let completed_check_count = completed_check_count - deferred_check_count in

  files_checked_count := !files_checked_count + completed_check_count;
  let delegate_progress =
    Typing_service_delegate.get_progress !delegate_state
  in
  ServerProgress.send_percentage_progress
    ~operation:"typechecking"
    ~done_count:!files_checked_count
    ~total_count:files_initial_count
    ~unit:"files"
    ~extra:delegate_progress;
  accumulate_job_output produced_by_job acc

let next
    (workers : MultiWorker.worker list option)
    (delegate_state : Delegate.state ref)
    (files_to_process : file_computation BigList.t ref)
    (files_in_progress : file_computation Hash_set.Poly.t)
    (record : Measure.record) =
  let max_size = Bucket.max_size () in
  let num_workers =
    match workers with
    | Some w -> List.length w
    | None -> 1
  in
  let return_bucket_job kind ~current_bucket ~remaining_jobs =
    (* Update our shared mutable state, because hey: it's not like we're
       writing OCaml or anything. *)
    files_to_process := remaining_jobs;
    List.iter ~f:(Hash_set.Poly.add files_in_progress) current_bucket;
    Bucket.Job
      {
        kind;
        progress = { completed = []; remaining = current_bucket; deferred = [] };
      }
  in
  fun () ->
    Measure.time ~record "time" @@ fun () ->
    let (state, delegate_job) =
      Typing_service_delegate.next
        !files_to_process
        files_in_progress
        !delegate_state
    in
    delegate_state := state;

    let (stolen, state) = Typing_service_delegate.steal state max_size in
    (* If a delegate job is returned, then that means that it should be done
      by the next MultiWorker worker (the one for whom we're creating a job
      in this function). If delegate job is None, then the regular (local
      type checking) logic applies. *)
    match delegate_job with
    | Some { current_bucket; remaining_jobs; job } ->
      return_bucket_job (DelegateProgress job) current_bucket remaining_jobs
    | None ->
      (* WARNING: the following List.length is costly - for a full init, files_to_process starts
      out as the size of the entire repo, and we're traversing the entire list. *)
      let files_to_process_length = BigList.length !files_to_process in
      (match (files_to_process_length, stolen) with
      | (0, []) when Hash_set.Poly.is_empty files_in_progress -> Bucket.Done
      | (0, []) -> Bucket.Wait
      | (_, stolen_jobs) ->
        let jobs =
          if files_to_process_length > List.length stolen_jobs then
            !files_to_process
          else begin
            Hh_logger.log
              "Steal payload from local workers: %d jobs"
              (List.length stolen_jobs);
            delegate_state := state;
            let stolen_jobs =
              List.map stolen_jobs ~f:(fun job ->
                  Hash_set.Poly.remove files_in_progress job;
                  match job with
                  | Check { path; deferred_count } ->
                    Check { path; deferred_count = deferred_count + 1 }
                  | _ -> failwith "unexpected state")
            in
            BigList.rev_append stolen_jobs !files_to_process
          end
        in
        begin
          match num_workers with
          (* When num_workers is zero, the execution mode is delegate-only, so we give an empty bucket to MultiWorker for execution. *)
          | 0 -> return_bucket_job Progress [] jobs
          | _ ->
            let bucket_size =
              Bucket.calculate_bucket_size
                ~num_jobs:files_to_process_length
                ~num_workers
                ~max_size
            in
            let (current_bucket, remaining_jobs) =
              BigList.split_n jobs bucket_size
            in
            return_bucket_job Progress current_bucket remaining_jobs
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

(**
  `next` and `merge` both run in the master process and update mutable
  state in order to track work in progress and work remaining.
  `job` runs in each worker and does not have access to this mutable state.
 *)
let process_in_parallel
    (ctx : Provider_context.t)
    (dynamic_view_files : Relative_path.Set.t)
    (workers : MultiWorker.worker list option)
    (delegate_state : Delegate.state)
    (telemetry : Telemetry.t)
    (fnl : file_computation BigList.t)
    ~(interrupt : 'a MultiWorker.interrupt_config)
    ~(memory_cap : int option)
    ~(longlived_workers : bool)
    ~(remote_execution : bool)
    ~(check_info : check_info) :
    typing_result * Delegate.state * Telemetry.t * 'a * Relative_path.t list =
  let record = Measure.create () in
  (* [record] is used by [next] *)
  let delegate_state = ref delegate_state in
  let files_to_process = ref fnl in
  let files_in_progress = Hash_set.Poly.create () in
  let files_processed_count = ref 0 in
  let files_initial_count = BigList.length fnl in
  let delegate_progress =
    Typing_service_delegate.get_progress !delegate_state
  in
  ServerProgress.send_percentage_progress
    ~operation:"typechecking"
    ~done_count:0
    ~total_count:files_initial_count
    ~unit:"files"
    ~extra:delegate_progress;

  let next =
    next workers delegate_state files_to_process files_in_progress record
  in
  let should_prefetch_deferred_files =
    Vfs.is_vfs ()
    && TypecheckerOptions.prefetch_deferred_files
         (Provider_context.get_tcopt ctx)
  in
  let job =
    load_and_process_files
      ctx
      dynamic_view_files
      ~memory_cap
      ~longlived_workers
      ~remote_execution
      ~check_info
  in
  let job (typing_result : typing_result) (progress : progress) =
    let (typing_result, computation_progress) =
      match progress.kind with
      | Progress -> job typing_result progress.progress
      | DelegateProgress job -> Delegate.process job
    in
    (typing_result, { progress with progress = computation_progress })
  in
  let (typing_result, env, cancelled_results) =
    MultiWorker.call_with_interrupt
      workers
      ~job
      ~neutral:(neutral ())
      ~merge:
        (merge
           ~should_prefetch_deferred_files
           delegate_state
           files_to_process
           files_initial_count
           files_in_progress
           files_processed_count)
      ~next
      ~on_cancelled:(on_cancelled next files_to_process files_in_progress)
      ~interrupt
  in
  let telemetry =
    Typing_service_delegate.add_telemetry !delegate_state telemetry
    |> Telemetry.object_
         ~key:"next"
         ~value:(Measure.stats_to_telemetry ~record ())
  in
  let paths_of (cancelled_results : progress list) : Relative_path.t list =
    let paths_of (cancelled_progress : progress) =
      let cancelled_computations = cancelled_progress.progress.remaining in
      let paths_of paths (cancelled_computation : file_computation) =
        match cancelled_computation with
        | Check { path; _ } -> path :: paths
        | _ -> paths
      in
      List.fold cancelled_computations ~init:[] ~f:paths_of
    in
    List.concat (List.map cancelled_results ~f:paths_of)
  in
  (typing_result, !delegate_state, telemetry, env, paths_of cancelled_results)

type ('a, 'b, 'c, 'd) job_result = 'a * 'b * 'c * 'd * Relative_path.t list

module type Mocking_sig = sig
  val with_test_mocking :
    (* real job payload, that we can modify... *)
    file_computation BigList.t ->
    ((* ... before passing it to the real job executor... *)
     file_computation BigList.t ->
    ('a, 'b, 'c, 'd) job_result) ->
    (* ... which output we can also modify. *)
    ('a, 'b, 'c, 'd) job_result
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
    let (res, delegate_state, telemetry, env, cancelled) =
      f (BigList.create fnl)
    in
    (res, delegate_state, telemetry, env, mock_cancelled @ cancelled)
end

module Mocking =
( val if Injector_config.use_test_stubbing then
        (module TestMocking : Mocking_sig)
      else
        (module NoMocking : Mocking_sig) )

let should_process_sequentially
    (opts : TypecheckerOptions.t) (fnl : file_computation BigList.t) : bool =
  (* If decls can be deferred, then we should process in parallel, since
    we are likely to have more computations than there are files to type check. *)
  let defer_threshold =
    TypecheckerOptions.defer_class_declaration_threshold opts
  in
  let parallel_threshold =
    TypecheckerOptions.parallel_type_checking_threshold opts
  in
  match (defer_threshold, BigList.length fnl) with
  | (None, file_count) when file_count < parallel_threshold -> true
  | _ -> false

let go_with_interrupt
    (ctx : Provider_context.t)
    (workers : MultiWorker.worker list option)
    (delegate_state : Delegate.state)
    (telemetry : Telemetry.t)
    (dynamic_view_files : Relative_path.Set.t)
    (fnl : Relative_path.t list)
    ~(interrupt : 'a MultiWorker.interrupt_config)
    ~(memory_cap : int option)
    ~(longlived_workers : bool)
    ~(remote_execution : bool)
    ~(check_info : check_info)
    ~(profiling : CgroupProfiler.Profiling.t) :
    (Errors.t, Delegate.state, Telemetry.t, 'a) job_result =
  let opts = Provider_context.get_tcopt ctx in
  let sample_rate = GlobalOptions.tco_typecheck_sample_rate opts in
  let fnl = BigList.create fnl in
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
    BigList.map fnl ~f:(fun path -> Check { path; deferred_count = 0 })
  in
  Mocking.with_test_mocking fnl @@ fun fnl ->
  let (typing_result, delegate_state, telemetry, env, cancelled_fnl) =
    if should_process_sequentially opts fnl then begin
      Hh_logger.log "Type checking service will process files sequentially";
      let progress =
        { completed = []; remaining = BigList.as_list fnl; deferred = [] }
      in
      let (typing_result, _progress) =
        process_files
          dynamic_view_files
          ctx
          (neutral ())
          progress
          ~memory_cap:None
          ~longlived_workers
          ~remote_execution
          ~check_info
      in
      ( typing_result,
        delegate_state,
        telemetry,
        interrupt.MultiThreadedCall.env,
        [] )
    end else begin
      Hh_logger.log "Type checking service will process files in parallel";
      let workers =
        match (workers, TypecheckerOptions.num_local_workers opts) with
        | (Some workers, Some num_local_workers) ->
          let (workers, _) = List.split_n workers num_local_workers in
          Some workers
        | (None, _)
        | (_, None) ->
          workers
      in
      process_in_parallel
        ctx
        dynamic_view_files
        workers
        delegate_state
        telemetry
        fnl
        ~interrupt
        ~memory_cap
        ~longlived_workers
        ~remote_execution
        ~check_info
    end
  in
  Typing_deps.register_discovered_dep_edges typing_result.dep_edges;
  let cgroup_total_max =
    Base.Option.value ~default:0.0 (Measure.get_max "worker_cgroup_total")
  in
  CgroupProfiler.Profiling.record_stats
    ~profiling
    ~stage:"type check"
    ~metric:"cgroup_total"
    ~value:cgroup_total_max;

  if check_info.profile_log then
    Hh_logger.log
      "Typecheck perf: %s"
      (HackEventLogger.ProfileTypeCheck.get_telemetry_url
         ~init_id:check_info.init_id
         ~recheck_id:check_info.recheck_id);
  let job_size_telemetry =
    Telemetry.create ()
    |> Telemetry.object_
         ~key:"finished_to_end"
         ~value:
           (Measure.stats_to_telemetry
              ~record:typing_result.jobs_finished_to_end
              ())
    |> Telemetry.object_
         ~key:"finished_early"
         ~value:
           (Measure.stats_to_telemetry
              ~record:typing_result.jobs_finished_early
              ())
  in
  let telemetry =
    telemetry
    |> Telemetry.object_ ~key:"profiling_info" ~value:typing_result.telemetry
    |> Telemetry.object_ ~key:"job_sizes" ~value:job_size_telemetry
  in
  (typing_result.errors, delegate_state, telemetry, env, cancelled_fnl)

let go
    ?(profiling : CgroupProfiler.Profiling.t = CgroupProfiler.Profiling.empty)
    (ctx : Provider_context.t)
    (workers : MultiWorker.worker list option)
    (delegate_state : Delegate.state)
    (telemetry : Telemetry.t)
    (dynamic_view_files : Relative_path.Set.t)
    (fnl : Relative_path.t list)
    ~(memory_cap : int option)
    ~(longlived_workers : bool)
    ~(remote_execution : bool)
    ~(check_info : check_info) : Errors.t * Delegate.state * Telemetry.t =
  let interrupt = MultiThreadedCall.no_interrupt () in
  let (res, delegate_state, telemetry, (), cancelled) =
    go_with_interrupt
      ctx
      workers
      delegate_state
      telemetry
      dynamic_view_files
      fnl
      ~interrupt
      ~memory_cap
      ~longlived_workers
      ~remote_execution
      ~check_info
      ~profiling
  in
  assert (List.is_empty cancelled);
  (res, delegate_state, telemetry)
