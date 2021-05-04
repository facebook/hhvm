(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* On the fly type-declaration are called when the user modified a file
 * we are not at initilalization time anymore. Therefore, we have a bit more
 * work to do. We need calculate what must be re-checked.
 *)
(*****************************************************************************)
open Hh_prelude
open Reordered_argument_collections
open Typing_deps

type get_classes_in_file = Relative_path.t -> SSet.t

type redo_type_decl_result = {
  errors: Errors.t;
  changed: DepSet.t;
  to_redecl: DepSet.t;
  to_recheck: DepSet.t;
}

let lvl = Hh_logger.Level.Debug

let shallow_decl_enabled (ctx : Provider_context.t) =
  TypecheckerOptions.shallow_class_decl (Provider_context.get_tcopt ctx)

(*****************************************************************************)
(* The neutral element of declaration (cf procs/multiWorker.mli) *)
(*****************************************************************************)
let on_the_fly_neutral = Errors.empty

let compute_deps_neutral mode =
  let empty = DepSet.make mode in
  (empty, empty, empty)

(*****************************************************************************)
(* This is the place where we are going to put everything necessary for
 * the redeclaration. We could "pass" the values directly to the workers,
 * but it gives too much work to the master and slows things down,
 * so what we do instead is pass the data through shared memory via
 * OnTheFlyStore.
 * I tried replicating the data to speed things up but it had no effect.
 *)
(*****************************************************************************)

module OnTheFlyStore = GlobalStorage.Make (struct
  type t = Naming_table.fast
end)

(*****************************************************************************)
(* Re-declaring the types in a file *)
(*****************************************************************************)

let on_the_fly_decl_file ctx errors fn =
  let (decl_errors, ()) =
    Errors.do_with_context fn Errors.Decl (fun () ->
        Decl.make_env ~sh:SharedMem.Uses ctx fn)
  in
  Errors.merge decl_errors errors

(*****************************************************************************)
(* Given a set of classes, compare the old and the new type and deduce
 * what must be rechecked accordingly.
 *)
(*****************************************************************************)

let compute_classes_deps ctx old_classes new_classes acc classes =
  let (changed, to_redecl, to_recheck) = acc in
  let (rc, rdd, rdc) =
    Decl_compare.get_classes_deps ~ctx old_classes new_classes classes
  in
  let changed = DepSet.union rc changed in
  let to_redecl = DepSet.union rdd to_redecl in
  let to_recheck = DepSet.union rdc to_recheck in
  (changed, to_redecl, to_recheck)

(*****************************************************************************)
(* Given a set of functions, compare the old and the new type and deduce
 * what must be rechecked accordingly.
 *)
(*****************************************************************************)

let compute_funs_deps ctx old_funs (changed, to_redecl, to_recheck) funs =
  let (rc, rdd, rdc) = Decl_compare.get_funs_deps ~ctx old_funs funs in
  let changed = DepSet.union rc changed in
  let to_redecl = DepSet.union rdd to_redecl in
  let to_recheck = DepSet.union rdc to_recheck in
  (changed, to_redecl, to_recheck)

(*****************************************************************************)
(* Given a set of typedefs, compare the old and the new type and deduce
 * what must be rechecked accordingly.
 *)
(*****************************************************************************)

let compute_types_deps ctx old_types (changed, to_redecl, to_recheck) types =
  let (rc, rdc) = Decl_compare.get_types_deps ~ctx old_types types in
  let changed = DepSet.union rc changed in
  let to_recheck = DepSet.union rdc to_recheck in
  (changed, to_redecl, to_recheck)

let compute_record_defs_deps ctx old_record_defs acc record_defs =
  let (changed, to_redecl, to_recheck) = acc in
  let (rc, rdd, rdc) =
    Decl_compare.get_record_defs_deps ~ctx old_record_defs record_defs
  in
  let changed = DepSet.union rc changed in
  let to_redecl = DepSet.union rdd to_redecl in
  let to_recheck = DepSet.union rdc to_recheck in
  (changed, to_redecl, to_recheck)

(*****************************************************************************)
(* Given a set of global constants, compare the old and the new type and
 * deduce what must be rechecked accordingly.
 *)
(*****************************************************************************)

let compute_gconsts_deps
    ctx old_gconsts (changed, to_redecl, to_recheck) gconsts =
  let (rc, rdd, rdc) = Decl_compare.get_gconsts_deps ~ctx old_gconsts gconsts in
  let changed = DepSet.union rc changed in
  let to_redecl = DepSet.union rdd to_redecl in
  let to_recheck = DepSet.union rdc to_recheck in
  (changed, to_redecl, to_recheck)

(*****************************************************************************)
(* Redeclares a list of files
 * And then computes the files that must be redeclared/rechecked by looking
 * at what changed in the signatures of the classes/functions.
 *)
(*****************************************************************************)

let redeclare_files ctx filel =
  let errors =
    List.fold_left filel ~f:(on_the_fly_decl_file ctx) ~init:Errors.empty
  in
  (List.length filel, errors)

let on_the_fly_decl_files filel =
  SharedMem.invalidate_caches ();

  (* Redeclaring the files *)
  redeclare_files filel

let compute_deps ctx fast (filel : Relative_path.t list) =
  let infol = List.map filel (fun fn -> Relative_path.Map.find fast fn) in
  let names =
    List.fold_left infol ~f:FileInfo.merge_names ~init:FileInfo.empty_names
  in
  let { FileInfo.n_classes; n_record_defs; n_funs; n_types; n_consts } =
    names
  in
  let empty = DepSet.make (Provider_context.get_deps_mode ctx) in
  let acc = (empty, empty, empty) in
  (* Fetching everything at once is faster *)
  let old_funs = Decl_heap.Funs.get_old_batch n_funs in
  let acc = compute_funs_deps ctx old_funs acc n_funs in
  let old_types = Decl_heap.Typedefs.get_old_batch n_types in
  let acc = compute_types_deps ctx old_types acc n_types in
  let old_record_defs = Decl_heap.RecordDefs.get_old_batch n_record_defs in
  let acc = compute_record_defs_deps ctx old_record_defs acc n_record_defs in
  let old_consts = Decl_heap.GConsts.get_old_batch n_consts in
  let acc = compute_gconsts_deps ctx old_consts acc n_consts in
  let acc =
    if shallow_decl_enabled ctx then
      acc
    else
      let old_classes = Decl_heap.Classes.get_old_batch n_classes in
      let new_classes = Decl_heap.Classes.get_batch n_classes in
      compute_classes_deps ctx old_classes new_classes acc n_classes
  in
  let (changed, to_redecl, to_recheck) = acc in
  (changed, to_redecl, to_recheck)

(*****************************************************************************)
(* Load the environment and then redeclare *)
(*****************************************************************************)

let load_and_on_the_fly_decl_files ctx _ filel =
  try on_the_fly_decl_files ctx filel
  with e ->
    Printf.printf "Error: %s\n" (Exn.to_string e);
    Out_channel.flush stdout;
    raise e

let load_and_compute_deps ctx _acc (filel : Relative_path.t list) :
    DepSet.t * DepSet.t * DepSet.t * int =
  try
    let fast = OnTheFlyStore.load () in
    let (changed, to_redecl, to_recheck) = compute_deps ctx fast filel in
    (changed, to_redecl, to_recheck, List.length filel)
  with e ->
    Printf.printf "Error: %s\n" (Exn.to_string e);
    Out_channel.flush stdout;
    raise e

(*****************************************************************************)
(* Merges the results coming back from the different workers *)
(*****************************************************************************)

let merge_on_the_fly
    files_initial_count files_declared_count (count, errorl1) errorl2 =
  files_declared_count := !files_declared_count + count;
  ServerProgress.send_percentage_progress
    ~operation:"declaring"
    ~done_count:!files_declared_count
    ~total_count:files_initial_count
    ~unit:"files"
    ~extra:None;

  Errors.merge errorl1 errorl2

let merge_compute_deps
    files_initial_count
    files_computed_count
    (changed1, to_redecl1, to_recheck1, computed_count)
    (changed2, to_redecl2, to_recheck2) =
  files_computed_count := !files_computed_count + computed_count;

  let (changed, to_redecl, to_recheck) =
    ( DepSet.union changed1 changed2,
      DepSet.union to_redecl1 to_redecl2,
      DepSet.union to_recheck1 to_recheck2 )
  in

  ServerProgress.send_percentage_progress
    ~operation:"computing dependencies of"
    ~done_count:!files_computed_count
    ~total_count:files_initial_count
    ~unit:"files"
    ~extra:None;

  (changed, to_redecl, to_recheck)

(*****************************************************************************)
(* The parallel worker *)
(*****************************************************************************)
let parallel_on_the_fly_decl
    (ctx : Provider_context.t)
    (workers : MultiWorker.worker list option)
    (bucket_size : int)
    (fast : FileInfo.names Relative_path.Map.t)
    (fnl : Relative_path.t list) : Errors.t * DepSet.t * DepSet.t * DepSet.t =
  try
    OnTheFlyStore.store fast;
    let files_initial_count = List.length fnl in
    let files_declared_count = ref 0 in
    let t = Unix.gettimeofday () in
    Hh_logger.log ~lvl "Declaring on-the-fly %d files" files_initial_count;
    ServerProgress.send_percentage_progress
      ~operation:"declaring"
      ~done_count:!files_declared_count
      ~total_count:files_initial_count
      ~unit:"files"
      ~extra:None;
    let errors =
      MultiWorker.call
        workers
        ~job:(load_and_on_the_fly_decl_files ctx)
        ~neutral:on_the_fly_neutral
        ~merge:(merge_on_the_fly files_initial_count files_declared_count)
        ~next:(MultiWorker.next ~max_size:bucket_size workers fnl)
    in
    let t = Hh_logger.log_duration ~lvl "Finished declaring on-the-fly" t in
    Hh_logger.log ~lvl "Computing dependencies of %d files" files_initial_count;
    let files_computed_count = ref 0 in
    ServerProgress.send_percentage_progress
      ~operation:"computing dependencies of"
      ~done_count:!files_computed_count
      ~total_count:files_initial_count
      ~unit:"files"
      ~extra:None;
    let (changed, to_redecl, to_recheck) =
      MultiWorker.call
        workers
        ~job:(load_and_compute_deps ctx)
        ~neutral:(compute_deps_neutral (Provider_context.get_deps_mode ctx))
        ~merge:(merge_compute_deps files_initial_count files_computed_count)
        ~next:(MultiWorker.next ~max_size:bucket_size workers fnl)
    in
    let (_t : float) =
      Hh_logger.log_duration ~lvl "Finished computing dependencies" t
    in
    OnTheFlyStore.clear ();
    (errors, changed, to_redecl, to_recheck)
  with e ->
    if SharedMem.is_heap_overflow () then
      Exit.exit Exit_status.Redecl_heap_overflow
    else
      raise e

(*****************************************************************************)
(* Code invalidating the heap *)
(*****************************************************************************)
let oldify_defs
    (ctx : Provider_context.t)
    { FileInfo.n_funs; n_classes; n_record_defs; n_types; n_consts }
    (elems : Decl_class_elements.t SMap.t)
    ~(collect_garbage : bool) : unit =
  Decl_heap.Funs.oldify_batch n_funs;
  Decl_class_elements.oldify_all elems;
  Decl_heap.Classes.oldify_batch n_classes;
  Shallow_classes_provider.oldify_batch ctx n_classes;
  Decl_heap.RecordDefs.oldify_batch n_record_defs;
  Decl_heap.Typedefs.oldify_batch n_types;
  Decl_heap.GConsts.oldify_batch n_consts;
  if collect_garbage then SharedMem.collect `gentle;
  ()

let remove_old_defs
    (ctx : Provider_context.t)
    { FileInfo.n_funs; n_classes; n_record_defs; n_types; n_consts }
    (elems : Decl_class_elements.t SMap.t) : unit =
  Decl_heap.Funs.remove_old_batch n_funs;
  Decl_class_elements.remove_old_all elems;
  Decl_heap.Classes.remove_old_batch n_classes;
  Shallow_classes_provider.remove_old_batch ctx n_classes;
  Decl_heap.RecordDefs.remove_old_batch n_record_defs;
  Decl_heap.Typedefs.remove_old_batch n_types;
  Decl_heap.GConsts.remove_old_batch n_consts;
  SharedMem.collect `gentle;
  ()

let remove_defs
    (ctx : Provider_context.t)
    { FileInfo.n_funs; n_classes; n_record_defs; n_types; n_consts }
    (elems : Decl_class_elements.t SMap.t)
    ~(collect_garbage : bool) : unit =
  Decl_heap.Funs.remove_batch n_funs;
  Decl_class_elements.remove_all elems;
  Decl_heap.Classes.remove_batch n_classes;
  Shallow_classes_provider.remove_batch ctx n_classes;
  Linearization_provider.remove_batch ctx n_classes;
  Decl_heap.RecordDefs.remove_batch n_record_defs;
  Decl_heap.Typedefs.remove_batch n_types;
  Decl_heap.GConsts.remove_batch n_consts;
  if collect_garbage then SharedMem.collect `gentle;
  ()

let is_dependent_class_of_any ctx classes (c : string) : bool =
  if SSet.mem classes c then
    true
  else if shallow_decl_enabled ctx then
    true
  else
    match Decl_heap.Classes.get c with
    | None -> false
    (* it might be a dependent class, but we are only doing this
     * check for the purpose of invalidating things from the heap
     * - if it's already not there, then we don't care. *)
    | Some c ->
      let intersection_nonempty s1 s2 = SSet.exists s1 ~f:(SSet.mem s2) in
      SMap.exists c.Decl_defs.dc_ancestors ~f:(fun c _ -> SSet.mem classes c)
      || intersection_nonempty c.Decl_defs.dc_extends classes
      || intersection_nonempty c.Decl_defs.dc_xhp_attr_deps classes
      || intersection_nonempty c.Decl_defs.dc_condition_types classes
      || intersection_nonempty c.Decl_defs.dc_req_ancestors_extends classes
      || intersection_nonempty c.Decl_defs.dc_condition_types classes

let get_maybe_dependent_classes
    (get_classes : Relative_path.t -> SSet.t)
    (classes : SSet.t)
    (files : Relative_path.Set.t) : string list =
  Relative_path.Set.fold files ~init:classes ~f:(fun x acc ->
      SSet.union acc @@ get_classes x)
  |> SSet.elements

let get_dependent_classes_files (mode : Typing_deps_mode.t) (classes : SSet.t) :
    Relative_path.Set.t =
  let visited = VisitedSet.make mode in
  SSet.fold
    classes
    ~init:Typing_deps.(DepSet.make mode)
    ~f:(fun c acc ->
      let source_class = Dep.make (hash_mode mode) (Dep.Type c) in
      Typing_deps.get_extend_deps ~mode ~visited ~source_class ~acc)
  |> Typing_deps.Files.get_files

let filter_dependent_classes
    (ctx : Provider_context.t)
    (classes : SSet.t)
    (maybe_dependent_classes : string list) : string list =
  List.filter maybe_dependent_classes ~f:(is_dependent_class_of_any ctx classes)

module ClassSetStore = GlobalStorage.Make (struct
  type t = SSet.t
end)

let load_and_filter_dependent_classes
    (ctx : Provider_context.t) (maybe_dependent_classes : string list) :
    string list * int =
  let classes = ClassSetStore.load () in
  ( filter_dependent_classes ctx classes maybe_dependent_classes,
    List.length maybe_dependent_classes )

let merge_dependent_classes
    classes_initial_count
    classes_filtered_count
    (dependent_classes, filtered)
    acc =
  classes_filtered_count := !classes_filtered_count + filtered;
  ServerProgress.send_percentage_progress
    ~operation:"filtering"
    ~done_count:!classes_filtered_count
    ~total_count:classes_initial_count
    ~unit:"classes"
    ~extra:None;
  dependent_classes @ acc

let filter_dependent_classes_parallel
    (ctx : Provider_context.t)
    (workers : MultiWorker.worker list option)
    ~(bucket_size : int)
    (classes : SSet.t)
    (maybe_dependent_classes : string list) : string list =
  if List.length maybe_dependent_classes < 10 then
    filter_dependent_classes ctx classes maybe_dependent_classes
  else (
    ClassSetStore.store classes;
    let classes_initial_count = List.length maybe_dependent_classes in
    let classes_filtered_count = ref 0 in
    let t = Unix.gettimeofday () in
    Hh_logger.log ~lvl "Filtering %d dependent classes" classes_initial_count;
    ServerProgress.send_percentage_progress
      ~operation:"filtering"
      ~done_count:!classes_filtered_count
      ~total_count:classes_initial_count
      ~unit:"classes"
      ~extra:None;
    let res =
      MultiWorker.call
        workers
        ~job:(fun _ c -> load_and_filter_dependent_classes ctx c)
        ~merge:
          (merge_dependent_classes classes_initial_count classes_filtered_count)
        ~neutral:[]
        ~next:
          (MultiWorker.next
             ~max_size:bucket_size
             workers
             maybe_dependent_classes)
    in
    let (_t : float) =
      Hh_logger.log_duration ~lvl "Finished filtering dependent classes" t
    in
    ClassSetStore.clear ();
    res
  )

let get_dependent_classes
    (ctx : Provider_context.t)
    (workers : MultiWorker.worker list option)
    ~(bucket_size : int)
    (get_classes : Relative_path.t -> SSet.t)
    (classes : SSet.t) : SSet.t =
  get_dependent_classes_files (Provider_context.get_deps_mode ctx) classes
  |> get_maybe_dependent_classes get_classes classes
  |> filter_dependent_classes_parallel ctx workers ~bucket_size classes
  |> SSet.of_list

let merge_elements
    classes_initial_count classes_processed_count (elements, count) acc =
  classes_processed_count := !classes_processed_count + count;

  let acc = SMap.union elements acc in
  ServerProgress.send_percentage_progress
    ~operation:"getting members of"
    ~done_count:!classes_processed_count
    ~total_count:classes_initial_count
    ~unit:"classes"
    ~extra:(Some (Printf.sprintf "%d elements" (SMap.cardinal acc)));
  acc

(**
 * Get the [Decl_class_elements.t]s corresponding to the classes contained in
 * [defs]. *)
let get_elems
    (ctx : Provider_context.t)
    (workers : MultiWorker.worker list option)
    ~(bucket_size : int)
    ~(old : bool)
    (defs : FileInfo.names) : Decl_class_elements.t SMap.t =
  if shallow_decl_enabled ctx then
    SMap.empty
  else
    let classes = SSet.elements defs.FileInfo.n_classes in
    (* Getting the members of a class requires fetching the class from the heap.
     * Doing this for too many classes will cause a large amount of allocations
     * to be performed on the master process triggering the GC and slowing down
     * redeclaration. Using the workers prevents this from occurring
     *)
    let classes_initial_count = List.length classes in
    let t = Unix.gettimeofday () in
    Hh_logger.log ~lvl "Getting elements of %d classes" classes_initial_count;
    let elements =
      if classes_initial_count < 10 then
        Decl_class_elements.get_for_classes ~old classes
      else
        let classes_processed_count = ref 0 in
        ServerProgress.send_percentage_progress
          ~operation:"getting members of"
          ~done_count:!classes_processed_count
          ~total_count:classes_initial_count
          ~unit:"classes"
          ~extra:None;
        MultiWorker.call
          workers
          ~job:(fun _ c ->
            (Decl_class_elements.get_for_classes ~old c, List.length c))
          ~merge:(merge_elements classes_initial_count classes_processed_count)
          ~neutral:SMap.empty
          ~next:(MultiWorker.next ~max_size:bucket_size workers classes)
    in

    let (_t : float) =
      Hh_logger.log_duration ~lvl "Finished getting elements" t
    in
    elements

(*****************************************************************************)
(* The main entry point *)
(*****************************************************************************)

let redo_type_decl
    (ctx : Provider_context.t)
    (workers : MultiWorker.worker list option)
    ~(bucket_size : int)
    (get_classes : Relative_path.t -> SSet.t)
    ~(previously_oldified_defs : FileInfo.names)
    ~(defs : FileInfo.names Relative_path.Map.t) : redo_type_decl_result =
  let all_defs =
    Relative_path.Map.fold defs ~init:FileInfo.empty_names ~f:(fun _ ->
        FileInfo.merge_names)
  in
  (* Some of the defintions are already in the old heap, left there by a
   * previous lazy check *)
  let (oldified_defs, current_defs) =
    Decl_utils.split_defs all_defs previously_oldified_defs
  in
  (* Oldify the remaining defs along with their elements *)
  let get_elems = get_elems ctx workers ~bucket_size in
  let current_elems = get_elems current_defs ~old:false in
  oldify_defs ctx current_defs current_elems ~collect_garbage:true;

  (* Fetch the already oldified elements too so we can remove them later *)
  let oldified_elems = get_elems oldified_defs ~old:true in
  let all_elems = SMap.union current_elems oldified_elems in
  let fnl = Relative_path.Map.keys defs in
  (* If there aren't enough files, let's do this ourselves ... it's faster! *)
  let (errors, changed, to_redecl, to_recheck) =
    if List.length fnl < 10 then
      let ((_declared : int), errors) = on_the_fly_decl_files ctx fnl in
      let (changed, to_redecl, to_recheck) = compute_deps ctx defs fnl in
      (errors, changed, to_redecl, to_recheck)
    else
      parallel_on_the_fly_decl ctx workers bucket_size defs fnl
  in
  let (changed, to_recheck) =
    if shallow_decl_enabled ctx then (
      let AffectedDeps.{ changed = changed'; mro_invalidated; needs_recheck } =
        Shallow_decl_compare.compute_class_fanout ctx get_classes fnl
      in
      let changed = DepSet.union changed changed' in
      let to_recheck = DepSet.union to_recheck needs_recheck in
      let mro_invalidated =
        mro_invalidated
        |> Typing_deps.Files.get_files
        |> Relative_path.Set.fold ~init:SSet.empty ~f:(fun path acc ->
               SSet.union acc (get_classes path))
      in
      Linearization_provider.remove_batch ctx mro_invalidated;
      (changed, to_recheck)
    ) else
      (changed, to_recheck)
  in
  remove_old_defs ctx all_defs all_elems;

  Hh_logger.log "Finished recomputing type declarations:";
  Hh_logger.log "  changed: %d" (DepSet.cardinal changed);
  Hh_logger.log "  to_redecl: %d" (DepSet.cardinal to_redecl);
  Hh_logger.log "  to_recheck: %d" (DepSet.cardinal to_recheck);

  { errors; changed; to_redecl; to_recheck }

let oldify_type_decl
    (ctx : Provider_context.t)
    ?(collect_garbage = true)
    (workers : MultiWorker.worker list option)
    (get_classes : Relative_path.t -> SSet.t)
    ~(bucket_size : int)
    ~(previously_oldified_defs : FileInfo.names)
    ~(defs : FileInfo.names) : unit =
  (* Some defs are already oldified, waiting for their recheck *)
  let (oldified_defs, current_defs) =
    Decl_utils.split_defs defs previously_oldified_defs
  in
  let get_elems = get_elems ctx workers ~bucket_size in
  (* Oldify things that are not oldified yet *)
  let current_elems = get_elems current_defs ~old:false in
  oldify_defs ctx current_defs current_elems ~collect_garbage;

  (* For the rest, just invalidate their current versions *)
  let oldified_elems = get_elems oldified_defs ~old:false in
  remove_defs ctx oldified_defs oldified_elems ~collect_garbage;

  (* Oldifying/removing classes also affects their elements
   * (see Decl_class_elements), which might be shared with other classes. We
   * need to remove all of them too to avoid dangling references *)
  let all_classes = defs.FileInfo.n_classes in
  let dependent_classes =
    get_dependent_classes ctx workers get_classes ~bucket_size all_classes
  in
  let dependent_classes =
    FileInfo.
      { empty_names with n_classes = SSet.diff dependent_classes all_classes }
  in
  remove_defs ctx dependent_classes SMap.empty ~collect_garbage

let remove_old_defs
    (ctx : Provider_context.t)
    ~(bucket_size : int)
    (workers : MultiWorker.worker list option)
    (names : FileInfo.names) : unit =
  let elems = get_elems ctx workers ~bucket_size names ~old:true in
  remove_old_defs ctx names elems
