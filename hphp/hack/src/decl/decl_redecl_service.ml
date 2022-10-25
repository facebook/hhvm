(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** On the fly defs-declaration are called when the user modified a file
  and we are not at initilalization time anymore. Therefore, we have a bit more
  work to do. We need to calculate what must be re-checked. *)

open Hh_prelude
open Reordered_argument_collections
open Typing_deps

type get_classes_in_file = Relative_path.t -> SSet.t

type fanout = {
  changed: DepSet.t;
  to_redecl: DepSet.t;
  to_recheck: DepSet.t;
}

type redo_type_decl_result = {
  errors: Errors.t;
  fanout: fanout;
  old_decl_missing_count: int;
}

let lvl = Hh_logger.Level.Debug

let shallow_decl_enabled (ctx : Provider_context.t) =
  TypecheckerOptions.shallow_class_decl (Provider_context.get_tcopt ctx)

let force_shallow_decl_fanout_enabled (ctx : Provider_context.t) =
  TypecheckerOptions.force_shallow_decl_fanout (Provider_context.get_tcopt ctx)

let empty_fanout =
  let empty = DepSet.make () in
  { changed = empty; to_redecl = empty; to_recheck = empty }

let compute_deps_neutral = (empty_fanout, 0)

(** This is the place where we are going to put everything necessary for
  the redeclaration. We could "pass" the values directly to the workers,
  but it gives too much work to the master and slows things down,
  so what we do instead is pass the data through shared memory via
  OnTheFlyStore.
  I tried replicating the data to speed things up but it had no effect. *)
module OnTheFlyStore = GlobalStorage.Make (struct
  type t = Naming_table.defs_per_file
end)

(** Compute the decls in a file *)
let decl_file ctx errors fn =
  let (decl_errors, ()) =
    Errors.do_with_context fn Errors.Decl (fun () ->
        Decl.make_env ~sh:SharedMem.Uses ctx fn)
  in
  Errors.merge decl_errors errors

(** Given a set of classes, compare the old and the new decl and deduce
  what must be rechecked accordingly. *)
let compare_classes_and_get_fanout ctx old_classes new_classes acc classes =
  let { changed; to_redecl; to_recheck } = acc in
  let ((rc, rdd, rdc), old_classes_missing) =
    Decl_compare.get_classes_deps ~ctx old_classes new_classes classes
  in
  let changed = DepSet.union rc changed in
  let to_redecl = DepSet.union rdd to_redecl in
  let to_recheck = DepSet.union rdc to_recheck in
  ({ changed; to_redecl; to_recheck }, old_classes_missing)

(** Given a set of functions, compare the old and the new decl and deduce
  what must be rechecked accordingly. *)
let compare_funs_and_get_fanout
    ctx old_funs { changed; to_redecl; to_recheck } funs =
  let ((rc, rdd, rdc), old_funs_missing) =
    Decl_compare.get_funs_deps ~ctx old_funs funs
  in
  let changed = DepSet.union rc changed in
  let to_redecl = DepSet.union rdd to_redecl in
  let to_recheck = DepSet.union rdc to_recheck in
  ({ changed; to_redecl; to_recheck }, old_funs_missing)

(** Given a set of typedefs, compare the old and the new decl and deduce
  what must be rechecked accordingly. *)
let compare_types_and_get_fanout
    ctx old_types { changed; to_redecl; to_recheck } types =
  let ((rc, rdc), old_types_missing) =
    Decl_compare.get_types_deps ~ctx old_types types
  in
  let changed = DepSet.union rc changed in
  let to_recheck = DepSet.union rdc to_recheck in
  ({ changed; to_redecl; to_recheck }, old_types_missing)

(* Given a set of global constants, compare the old and the new decl and
  deduce what must be rechecked accordingly. *)
let compare_gconsts_and_get_fanout
    ctx old_gconsts { changed; to_redecl; to_recheck } gconsts =
  let ((rc, rdd, rdc), old_gconsts_missing) =
    Decl_compare.get_gconsts_deps ~ctx old_gconsts gconsts
  in
  let changed = DepSet.union rc changed in
  let to_redecl = DepSet.union rdd to_redecl in
  let to_recheck = DepSet.union rdc to_recheck in
  ({ changed; to_redecl; to_recheck }, old_gconsts_missing)

(* Given a set of modules, compare the old and the new decl and
  deduce what must be rechecked accordingly. *)
let compare_modules_and_get_fanout
    ctx old_modules { changed; to_redecl; to_recheck } modules =
  let ((rc, rdd, rdc), old_modules_missing) =
    Decl_compare.get_modules_deps ~ctx ~old_modules ~modules
  in
  let changed = DepSet.union rc changed in
  let to_redecl = DepSet.union rdd to_redecl in
  let to_recheck = DepSet.union rdc to_recheck in
  ({ changed; to_redecl; to_recheck }, old_modules_missing)

(*****************************************************************************)
(* Redeclares a list of files
 * And then computes the files that must be redeclared/rechecked by looking
 * at what changed in the signatures of the classes/functions.
 *)
(*****************************************************************************)

(** Compute decls in files. Return errors raised during decling. *)
let redeclare_files ctx filel =
  List.fold_left filel ~f:(decl_file ctx) ~init:Errors.empty

(** Invalidate local caches and compute decls in files. Return errors raised during decling. *)
let decl_files ctx filel =
  SharedMem.invalidate_local_caches ();
  redeclare_files ctx filel

let compare_decls_and_get_fanout
    ctx defs_per_file (filel : Relative_path.t list) : fanout * int =
  let defs_in_files =
    List.map filel ~f:(fun fn -> Relative_path.Map.find defs_per_file fn)
  in
  let all_defs =
    List.fold_left
      defs_in_files
      ~f:FileInfo.merge_names
      ~init:FileInfo.empty_names
  in
  let { FileInfo.n_classes; n_funs; n_types; n_consts; n_modules } = all_defs in
  let acc = empty_fanout in
  (* Fetching everything at once is faster *)
  let (old_funs, old_types, old_consts, old_modules) =
    match Provider_backend.get () with
    | Provider_backend.Rust_provider_backend be ->
      Rust_provider_backend.Decl.get_old_defs be all_defs
    | _ ->
      ( Decl_heap.Funs.get_old_batch n_funs,
        Decl_heap.Typedefs.get_old_batch n_types,
        Decl_heap.GConsts.get_old_batch n_consts,
        Decl_heap.Modules.get_old_batch n_modules )
  in
  let (acc, old_funs_missing) =
    compare_funs_and_get_fanout ctx old_funs acc n_funs
  in
  let (acc, old_types_missing) =
    compare_types_and_get_fanout ctx old_types acc n_types
  in
  let (acc, old_gconsts_missing) =
    compare_gconsts_and_get_fanout ctx old_consts acc n_consts
  in
  let (acc, old_classes_missing) =
    if shallow_decl_enabled ctx || force_shallow_decl_fanout_enabled ctx then
      (acc, 0)
    else
      let old_classes = Decl_heap.Classes.get_old_batch n_classes in
      let new_classes = Decl_heap.Classes.get_batch n_classes in
      compare_classes_and_get_fanout ctx old_classes new_classes acc n_classes
  in
  let (acc, old_modules_missing) =
    compare_modules_and_get_fanout ctx old_modules acc n_modules
  in
  let old_decl_missing_count =
    old_funs_missing
    + old_types_missing
    + old_gconsts_missing
    + old_classes_missing
    + old_modules_missing
  in
  let { changed; to_redecl; to_recheck } = acc in
  ({ changed; to_redecl; to_recheck }, old_decl_missing_count)

(*****************************************************************************)
(* Load the environment and then redeclare *)
(*****************************************************************************)

(** Invalidate local caches and compute decls in files. Return the file count and errors raised during decling. *)
let decl_files_job ctx _ filel =
  try (List.length filel, decl_files ctx filel) with
  | exn ->
    let e = Exception.wrap exn in
    Printf.printf "Error: %s\n" (Exception.get_ctor_string e);
    Out_channel.flush stdout;
    Exception.reraise e

let load_defs_compare_and_get_fanout ctx _acc (filel : Relative_path.t list) :
    (fanout * int) * int =
  try
    let defs_per_file = OnTheFlyStore.load () in
    let (fanout, old_decl_missing_count) =
      compare_decls_and_get_fanout ctx defs_per_file filel
    in
    ((fanout, List.length filel), old_decl_missing_count)
  with
  | exn ->
    let e = Exception.wrap exn in
    Printf.printf "Error: %s\n" (Exception.get_ctor_string e);
    Out_channel.flush stdout;
    Exception.reraise e

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
    ((fanout1, computed_count), old_decl_missing_count1)
    (fanout2, old_decl_missing_count2) =
  files_computed_count := !files_computed_count + computed_count;

  let { changed = changed1; to_redecl = to_redecl1; to_recheck = to_recheck1 } =
    fanout1
  in
  let { changed = changed2; to_redecl = to_redecl2; to_recheck = to_recheck2 } =
    fanout2
  in
  let fanout =
    {
      changed = DepSet.union changed1 changed2;
      to_redecl = DepSet.union to_redecl1 to_redecl2;
      to_recheck = DepSet.union to_recheck1 to_recheck2;
    }
  in

  ServerProgress.send_percentage_progress
    ~operation:"computing dependencies of"
    ~done_count:!files_computed_count
    ~total_count:files_initial_count
    ~unit:"files"
    ~extra:None;

  (fanout, old_decl_missing_count1 + old_decl_missing_count2)

(*****************************************************************************)
(* The parallel worker *)
(*****************************************************************************)

(** Invalidate local decl caches and recompute decls in files in parallel.
  Compare new and old decls and deduce the fanout.
  Return errors raised during decling, fanout and missing old decl count. *)
let parallel_redecl_compare_and_get_fanout
    (ctx : Provider_context.t)
    (workers : MultiWorker.worker list option)
    (bucket_size : int)
    (defs_per_file : FileInfo.names Relative_path.Map.t)
    (fnl : Relative_path.t list) : (Errors.t * fanout) * int =
  try
    OnTheFlyStore.store defs_per_file;
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
        ~job:(decl_files_job ctx)
        ~neutral:Errors.empty
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
    let (fanout, old_decl_missing_count) =
      MultiWorker.call
        workers
        ~job:(load_defs_compare_and_get_fanout ctx)
        ~neutral:compute_deps_neutral
        ~merge:(merge_compute_deps files_initial_count files_computed_count)
        ~next:(MultiWorker.next ~max_size:bucket_size workers fnl)
    in
    let (_t : float) =
      Hh_logger.log_duration ~lvl "Finished computing dependencies" t
    in
    OnTheFlyStore.clear ();
    ((errors, fanout), old_decl_missing_count)
  with
  | exn ->
    let e = Exception.wrap exn in
    if SharedMem.SMTelemetry.is_heap_overflow () then
      Exit.exit Exit_status.Redecl_heap_overflow
    else
      Exception.reraise e

(*****************************************************************************)
(* Code invalidating the heap *)
(*****************************************************************************)
let[@warning "-21"] oldify_defs (* -21 for dune stubs *)
    (ctx : Provider_context.t)
    ({ FileInfo.n_funs; n_classes; n_types; n_consts; n_modules } as names)
    (elems : Decl_class_elements.t SMap.t)
    ~(collect_garbage : bool) : unit =
  match Provider_backend.get () with
  | Provider_backend.Rust_provider_backend be ->
    Rust_provider_backend.Decl.oldify_defs be (names, elems)
  | _ ->
    Decl_heap.Funs.oldify_batch n_funs;
    Decl_class_elements.oldify_all elems;
    Decl_heap.Classes.oldify_batch n_classes;
    Shallow_classes_provider.oldify_batch ctx n_classes;
    Decl_heap.Typedefs.oldify_batch n_types;
    Decl_heap.GConsts.oldify_batch n_consts;
    Decl_heap.Modules.oldify_batch n_modules;
    if collect_garbage then SharedMem.GC.collect `gentle;
    ()

let[@warning "-21"] remove_old_defs (* -21 for dune stubs *)
    (ctx : Provider_context.t)
    ({ FileInfo.n_funs; n_classes; n_types; n_consts; n_modules } as names)
    (elems : Decl_class_elements.t SMap.t) : unit =
  match Provider_backend.get () with
  | Provider_backend.Rust_provider_backend be ->
    Rust_provider_backend.Decl.remove_old_defs be (names, elems)
  | _ ->
    Decl_heap.Funs.remove_old_batch n_funs;
    Decl_class_elements.remove_old_all elems;
    Decl_heap.Classes.remove_old_batch n_classes;
    Shallow_classes_provider.remove_old_batch ctx n_classes;
    Decl_heap.Typedefs.remove_old_batch n_types;
    Decl_heap.GConsts.remove_old_batch n_consts;
    Decl_heap.Modules.remove_old_batch n_modules;
    SharedMem.GC.collect `gentle;
    ()

let[@warning "-21"] remove_defs (* -21 for dune stubs *)
    (ctx : Provider_context.t)
    ({ FileInfo.n_funs; n_classes; n_types; n_consts; n_modules } as names)
    (elems : Decl_class_elements.t SMap.t)
    ~(collect_garbage : bool) : unit =
  match Provider_backend.get () with
  | Provider_backend.Rust_provider_backend be ->
    Rust_provider_backend.Decl.remove_defs be (names, elems)
  | _ ->
    Decl_heap.Funs.remove_batch n_funs;
    Decl_class_elements.remove_all elems;
    Decl_heap.Classes.remove_batch n_classes;
    Shallow_classes_provider.remove_batch ctx n_classes;
    Linearization_provider.remove_batch ctx n_classes;
    Decl_heap.Typedefs.remove_batch n_types;
    Decl_heap.GConsts.remove_batch n_consts;
    Decl_heap.Modules.remove_batch n_modules;
    if collect_garbage then SharedMem.GC.collect `gentle;
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
      || intersection_nonempty c.Decl_defs.dc_req_ancestors_extends classes

let get_maybe_dependent_classes
    (get_classes : Relative_path.t -> SSet.t)
    (classes : SSet.t)
    (files : Relative_path.Set.t) : string list =
  Relative_path.Set.fold files ~init:classes ~f:(fun x acc ->
      SSet.union acc @@ get_classes x)
  |> SSet.elements

let get_dependent_classes_files (ctx : Provider_context.t) (classes : SSet.t) :
    Relative_path.Set.t =
  let mode = Provider_context.get_deps_mode ctx in
  let visited = VisitedSet.make () in
  SSet.fold
    classes
    ~init:Typing_deps.(DepSet.make ())
    ~f:(fun c acc ->
      let source_class = Dep.make (Dep.Type c) in
      Typing_deps.get_extend_deps ~mode ~visited ~source_class ~acc)
  |> Naming_provider.get_files ctx

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
  get_dependent_classes_files ctx classes
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

let invalidate_folded_classes_for_shallow_fanout
    ctx workers ~bucket_size ~get_classes_in_file changed_classes =
  let invalidated =
    changed_classes
    |> Typing_deps.add_extend_deps (Provider_context.get_deps_mode ctx)
    |> Shallow_class_fanout.class_names_from_deps ~ctx ~get_classes_in_file
  in
  let get_elems n_classes =
    get_elems ctx workers ~bucket_size FileInfo.{ empty_names with n_classes }
  in
  Decl_class_elements.remove_old_all (get_elems invalidated ~old:true);
  Decl_class_elements.remove_all (get_elems invalidated ~old:false);
  Decl_heap.Classes.remove_old_batch invalidated;
  Decl_heap.Classes.remove_batch invalidated;
  SharedMem.invalidate_local_caches ();
  SharedMem.GC.collect `gentle;
  ()

(*****************************************************************************)
(* The main entry point *)
(*****************************************************************************)

(** Oldify any defs in [defs] which aren't already in
[previously_oldified_defs], then determines which symbols need to be
re-typechecked as a result of comparing the current versions of the symbols
to their old versions. *)
let redo_type_decl
    (ctx : Provider_context.t)
    (workers : MultiWorker.worker list option)
    ~(bucket_size : int)
    (get_classes : Relative_path.t -> SSet.t)
    ~(previously_oldified_defs : FileInfo.names)
    ~(defs : FileInfo.names Relative_path.Map.t)
    ~(telemetry_label : string) : redo_type_decl_result =
  let all_defs =
    Relative_path.Map.fold defs ~init:FileInfo.empty_names ~f:(fun _ ->
        FileInfo.merge_names)
  in
  (* Some of the definitions are already in the old heap, left there by a
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

  let ((errors, { changed; to_redecl; to_recheck }), old_decl_missing_count) =
    (* If there aren't enough files, let's do this ourselves ... it's faster! *)
    if List.length fnl < 10 then
      let errors = decl_files ctx fnl in
      let (fanout, old_decl_missing_count) =
        compare_decls_and_get_fanout ctx defs fnl
      in
      ((errors, fanout), old_decl_missing_count)
    else
      parallel_redecl_compare_and_get_fanout ctx workers bucket_size defs fnl
  in
  let (changed, to_recheck) =
    if shallow_decl_enabled ctx then (
      let AffectedDeps.{ changed = changed'; mro_invalidated; needs_recheck } =
        Shallow_decl_compare.compute_class_fanout
          ctx
          ~defs
          ~fetch_old_decls:
            (Remote_old_decl_client.fetch_old_decls ~ctx ~telemetry_label)
          fnl
      in
      let changed = DepSet.union changed changed' in
      let to_recheck = DepSet.union to_recheck needs_recheck in
      let mro_invalidated =
        mro_invalidated
        |> Naming_provider.get_files ctx
        |> Relative_path.Set.fold ~init:SSet.empty ~f:(fun path acc ->
               SSet.union acc (get_classes path))
      in
      Linearization_provider.remove_batch ctx mro_invalidated;
      (changed, to_recheck)
    ) else if force_shallow_decl_fanout_enabled ctx then (
      let AffectedDeps.
            { changed = changed'; mro_invalidated = _; needs_recheck } =
        Shallow_decl_compare.compute_class_fanout
          ctx
          ~defs
          ~fetch_old_decls:
            (Remote_old_decl_client.fetch_old_decls ~ctx ~telemetry_label)
          fnl
      in

      invalidate_folded_classes_for_shallow_fanout
        ctx
        workers
        ~bucket_size
        ~get_classes_in_file:get_classes
        changed';

      let changed = DepSet.union changed changed' in
      let to_recheck = DepSet.union to_recheck needs_recheck in

      (changed, to_recheck)
    ) else
      (changed, to_recheck)
  in
  remove_old_defs ctx all_defs all_elems;

  Hh_logger.log "Finished recomputing type declarations:";
  Hh_logger.log "  changed: %d" (DepSet.cardinal changed);
  Hh_logger.log "  to_redecl: %d" (DepSet.cardinal to_redecl);
  Hh_logger.log "  to_recheck: %d" (DepSet.cardinal to_recheck);

  {
    errors;
    fanout = { changed; to_redecl; to_recheck };
    old_decl_missing_count;
  }

(** Mark all provided [defs] as old, as long as they were not previously
oldified. All definitions in [previously_oldified_defs] are then removed from
the decl heaps.

Typically, there are only any [previously_oldified_defs] during two-phase
redeclaration. *)
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
