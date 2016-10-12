(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
open Core
open Reordered_argument_collections

module RP = Relative_path

(* Throttle errors - pause pushing errors after editor is aware of errors in
 * that many files. Resume when user fixes some of them. *)
let errors_limit = 50

type t = {
  id : int;
  (* Most recent diagnostics pushed to client for those files had errors *)
  pushed_errors : RP.Set.t;
  (* Push diagnostics not yet sent to client *)
  pending_errors : (Errors.error list) RP.Map.t;
}

let errors_to_map errors =
  List.fold_right (Errors.get_sorted_error_list errors)
    ~init:RP.Map.empty
    ~f:begin fun e acc ->
      let file = Errors.get_pos e |> Pos.filename in

      RP.Map.add acc file (match RP.Map.get acc file with
      | None -> [e]
      | Some x -> e :: x)
    end

let of_id ~id ~init = {
  id;
  pushed_errors = RP.Set.empty;
  pending_errors = errors_to_map init;
}

let get_id ds = ds.id

let mark_as_pushed ds path errors =
  (* Move pending_errors to pushed_errors, unless the error list was empty
    - in that case we can completely remove the file from both lists. *)
  let pending_errors = RP.Map.remove ds.pending_errors path in
  let pushed_errors = match errors with
    | [] -> RP.Set.remove ds.pushed_errors path
    | _ -> RP.Set.add ds.pushed_errors path
  in
  { ds with pushed_errors; pending_errors }

let pop_key k (ds, acc) =
  match RP.Map.get ds.pending_errors k with
  | Some v -> (mark_as_pushed ds k v), (RP.Map.add acc k v)
  | None -> ds, acc

let pop_if_in_map map acc =
  Relative_path.Map.fold map ~init:acc ~f:(fun k _ acc -> pop_key k acc)

let pop_if_in_set set acc =
  RP.Set.fold set ~init:acc ~f:(fun k acc -> pop_key k acc)

let rec pop_while_less_than_limit (ds, acc) =
  if RP.Set.cardinal ds.pushed_errors < errors_limit then begin
    match RP.Map.choose ds.pending_errors with
    | Some (k, v) ->
      let ds, acc = (mark_as_pushed ds k v), (RP.Map.add acc k v) in
      pop_while_less_than_limit (ds, acc)
    | None -> ds, acc
  end else (ds, acc)

let pop_errors ds edited_files =
  let ds, acc =
    (* Always push errors for files open in editor...*)
    pop_if_in_map edited_files (ds, RP.Map.empty)
    (* ... and for files which editor is aware of having errors. *)
    |> pop_if_in_set ds.pushed_errors
    (* Push the remaining errors as long as editor doesn't have to display too
     * many of them *)
    |> pop_while_less_than_limit
  in
  ds, RP.Map.fold acc ~init:SMap.empty
  ~f:begin fun path errors acc ->
    let path = RP.to_absolute path in
    let errors = List.map errors ~f:Errors.to_absolute in
    SMap.add acc path errors
  end

let file_has_errors_in_ide ds path =
  (RP.Set.mem ds.pushed_errors path) || (RP.Map.mem ds.pending_errors path)

let clear_pending_errors_for_file path pushed_errors pending_errors =
  if RP.Set.mem pushed_errors path then
    (* We already pushed errors for this file - we need to push a "clear errors"
     * update too *)
    RP.Map.add pending_errors path []
  else
    (* The errors were cleared before we pushed them to client. Just forget
     * about them. *)
    RP.Map.remove pending_errors path

let update ds checked_files errors =
  let new_errors = errors_to_map errors in
  (* Merge errors overwriting old ones with new ones *)
  let pending_errors = RP.Map.merge ds.pending_errors new_errors
    begin fun _ x y -> Option.merge x y (fun _ y -> y) end
  in

  (* If a file was checked, but is not in error list, it means that we are sure
  * that it has no errors now*)
  let pending_errors = Relative_path.Map.fold checked_files
    ~init:pending_errors
    ~f:begin fun k _ acc ->
      if RP.Map.mem new_errors k then acc
      else clear_pending_errors_for_file k ds.pushed_errors acc
    end
  in
  { ds with pending_errors }
