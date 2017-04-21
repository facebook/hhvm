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
(* Starts the process *)
(*****************************************************************************)
val go: Worker.t list option -> bucket_size:int -> TypecheckerOptions.t ->
  FileInfo.fast -> result
val merge_decl: result -> result -> result
val merge_lazy_decl: lazy_decl_result -> lazy_decl_result -> lazy_decl_result
