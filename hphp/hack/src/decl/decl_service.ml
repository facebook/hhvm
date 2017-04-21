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
(* Module declaring the types in parallel *)
(*****************************************************************************)

open Core
open Utils

(* The set of files that failed *)
type failed = Relative_path.Set.t

(* The result expected from the service *)
type result = Errors.t * failed

type error_info = {
  errs : failed;
  lazy_decl_errs: failed;
}
(* Used for lazy typechecking *)
type lazy_decl_result = Errors.t * error_info

(*****************************************************************************)
(* The place where we store the shared data in cache *)
(*****************************************************************************)

module TypeDeclarationStore = GlobalStorage.Make(struct
  type t = TypecheckerOptions.t
end)

(*****************************************************************************)
(* Synchronizes the typing environment with the cache *)
(*****************************************************************************)

(*****************************************************************************)
(* The job that will be run on the workers *)
(*****************************************************************************)

let decl_file tcopt (errorl, failed) fn =
  let errorl', (), _ = Errors.do_ begin fun () ->
    d ("Typing decl: "^Relative_path.to_absolute fn);
    Decl.make_env tcopt fn;
    dn "OK";
  end
  in
  let failed =
    if Errors.is_empty errorl' then failed
    else Relative_path.Set.add failed fn in
  let errorl = Errors.merge errorl' errorl in
  errorl, failed

let decl_files (errors, failed) fnl =
  let tcopt = TypeDeclarationStore.load() in
  List.fold_left fnl ~f:(decl_file tcopt) ~init:(errors, failed)

(*****************************************************************************)
(* Merges the results (used by the master) *)
(*****************************************************************************)

let merge_decl (errors1, failed1) (errors2, failed2) =
  Errors.merge errors1 errors2,
  Relative_path.Set.union failed1 failed2

let merge_lazy_decl
    (errors1, {errs = failed1; lazy_decl_errs = failed_decl1})
    (errors2, {errs = failed2; lazy_decl_errs = failed_decl2}) =
  Errors.merge errors1 errors2,
    { errs = Relative_path.Set.union failed1 failed2;
      lazy_decl_errs = Relative_path.Set.union failed_decl1 failed_decl2;
    }
(*****************************************************************************)
(* Let's go! That's where the action is *)
(*****************************************************************************)

let go (workers:Worker.t list option) ~bucket_size tcopt fast =
  TypeDeclarationStore.store tcopt;
  let fast_l =
    Relative_path.Map.fold fast ~init:[] ~f:(fun x _ y -> x :: y) in
  let neutral = Errors.empty, Relative_path.Set.empty in
  dn "Declaring the types";
  let result =
    MultiWorker.call
      workers
      ~job:decl_files
      ~neutral
      ~merge:merge_decl
      ~next:(MultiWorker.next ~max_size:bucket_size workers fast_l)
  in
  TypeDeclarationStore.clear();
  result
