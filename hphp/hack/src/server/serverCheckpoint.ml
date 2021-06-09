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

let checkpoints = ref SMap.empty

let process_updates updates =
  (* Appending changed files to each checkpoint in the map *)
  checkpoints :=
    SMap.map !checkpoints ~f:(fun cur_set ->
        Relative_path.Set.fold
          updates
          ~f:
            begin
              fun path acc ->
              Relative_path.Set.add acc path
            end
          ~init:cur_set)

let create_checkpoint x =
  checkpoints := SMap.add !checkpoints ~key:x ~data:Relative_path.Set.empty

let retrieve_checkpoint x =
  match SMap.find_opt !checkpoints x with
  | Some files ->
    Some
      (List.map (Relative_path.Set.elements files) ~f:Relative_path.to_absolute)
  | None -> None

let delete_checkpoint x =
  match SMap.find_opt !checkpoints x with
  | Some _ ->
    checkpoints := SMap.remove !checkpoints x;
    true
  | None -> false
