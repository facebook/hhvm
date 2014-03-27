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

let check_if_extends_class target_class_name class_name acc =
  let class_ = Typing_env.Classes.get class_name in
  match class_ with
  | None -> acc
  | Some { Typing_defs.tc_ancestors = imps; _ }
      when SMap.mem target_class_name imps -> SSet.add class_name acc
  | _ -> acc

let find_child_classes target_class_name files_info files =
  SharedMem.invalidate_caches();
  SSet.fold begin fun fn acc ->
    (try
      let { FileInfo.classes } = SMap.find_unsafe fn files_info in
      List.fold_left begin fun acc cid ->
         check_if_extends_class target_class_name (snd cid) acc
        end acc classes
    with Not_found ->
      acc)
  end files SSet.empty

let get_child_classes workers files_info class_name =
  match Naming_heap.ClassHeap.get class_name with
  | Some class_ ->
    (* Find the files that contain classes that extend class_ *)
    let cid = snd class_.Nast.c_name in
    let trace = ref ISet.empty in
    let cid_hash = Typing_deps.Dep.make (Typing_deps.Dep.Class cid) in
    let extend_deps =
        Typing_compare.get_extend_deps trace cid_hash (ISet.singleton cid_hash)
    in
    let files = Typing_deps.get_files extend_deps in
    (* Iterate over classes defined in those files to build a set of all
       child classes *)
    find_child_classes class_name files_info files
  | _ ->
    SSet.empty

let get_deps_set classes =
  SSet.fold (fun class_name acc ->
    match Naming_heap.ClassHeap.get class_name with
    | Some class_ ->
        (* Get all files with dependencies on this class *)
        let fn = Pos.filename (fst class_.Nast.c_name) in
        let cid = snd class_.Nast.c_name in
        let dep = Typing_deps.Dep.Class cid in
        let bazooka = Typing_deps.get_bazooka dep in
        let files = Typing_deps.get_files bazooka in
        let files = SSet.add fn files in
        SSet.union files acc
    | _ -> acc) classes SSet.empty

let get_deps_set_function f_name =
  try
    let fun_ = Naming_heap.FunHeap.find_unsafe f_name in
    let fn = Pos.filename (fst fun_.Nast.f_name) in
    let fid = snd fun_.Nast.f_name in
    let dep = Typing_deps.Dep.Fun fid in
    let bazooka = Typing_deps.get_bazooka dep in
    let files = Typing_deps.get_files bazooka in
    SSet.add fn files
  with Not_found -> SSet.empty

let find_refs target_classes target_method acc file_names =
  Find_refs.find_refs_class_name := target_classes;
  Find_refs.find_refs_method_name := target_method;
  Find_refs.find_refs_results := Find_refs.PosMap.empty;
  ServerIdeUtils.recheck file_names;
  let result = !Find_refs.find_refs_results in
  Find_refs.find_refs_class_name := None;
  Find_refs.find_refs_method_name := None;
  Find_refs.find_refs_results := Find_refs.PosMap.empty;
  Find_refs.PosMap.fold begin fun p str acc ->
    (str, p) :: acc
  end result []

let parallel_find_refs workers files target_classes target_method =
  MultiWorker.call
    workers
    ~job:(find_refs target_classes target_method)
    ~neutral:([])
    ~merge:(List.rev_append)
    ~next:(Bucket.make files)

let find_references workers target_classes target_method file_list =
  if List.length file_list < 10 then
    find_refs target_classes target_method [] file_list
  else
    parallel_find_refs workers file_list target_classes target_method

let get_dependent_files_function workers f_name =
  (* This is performant enough to not need to go parallel for now *)
  get_deps_set_function f_name

let get_dependent_files workers input_set =
  (* This is performant enough to not need to go parallel for now *)
  get_deps_set input_set
