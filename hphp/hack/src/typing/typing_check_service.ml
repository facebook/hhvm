(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core

module Env = Typing_env
module TLazyHeap = Typing_lazy_heap

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
    Some (Typing.gconst_def opts cst)

let check_file dynamic_view_files opts errors (fn, file_infos) =

  let opts = {
      opts with
      GlobalOptions.tco_dynamic_view = Relative_path.Set.mem dynamic_view_files fn;
  } in
  let { FileInfo.n_funs; n_classes; n_types; n_consts } = file_infos in
  let ignore_type_fun opts fn name =
    ignore(type_fun opts fn name) in
  let ignore_type_class opts fn name =
    ignore(type_class opts fn name) in
  let ignore_check_typedef opts fn name =
    ignore(check_typedef opts fn name) in
  let ignore_check_const opts fn name =
    ignore(check_const opts fn name) in
  let errors', () = Errors.do_with_context fn Errors.Typing
      begin fun () ->
    SSet.iter (ignore_type_fun opts fn) n_funs;
    SSet.iter (ignore_type_class opts fn) n_classes;
    SSet.iter (ignore_check_typedef opts fn) n_types;
    SSet.iter (ignore_check_const opts fn) n_consts;
  end in
  Errors.merge errors' errors

let check_files dynamic_view_files opts errors fnl  =
  SharedMem.invalidate_caches();
  let check_file =
    if !Utils.profile
    then (fun acc fn ->
      let t = Sys.time () in
      let result = check_file dynamic_view_files opts acc fn in
      let t' = Sys.time () in
      let duration = t' -. t in
      let filepath = Relative_path.suffix (fst fn) in
        TypingLogger.log_typing_time duration filepath;
        !Utils.log (Printf.sprintf "%f %s [type-check]" duration filepath);
        result)
    else check_file dynamic_view_files opts in
  let errors = List.fold_left fnl ~f:check_file ~init:errors in
  TypingLogger.flush_buffer ();
  errors

let load_and_check_files dynamic_view_files acc fnl =
  let opts = TypeCheckStore.load() in
  check_files dynamic_view_files opts acc fnl

(*****************************************************************************)
(* Let's go! That's where the action is *)
(*****************************************************************************)

let parallel_check dynamic_view_files workers opts fnl =
  TypeCheckStore.store opts;
  let result =
    MultiWorker.call
      workers
      ~job:(load_and_check_files dynamic_view_files)
      ~neutral
      ~merge:Decl_service.merge_lazy_decl
      ~next:(MultiWorker.next workers fnl)
  in
  TypeCheckStore.clear();
  result

let go workers opts dynamic_view_files fast =
  let fnl = Relative_path.Map.elements fast in
  if List.length fnl < 10
  then check_files dynamic_view_files opts neutral fnl
  else parallel_check dynamic_view_files workers opts fnl
