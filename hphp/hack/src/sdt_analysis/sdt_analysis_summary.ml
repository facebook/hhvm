(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Sdt_analysis_types

let is_syntactically_nadable =
  Ast_defs.(
    function
    | Cclass _
    | Cinterface
    | Ctrait ->
      true
    | Cenum
    | Cenum_class _ ->
      false)

(** if the item (function, class, etc.) has no descendants, return Some (id :: ids_of_ancestors), else None *)
let to_hierarchy_for_final_item = function
  | H.CustomInterConstraint
      CustomInterConstraint.{ hierarchy_for_final_item; _ } ->
    hierarchy_for_final_item
  | _ -> None

let calc reader : summary =
  let filter_to_syntactically_nadable =
    Sequence.filter ~f:(function
        | H.Id.Function _ -> true
        | H.Id.ClassLike _ as id ->
          H.Read.get_inters reader id
          |> Sequence.exists ~f:(function
                 | H.CustomInterConstraint
                     CustomInterConstraint.{ classish_kind_opt; _ } ->
                   classish_kind_opt
                   |> Option.value_exn
                        ~message:
                          "every class with a CustomInterConstraint is expected to have a classish_kind"
                   |> is_syntactically_nadable
                 | _ -> false))
  in

  let filter_to_no_needs_sdt =
    Sequence.filter ~f:(fun id ->
        H.Read.get_intras reader id
        |> Sequence.for_all ~f:(function Constraint.NeedsSDT -> false))
  in
  let group_nadables =
    Sequence.bind ~f:(fun id ->
        H.Read.get_inters reader id
        |> Sequence.filter_map ~f:to_hierarchy_for_final_item
        |> Sequence.map ~f:(List.map ~f:(fun sid -> H.Id.ClassLike sid))
        |> Sequence.map ~f:(fun sids -> id :: sids))
  in
  let mk_syntactically_nadable () =
    H.Read.get_keys reader |> filter_to_syntactically_nadable
  in
  let mk_nadable () = mk_syntactically_nadable () |> filter_to_no_needs_sdt in
  let nadable_groups = mk_nadable () |> group_nadables in

  {
    id_cnt = H.Read.get_keys reader |> Sequence.length;
    syntactically_nadable_cnt = mk_syntactically_nadable () |> Sequence.length;
    nadable_cnt = mk_nadable () |> Sequence.length;
    nadable_groups;
  }
