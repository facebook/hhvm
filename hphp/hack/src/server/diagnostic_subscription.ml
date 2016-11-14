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

(* Throttle errors - pause pushing errors after editor is aware of errors in
 * that many files. Resume when user fixes some of them. *)
let errors_limit = 50

type t = {
  id : int;
  (* Most recent diagnostics pushed to client for those files had errors *)
  pushed_errors : Relative_path.Set.t;
  (* Push diagnostics not yet sent to client *)
  pending_errors : (Errors.error list) Relative_path.Map.t;
}

let errors_to_map errors =
  List.fold_right (Errors.get_sorted_error_list errors)
    ~init:Relative_path.Map.empty
    ~f:begin fun e acc ->
      let file = Errors.get_pos e |> Pos.filename in

      Relative_path.Map.add acc file (match Relative_path.Map.get acc file with
      | None -> [e]
      | Some x -> e :: x)
    end

let of_id ~id ~init = {
  id;
  pushed_errors = Relative_path.Set.empty;
  pending_errors = errors_to_map init;
}

let get_id ds = ds.id

let mark_as_pushed ds path errors =
  (* Move pending_errors to pushed_errors, unless the error list was empty
    - in that case we can completely remove the file from both lists. *)
  let pending_errors = Relative_path.Map.remove ds.pending_errors path in
  let pushed_errors = match errors with
    | [] -> Relative_path.Set.remove ds.pushed_errors path
    | _ -> Relative_path.Set.add ds.pushed_errors path
  in
  { ds with pushed_errors; pending_errors }

let pop_key k (ds, acc) =
  match Relative_path.Map.get ds.pending_errors k with
  | Some v -> (mark_as_pushed ds k v), (Relative_path.Map.add acc k v)
  | None -> ds, acc

let pop_if_in_set set acc =
  Relative_path.Set.fold set ~init:acc ~f:(fun k acc -> pop_key k acc)

let rec pop_while_less_than_limit (ds, acc) =
  if Relative_path.Set.cardinal ds.pushed_errors < errors_limit then begin
    match Relative_path.Map.choose ds.pending_errors with
    | Some (k, v) ->
      let ds, acc = (mark_as_pushed ds k v), (Relative_path.Map.add acc k v) in
      pop_while_less_than_limit (ds, acc)
    | None -> ds, acc
  end else (ds, acc)

let pop_errors ds edited_files =
  let ds, acc =
    (* Always push errors for files open in editor...*)
    pop_if_in_set edited_files (ds, Relative_path.Map.empty)
    (* ... and for files which editor is aware of having errors. *)
    |> pop_if_in_set ds.pushed_errors
    (* Push the remaining errors as long as editor doesn't have to display too
     * many of them *)
    |> pop_while_less_than_limit
  in
  ds, Relative_path.Map.fold acc ~init:SMap.empty
  ~f:begin fun path errors acc ->
    let path = Relative_path.to_absolute path in
    let errors = List.map errors ~f:Errors.to_absolute in
    SMap.add acc path errors
  end

let file_has_errors_in_ide ds path =
  (Relative_path.Set.mem ds.pushed_errors path) ||
  (Relative_path.Map.mem ds.pending_errors path)

let update ds errors =
  let pending_errors = Relative_path.Set.fold ds.pushed_errors
    ~init:(errors_to_map errors)
    ~f:begin fun k acc ->
      if Relative_path.Map.mem acc k then acc
      (* We already pushed errors for this file - we need to push a
       * "clear errors" update too *)
      else Relative_path.Map.add acc k []
    end
  in
  { ds with pending_errors }
