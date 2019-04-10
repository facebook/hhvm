(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Hack_bucket = Bucket
open Core_kernel
module Bucket = Hack_bucket

module Env = Typing_env
module TLazyHeap = Typing_lazy_heap

type file = Relative_path.t * FileInfo.names
type progress = {
  checked_files: file list;
  unchecked_files: file list;
}

(*****************************************************************************)
(* The place where we store the shared data in cache *)
(*****************************************************************************)

module TypeCheckStore = GlobalStorage.Make(struct
  type t = TypecheckerOptions.t
end)

let neutral = Errors.empty

(*****************************************************************************)
(* The job that will be run on the workers *)
(*****************************************************************************)

let type_fun opts fn x =
  match Parser_heap.find_fun_in_file ~full:true fn x with
  | Some f ->
    let fun_ = Naming.fun_ f in
    Nast_check.def (Nast.Fun fun_);
    let def_opt = Typing.fun_def opts fun_
      |> Option.map ~f:(fun f -> Tast.Fun f) in
    Option.iter def_opt Tast_check.def;
    def_opt
  | None -> None

let type_class opts fn x =
  match Parser_heap.find_class_in_file ~full:true fn x with
  | Some cls ->
    let class_ = Naming.class_ cls in
    Nast_check.def (Nast.Class class_);
    let def_opt =
      Typing.class_def opts class_
      |> Option.map ~f:(fun c -> Tast.Class c)
    in
    Option.iter def_opt Tast_check.def;
    def_opt
  | None -> None

let check_typedef opts fn x =
  match Parser_heap.find_typedef_in_file ~full:true fn x with
  | Some t ->
    let typedef = Naming.typedef t in
    Nast_check.def (Nast.Typedef typedef);
    let ret = Typing.typedef_def opts typedef in
    Typing_variance.typedef opts x;
    let def = Tast.Typedef ret in
    Tast_check.def def;
    Some def
  | None -> None

let check_const opts fn x =
  match Parser_heap.find_const_in_file ~full:true fn x with
  | None -> None
  | Some cst ->
    let cst = Naming.global_const cst in
    let def = Tast.Constant (Typing.gconst_def opts cst) in
    Tast_check.def def;
    Some def

let check_file dynamic_view_files opts errors (fn, file_infos) =
  let tco_new_inference = opts.GlobalOptions.tco_new_inference in
  let opts = {
      opts with
      GlobalOptions.tco_dynamic_view = Relative_path.Set.mem dynamic_view_files fn;
  } in
  (* Only use new_inference on rough proportion of files specified by new_inference option *)
  let opts =
    if float (Hashtbl.hash fn % 1000) >= tco_new_inference *. 1000.0
    then { opts with GlobalOptions.tco_new_inference = 0.0 }
    else (Measure.sample "new_inference" 1.0; opts) in
  let { FileInfo.n_funs; n_classes; n_types; n_consts } = file_infos in
  let ignore_type_fun opts fn name =
    ignore(type_fun opts fn name) in
  let ignore_type_class opts fn name =
    ignore(type_class opts fn name) in
  let ignore_check_typedef opts fn name =
    ignore(check_typedef opts fn name) in
  let ignore_check_const opts fn name =
    ignore(check_const opts fn name) in
  try
    let errors', () = Errors.do_with_context fn Errors.Typing
        begin fun () ->
      SSet.iter (ignore_type_fun opts fn) n_funs;
      SSet.iter (ignore_type_class opts fn) n_classes;
      SSet.iter (ignore_check_typedef opts fn) n_types;
      SSet.iter (ignore_check_const opts fn) n_consts;
    end in
    Errors.merge errors' errors
  with e ->
    let stack = Caml.Printexc.get_raw_backtrace () in
    let () = prerr_endline ("Exception on file " ^ (Relative_path.S.to_string fn)) in
    Caml.Printexc.raise_with_backtrace e stack

let check_files dynamic_view_files opts errors progress ~memory_cap =
  SharedMem.invalidate_caches();
  File_heap.FileHeap.LocalChanges.push_stack ();
  Parser_heap.ParserHeap.LocalChanges.push_stack ();
  let check_file =
    if !Utils.profile
    then (fun acc fn ->
      let t = Sys.time () in
      let result = check_file dynamic_view_files opts acc fn in
      let t' = Sys.time () in
      let duration = t' -. t in
      let filepath = Relative_path.suffix (fst fn) in
      TypingLogger.TypingTimes.log duration filepath;
      !Utils.log (Printf.sprintf "%f %s [type-check]" duration filepath);
      result)
    else check_file dynamic_view_files opts
  in
  let rec check_or_exit errors progress =
    match progress.unchecked_files with
    | fn :: fns ->
      let errors = check_file errors fn in
      let progress = {
        checked_files = fn :: progress.checked_files;
        unchecked_files = fns;
      } in
      (* Use [quick_stat] instead of [stat] in order to avoid walking the major
         heap on each call, and just check the major heap because the minor
         heap is a) small and b) fixed size. *)
      let should_exit = match memory_cap with
        | None -> false
        | Some max_heap_mb ->
          let heap_size_mb = Gc.((quick_stat ()).Stat.heap_words) * 8 / 1024 / 1024 in
          if heap_size_mb > max_heap_mb then
            let error_msg =
              Printf.sprintf "Exiting worker due to memory pressure: %d MB" heap_size_mb
            in
            !Utils.log error_msg;
            true
          else false
      in
      if should_exit then (errors, progress) else check_or_exit errors progress
    | [] -> errors, progress
  in
  let result = check_or_exit errors progress in
  TypingLogger.flush_buffers ();
  Parser_heap.ParserHeap.LocalChanges.pop_stack ();
  File_heap.FileHeap.LocalChanges.pop_stack ();
  result

let load_and_check_files dynamic_view_files errors progress ~memory_cap =
  let opts = TypeCheckStore.load() in
  check_files dynamic_view_files opts errors progress ~memory_cap

(*****************************************************************************)
(* Let's go! That's where the action is *)
(*****************************************************************************)

(** Merge the results from multiple workers.

    We don't really care about which files are left unchecked since we use
    (gasp) mutation to track that, so combine the errors but always return an
    empty list for the list of unchecked files. *)
let merge files_to_check files_to_check_count files_in_progress files_checked
    files_checked_count (errors, results) acc =
  files_to_check := results.unchecked_files @ !files_to_check;
  List.iter ~f:(Hash_set.Poly.remove files_in_progress) results.checked_files;
  files_checked := results.checked_files @ !files_checked;
  files_checked_count := (List.length results.checked_files) + !files_checked_count;
  ServerProgress.send_percentage_progress_to_monitor
    "typechecking" !files_checked_count files_to_check_count "files";
  Decl_service.merge_lazy_decl errors acc

let next workers ~num_jobs files_to_check files_in_progress =
  let max_size = Bucket.max_size () in
  let num_workers = (match workers with Some w -> List.length w | None -> 1) in
  let bucket_size = Bucket.calculate_bucket_size ~num_jobs ~num_workers ~max_size in
  fun () ->
    match !files_to_check with
    | [] when Hash_set.Poly.is_empty files_in_progress -> Bucket.Done
    | [] -> Bucket.Wait
    | jobs ->
      let unchecked_files, remaining_jobs = List.split_n jobs bucket_size in

      (* Update our shared mutable state, because hey: it's not like we're
         writing OCaml or anything. *)
      files_to_check := remaining_jobs;
      List.iter ~f:(Hash_set.Poly.add files_in_progress) unchecked_files;
      Bucket.Job { checked_files = []; unchecked_files }

let on_cancelled next files_to_check files_in_progress =
  fun () ->
    (* The size of [files_to_check] is bounded only by repo size, but
      [files_in_progress] is capped at [(worker count) * (max bucket size)]. *)
    files_to_check := (Hash_set.Poly.to_list files_in_progress) @ !files_to_check;
    let rec add_next acc =
      match next () with
      | Bucket.Job j -> add_next (j :: acc)
      | Bucket.Wait
      | Bucket.Done -> acc
    in
    add_next []

let parallel_check dynamic_view_files workers opts fnl ~interrupt ~memory_cap =
  TypeCheckStore.store opts;
  let files_to_check = ref fnl in
  let files_in_progress = Hash_set.Poly.create () in
  let files_checked = ref [] in
  let files_checked_count = ref 0 in
  let num_jobs = List.length fnl in
  ServerProgress.send_percentage_progress_to_monitor
    "typechecking" 0 num_jobs "files";
  let next = next ~num_jobs workers files_to_check files_in_progress in
  let errors, env, cancelled =
    MultiWorker.call_with_interrupt
      workers
      ~job:(load_and_check_files dynamic_view_files ~memory_cap)
      ~neutral
      ~merge:(merge files_to_check num_jobs files_in_progress files_checked files_checked_count)
      ~next
      ~on_cancelled:(on_cancelled next files_to_check files_in_progress)
      ~interrupt
  in
  TypeCheckStore.clear();
  let sort =
    List.sort ~compare:(fun (path1, _) (path2, _) -> Relative_path.S.compare path1 path2)
  in
  (* TODO: (wipi) Remove this check and [files_checked] after we have more
  confidence that the worker memory cap logic is well-formed. *)
  if cancelled = [] && Option.is_some memory_cap then assert (sort fnl = sort !files_checked);
  errors, env, List.concat (cancelled |> List.map ~f:(fun progress -> progress.unchecked_files))

type 'a job = Relative_path.t * 'a
type ('a, 'b, 'c) job_result = 'b * 'c * 'a job list

module type Mocking_sig = sig
  val with_test_mocking :
    (* real job payload, that we can modify... *)
    'a job list ->
    (* ... before passing it to the real job executor... *)
    ('a job list -> ('a, 'b, 'c) job_result) ->
    (* ... which output we can also modify. *)
    ('a, 'b, 'c) job_result
end

module NoMocking = struct
  let with_test_mocking fnl f = f fnl
end

module TestMocking = struct

  let cancelled = ref Relative_path.Set.empty
  let set_is_cancelled x =
    cancelled := Relative_path.Set.add !cancelled x
  let is_cancelled x = Relative_path.Set.mem !cancelled x

  let with_test_mocking fnl f =
    let mock_cancelled, fnl =
      List.partition_tf fnl ~f:(fun (path, _) -> is_cancelled path) in
    (* Only cancel once to avoid infinite loops *)
    cancelled := Relative_path.Set.empty;
    let res, env, cancelled = f fnl in
    res, env, mock_cancelled @ cancelled
end

module Mocking =
  (val (if Injector_config.use_test_stubbing
  then (module TestMocking : Mocking_sig)
  else (module NoMocking : Mocking_sig)
))

let go_with_interrupt workers opts dynamic_view_files fast ~interrupt ~memory_cap =
  let fnl = Relative_path.Map.elements fast in
  Mocking.with_test_mocking fnl @@ fun fnl ->
    if List.length fnl < 10
    then
      let progress = { checked_files = []; unchecked_files = fnl } in
      let (errors, _) = check_files dynamic_view_files opts neutral progress ~memory_cap:None in
      errors, interrupt.MultiThreadedCall.env, []
    else parallel_check dynamic_view_files workers opts fnl ~interrupt ~memory_cap

let go workers opts dynamic_view_files fast ~memory_cap =
  let interrupt = MultiThreadedCall.no_interrupt () in
  let res, (), cancelled =
    go_with_interrupt workers opts dynamic_view_files fast ~interrupt ~memory_cap in
  assert (cancelled = []);
  res
