(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)



(*****************************************************************************)
(* On the fly type-declaration are called when the user modified a file
 * we are not at initilalization time anymore. Therefore, we have a bit more
 * work to do. We need calculate what must be re-checked.
 *)
(*****************************************************************************)
open Hh_core
open Reordered_argument_collections
open Typing_deps

(*****************************************************************************)
(* The neutral element of declaration (cf procs/multiWorker.mli) *)
(*****************************************************************************)
let otf_neutral =  Errors.empty, Relative_path.Set.empty
let compute_deps_neutral = DepSet.empty, DepSet.empty

(*****************************************************************************)
(* This is the place where we are going to put everything necessary for
 * the redeclaration. We could "pass" the values directly to the workers,
 * but it gives too much work to the master and slows things downn.
 * So what we do instead is pass the data through shared memory via
 * OnTheFlyStore.
 * I tried replicating the data to speed things up but it had no effect.
 *)
(*****************************************************************************)

module OnTheFlyStore = GlobalStorage.Make(struct
  type t = TypecheckerOptions.t * FileInfo.fast
end)

(*****************************************************************************)
(* Re-declaring the types in a file *)
(*****************************************************************************)

(* Returns a list of files that are considered to have failed decl and must
 * be redeclared every time the typechecker discovers a file change *)
let get_decl_failures decl_errors fn =
  List.fold_left decl_errors ~f:begin fun failed error ->
    (* It is important to add the file that is the cause of the failure.
     * What can happen is that during a declaration phase, we realize
     * that a parent class is outdated. When this happens, we redeclare
     * the class, even if it is in a different file. Therefore, the file
     * where the error occurs might be different from the file we
     * are declaring right now.
     *)
    let file_with_error = Pos.filename (Errors.get_pos error) in
    assert (file_with_error <> Relative_path.default);
    let failed = Relative_path.Set.add failed file_with_error in
    let failed = Relative_path.Set.add failed fn in
    failed
  end ~init:Relative_path.Set.empty

let on_the_fly_decl_file tcopt (errors, failed) fn =
  let decl_errors, (), _ = Errors.do_ begin fun () ->
    Decl.make_env tcopt fn
  end in
  let failed' = get_decl_failures (Errors.get_error_list decl_errors) fn in
  Errors.merge decl_errors errors, Relative_path.Set.union failed failed'

(*****************************************************************************)
(* Given a set of classes, compare the old and the new type and deduce
 * what must be rechecked accordingly.
 *)
(*****************************************************************************)

let compute_classes_deps old_classes new_classes acc classes =
  let to_redecl, to_recheck = acc in
  let rdd, rdc =
    Decl_compare.get_classes_deps old_classes new_classes classes
  in
  let to_redecl = DepSet.union rdd to_redecl in
  let to_recheck = DepSet.union rdc to_recheck in
  to_redecl, to_recheck

(*****************************************************************************)
(* Given a set of functions, compare the old and the new type and deduce
 * what must be rechecked accordingly.
 *)
(*****************************************************************************)

let compute_funs_deps old_funs (to_redecl, to_recheck) funs =
  let rdd, rdc = Decl_compare.get_funs_deps old_funs funs in
  let to_redecl = DepSet.union rdd to_redecl in
  let to_recheck = DepSet.union rdc to_recheck in
  to_redecl, to_recheck

(*****************************************************************************)
(* Given a set of typedefs, compare the old and the new type and deduce
 * what must be rechecked accordingly.
 *)
(*****************************************************************************)

let compute_types_deps old_types (to_redecl, to_recheck) types =
  let rdc = Decl_compare.get_types_deps old_types types in
  let to_redecl = DepSet.union rdc to_redecl in
  let to_recheck = DepSet.union rdc to_recheck in
  to_redecl, to_recheck

(*****************************************************************************)
(* Given a set of global constants, compare the old and the new type and
 * deduce what must be rechecked accordingly.
 *)
(*****************************************************************************)

let compute_gconsts_deps old_gconsts (to_redecl, to_recheck) gconsts =
  let rdd, rdc = Decl_compare.get_gconsts_deps old_gconsts gconsts in
  let to_redecl = DepSet.union rdd to_redecl in
  let to_recheck = DepSet.union rdc to_recheck in
  to_redecl, to_recheck

(*****************************************************************************)
(* Redeclares a list of files
 * And then computes the files that must be redeclared/rechecked by looking
 * at what changed in the signatures of the classes/functions.
 *)
(*****************************************************************************)

let redeclare_files tcopt filel =
  List.fold_left filel
    ~f:(on_the_fly_decl_file tcopt)
    ~init:(Errors.empty, Relative_path.Set.empty)

let otf_decl_files tcopt filel =
  SharedMem.invalidate_caches();
  (* Redeclaring the files *)
  let errors, failed = redeclare_files tcopt filel in
  errors, failed

let compute_deps fast filel =
  let infol =
    List.map filel (fun fn -> Relative_path.Map.find_unsafe fast fn) in
  let names =
    List.fold_left infol ~f:FileInfo.merge_names ~init:FileInfo.empty_names in
  let { FileInfo.n_classes; n_funs; n_types; n_consts } = names in
  let acc = DepSet.empty, DepSet.empty in
  (* Fetching everything at once is faster *)
  let old_funs = Decl_heap.Funs.get_old_batch n_funs in
  let acc = compute_funs_deps old_funs acc n_funs in

  let old_types = Decl_heap.Typedefs.get_old_batch n_types in
  let acc = compute_types_deps old_types acc n_types in

  let old_consts = Decl_heap.GConsts.get_old_batch n_consts in
  let acc = compute_gconsts_deps old_consts acc n_consts in

  let old_classes = Decl_heap.Classes.get_old_batch n_classes in
  let new_classes = Decl_heap.Classes.get_batch n_classes in
  let compare_classes = compute_classes_deps old_classes new_classes in
  let (to_redecl, to_recheck) = compare_classes acc n_classes in

  to_redecl, to_recheck

(*****************************************************************************)
(* Load the environment and then redeclare *)
(*****************************************************************************)

let load_and_otf_decl_files _ filel =
  try
    let tcopt, _ = OnTheFlyStore.load() in
    otf_decl_files tcopt filel
  with e ->
    Printf.printf "Error: %s\n" (Printexc.to_string e);
    flush stdout;
    raise e

let load_and_compute_deps _acc filel =
  try
    let _, fast = OnTheFlyStore.load() in
    compute_deps fast filel
  with e ->
    Printf.printf "Error: %s\n" (Printexc.to_string e);
    flush stdout;
    raise e

(*****************************************************************************)
(* Merges the results coming back from the different workers *)
(*****************************************************************************)

let merge_on_the_fly (errorl1, failed1) (errorl2, failed2) =
  Errors.merge errorl1 errorl2, Relative_path.Set.union failed1 failed2

let merge_compute_deps (to_redecl1, to_recheck1) (to_redecl2, to_recheck2) =
  DepSet.union to_redecl1 to_redecl2, DepSet.union to_recheck1 to_recheck2

(*****************************************************************************)
(* The parallel worker *)
(*****************************************************************************)
let parallel_otf_decl workers bucket_size tcopt fast fnl =
  try
    OnTheFlyStore.store (tcopt, fast);
    let errors, failed =
      MultiWorker.call
        workers
        ~job:load_and_otf_decl_files
        ~neutral:otf_neutral
        ~merge:merge_on_the_fly
        ~next:(MultiWorker.next ~max_size:bucket_size workers fnl)
    in
    let to_redecl, to_recheck =
      MultiWorker.call
        workers
        ~job:load_and_compute_deps
        ~neutral:compute_deps_neutral
        ~merge:merge_compute_deps
        ~next:(MultiWorker.next ~max_size:bucket_size workers fnl)
    in
    OnTheFlyStore.clear();
    errors, failed, to_redecl, to_recheck
  with e ->
    if SharedMem.is_heap_overflow () then
      Exit_status.exit Exit_status.Redecl_heap_overflow
    else
      raise e

(*****************************************************************************)
(* Code invalidating the heap *)
(*****************************************************************************)
let oldify_defs { FileInfo.n_funs; n_classes; n_types; n_consts }
    elems ~collect_garbage =
  Decl_heap.Funs.oldify_batch n_funs;
  Decl_class_elements.oldify_all elems;
  Decl_heap.Classes.oldify_batch n_classes;
  Decl_heap.Typedefs.oldify_batch n_types;
  Decl_heap.GConsts.oldify_batch n_consts;
  if collect_garbage then SharedMem.collect `gentle;
  ()

let remove_old_defs { FileInfo.n_funs; n_classes; n_types; n_consts } elems =
  Decl_heap.Funs.remove_old_batch n_funs;
  Decl_class_elements.remove_old_all elems;
  Decl_heap.Classes.remove_old_batch n_classes;
  Decl_heap.Typedefs.remove_old_batch n_types;
  Decl_heap.GConsts.remove_old_batch n_consts;
  SharedMem.collect `gentle;
  ()

let remove_defs { FileInfo.n_funs; n_classes; n_types; n_consts }
    elems ~collect_garbage =
  Decl_heap.Funs.remove_batch n_funs;
  Decl_class_elements.remove_all elems;
  Decl_heap.Classes.remove_batch n_classes;
  Decl_heap.Typedefs.remove_batch n_types;
  Decl_heap.GConsts.remove_batch n_consts;
  if collect_garbage then SharedMem.collect `gentle;
  ()

let intersection_nonempty s1 mem_f s2  =
  SSet.exists s1 ~f:(mem_f s2)

let is_dependent_class_of_any classes c =
  if SSet.mem classes c then true else
  match Decl_heap.Classes.get c with
  | None -> false (* it might be a dependent class, but we are only doing this
                   * check for the purpose of invalidating things from the heap
                   * - if it's already not there, then we don't care. *)
  | Some c ->
    (intersection_nonempty classes SMap.mem c.Decl_defs.dc_ancestors) ||
    (intersection_nonempty classes SSet.mem c.Decl_defs.dc_extends) ||
    (intersection_nonempty classes SSet.mem
      c.Decl_defs.dc_req_ancestors_extends)

let get_maybe_dependent_classes_in_file file_info path =
  match Relative_path.Map.get file_info path with
  | None -> SSet.empty
  | Some info -> SSet.of_list @@ List.map info.FileInfo.classes snd

let get_maybe_dependent_classes file_info classes files =
  Relative_path.Set.fold files
    ~init:classes
    ~f:begin fun x acc ->
      SSet.union acc @@ get_maybe_dependent_classes_in_file file_info x
    end
  |> SSet.elements

let get_dependent_classes_files c =
  let c_hash = Dep.make (Dep.Class c) in
  let c_deps = Decl_compare.get_extend_deps c_hash Typing_deps.DepSet.empty in
  Typing_deps.get_files c_deps

let get_dependent_classes_files classes =
  SSet.fold classes
    ~init:Relative_path.Set.empty
    ~f:begin fun x acc ->
      Relative_path.Set.union acc @@ get_dependent_classes_files x
    end

let filter_dependent_classes classes maybe_dependent_classes =
  List.filter maybe_dependent_classes ~f:(is_dependent_class_of_any classes)

module ClassSetStore = GlobalStorage.Make(struct
  type t = SSet.t
end)

let load_and_filter_dependent_classes maybe_dependent_classes =
  let classes = ClassSetStore.load () in
  filter_dependent_classes classes maybe_dependent_classes

let filter_dependent_classes_parallel workers ~bucket_size
    classes maybe_dependent_classes =
  if List.length maybe_dependent_classes < 10 then
    filter_dependent_classes classes maybe_dependent_classes
  else begin
    ClassSetStore.store classes;
    let res = MultiWorker.call
      workers
      ~job:(fun _ c -> load_and_filter_dependent_classes c)
      ~merge:(@)
      ~neutral:[]
      ~next:(MultiWorker.next ~max_size:bucket_size
        workers maybe_dependent_classes)
    in
    ClassSetStore.clear ();
    res
  end

let get_dependent_classes workers ~bucket_size file_info classes =
  get_dependent_classes_files classes |>
  get_maybe_dependent_classes file_info classes |>
  filter_dependent_classes_parallel workers ~bucket_size classes |>
  SSet.of_list

let get_elems workers ~bucket_size ~old defs =
  let classes = SSet.elements defs.FileInfo.n_classes in
  (* Getting the members of a class requires fetching the class from the heap.
   * Doing this for too many classes will cause a large amount of allocations
   * to be performed on the master process triggering the GC and slowing down
   * redeclaration. Using the workers prevents this from occurring
   *)
  if List.length classes < 10
    then
      Decl_class_elements.get_for_classes ~old classes
    else
      MultiWorker.call
        workers
        ~job:(fun _ c -> Decl_class_elements.get_for_classes ~old c)
        ~merge:SMap.union
        ~neutral:SMap.empty
        ~next:(MultiWorker.next ~max_size:bucket_size workers classes)

(*****************************************************************************)
(* The main entry point *)
(*****************************************************************************)

let redo_type_decl workers ~bucket_size tcopt all_oldified_defs fast defs =
  (* Some of the defintions are already in the old heap, left there by a
   * previous lazy check *)
  let oldified_defs, current_defs =
    Decl_utils.split_defs defs all_oldified_defs in

  (* Oldify the remaining defs along with their elements *)
  let get_elems = get_elems workers ~bucket_size in
  let current_elems = get_elems current_defs ~old:false in
  oldify_defs current_defs current_elems ~collect_garbage:true;

  (* Fetch the already oldified elements too so we can remove them later *)
  let oldified_elems = get_elems oldified_defs ~old:true in
  let all_elems = SMap.union current_elems oldified_elems in

  let fnl = Relative_path.Map.keys fast in
  (* If there aren't enough files, let's do this ourselves ... it's faster! *)
  let result =
    if List.length fnl < 10
    then
      let errors, failed = otf_decl_files tcopt fnl in
      let to_redecl, to_recheck = compute_deps fast fnl in
      errors, failed, to_redecl, to_recheck
    else parallel_otf_decl workers bucket_size tcopt fast fnl
  in
  remove_old_defs defs all_elems;
  result

let oldify_type_decl
    ?collect_garbage:(collect_garbage=true)
    workers file_info ~bucket_size all_oldified_defs defs =

  (* Some defs are already oldified, waiting for their recheck *)
  let oldified_defs, current_defs =
    Decl_utils.split_defs defs all_oldified_defs in

  let get_elems = get_elems workers ~bucket_size in
  (* Oldify things that are not oldified yet *)
  let current_elems = get_elems current_defs ~old:false in
  oldify_defs current_defs current_elems ~collect_garbage;

  (* For the rest, just invalidate their current versions *)
  let oldified_elems = get_elems oldified_defs ~old:false in
  remove_defs oldified_defs oldified_elems ~collect_garbage;

  (* Oldifying/removing classes also affects their elements
   * (see Decl_class_elements), which might be shared with other classes. We
   * need to remove all of them too to avoid dangling references *)
  let all_classes = defs.FileInfo.n_classes in
  let dependent_classes =
    get_dependent_classes workers file_info ~bucket_size all_classes in

  let dependent_classes = FileInfo.({ empty_names with
    n_classes = SSet.diff dependent_classes all_classes
  }) in

  remove_defs dependent_classes SMap.empty ~collect_garbage
