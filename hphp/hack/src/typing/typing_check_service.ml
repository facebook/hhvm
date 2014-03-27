(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


open Utils

(*****************************************************************************)
(* The place where we store the shared data in cache *)
(*****************************************************************************)

module TypeCheckStore = GlobalStorage.Make(struct
  type t = FileInfo.fast
end)

let neutral = [], SSet.empty

(*****************************************************************************)
(* The job that will be run on the workers *)
(*****************************************************************************)

let type_fun x =
  try
    let fun_ = Naming_heap.FunHeap.find_unsafe x in
    let tenv = Typing_env.empty (Pos.filename (fst fun_.Nast.f_name)) in
    Typing.fun_def tenv x fun_;
  with Not_found ->
    ()

let type_class x =
  try
    (match Naming_heap.ClassStatus.find_unsafe x with
    | Naming_heap.Error -> ()
    | Naming_heap.Ok ->
        let class_ = Naming_heap.ClassHeap.find_unsafe x in
        let filename = Pos.filename (fst class_.Nast.c_name) in
        let tenv = Typing_env.empty filename in
        Typing.class_def tenv x class_
    )
  with Not_found ->
    ()

let check_typedef x =
  try
    (match Naming_heap.TypedefStatus.find_unsafe x with
    | Naming_heap.Error -> ()
    | Naming_heap.Ok ->
        let _, args, hint as typedef = Naming_heap.TypedefHeap.find_unsafe x in
        let filename = Pos.filename (fst hint) in
        let tenv = Typing_env.empty filename in
        let tenv = Typing_env.set_root tenv (Typing_deps.Dep.Class x) in
        (* TODO It's pretty ugly that this try/catch is here whereas the others
         * are in Typing.fun_def and friends. *)
        try NastCheck.typedef tenv typedef with Typing_defs.Ignore -> ()
    )
  with Not_found ->
    ()

let check_const x =
  try
    match Typing_env.GConsts.get x with
    | None -> ()
    | Some declared_type ->
        let cst = Naming_heap.ConstHeap.find_unsafe x in
        match cst.Nast.cst_value with
        | Some v -> begin
          let filename = Pos.filename (fst cst.Nast.cst_name) in
          let env = Typing_env.empty filename in
          let env = Typing_env.set_mode env cst.Nast.cst_mode in
          let root = Typing_deps.Dep.GConst (snd cst.Nast.cst_name) in
          let env = Typing_env.set_root env root in
          let env, value_type = Typing.expr env v in
          let _env = Typing_subtype.sub_type env declared_type value_type in
          ()
        end
        | None -> ()
  with Not_found ->
    ()

let check_file fast (errors, failed) fn = try
  match SMap.get fn fast with
  | None ->
      errors, failed
  | Some { FileInfo.n_funs; n_classes; n_types; n_consts } ->
      SSet.iter type_fun n_funs;
      SSet.iter type_class n_classes;
      SSet.iter check_typedef n_types;
      SSet.iter check_const n_consts;
      errors, failed
with Utils.Error l ->
  l :: errors, SSet.add fn failed

let check_files fast (errors, failed) fnl =
  SharedMem.invalidate_caches();
  let errors, failed = List.fold_left (check_file fast) (errors, failed) fnl in
  errors, failed

let load_and_check_files acc fnl =
  let fast = TypeCheckStore.load() in
  check_files fast acc fnl


(*****************************************************************************)
(* Let's go! That's where the action is *)
(*****************************************************************************)

let parallel_check workers fast fnl =
  TypeCheckStore.store fast;
  let result =
    MultiWorker.call
      workers
      ~job:load_and_check_files
      ~neutral
      ~merge:Typing_decl_service.merge_decl
      ~next:(Bucket.make fnl)
  in
  TypeCheckStore.clear();
  result

let go workers fast =
  let fnl = SMap.fold (fun x _ y -> x :: y) fast [] in
  if List.length fnl < 10
  then check_files fast neutral fnl
  else parallel_check workers fast fnl
