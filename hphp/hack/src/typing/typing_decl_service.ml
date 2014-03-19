(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)


(*****************************************************************************)
(* Module declaring the types in parallel *)
(*****************************************************************************)
open Utils
module PHeap = Parser_heap.ParserHeap

(* filename => functions defined, classes defined *)
type fast = (SSet.t * SSet.t * SSet.t * SSet.t) SMap.t

(* The set of files that failed *)
type failed = SSet.t

(* The result excepted from the service *)
type result = Utils.error list * failed

(*****************************************************************************)
(* The place where we store the shared data in cache *)
(*****************************************************************************)

module TypeDeclarationStore = GlobalStorage.Make(struct
  type classes = SSet.t SMap.t
  type t = classes * Naming.env
end)

(*****************************************************************************)
(* Synchronizes the typing environment with the cache *)
(*****************************************************************************)

(*****************************************************************************)
(* The job that will be run on the workers *)
(*****************************************************************************)

let decl_file all_classes nenv (errors, failed) fn = try
  d ("Typing decl: "^fn);
  Typing_decl.make_env nenv all_classes fn;
  dn "OK";
  errors, failed
with Utils.Error l ->
  dn "FAILED";
   l :: errors, SSet.add fn failed

let decl_files (errors, failed) fnl =
  let all_classes, nenv = TypeDeclarationStore.load() in
  List.fold_left (decl_file all_classes nenv) (errors, failed) fnl

(*****************************************************************************)
(* Merges the results (used by the master) *)
(*****************************************************************************)

let merge_decl (errors1, failed1) (errors2, failed2) =
  errors1 @ errors2,
  SSet.union failed1 failed2

(*****************************************************************************)
(* We need to know all the classes defined, because we want to declare
 * the types in their topological order.
 * We keep the files in which the classes are defined, sometimes there
 * can be more that one file when there are name collitions.
 *)
(*****************************************************************************)

let get_classes fast =
  SMap.fold begin fun fn {FileInfo.n_classes = classes; _} acc ->
    SSet.fold begin fun c_name acc ->
      let files =
        try SMap.find_unsafe c_name acc with Not_found -> SSet.empty
      in
      let files = SSet.add fn files in
      SMap.add c_name files acc
    end classes acc
  end fast SMap.empty

(*****************************************************************************)
(* The bucket size, given a list of files, give me a sublist such as
 * every worker is busy long enough. If the bucket is too big, it hurts
 * load balancing, if it is too small, the overhead in synchronization time
 * hurts *)
(*****************************************************************************)

(* The bucket size at initialization, we want high throughput and we don't
 * care about the latency
*)
let init_bucket_size = 1000

(*****************************************************************************)
(* Let's go! That's where the action is *)
(*****************************************************************************)

let go workers nenv fast =
  let all_classes = get_classes fast in
  TypeDeclarationStore.store (all_classes, nenv);
  let fast_l = SMap.fold (fun x _ y -> x :: y) fast [] in
  let neutral = [], SSet.empty in
  dn "Declaring the types";
  let result =
    MultiWorker.call
      workers
      ~job:decl_files
      ~neutral
      ~merge:merge_decl
      ~next:(Bucket.make fast_l)
  in
  TypeDeclarationStore.clear();
  result
