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

      let entry = Relative_path.Map.singleton file [e] in
      Relative_path.Map.merge acc entry
        (fun _ x y -> Option.merge x y ~f:(fun x y -> x @ y))
    end

let of_id ~id ~init = {
  id;
  pushed_errors = Relative_path.Set.empty;
  pending_errors = errors_to_map init;
}

let get_id ds = ds.id

let mark_as_pushed ds =
  (* Move pending_errors to pushed_errors, unless the error list was empty
    - in that case we can completely remove the file from both lists. *)
  let pushed_errors = Relative_path.Map.fold ds.pending_errors
    ~init:ds.pushed_errors
    ~f:begin fun path errors pushed_errors ->
      match errors with
      | [] -> Relative_path.Set.remove pushed_errors path
      | _ -> Relative_path.Set.add pushed_errors path
    end
  in
  { ds with pushed_errors; pending_errors = Relative_path.Map.empty }

let get_absolute_errors ds =
  Relative_path.Map.fold ds.pending_errors ~init:SMap.empty
  ~f:begin fun path errors acc ->
    let path = Relative_path.to_absolute path in
    let errors = List.map errors ~f:Errors.to_absolute in
    SMap.add acc path errors
  end

let update ds checked_files errors =
  let new_errors = errors_to_map errors in
  (* Merge errors overwriting old ones with new ones *)
  let pending_errors = Relative_path.Map.merge ds.pending_errors new_errors
    begin fun _ x y -> Option.merge x y (fun _ y -> y) end
  in
  (* With incremental push subscriptions we need to also send
   * an empty error list for files that used to have errors *)
  let pending_errors = Relative_path.Set.fold ds.pushed_errors
    ~init:pending_errors
    ~f:begin fun path pending_errors ->
      if Relative_path.Map.mem checked_files path &&
        (not @@ Relative_path.Map.mem pending_errors path) then
        Relative_path.Map.add pending_errors path []
      else pending_errors
    end
  in
  { ds with pending_errors }
