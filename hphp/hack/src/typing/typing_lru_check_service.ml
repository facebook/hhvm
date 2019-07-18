(**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Typing_check_service

(* Helper function to process a single [file_computation] *)
let process_file_computation
    ~(dynamic_view_files: Relative_path.Set.t)
    ~(opts: GlobalOptions.t)
    ~(fc: file_computation)
    ~(errors: Errors.t)
    : Errors.t * file_computation list =
  let process_file_wrapper = process_file dynamic_view_files opts in
  match fc with
  | path, Check info ->
    process_file_wrapper errors (path, info)
  | path, Declare ->
    let errors = Decl_service.decl_file errors path in
    (errors, [])

let process_in_parallel
    (dynamic_view_files: Relative_path.Set.t)
    (lru_host_env: Shared_lru.host_env)
    (opts: TypecheckerOptions.t)
    (fnl: file_computation list)
    ~(interrupt: 'a MultiWorker.interrupt_config)
    : Errors.t * 'a * file_computation list =
  TypeCheckStore.store opts;
  let files_initial_count = List.length fnl in
  ServerProgress.send_percentage_progress_to_monitor
    "typechecking" 0 files_initial_count "files";

  let partition_list
      ~(fnl: file_computation list)
      ~(batch_size: int) =
    let rec partition_list_h
        ~(acc: (file_computation list) list)
        ~(fnl: file_computation list)
        ~(batch_size: int) =
      match fnl with
      | [] -> acc
      | _ -> begin
        let batch, remaining = List.split_n fnl batch_size in
        partition_list_h ~acc:(batch :: acc) ~fnl:remaining ~batch_size
      end
    in
    partition_list_h ~acc:[] ~fnl ~batch_size
  in

  let job (fc_lst: file_computation list) =
    let opts = TypeCheckStore.load() in

    (* inner job proceses a single file *)
    let inner_job (errors: Errors.t) (fc: file_computation) =
      let e2, _ =
        process_file_computation
          ~dynamic_view_files
          ~opts
          ~errors:Errors.empty
          ~fc
      in
      Errors.merge errors e2
    in
    List.fold fc_lst ~init:Errors.empty ~f:inner_job, (List.length fc_lst)
  in

  let reduce (e, m) (e2, m2) =
    ServerProgress.send_percentage_progress_to_monitor
      "typechecking" (m + m2) files_initial_count "files";
    Errors.merge e e2, (m + m2)
  in

  let errors, _ = Shared_lru.run
    ~host_env:lru_host_env
    ~job
    ~reduce
    ~inputs:(partition_list ~fnl ~batch_size:500)
  in
  let cancelled = [] in
  let env = interrupt.MultiThreadedCall.env in
  TypeCheckStore.clear();
  let updated_file_computations =
    List.concat (cancelled |> List.map ~f:(fun progress -> progress.remaining))
  in
  errors, env, updated_file_computations


(* Disclaimer: does not actually go with interrupt yet, although it will
 * in a future version. The function is named the same as the one in
 * [typing_check_service] to easily call the new one in [serverTypeCheck] *)
let go_with_interrupt
    (lru_host_env: Shared_lru.host_env)
    (opts: TypecheckerOptions.t)
    (dynamic_view_files: Relative_path.Set.t)
    (fnl: (Relative_path.t * FileInfo.names) list)
    ~(interrupt: 'a MultiWorker.interrupt_config)
    : (computation_kind, Errors.t, 'a) job_result =
  let fnl = List.map fnl ~f:(fun (path, names) -> path, Check names) in
  process_in_parallel
    dynamic_view_files
    lru_host_env
    opts
    fnl
    ~interrupt

let go
    (lru_host_env: Shared_lru.host_env)
    (opts: TypecheckerOptions.t)
    (dynamic_view_files: Relative_path.Set.t)
    (fnl: (Relative_path.t * FileInfo.names) list)
    : Errors.t =
  let interrupt = MultiThreadedCall.no_interrupt () in
  let res, (), cancelled =
    go_with_interrupt
      lru_host_env
      opts
      dynamic_view_files
      fnl
      ~interrupt
  in
  assert (cancelled = []);
  res
