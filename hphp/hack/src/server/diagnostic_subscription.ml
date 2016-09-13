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
  errors : (Errors.error list) Relative_path.Map.t;
}

let of_id ~id = {
  id;
  errors = Relative_path.Map.empty;
}

let get_id ds = ds.id

let clear ds = { ds with errors = Relative_path.Map.empty }

let get_absolute_errors ds =
  Relative_path.Map.fold ds.errors ~init:SMap.empty
  ~f:begin fun path errors acc ->
    let path = Relative_path.to_absolute path in
    let errors = List.map errors ~f:Errors.to_absolute in
    SMap.add acc path errors
  end

let errors_to_map errors =
  List.fold_right (Errors.get_sorted_error_list errors)
    ~init:Relative_path.Map.empty
    ~f:begin fun e acc ->
      let file = Errors.get_pos e |> Pos.filename in

      let entry = Relative_path.Map.singleton file [e] in
      Relative_path.Map.merge acc entry
        (fun _ x y -> Option.merge x y ~f:(fun x y -> x @ y))
    end

let update ds errors =
  let errors = Relative_path.Map.merge ds.errors (errors_to_map errors)
    begin fun _ x y -> match x, y with
      | None, None -> None
      | Some _, None ->
          (* This is why we merge the old map with new one, instead of just
           * replacing it - with incremental push subscriptions we need to send
           * an empty error list for files that used to have errors *)
          Some []
      | _, Some x -> Some x
    end
  in
  { ds with errors }
