(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Hack_bucket = Bucket
open Core_kernel
module Bucket = Hack_bucket

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

type check_file_computation = {
  path: Relative_path.t;
  names: FileInfo.names;
  deferred_count: int;
}

type file_computation =
  | Check of check_file_computation
  | Declare of Relative_path.t
  | Prefetch of Relative_path.t list

type progress = {
  completed: file_computation list;
  remaining: file_computation list;
  deferred: file_computation list;
}

type check_info = {
  init_id: string;
  recheck_id: string option;
}

(*****************************************************************************)
(* The place where we store the shared data in cache *)
(*****************************************************************************)

module TypeCheckStore = GlobalStorage.Make (struct
  type t = TypecheckerOptions.t
end)

let neutral = Errors.empty

(*****************************************************************************)
(* The job that will be run on the workers *)
(*****************************************************************************)

let type_fun (opts : TypecheckerOptions.t) (fn : Relative_path.t) (x : string)
    : Tast.def option =
  match Ast_provider.find_fun_in_file ~full:true fn x with
  | Some f ->
    let fun_ = Naming.fun_ f in
    Nast_check.def (Aast.Fun fun_);
    let def_opt =
      Typing.fun_def opts fun_ |> Option.map ~f:(fun f -> Aast.Fun f)
    in
    Option.iter def_opt (Tast_check.def opts);
    def_opt
  | None -> None

let type_class
    (opts : TypecheckerOptions.t) (fn : Relative_path.t) (x : string) :
    Tast.def option =
  match Ast_provider.find_class_in_file ~full:true fn x with
  | Some cls ->
    let class_ = Naming.class_ cls in
    Nast_check.def (Aast.Class class_);
    let def_opt =
      Typing.class_def opts class_ |> Option.map ~f:(fun c -> Aast.Class c)
    in
    Option.iter def_opt (Tast_check.def opts);
    def_opt
  | None -> None

let check_typedef
    (opts : TypecheckerOptions.t)
    (fn : Relative_path.t)
    (x : Decl_provider.typedef_key) : Tast.def option =
  match Ast_provider.find_typedef_in_file ~full:true fn x with
  | Some t ->
    let typedef = Naming.typedef t in
    Nast_check.def (Aast.Typedef typedef);
    let ret = Typing.typedef_def opts typedef in
    Typing_variance.typedef opts x;
    let def = Aast.Typedef ret in
    Tast_check.def opts def;
    Some def
  | None -> None

let check_const
    (opts : TypecheckerOptions.t) (fn : Relative_path.t) (x : string) :
    Tast.def option =
  match Ast_provider.find_gconst_in_file ~full:true fn x with
  | None -> None
  | Some cst ->
    let cst = Naming.global_const cst in
    let def = Aast.Constant (Typing.gconst_def opts cst) in
    Tast_check.def opts def;
    Some def

let should_enable_deferring
    (opts : GlobalOptions.t) (file : check_file_computation) =
  match GlobalOptions.tco_max_times_to_defer_type_checking opts with
  | Some max_times when file.deferred_count >= max_times -> false
  | _ -> true

let process_file
    (dynamic_view_files : Relative_path.Set.t)
    (opts : GlobalOptions.t)
    (errors : Errors.t)
    (file : check_file_computation) : Errors.t * file_computation list =
  Deferred_decl.reset ~enable:(should_enable_deferring opts file);
  let fn = file.path in
  let file_infos = file.names in
  let opts =
    {
      opts with
      GlobalOptions.tco_dynamic_view =
        Relative_path.Set.mem dynamic_view_files fn;
    }
  in
  let { FileInfo.n_funs; n_classes; n_types; n_consts } = file_infos in
  let ignore_type_fun opts fn name = ignore (type_fun opts fn name) in
  let ignore_type_class opts fn name = ignore (type_class opts fn name) in
  let ignore_check_typedef opts fn name =
    ignore (check_typedef opts fn name)
  in
  let ignore_check_const opts fn name = ignore (check_const opts fn name) in
  try
    let (errors', ()) =
      Errors.do_with_context fn Errors.Typing (fun () ->
          SSet.iter (ignore_type_fun opts fn) n_funs;
          SSet.iter (ignore_type_class opts fn) n_classes;
          SSet.iter (ignore_check_typedef opts fn) n_types;
          SSet.iter (ignore_check_const opts fn) n_consts)
    in
    let deferred_files = Deferred_decl.get ~f:(fun d -> Declare d) in
    let result =
      match deferred_files with
      | [] -> (Errors.merge errors' errors, [])
      | _ ->
        ( errors,
          List.concat
            [
              deferred_files;
              [Check { file with deferred_count = file.deferred_count + 1 }];
            ] )
    in
    result
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
    let heap_size_mb =
      Gc.((quick_stat ()).Stat.heap_words) * 8 / 1024 / 1024
    in
    if heap_size_mb > max_heap_mb then (
      let error_msg =
        Printf.sprintf
          "Exiting worker due to memory pressure: %d MB"
          heap_size_mb
      in
      !Utils.log error_msg;
      true
    ) else
      false

let process_files
    (dynamic_view_files : Relative_path.Set.t)
    (opts : GlobalOptions.t)
    (errors : Errors.t)
    (progress : progress)
    ~(memory_cap : int option)
    ~(check_info : check_info) : Errors.t * progress =
  SharedMem.invalidate_caches ();
  File_provider.local_changes_push_stack ();
  Ast_provider.local_changes_push_stack ();

  let process_file_profiled dynamic_view_files opts acc file =
    let start_time = Unix.gettimeofday () in
    let result = process_file dynamic_view_files opts acc file in
    let files_to_declare =
      List.count (snd result) ~f:(fun f ->
          match f with
          | Declare _ -> true
          | _ -> false)
    in
    let filepath = Relative_path.suffix file.path in
    TypingLogger.ProfileTypeCheck.log
      ~init_id:check_info.init_id
      ~recheck_id:check_info.recheck_id
      ~start_time
      ~times_checked:(file.deferred_count + 1)
      ~files_to_declare
      ~absolute:(Relative_path.to_absolute file.path)
      ~relative:filepath;
    let _t =
      Hh_logger.log_duration
        (Printf.sprintf "%s [type-check]" filepath)
        start_time
    in
    result
  in
  let process_file_wrapper =
    if !Utils.profile then
      process_file_profiled
    else
      process_file
  in
  let rec process_or_exit errors progress =
    match progress.remaining with
    | fn :: fns ->
      let (errors, deferred) =
        match fn with
        | Check file ->
          process_file_wrapper dynamic_view_files opts errors file
        | Declare path ->
          let errors = Decl_service.decl_file errors path in
          (errors, [])
        | Prefetch paths ->
          Vfs.prefetch paths;
          (Errors.empty, [])
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
  Ast_provider.local_changes_pop_stack ();
  File_provider.local_changes_pop_stack ();
  result

let load_and_process_files
    (dynamic_view_files : Relative_path.Set.t)
    (errors : Errors.t)
    (progress : progress)
    ~(memory_cap : int option)
    ~(check_info : check_info) : Errors.t * progress =
  let opts = TypeCheckStore.load () in
  process_files dynamic_view_files opts errors progress ~memory_cap ~check_info

(*****************************************************************************)
(* Let's go! That's where the action is *)
(*****************************************************************************)

(** Merge the results from multiple workers.

    We don't really care about which files are left unchecked since we use
    (gasp) mutation to track that, so combine the errors but always return an
    empty list for the list of unchecked files. *)
let merge
    ~(should_prefetch_deferred_files : bool)
    (files_to_process : file_computation list ref)
    (files_initial_count : int)
    (files_in_progress : file_computation Hash_set.t)
    (files_checked_count : int ref)
    ((errors : Errors.t), (results : progress))
    (acc : Errors.t) : Errors.t =
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
  List.iter ~f:(Hash_set.Poly.remove files_in_progress) results.completed;

  (* Let's re-add the deferred files here! *)
  List.iter ~f:(Hash_set.Poly.add files_in_progress) results.deferred;
  let is_check file =
    match file with
    | Check _ -> true
    | _ -> false
  in
  let deferred_check_count = List.count ~f:is_check results.deferred in
  let completed_check_count = List.count ~f:is_check results.completed in
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
    (files_to_process : file_computation list ref)
    (files_in_progress : file_computation Hash_set.Poly.t) =
  let max_size = Bucket.max_size () in
  let num_workers =
    match workers with
    | Some w -> List.length w
    | None -> 1
  in
  fun () ->
    let bucket_size =
      Bucket.calculate_bucket_size
        ~num_jobs:(List.length !files_to_process)
        ~num_workers
        ~max_size
    in
    match !files_to_process with
    | [] when Hash_set.Poly.is_empty files_in_progress -> Bucket.Done
    | [] -> Bucket.Wait
    | jobs ->
      let (current_bucket, remaining_files) = List.split_n jobs bucket_size in
      (* Update our shared mutable state, because hey: it's not like we're
         writing OCaml or anything. *)
      files_to_process := remaining_files;
      List.iter ~f:(Hash_set.Poly.add files_in_progress) current_bucket;
      Bucket.Job { completed = []; remaining = current_bucket; deferred = [] }

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

let process_in_parallel
    (dynamic_view_files : Relative_path.Set.t)
    (workers : MultiWorker.worker list option)
    (opts : TypecheckerOptions.t)
    (fnl : file_computation list)
    ~(interrupt : 'a MultiWorker.interrupt_config)
    ~(memory_cap : int option)
    ~(check_info : check_info) : Errors.t * 'a * Relative_path.t list =
  TypeCheckStore.store opts;
  let files_to_process = ref fnl in
  let files_in_progress = Hash_set.Poly.create () in
  let files_processed_count = ref 0 in
  let files_initial_count = List.length fnl in
  ServerProgress.send_percentage_progress_to_monitor
    ~operation:"typechecking"
    ~done_count:0
    ~total_count:files_initial_count
    ~unit:"files";
  let next = next workers files_to_process files_in_progress in
  let should_prefetch_deferred_files =
    Vfs.is_vfs () && TypecheckerOptions.prefetch_deferred_files opts
  in
  let (errors, env, cancelled) =
    MultiWorker.call_with_interrupt
      workers
      ~job:(load_and_process_files dynamic_view_files ~memory_cap ~check_info)
      ~neutral
      ~merge:
        (merge
           ~should_prefetch_deferred_files
           files_to_process
           files_initial_count
           files_in_progress
           files_processed_count)
      ~next
      ~on_cancelled:(on_cancelled next files_to_process files_in_progress)
      ~interrupt
  in
  TypeCheckStore.clear ();
  ( errors,
    env,
    List.concat
      ( cancelled
      |> List.map ~f:(fun progress ->
             progress.remaining
             |> List.fold ~init:[] ~f:(fun acc computation ->
                    match computation with
                    | Check { path; _ } -> path :: acc
                    | _ -> acc)) ) )

type ('b, 'c) job_result = 'b * 'c * Relative_path.t list

module type Mocking_sig = sig
  val with_test_mocking :
    (* real job payload, that we can modify... *)
    file_computation list ->
    ((* ... before passing it to the real job executor... *)
     file_computation list ->
    ('b, 'c) job_result) ->
    (* ... which output we can also modify. *)
    ('b, 'c) job_result
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
    let (res, env, cancelled) = f fnl in
    (res, env, mock_cancelled @ cancelled)
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
    (workers : MultiWorker.worker list option)
    (opts : TypecheckerOptions.t)
    (dynamic_view_files : Relative_path.Set.t)
    (fnl : (Relative_path.t * FileInfo.names) list)
    ~(interrupt : 'a MultiWorker.interrupt_config)
    ~(memory_cap : int option)
    ~(check_info : check_info) : (Errors.t, 'a) job_result =
  let fnl =
    List.map fnl ~f:(fun (path, names) ->
        Check { path; names; deferred_count = 0 })
  in
  Mocking.with_test_mocking fnl
  @@ fun fnl ->
  let result =
    if should_process_sequentially opts fnl then (
      Hh_logger.log "Type checking service will process files sequentially";
      let progress = { completed = []; remaining = fnl; deferred = [] } in
      let (errors, _) =
        process_files
          dynamic_view_files
          opts
          neutral
          progress
          ~memory_cap:None
          ~check_info
      in
      (errors, interrupt.MultiThreadedCall.env, [])
    ) else (
      Hh_logger.log "Type checking service will process files in parallel";
      process_in_parallel
        dynamic_view_files
        workers
        opts
        fnl
        ~interrupt
        ~memory_cap
        ~check_info
    )
  in
  if !Utils.profile then
    TypingLogger.ProfileTypeCheck.print_path
      ~init_id:check_info.init_id
      ~recheck_id:check_info.recheck_id;
  result

let go
    (workers : MultiWorker.worker list option)
    (opts : TypecheckerOptions.t)
    (dynamic_view_files : Relative_path.Set.t)
    (fnl : (Relative_path.t * FileInfo.names) list)
    ~(memory_cap : int option)
    ~(check_info : check_info) : Errors.t =
  let interrupt = MultiThreadedCall.no_interrupt () in
  let (res, (), cancelled) =
    go_with_interrupt
      workers
      opts
      dynamic_view_files
      fnl
      ~interrupt
      ~memory_cap
      ~check_info
  in
  assert (cancelled = []);
  res
