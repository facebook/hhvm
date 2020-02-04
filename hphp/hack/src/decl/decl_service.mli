(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*****************************************************************************)
(* Starts the process *)
(*****************************************************************************)
val go :
  Provider_context.t ->
  MultiWorker.worker list option ->
  bucket_size:int ->
  Naming_table.fast ->
  Errors.t

val decl_file : Provider_context.t -> Errors.t -> Relative_path.t -> Errors.t
