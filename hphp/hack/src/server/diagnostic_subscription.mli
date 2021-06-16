(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Reordered_argument_collections

type t [@@deriving show]

(** Makes a diagnostic subscription with ID [id]. The ID is used
    by callers to distinguish multiple subscriptions. *)
val of_id : id:int -> init:Errors.t -> t

val get_id : t -> int

(** Update diagnostics subscription based on an incremental recheck that
    was done. *)
val update :
  t ->
  priority_files:Relative_path.Set.t ->
  reparsed:Relative_path.Set.t ->
  rechecked:Relative_path.Set.t ->
  global_errors:Errors.t ->
  full_check_done:bool ->
  t

val error_sources : t -> Relative_path.Set.t

(** Pop errors ready for sending to client *)
val pop_errors :
  t -> global_errors:Errors.t -> t * Errors.finalized_error list SMap.t

val get_pushed_error_length : t -> bool * int
