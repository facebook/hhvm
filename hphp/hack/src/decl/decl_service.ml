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

open Hh_prelude

(*****************************************************************************)
(* The job that will be run on the workers *)
(*****************************************************************************)

let decl_file ctx errorl fn =
  let (errorl', ()) =
    Errors.do_with_context fn Errors.Decl (fun () ->
        Hh_logger.debug "Typing decl: %s" (Relative_path.to_absolute fn);
        Decl.make_env ~sh:SharedMem.Uses ctx fn;
        Hh_logger.debug "Typing dec OK")
  in
  Errors.merge errorl' errorl

let decl_files ctx errors fnl =
  List.fold_left fnl ~f:(decl_file ctx) ~init:errors

(*****************************************************************************)
(* Let's go! That's where the action is *)
(*****************************************************************************)

let go
    (ctx : Provider_context.t)
    (workers : MultiWorker.worker list option)
    ~bucket_size
    fast =
  let fast_l = Relative_path.Map.fold fast ~init:[] ~f:(fun x _ y -> x :: y) in
  let neutral = Errors.empty in
  Hh_logger.debug "Declaring the types";
  let result =
    MultiWorker.call
      workers
      ~job:(decl_files ctx)
      ~neutral
      ~merge:Errors.merge
      ~next:(MultiWorker.next ~max_size:bucket_size workers fast_l)
  in
  result
