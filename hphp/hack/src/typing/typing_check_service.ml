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

let type_fun tcopt fn x =
  match Parser_heap.find_fun_in_file fn x with
  | Some f ->
    let fun_ = Naming.fun_ tcopt f in
    Typing.fun_def tcopt fun_
  | None -> ()

let type_class tcopt fn x =
  match Parser_heap.find_class_in_file fn x with
  | Some cls ->
    let class_ = Naming.class_ tcopt cls in
    Typing.class_def tcopt class_
  | None -> ()

let check_typedef tcopt fn x =
  match Parser_heap.find_typedef_in_file fn x with
  | Some t ->
    let typedef = Naming.typedef tcopt t in
    Typing.typedef_def typedef;
    Typing_variance.typedef tcopt x
  | None -> ()

let check_const tcopt fn x =
  match Parser_heap.find_const_in_file fn x with
  | None -> ()
  | Some cst ->
    let cst = Naming.global_const tcopt cst in
    match cst.Nast.cst_value with
    | None -> ()
    | Some v ->
      let filename = Pos.filename (fst cst.Nast.cst_name) in
      let dep = Typing_deps.Dep.GConst (snd cst.Nast.cst_name) in
      let env =
        Typing_env.empty TypecheckerOptions.default filename (Some dep) in
      let env = Typing_env.set_mode env cst.Nast.cst_mode in
      let env, value_type = Typing.expr env v in
      match cst.Nast.cst_type with
      | Some h ->
        let declared_type = Decl_hint.hint env.Typing_env.decl_env h in
        let env, dty = Typing_phase.localize_with_self env declared_type in
        let _env = Typing_utils.sub_type env value_type dty in
        ()
      | None -> ()

let check_file tcopt (errors, failed, decl_failed) (fn, file_infos) =
  let { FileInfo.n_funs; n_classes; n_types; n_consts } = file_infos in
  let errors', (), err_flags = Errors.do_ begin fun () ->
    SSet.iter (type_fun tcopt fn) n_funs;
    SSet.iter (type_class tcopt fn) n_classes;
    SSet.iter (check_typedef tcopt fn) n_types;
    SSet.iter (check_const tcopt fn) n_consts;
  end in
  let lazy_decl_err = Errors.get_lazy_decl_flag err_flags in
  let errors = Errors.merge errors' errors in
  let failed =
    if not (Errors.is_empty errors')
    then Relative_path.Set.add failed fn
    else failed in
  let decl_failed =
    if lazy_decl_err
    then Relative_path.Set.add decl_failed fn
    else decl_failed in
  errors, failed, decl_failed

let check_files tcopt (errors, err_info) fnl =
  SharedMem.invalidate_caches();
  let { Decl_service.
    errs = failed;
    lazy_decl_errs = decl_failed;
  } = err_info in
  let check_file =
    if !Utils.profile
    then (fun acc fn ->
      let t = Unix.gettimeofday () in
      let result = check_file tcopt acc fn in
      let t' = Unix.gettimeofday () in
      let msg =
        Printf.sprintf "%f %s [type-check]" (t' -. t)
          (Relative_path.suffix (fst fn)) in
      !Utils.log msg;
      result)
    else check_file tcopt in
  let errors, failed, decl_failed = List.fold_left fnl
    ~f:check_file ~init:(errors, failed, decl_failed) in
  let error_record = { Decl_service.
    errs = failed;
    lazy_decl_errs = decl_failed;
  } in
  errors, error_record

let load_and_check_files acc fnl =
  let tcopt = TypeCheckStore.load() in
  check_files tcopt acc fnl

(*****************************************************************************)
(* Let's go! That's where the action is *)
(*****************************************************************************)

let parallel_check workers tcopt fnl =
  TypeCheckStore.store tcopt;
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

let go workers tcopt fast =
  let fnl = Relative_path.Map.elements fast in
  if List.length fnl < 10
  then check_files tcopt neutral fnl
  else parallel_check workers tcopt fnl
