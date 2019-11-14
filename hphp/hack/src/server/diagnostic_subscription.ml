(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel
open Reordered_argument_collections
module RP = Relative_path

(* Throttle errors - pause pushing errors after editor is aware of errors in
 * that many files. Resume when user fixes some of them. *)
let errors_limit = 10

type errors = Errors.error list

type t = {
  id: int;
  (* Hack server indexes (in ServerEnv.errorl) errors based on where they were
   * GENERATED: for example A -> decl -> e means that error e is uncovered
   * during declaration of A. But the error itself can be reported in completely
   * different file (like As parent C).
   *
   * Things like IDE don't want this mapping - when asking about A they want to
   * get all errors REPORTED, not GENERATED in A. local_errors maintains reverse
   * mapping from files with reported errors to files that cause them to be
   * reported, i.e A -> [A, C] means that A is a file that IDE cares about and
   * to get its error list we need to analyse A and C.
   *
   * To keep things responsive, we'll keep
   *
   * |local_errors| < max (errors_limit , |priority_files| + |pushed_errors|)
   *
   * i.e we always report things in priority_files, and if we reported some
   * errors in a file, we'll keep them fresh until they are fixed. If after
   * that we still report errors in less than errors_limit files, we'll "top up"
   * with errors from some arbitrary files *)
  local_errors: RP.Set.t RP.Map.t;
  (* Copy of errors most recently pushed to subscribers, used to avoid pushing
   * no-op duplicates *)
  pushed_errors: errors RP.Map.t;
      [@printer
        fun fmt errors ->
          RP.Map.pp
            (fun fmt -> Format.fprintf fmt "%d")
            fmt
            (RP.Map.map errors List.length)]
  (* Union of all values in local_errors *)
  sources: Relative_path.Set.t;
  has_new_errors: bool;
  is_truncated: bool;
      (* was 'local_errors' truncated with respect to all the errors? *)
}
[@@deriving show]

let filter_filter map_map ~f =
  RP.Map.map map_map ~f:(fun v -> RP.Set.filter v ~f:(fun e -> f e))
  |> RP.Map.filter ~f:(fun _ v -> not @@ RP.Set.is_empty v)

let error_filename e = Errors.get_pos e |> Pos.filename

let get_id ds = ds.id

let error_sources ds = ds.sources

(* Update diagnostics subscription based on an incremental recheck that
 * was done. *)
let update
    ds
    ~(* to keep things responsive in a scenario with thousands of errors,
      * we'll drop some of them. priority_files will never have errors dropped *)
    priority_files
    ~(* set of files that were reparsed during incremental recheck. If a file is
      * in this set, but not in global_errors, it means that it doesn't have
      * errors any more *)
    reparsed
    ~(* rechecked is used exactly the same as reparsed. It's split in two
      * arguments because those collections are stored as two different
      * structures in ServerTypeCheck and we don't want to force the caller
      * to merge them. *)
    rechecked
    ~(* new set of errors after incremental recheck *)
    global_errors
    ~(* If incremental recheck was not full, only a subset of errors in
      * global_errors is guaranteed to be up to date. ServerTypeCheck keeps this
      * subset to be union of priority_files and Diagnsotic_subscription.sources
      *)
    full_check_done =
  (* Update local_errors with possibly changed priority_files *)
  let local_errors =
    RP.Set.fold priority_files ~init:ds.local_errors ~f:(fun path acc ->
        (* We already track this file. *)
        if RP.Map.mem acc path then
          acc
        (* Initially the only known source of errors in a file is that file
         * itself. There might be more errors that will eventually be reported in
      * this file, but we'll not know them until next full check. *)
        else
          RP.Map.add acc path (RP.Set.singleton path))
  in
  (* Merge rechecked and reparsed into single collection. *)
  let rechecked = RP.Set.union reparsed rechecked in
  (* Look through rechecked files for more sources of tracked files *)
  let local_errors =
    RP.Set.fold rechecked ~init:local_errors ~f:(fun source acc ->
        Errors.fold_errors_in global_errors ~source ~init:acc ~f:(fun e acc ->
            let file = error_filename e in
            match RP.Map.find_opt acc file with
            | None -> acc (* not an IDE-relevant file *)
            | Some sources -> RP.Map.add acc file (RP.Set.add sources source)))
  in
  (* Remove sources that no longer produce errors in files we care about *)
  let local_errors =
    filter_filter local_errors ~f:(fun source ->
        Errors.fold_errors_in global_errors ~source ~init:false ~f:(fun e acc ->
            acc || RP.Map.mem local_errors (error_filename e)))
  in
  (* If we've done a full check, that it's safe to look through all of the
   * errors in global_errors, no only those that were just rechecked *)
  let (is_truncated, local_errors) =
    if not full_check_done then
      (false, local_errors)
    else
      Errors.fold_errors
        global_errors
        ~init:(false, local_errors)
        ~f:(fun source e (is_truncated, acc) ->
          let file = error_filename e in
          match RP.Map.find_opt acc file with
          | Some sources ->
            (* Add a source to already tracked file *)
            (is_truncated, RP.Map.add acc file (RP.Set.add sources source))
          | None when RP.Map.cardinal acc < errors_limit ->
            (* not an IDE-relevant file, but we still have room for it *)
            (is_truncated, RP.Map.add acc file (RP.Set.singleton source))
          | None ->
            (* we're at error limit, ignore *)
            (true, acc))
  in
  let sources =
    RP.Map.fold local_errors ~init:RP.Set.empty ~f:(fun _ x acc ->
        RP.Set.union acc x)
  in
  { ds with local_errors; has_new_errors = true; is_truncated; sources }

let of_id ~id ~init =
  let res =
    {
      id;
      local_errors = RP.Map.empty;
      pushed_errors = RP.Map.empty;
      sources = RP.Set.empty;
      has_new_errors = false;
      is_truncated = false;
    }
  in
  update
    res
    ~priority_files:Relative_path.Set.empty
    ~reparsed:Relative_path.Set.empty
    ~rechecked:Relative_path.Set.empty
    ~global_errors:init
    ~full_check_done:true

(* Return and record errors as send to subscriber. ds is current diagnostics
 * subscription used to choose which errors from global error list to push. *)
let pop_errors ds ~global_errors =
  if not ds.has_new_errors then
    (ds, SMap.empty)
  else
    (* Go over tracked files...*)
    let new_pushed_errors =
      RP.Map.mapi ds.local_errors (fun path sources ->
          (* ... and sources of errors reported in them... *)
          RP.Set.fold sources ~init:[] ~f:(fun source acc ->
              (* ... and the errors themselves... *)
              Errors.fold_errors_in
                global_errors
                ~source
                ~init:acc
                ~f:(fun e acc ->
                  (* ... filtering out ones irrelevant to tracked files. *)
                  if error_filename e <> path then
                    acc
                  else
                    e :: acc))
          |> Errors.sort)
    in
    (* Ignore unchanged errors, add "[]" messages for cleared errors. *)
    let results =
      RP.Map.merge ds.pushed_errors new_pushed_errors ~f:(fun _ old new_ ->
          match (old, new_) with
          | (None, Some new_) -> Some new_
          | (Some old, Some new_) when old <> new_ -> Some new_
          | (Some _, None) -> Some []
          | _ -> None)
    in
    (* Convert to absolute paths *)
    let results =
      RP.Map.fold results ~init:SMap.empty ~f:(fun path el acc ->
          SMap.add
            acc
            (Relative_path.to_absolute path)
            (List.map el ~f:Errors.to_absolute))
    in
    ( { ds with pushed_errors = new_pushed_errors; has_new_errors = false },
      results )

let get_pushed_error_length ds =
  let length =
    RP.Map.fold ds.pushed_errors ~init:0 ~f:(fun _rp errors acc ->
        acc + List.length errors)
  in
  (ds.is_truncated, length)
