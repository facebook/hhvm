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

module Env = Typing_env
module TLazyHeap = Typing_lazy_heap

(*****************************************************************************)
(* The place where we store the shared data in cache *)
(*****************************************************************************)

module TypeCheckStore = GlobalStorage.Make(struct
  type t = TypecheckerOptions.t
end)

let neutral = Errors.empty,
  {
    Decl_service.errs = Relative_path.Set.empty;
    lazy_decl_errs = Relative_path.Set.empty;
  }

(*****************************************************************************)
(* The job that will be run on the workers *)
(*****************************************************************************)

let type_fun opts fn x =
  match Parser_heap.find_fun_in_file ~full:true opts fn x with
  | Some f ->
    let fun_ = Naming.fun_ opts f in
    Some (Typing.fun_def opts fun_)
  | None -> None

let type_class opts fn x =
  match Parser_heap.find_class_in_file ~full:true opts fn x with
  | Some cls ->
    let class_ = Naming.class_ opts cls in
    Typing.class_def opts class_
  | None -> None

let check_typedef opts fn x =
  match Parser_heap.find_typedef_in_file ~full:true opts fn x with
  | Some t ->
    let typedef = Naming.typedef opts t in
    let ret = Typing.typedef_def opts typedef in
    Typing_variance.typedef opts x;
    Some ret
  | None -> None

let check_const opts fn x =
  match Parser_heap.find_const_in_file ~full:true opts fn x with
  | None -> None
  | Some cst ->
    let cst = Naming.global_const opts cst in
    Some (Typing.gconst_def cst opts)

let check_file opts (errors, failed, decl_failed) (fn, file_infos) =
  let { FileInfo.n_funs; n_classes; n_types; n_consts } = file_infos in
  let ignore_type_fun opts fn name =
    ignore(type_fun opts fn name) in
  let ignore_type_class opts fn name =
    ignore(type_class opts fn name) in
  let ignore_check_typedef opts fn name =
    ignore(check_typedef opts fn name) in
  let ignore_check_const opts fn name =
    ignore(check_const opts fn name) in
  let errors', (), err_flags = Errors.do_ begin fun () ->
    SSet.iter (ignore_type_fun opts fn) n_funs;
    SSet.iter (ignore_type_class opts fn) n_classes;
    SSet.iter (ignore_check_typedef opts fn) n_types;
    SSet.iter (ignore_check_const opts fn) n_consts;
  end in
  let lazy_decl_err = Errors.get_lazy_decl_flag err_flags in
  let errors = Errors.merge errors' errors in
  let failed =
    if not (Errors.is_empty errors')
    then Relative_path.Set.add failed fn
    else failed in
  let decl_failed =
    match lazy_decl_err with
    | Some file ->  Relative_path.Set.add
                      (Relative_path.Set.add decl_failed fn)
                      file
    | None -> decl_failed in
  errors, failed, decl_failed

let check_files opts (errors, err_info) fnl =
  SharedMem.invalidate_caches();
  let { Decl_service.
    errs = failed;
    lazy_decl_errs = decl_failed;
  } = err_info in
  let check_file =
    if !Utils.profile
    then (fun acc fn ->
      let t = Sys.time () in
      let result = check_file opts acc fn in
      let t' = Sys.time () in
      let duration = t' -. t in
      let filepath = Relative_path.suffix (fst fn) in
        TypingLogger.log_typing_time duration filepath;
        !Utils.log (Printf.sprintf "%f %s [type-check]" duration filepath);
        result)
    else check_file opts in
  let errors, failed, decl_failed = List.fold_left fnl
    ~f:check_file ~init:(errors, failed, decl_failed) in
  TypingLogger.flush_buffer ();

  let error_record = { Decl_service.
    errs = failed;
    lazy_decl_errs = decl_failed;
  } in
  errors, error_record

let load_and_check_files acc fnl =
  let opts = TypeCheckStore.load() in
  check_files opts acc fnl

(*****************************************************************************)
(* Let's go! That's where the action is *)
(*****************************************************************************)

let parallel_check workers opts fnl =
  TypeCheckStore.store opts;
  let result =
    MultiWorker.call
      workers
      ~job:load_and_check_files
      ~neutral
      ~merge:Decl_service.merge_lazy_decl
      ~next:(MultiWorker.next workers fnl)
  in
  TypeCheckStore.clear();
  result

let go workers opts fast =
  let fnl = Relative_path.Map.elements fast in
  if List.length fnl < 10
  then check_files opts neutral fnl
  else parallel_check workers opts fnl
