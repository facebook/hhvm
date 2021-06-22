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

type errors = Errors.error list

type id = int [@@deriving show]

type t = {
  id: id;
      (** Unique ID used by users of this module to distinguish multiple subscriptions. *)
  error_limit: int;
  diagnosed_files: RP.Set.t;
      (** Files to send diagnostic for. We don't send diagnostic for all the files with errors
          for performance reason: to keep things responsive, we'll keep make sure to push
          at most [errors_limit] errors. *)
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
  is_truncated: int option;
      (** [None] if [diagnosed_files] contains all the files with errors,
          [Some n] with n the total number of errors otherwise *)
}
[@@deriving show]

(** Throttle errors - pause pushing errors after editor is aware of that many errors.
    Resume when user fixes some of them. *)
let default_error_limit = 1000

let get_id ds = ds.id

let diagnosed_files ds = ds.diagnosed_files

let set_error_limit : t -> int -> t =
 (fun diag error_limit -> { diag with error_limit })

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
  let { diagnosed_files; error_limit; _ } = ds in
  let diagnosed_files = RP.Set.union diagnosed_files priority_files in
  (* Remove files which no longer have errors *)
  let (error_count, diagnosed_files) =
    RP.Set.fold
      diagnosed_files
      ~init:(0, RP.Set.empty)
      ~f:(fun file (count, diagnosed_files) ->
        let errors = Errors.get_file_errors global_errors file in
        let n = Errors.per_file_error_count errors in
        let diagnosed_files =
          if Int.equal 0 n then
            diagnosed_files
          else
            RP.Set.add diagnosed_files file
        in
        let count = n + count in
        (count, diagnosed_files))
  in
  (* If we've done a full check, then it's safe to look through all of the
   * errors in global_errors, not only those that were just rechecked *)
  let (is_truncated, diagnosed_files) =
    if not full_check_done then
      (None, diagnosed_files)
    else
      let (diagnosed_files, error_count, is_truncated) =
        (* Add files to diagnosed_files until we've reached the error limit. *)
        Errors.fold_per_file
          global_errors
          ~init:(diagnosed_files, error_count, false)
          ~f:(fun file errors (diagnosed_files, error_count, is_truncated) ->
            if RP.Set.mem diagnosed_files file then
              (diagnosed_files, error_count, is_truncated)
            else
              let n = Errors.per_file_error_count errors in
              let error_count = error_count + n in
              if is_truncated || error_count > error_limit then
                (diagnosed_files, error_count, true)
              else
                let diagnosed_files = RP.Set.add diagnosed_files file in
                (diagnosed_files, error_count, false))
      in
      let is_truncated =
        if is_truncated then
          Some error_count
        else
          None
      in
      (is_truncated, diagnosed_files)
  in
  { ds with diagnosed_files; has_new_errors = true; is_truncated }

let of_id ?error_limit ~initial_errors id =
  let res =
    {
      id;
      error_limit = Option.value error_limit ~default:default_error_limit;
      diagnosed_files = RP.Set.empty;
      pushed_errors = RP.Map.empty;
      has_new_errors = false;
      is_truncated = None;
    }
  in
  update
    res
    ~priority_files:Relative_path.Set.empty
    ~global_errors:initial_errors
    ~full_check_done:true

(** Return and record errors to send to subscriber. [ds] is current diagnostics
    subscription used to choose which errors from global error list to push. *)
let pop_errors ds ~global_errors =
  if not ds.has_new_errors then
    (ds, SMap.empty, None)
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
      errors_to_send,
      ds.is_truncated )

let get_pushed_error_length ds =
  let length =
    RP.Map.fold ds.pushed_errors ~init:0 ~f:(fun _rp errors acc ->
        acc + List.length errors)
  in
  (ds.is_truncated, length)
