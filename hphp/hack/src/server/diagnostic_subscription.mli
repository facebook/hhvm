(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Reordered_argument_collections

type id = int

type t [@@deriving show]

(** Makes a diagnostic subscription with ID [id]. The ID is used
    by callers to distinguish multiple subscriptions. *)
val of_id : ?error_limit:int -> initial_errors:Errors.t -> id -> t

val get_id : t -> int

(** Set a limit on the number of errors to push. Default is 1000. *)
val set_error_limit : t -> int -> t

(** Update diagnostics subscription based on an incremental recheck that
    was done. *)
val update :
  t ->
  priority_files:Relative_path.Set.t ->
  global_errors:Errors.t ->
  full_check_done:bool ->
  t

(** Files to send diagnostic for. We don't send diagnostic for all the files with errors
    for performance reason *)
val diagnosed_files : t -> Relative_path.Set.t

(** Pop errors ready for sending to client.
    The option returned indicates whether the list of errors has been truncated:
    we return [None] if the list has not been truncated, otherwise we return
    [Some n] where n is the total number of errors. *)
val pop_errors :
  t ->
  global_errors:Errors.t ->
  t * Errors.finalized_error list SMap.t * int option

val get_pushed_error_length : t -> int option * int
