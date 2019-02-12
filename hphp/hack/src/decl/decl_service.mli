(**
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

(* The result expected from the service *)
type result = Errors.t

(* Used for lazy typechecking *)
type lazy_decl_result = Errors.t

(*****************************************************************************)
(* Starts the process *)
(*****************************************************************************)
val go: MultiWorker.worker list option -> bucket_size:int ->
  Naming_table.fast -> result
val merge_lazy_decl: lazy_decl_result -> lazy_decl_result -> lazy_decl_result
