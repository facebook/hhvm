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
include Typing_service_types

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

The deferred list needs further explanation, so read on.

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

let neutral = Errors.empty

(*****************************************************************************)
(* The job that will be run on the workers *)
(*****************************************************************************)

let handle_exn_as_error : type res. Pos.t -> (unit -> res option) -> res option
    =
 fun pos f ->
  try f ()
  with e ->
    let stack = Caml.Printexc.get_raw_backtrace () in
    prerr_endline
      (Printf.sprintf
         "Exception while typechecking definition at position %s"
         (Pos.string (Pos.to_absolute pos)));
    prerr_endline (Caml.Printexc.to_string e);
    Caml.Printexc.print_raw_backtrace stderr stack;
    Errors.exception_occurred pos;
    None

let type_fun (ctx : Provider_context.t) (fn : Relative_path.t) (x : string) :
    (Tast.def * Typing_inference_env.t_global_with_pos) option =
  match Ast_provider.find_fun_in_file ~full:true ctx fn x with
  | Some f ->
    handle_exn_as_error f.Aast.f_span (fun () ->
        let fun_ = Naming.fun_ ctx f in
        Nast_check.def ctx (Aast.Fun fun_);
        let def_opt =
          Typing_toplevel.fun_def ctx fun_
          |> Option.map ~f:(fun (f, global_tvenv) -> (Aast.Fun f, global_tvenv))
        in
        Option.iter def_opt (fun (f, _) -> Tast_check.def ctx f);
        def_opt)
  | None -> None

let type_class (ctx : Provider_context.t) (fn : Relative_path.t) (x : string) :
    (Tast.def * Typing_inference_env.t_global_with_pos list) option =
  match Ast_provider.find_class_in_file ~full:true ctx fn x with
  | Some cls ->
    handle_exn_as_error cls.Aast.c_span (fun () ->
        let class_ = Naming.class_ ctx cls in
        Nast_check.def ctx (Aast.Class class_);
        let def_opt =
          Typing_toplevel.class_def ctx class_
          |> Option.map ~f:(fun (c, global_tvenv) ->
                 (Aast.Class c, global_tvenv))
        in
        Option.iter def_opt (fun (f, _) -> Tast_check.def ctx f);
        def_opt)
  | None -> None

let type_record_def
    (ctx : Provider_context.t) (fn : Relative_path.t) (x : string) :
    Tast.def option =
  match Ast_provider.find_record_def_in_file ~full:true ctx fn x with
  | Some rd ->
    handle_exn_as_error rd.Aast.rd_span (fun () ->
        let rd = Naming.record_def ctx rd in
        Nast_check.def ctx (Aast.RecordDef rd);

        let def = Aast.RecordDef (Typing_toplevel.record_def_def ctx rd) in
        Tast_check.def ctx def;
        Some def)
  | None -> None

let check_typedef (ctx : Provider_context.t) (fn : Relative_path.t) (x : string)
    : Tast.def option =
  match Ast_provider.find_typedef_in_file ~full:true ctx fn x with
  | Some t ->
    handle_exn_as_error Pos.none (fun () ->
        let typedef = Naming.typedef ctx t in
        Nast_check.def ctx (Aast.Typedef typedef);
        let ret = Typing.typedef_def ctx typedef in
        Typing_variance.typedef ctx x;
        let def = Aast.Typedef ret in
        Tast_check.def ctx def;
        Some def)
  | None -> None

let check_const (ctx : Provider_context.t) (fn : Relative_path.t) (x : string) :
    Tast.def option =
  match Ast_provider.find_gconst_in_file ~full:true ctx fn x with
  | None -> None
  | Some cst ->
    handle_exn_as_error cst.Aast.cst_span (fun () ->
        let cst = Naming.global_const ctx cst in
        Nast_check.def ctx (Aast.Constant cst);
        let def = Aast.Constant (Typing_toplevel.gconst_def ctx cst) in
        Tast_check.def ctx def;
        Some def)

let should_enable_deferring
    (opts : GlobalOptions.t) (file : check_file_computation) =
  match GlobalOptions.tco_max_times_to_defer_type_checking opts with
  | Some max_times when file.deferred_count >= max_times -> false
  | _ -> true

type process_file_results = {
  errors: Errors.t;
  computation: file_computation list;
  counters: Telemetry.t;
}

let process_file
    (dynamic_view_files : Relative_path.Set.t)
    (ctx : Provider_context.t)
    ~(profile_log : bool)
    (errors : Errors.t)
    (file : check_file_computation) : process_file_results =
  let fn = file.path in
  let ast = Ast_provider.get_ast ~full:true ctx fn in
  let opts =
    {
      (Provider_context.get_tcopt ctx) with
      GlobalOptions.tco_dynamic_view =
        Relative_path.Set.mem dynamic_view_files fn;
    }
  in
  Deferred_decl.reset
    ~enable:(should_enable_deferring opts file)
    ~threshold_opt:(GlobalOptions.tco_defer_class_declaration_threshold opts);
  let prev_counters_state = Counters.reset ~enable:profile_log in
  let (funs, classes, record_defs, typedefs, gconsts) = Nast.get_defs ast in
  let ctx = Provider_context.map_tcopt ctx ~f:(fun _tcopt -> opts) in
  let ignore_type_record_def opts fn name =
    ignore (type_record_def opts fn name)
  in
  let ignore_check_typedef opts fn name = ignore (check_typedef opts fn name) in
  let ignore_check_const opts fn name = ignore (check_const opts fn name) in
  try
    let (errors', (tasts, global_tvenvs)) =
      Errors.do_with_context fn Errors.Typing (fun () ->
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
          (fun_tasts @ class_tasts, fun_global_tvenvs @ class_global_tvenvs))
    in
    if GlobalOptions.tco_global_inference opts then
      Typing_global_inference.StateSubConstraintGraphs.build_and_save
        ctx
        tasts
        global_tvenvs;
    let deferred_files = Deferred_decl.get_deferments ~f:(fun d -> Declare d) in
    let counters = Counters.get_counters () in
    Counters.restore_state prev_counters_state;
    match deferred_files with
    | [] -> { errors = Errors.merge errors' errors; computation = []; counters }
    | _ ->
      let computation =
        List.concat
          [
            deferred_files;
            [Check { file with deferred_count = file.deferred_count + 1 }];
          ]
      in
      { errors; computation; counters }
  with e ->
    let stack = Caml.Printexc.get_raw_backtrace () in
    let () =
      prerr_endline ("Exception on file " ^ Relative_path.S.to_string fn)
    in
    Caml.Printexc.raise_with_backtrace e stack

let should_exit ~(memory_cap : int option) =
  match memory_cap with
  | None -> false
  | Some max_heap_mb ->
    (* Use [quick_stat] instead of [stat] in order to avoid walking the major
       heap on each call, and just check the major heap because the minor
       heap is a) small and b) fixed size. *)
    let heap_size_mb = Gc.((quick_stat ()).Stat.heap_words) * 8 / 1024 / 1024 in
    if heap_size_mb > max_heap_mb then (
      Hh_logger.debug
        "Exiting worker due to memory pressure: %d MB"
        heap_size_mb;
      true
    ) else
      false

let process_files
    (dynamic_view_files : Relative_path.Set.t)
    (ctx : Provider_context.t)
    (errors : Errors.t)
    (progress : computation_progress)
    ~(memory_cap : int option)
    ~(check_info : check_info) : Errors.t * computation_progress =
  SharedMem.invalidate_caches ();
  File_provider.local_changes_push_sharedmem_stack ();
  Ast_provider.local_changes_push_sharedmem_stack ();

  let profile_log start_time second_start_time_opt file result =
    let { computation; counters; _ } = result in
    let end_time = Unix.gettimeofday () in
    let times_checked = file.deferred_count + 1 in
    let files_to_declare =
      List.count computation ~f:(fun f ->
          match f with
          | Declare _ -> true
          | _ -> false)
    in
    let (time_decl_and_typecheck, time_typecheck_opt) =
      match second_start_time_opt with
      | None -> (end_time -. start_time, None)
      | Some second_start_time ->
        (second_start_time -. start_time, Some (end_time -. second_start_time))
    in
    (* "deciding_time" is what we compare against the threshold, *)
    (* to see if we should log. *)
    let deciding_time =
      Option.value time_typecheck_opt ~default:time_decl_and_typecheck
    in
    let should_log =
      deciding_time >= check_info.profile_type_check_duration_threshold
      || times_checked > 1
      || files_to_declare > 0
    in
    if should_log then (
      let filesize_opt =
        try Some (Relative_path.to_absolute file.path |> Unix.stat).Unix.st_size
        with _ -> None
      in
      let deferment_telemetry =
        Telemetry.create ()
        |> Telemetry.int_ ~key:"times_checked" ~value:times_checked
        |> Telemetry.int_ ~key:"files_to_declare" ~value:files_to_declare
      in
      let telemetry =
        counters
        |> Telemetry.float_
             ~key:"duration_decl_and_typecheck"
             ~value:time_decl_and_typecheck
        |> Telemetry.float_opt
             ~key:"duration_typecheck"
             ~value:time_typecheck_opt
        |> Telemetry.int_opt ~key:"filesize" ~value:filesize_opt
        |> Telemetry.object_ ~key:"deferment" ~value:deferment_telemetry
      in
      HackEventLogger.ProfileTypeCheck.process_file
        ~recheck_id:check_info.recheck_id
        ~path:file.path
        ~telemetry;
      let _t : float =
        Hh_logger.log_duration
          (Printf.sprintf "%s [type-check]" (Relative_path.suffix file.path))
          start_time
      in
      ()
    )
  in
  let rec process_or_exit errors progress =
    match progress.remaining with
    | fn :: fns ->
      let (errors, deferred) =
        match fn with
        | Check file ->
          let start_time = Unix.gettimeofday () in
          let result =
            process_file
              dynamic_view_files
              ctx
              check_info.profile_log
              errors
              file
          in
          let second_start_time =
            if check_info.profile_type_check_twice then
              let t = Unix.gettimeofday () in
              (* we're running this routine solely for the side effect *)
              (* of seeing how long it takes to run. *)
              let (_ignored : process_file_results) =
                process_file
                  dynamic_view_files
                  ctx
                  ~profile_log:false
                  errors
                  file
              in
              Some t
            else
              None
          in
          if check_info.profile_log then
            profile_log start_time second_start_time file result;
          (result.errors, result.computation)
        | Declare path ->
          let errors = Decl_service.decl_file ctx errors path in
          (errors, [])
        | Prefetch paths ->
          Vfs.prefetch paths;
          (errors, [])
      in
      let progress =
        {
          completed = fn :: progress.completed;
          remaining = fns;
          deferred = List.concat [deferred; progress.deferred];
        }
      in
      if should_exit memory_cap then
        (errors, progress)
      else
        process_or_exit errors progress
    | [] -> (errors, progress)
  in
  let result = process_or_exit errors progress in
  TypingLogger.flush_buffers ();
  Ast_provider.local_changes_pop_sharedmem_stack ();
  File_provider.local_changes_pop_sharedmem_stack ();
  result

let load_and_process_files
    (ctx : Provider_context.t)
    (dynamic_view_files : Relative_path.Set.t)
    (errors : Errors.t)
    (progress : computation_progress)
    ~(memory_cap : int option)
    ~(check_info : check_info) : Errors.t * computation_progress =
  (* When the type-checking worker receives SIGUSR1, display a position which
     corresponds approximately with the function/expression being checked. *)
  Sys_utils.set_signal
    Sys.sigusr1
    (Sys.Signal_handle Typing.debug_print_last_pos);
  process_files dynamic_view_files ctx errors progress ~memory_cap ~check_info

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
    (files_to_process : file_computation list ref)
    (files_initial_count : int)
    (files_in_progress : file_computation Hash_set.t)
    (files_checked_count : int ref)
    ((errors : Errors.t), (results : progress))
    (acc : Errors.t) : Errors.t =
  let () =
    match results.kind with
    | Progress -> ()
    | DelegateProgress _ ->
      delegate_state := Delegate.merge !delegate_state errors results.progress
  in
  let results = results.progress in

  files_to_process := results.remaining @ !files_to_process;

  (* Let's also prepend the deferred files! *)
  files_to_process := results.deferred @ !files_to_process;

  (* Prefetch the deferred files, if necessary *)
  files_to_process :=
    if should_prefetch_deferred_files && List.length results.deferred > 10 then
      let files_to_prefetch =
        List.fold results.deferred ~init:[] ~f:(fun acc computation ->
            match computation with
            | Declare path -> path :: acc
            | _ -> acc)
      in
      Prefetch files_to_prefetch :: !files_to_process
    else
      !files_to_process;

  (* If workers can steal work from each other, then it's possible that
    some of the files that the current worker completed checking have already
    been removed from the in-progress set. Thus, we should calculate
    the checked count by subtracting the old in-progress count from the new one
    after we remove the completed files from the set. This way, we get
    an accurate count whether there's work stealing going on or not. *)
  let old_in_progress_count = Hash_set.Poly.length files_in_progress in
  List.iter ~f:(Hash_set.Poly.remove files_in_progress) results.completed;

  (* Let's re-add the deferred files here! *)
  List.iter ~f:(Hash_set.Poly.add files_in_progress) results.deferred;
  let is_check file =
    match file with
    | Check _ -> true
    | _ -> false
  in
  let deferred_check_count = List.count ~f:is_check results.deferred in
  let completed_check_count =
    old_in_progress_count - Hash_set.Poly.length files_in_progress
  in
  files_checked_count :=
    !files_checked_count + completed_check_count - deferred_check_count;
  ServerProgress.send_percentage_progress_to_monitor
    ~operation:"typechecking"
    ~done_count:!files_checked_count
    ~total_count:files_initial_count
    ~unit:"files";
  Errors.merge errors acc

let next
    (workers : MultiWorker.worker list option)
    (delegate_state : Delegate.state ref)
    (files_to_process : file_computation list ref)
    (files_in_progress : file_computation Hash_set.Poly.t) =
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
      begin
        match (!files_to_process, stolen) with
        | ([], []) when Hash_set.Poly.is_empty files_in_progress -> Bucket.Done
        | ([], []) -> Bucket.Wait
        | (jobs, stolen_jobs) ->
          ignore (jobs, stolen_jobs, num_workers, state);
          let jobs =
            if List.length jobs > List.length stolen_jobs then
              jobs
            else begin
              delegate_state := state;
              let stolen_jobs =
                List.map stolen_jobs ~f:(fun job ->
                    Hash_set.Poly.remove files_in_progress job;
                    match job with
                    | Check { path; deferred_count } ->
                      Check { path; deferred_count = deferred_count + 1 }
                    | _ -> failwith "unexpected state")
              in
              List.rev_append stolen_jobs jobs
            end
          in
          begin
            match num_workers with
            (* When num_workers is zero, the execution mode is delegate-only, so we give an empty bucket to MultiWorker for execution. *)
            | 0 -> return_bucket_job Progress [] jobs
            | _ ->
              let bucket_size =
                Bucket.calculate_bucket_size
                  ~num_jobs:(List.length !files_to_process)
                  ~num_workers
                  ~max_size
              in
              let (current_bucket, remaining_jobs) =
                List.split_n jobs bucket_size
              in
              return_bucket_job Progress current_bucket remaining_jobs
          end
      end

let on_cancelled
    (next : unit -> 'a Bucket.bucket)
    (files_to_process : 'b Hash_set.Poly.elt list ref)
    (files_in_progress : 'b Hash_set.Poly.t) : unit -> 'a list =
 fun () ->
  (* The size of [files_to_process] is bounded only by repo size, but
      [files_in_progress] is capped at [(worker count) * (max bucket size)]. *)
  files_to_process :=
    Hash_set.Poly.to_list files_in_progress @ !files_to_process;
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
    (fnl : file_computation list)
    ~(interrupt : 'a MultiWorker.interrupt_config)
    ~(memory_cap : int option)
    ~(check_info : check_info) :
    Errors.t * Delegate.state * Telemetry.t * 'a * Relative_path.t list =
  let delegate_state = ref delegate_state in
  let files_to_process = ref fnl in
  let files_in_progress = Hash_set.Poly.create () in
  let files_processed_count = ref 0 in
  let files_initial_count = List.length fnl in
  ServerProgress.send_percentage_progress_to_monitor
    ~operation:"typechecking"
    ~done_count:0
    ~total_count:files_initial_count
    ~unit:"files";
  let next = next workers delegate_state files_to_process files_in_progress in
  let should_prefetch_deferred_files =
    Vfs.is_vfs ()
    && TypecheckerOptions.prefetch_deferred_files
         (Provider_context.get_tcopt ctx)
  in
  let job =
    load_and_process_files ctx dynamic_view_files ~memory_cap ~check_info
  in
  let job (errors : Errors.t) (progress : progress) =
    let (errors, computation_progress) =
      match progress.kind with
      | Progress -> job errors progress.progress
      | DelegateProgress job -> Delegate.process job
    in
    (errors, { progress with progress = computation_progress })
  in
  let (errors, env, cancelled_results) =
    MultiWorker.call_with_interrupt
      workers
      ~job
      ~neutral
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
  (errors, !delegate_state, telemetry, env, paths_of cancelled_results)

type ('a, 'b, 'c, 'd) job_result = 'a * 'b * 'c * 'd * Relative_path.t list

module type Mocking_sig = sig
  val with_test_mocking :
    (* real job payload, that we can modify... *)
    file_computation list ->
    ((* ... before passing it to the real job executor... *)
     file_computation list ->
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
      List.partition_map fnl ~f:(fun computation ->
          match computation with
          | Check { path; _ } ->
            if is_cancelled path then
              `Fst path
            else
              `Snd computation
          | _ -> `Snd computation)
    in
    (* Only cancel once to avoid infinite loops *)
    cancelled := Relative_path.Set.empty;
    let (res, delegate_state, telemetry, env, cancelled) = f fnl in
    (res, delegate_state, telemetry, env, mock_cancelled @ cancelled)
end

module Mocking =
( val if Injector_config.use_test_stubbing then
        (module TestMocking : Mocking_sig)
      else
        (module NoMocking : Mocking_sig) )

let should_process_sequentially
    (opts : TypecheckerOptions.t) (fnl : file_computation list) : bool =
  (* If decls can be deferred, then we should process in parallel, since
    we are likely to have more computations than there are files to type check. *)
  let defer_threshold =
    TypecheckerOptions.defer_class_declaration_threshold opts
  in
  match (defer_threshold, List.length fnl) with
  | (None, file_count) when file_count < 10 -> true
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
    ~(check_info : check_info) :
    (Errors.t, Delegate.state, Telemetry.t, 'a) job_result =
  let fnl = List.map fnl ~f:(fun path -> Check { path; deferred_count = 0 }) in
  Mocking.with_test_mocking fnl @@ fun fnl ->
  let result =
    if should_process_sequentially (Provider_context.get_tcopt ctx) fnl then (
      Hh_logger.log "Type checking service will process files sequentially";
      let progress = { completed = []; remaining = fnl; deferred = [] } in
      let (errors, _) =
        process_files
          dynamic_view_files
          ctx
          neutral
          progress
          ~memory_cap:None
          ~check_info
      in
      (errors, delegate_state, telemetry, interrupt.MultiThreadedCall.env, [])
    ) else (
      Hh_logger.log "Type checking service will process files in parallel";
      process_in_parallel
        ctx
        dynamic_view_files
        workers
        delegate_state
        telemetry
        fnl
        ~interrupt
        ~memory_cap
        ~check_info
    )
  in
  let url_opt =
    HackEventLogger.ProfileTypeCheck.get_telemetry_url_opt
      ~profile_log:check_info.profile_log
      ~init_id:check_info.init_id
      ~recheck_id:check_info.recheck_id
  in
  Option.iter url_opt ~f:(fun s -> Hh_logger.log "%s" s);
  result

let go
    (ctx : Provider_context.t)
    (workers : MultiWorker.worker list option)
    (delegate_state : Delegate.state)
    (telemetry : Telemetry.t)
    (dynamic_view_files : Relative_path.Set.t)
    (fnl : Relative_path.t list)
    ~(memory_cap : int option)
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
      ~check_info
  in
  assert (List.is_empty cancelled);
  (res, delegate_state, telemetry)
