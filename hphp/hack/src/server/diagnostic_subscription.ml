(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Reordered_argument_collections
module RP = Relative_path

(* Throttle errors - pause pushing errors after editor is aware of errors in
 * that many files. Resume when user fixes some of them. *)
let errors_limit = 10

type errors = Errors.error list

type t = {
  id: int;
      (** Unique ID used by users of this module to distinguish multiple subscriptions. *)
  diagnosed_files: RP.Set.t;
      (** Files to send diagnostic for. We don't send diagnostic for all the files with errors
          for performance reason: to keep things responsive, we'll keep

          |diagnosed_files| < max (errors_limit , |priority_files| + |pushed_errors|

          i.e we always report things in priority_files, and if we reported some
          errors in a file, we'll keep them fresh until they are fixed. If after
          that we still report errors in less than errors_limit files, we'll "top up"
          with errors from some arbitrary files. *)
  pushed_errors: errors RP.Map.t;
      [@printer
        fun fmt errors ->
          RP.Map.pp
            (fun fmt -> Format.fprintf fmt "%d")
            fmt
            (RP.Map.map errors ~f:List.length)]
      (** Copy of errors most recently pushed to subscribers, used to avoid pushing
          no-op duplicates *)
  has_new_errors: bool;
  is_truncated: bool;
      (** Whether [diagnosed_files] contains all the files with errors. *)
}
[@@deriving show]

let get_id ds = ds.id

let diagnosed_files ds = ds.diagnosed_files

(** Update diagnostics subscription based on an incremental recheck that
    was done. *)
let update
    ds
    ~(* to keep things responsive in a scenario with thousands of errors,
      * we'll drop some of them. priority_files will never have errors dropped *)
    priority_files
    ~(* new set of errors after incremental recheck *)
    global_errors
    ~(* If incremental recheck was not full, only a subset of errors in
      * global_errors is guaranteed to be up to date. ServerTypeCheck keeps this
      * subset to be union of priority_files and Diagnsotic_subscription.sources
      *)
    full_check_done =
  let diagnosed_files = RP.Set.union ds.diagnosed_files priority_files in
  (* Remove files which no longer have errors *)
  let diagnosed_files =
    RP.Set.filter diagnosed_files ~f:(Errors.file_has_errors global_errors)
  in
  (* If we've done a full check, then it's safe to look through all of the
   * errors in global_errors, not only those that were just rechecked *)
  let (is_truncated, diagnosed_files) =
    if not full_check_done then
      (false, diagnosed_files)
    else
      Errors.fold_errors
        global_errors
        ~init:(false, diagnosed_files)
        ~f:(fun source _e (is_truncated, diagnosed_files) ->
          if is_truncated || RP.Set.cardinal diagnosed_files >= errors_limit
          then
            (true, diagnosed_files)
          else
            (false, RP.Set.add diagnosed_files source))
  in
  { ds with diagnosed_files; has_new_errors = true; is_truncated }

let of_id ~id ~init =
  let res =
    {
      id;
      diagnosed_files = RP.Set.empty;
      pushed_errors = RP.Map.empty;
      has_new_errors = false;
      is_truncated = false;
    }
  in
  update
    res
    ~priority_files:Relative_path.Set.empty
    ~global_errors:init
    ~full_check_done:true

(** Return and record errors to send to subscriber. [ds] is current diagnostics
    subscription used to choose which errors from global error list to push. *)
let pop_errors ds ~global_errors =
  if not ds.has_new_errors then
    (ds, SMap.empty)
  else
    let new_pushed_errors =
      RP.Set.fold
        ds.diagnosed_files
        ~init:RP.Map.empty
        ~f:(fun diagnosed_file new_pushed_errors ->
          let errors =
            Errors.errors_in_file global_errors diagnosed_file |> Errors.sort
          in
          RP.Map.add new_pushed_errors ~key:diagnosed_file ~data:errors)
    in
    (* Ignore unchanged errors, add "[]" messages for cleared errors. *)
    let errors_to_send =
      RP.Map.merge ds.pushed_errors new_pushed_errors ~f:(fun _ old new_ ->
          match (old, new_) with
          | (None, None) -> None
          | (None, Some new_) -> Some new_
          | (Some _, None) -> (* Erase previous diagnostics *) Some []
          | (Some old, Some new_) when List.equal Errors.equal_error old new_ ->
            (* Errors haven't changed, do not send anything for that file. *)
            None
          | (Some _, Some new_) ->
            (* Errors have changed, send the new list. *)
            Some new_)
    in
    (* Convert to absolute paths *)
    let errors_to_send =
      RP.Map.fold errors_to_send ~init:SMap.empty ~f:(fun path el acc ->
          SMap.add
            acc
            ~key:(Relative_path.to_absolute path)
            ~data:(List.map el ~f:Errors.to_absolute))
    in
    ( { ds with pushed_errors = new_pushed_errors; has_new_errors = false },
      errors_to_send )

let get_pushed_error_length ds =
  let length =
    RP.Map.fold ds.pushed_errors ~init:0 ~f:(fun _rp errors acc ->
        acc + List.length errors)
  in
  (ds.is_truncated, length)
