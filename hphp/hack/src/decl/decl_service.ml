(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Module declaring the types in parallel *)
(*****************************************************************************)

open Core_kernel
open Utils

(*****************************************************************************)
(* The job that will be run on the workers *)
(*****************************************************************************)

let decl_file errorl fn =
  let (errorl', ()) =
    Errors.do_with_context fn Errors.Decl (fun () ->
        d ("Typing decl: " ^ Relative_path.to_absolute fn);
        Decl.make_env fn;
        dn "OK")
  in
  Errors.merge errorl' errorl

let decl_files errors fnl = List.fold_left fnl ~f:decl_file ~init:errors

(*****************************************************************************)
(* Let's go! That's where the action is *)
(*****************************************************************************)

let go (workers : MultiWorker.worker list option) ~bucket_size fast =
  let fast_l = Relative_path.Map.fold fast ~init:[] ~f:(fun x _ y -> x :: y) in
  let neutral = Errors.empty in
  dn "Declaring the types";
  let result =
    MultiWorker.call
      workers
      ~job:decl_files
      ~neutral
      ~merge:Errors.merge
      ~next:(MultiWorker.next ~max_size:bucket_size workers fast_l)
  in
  result
