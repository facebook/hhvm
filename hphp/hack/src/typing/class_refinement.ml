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

let has_type_ref pos_id cr = SMap.mem (snd pos_id) cr.cr_types

let get_type_ref (_, id) cr = SMap.find_opt id cr.cr_types

let fold_type_refs cr ~init:acc ~f = SMap.fold f cr.cr_types acc

let add_type_ref id tr cr =
  let combine
      (type a) (r1 : a class_type_refinement) (r2 : a class_type_refinement) =
    match (r1, r2) with
    | ((Texact _ as exact), _)
    | (_, (Texact _ as exact)) ->
      exact
    | ( Tloose { tr_lower = ls1; tr_upper = us1 },
        Tloose { tr_lower = ls2; tr_upper = us2 } ) ->
      Tloose { tr_lower = ls1 @ ls2; tr_upper = us1 @ us2 }
  in
  { cr_types = SMap.add ~combine id tr cr.cr_types }

let map_type_refinement (type a) f (r : a class_type_refinement) =
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
    let f name tr acc = ("type " ^ name ^ " " ^ tref_to_string tr) :: acc in
    SMap.fold f trs []
  in
  "{" ^ String.concat ~sep:"; " members_list ^ "}"
