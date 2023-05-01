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

let category = "decl_service"

(*****************************************************************************)
(* The job that will be run on the workers *)
(*****************************************************************************)

let decl_file ctx fn =
  Hh_logger.debug ~category "Typing decl: %s" (Relative_path.to_absolute fn);
  Decl.make_env ~sh:SharedMem.Uses ctx fn;
  Hh_logger.debug ~category "Typing decl OK";
  ()

let decl_files ctx fnl = List.iter fnl ~f:(decl_file ctx)

(*****************************************************************************)
(* Let's go! That's where the action is *)
(*****************************************************************************)

let go
    (ctx : Provider_context.t)
    (workers : MultiWorker.worker list option)
    ~bucket_size
    defs_per_file =
  let defs_per_file_l =
    Relative_path.Map.fold defs_per_file ~init:[] ~f:(fun x _ y -> x :: y)
  in
  Hh_logger.debug ~category "Declaring the types";
  let result =
    MultiWorker.call
      workers
      ~job:(fun () fnl -> decl_files ctx fnl)
      ~neutral:()
      ~merge:(fun () () -> ())
      ~next:(MultiWorker.next ~max_size:bucket_size workers defs_per_file_l)
  in
  result
