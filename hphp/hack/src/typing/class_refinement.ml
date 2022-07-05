(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Typing_defs

let is_empty { cr_types = trs } = SMap.is_empty trs

let map_type_refinement :
    type a. (a ty -> a ty) -> a class_type_refinement -> a class_type_refinement
    =
 fun f r ->
  match r with
  | Texact ty -> Texact (f ty)
  | Tloose { tr_lower = ls; tr_upper = us } ->
    Tloose { tr_lower = List.map ls ~f; tr_upper = List.map us ~f }

let map f { cr_types = trs } =
  { cr_types = SMap.map (map_type_refinement f) trs }

let fold_type_refinement r ~init:acc ~f =
  match r with
  | Texact ty -> f acc ty
  | Tloose { tr_lower = ls; tr_upper = us } ->
    let acc = List.fold ~f ~init:acc ls in
    let acc = List.fold ~f ~init:acc us in
    acc

let fold { cr_types = trs } ~init:acc ~f =
  SMap.fold (fun _ tr acc -> fold_type_refinement tr ~init:acc ~f) trs acc

let iter f r = fold r ~init:() ~f:(fun () -> f)

let to_string ty_to_string { cr_types = trs } =
  let tref_to_string r =
    match r with
    | Texact ty -> "= " ^ ty_to_string ty
    | Tloose { tr_lower = ls; tr_upper = us } ->
      let l1 = List.map ls ~f:(fun ty -> "as " ^ ty_to_string ty) in
      let l2 = List.map us ~f:(fun ty -> "super " ^ ty_to_string ty) in
      String.concat ~sep:" " (l1 @ l2)
  in
  let members_list =
    SMap.fold
      (fun name tr acc -> ("type " ^ name ^ " " ^ tref_to_string tr) :: acc)
      trs
      []
  in
  "{" ^ String.concat ~sep:"; " members_list ^ "}"
