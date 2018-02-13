(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Reordered_argument_collections

type t

val of_id : id:int -> init:Errors.t -> t

val get_id : t -> int

val update :
  t ->
  priority_files:Relative_path.Set.t ->
  reparsed:Relative_path.Set.t ->
  rechecked:FileInfo.fast ->
  global_errors:Errors.t ->
  full_check_done:bool ->
  t

val error_sources : t -> Relative_path.Set.t

(* Errors ready for sending to client *)
val pop_errors :
  t ->
  global_errors:Errors.t ->
  t * (Pos.absolute Errors.error_ list) SMap.t
