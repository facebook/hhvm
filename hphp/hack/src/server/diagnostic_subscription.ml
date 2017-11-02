(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)
open Hh_core
open Reordered_argument_collections

(* Throttle errors - pause pushing errors after editor is aware of errors in
 * that many files. Resume when user fixes some of them. *)
let errors_limit = 10

type errors = Errors.error list

type t = {
  id : int;
  (* Most recent diagnostics pushed to client for those files had errors *)
  pushed_errors : errors Relative_path.Map.t;
  (* Push diagnostics not yet sent to client *)
  pending_errors : errors Relative_path.Map.t;
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

let fill_to_limit ~from ~into ~limit =
  Relative_path.Map.fold from
    ~init:into
    ~f:begin fun path errors acc ->
      if Relative_path.Map.cardinal acc < limit then
        Relative_path.Map.add acc path errors
      else
        acc
    end

let of_id ~id ~init = {
  id;
  pushed_errors = Relative_path.Map.empty;
  pending_errors =
    fill_to_limit
      ~from:(errors_to_map init)
      ~into:Relative_path.Map.empty
      ~limit:errors_limit;
}

let get_id ds = ds.id

let pop_errors ds =
  (* Move pending_errors to pushed_errors, unless the error list was empty
  - in that case we can completely remove the file from both lists. *)
  let pushed_errors =
    Relative_path.Map.merge ds.pushed_errors ds.pending_errors
      ~f:begin fun _ pushed pending -> match pushed, pending with
        | Some _, Some pending -> if pending <> [] then Some pending else None
        | None, Some x
        | Some x, None -> Some x
        | _ -> None
      end
  in
  let result = Relative_path.Map.fold ds.pending_errors
    ~init:SMap.empty
    ~f:begin fun path errors acc ->
      let path = Relative_path.to_absolute path in
      let errors = List.map errors ~f:Errors.to_absolute in
      SMap.add acc path errors
    end
  in
  let ds = { ds with
    pushed_errors;
    pending_errors = Relative_path.Map.empty;
  } in
  ds, result

let file_has_errors_in_ide ds path =
  (Relative_path.Map.mem ds.pushed_errors path) ||
  (Relative_path.Map.mem ds.pending_errors path)

let map_to_set map acc =
  Relative_path.Map.fold map
    ~init:acc
    ~f:(fun path _ acc -> Relative_path.Set.add acc path)

let files_with_errors_in_ide ds =
  Relative_path.Set.(
    empty |>
    (map_to_set ds.pending_errors) |>
    (map_to_set ds.pushed_errors)
  )

let clear_pending_errors_for_file path pushed_errors pending_errors =
  if Relative_path.Map.mem pushed_errors path then
    (* We already pushed errors for this file - we need to push a "clear errors"
     * update too *)
    Relative_path.Map.add pending_errors path []
  else
    (* The errors were cleared before we pushed them to client. Just forget
     * about them. *)
    Relative_path.Map.remove pending_errors path

(* If the errors that were recomputed (and are now pending) are the
 * same as the errors already sent to editor (pushed), do not re-send them
 * again *)
let filter_unchanged ds new_errors =
  Relative_path.Map.filter new_errors begin fun path errors ->
    Relative_path.Map.get ds.pushed_errors path <> Some errors
  end

(* To keep the IDE state << O(codebase), cut down the errors we remember to be:
 * - errors in open files
 * - errors in other files that we've already sent and need to keep updating
 * - up to errors_limit of other errors
 *)
let throttle ds ide_files pending_errors =
  let ide_errors, other_errors = Relative_path.Map.partition pending_errors
    ~f:begin fun path _ ->
      Relative_path.Set.mem ide_files path ||
        Relative_path.Map.mem ds.pushed_errors path
    end
  in
  fill_to_limit
    ~from:other_errors
    ~into:ide_errors
    ~limit:(errors_limit - Relative_path.Map.cardinal ds.pushed_errors)


let update ds ide_files checked_files errors =

  let errors = errors_to_map errors in

  (* Merge errors overwriting old ones with new ones *)
  let pending_errors = Relative_path.Map.merge ds.pending_errors errors
    begin fun _ x y -> Option.merge x y (fun _ y -> y) end
  in

  let pending_errors = filter_unchanged ds pending_errors in

  (* If a file was checked, but is not in error list, it means that we are sure
   * that it has no errors now *)
  let pending_errors = Relative_path.Map.fold checked_files
    ~init:pending_errors
    ~f:begin fun k _ acc ->
      if Relative_path.Map.mem errors k then acc
      else clear_pending_errors_for_file k ds.pushed_errors acc
    end
  in
  let pending_errors = throttle ds ide_files pending_errors in
  { ds with pending_errors }
