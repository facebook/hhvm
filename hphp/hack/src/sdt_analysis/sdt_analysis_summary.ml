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

let calc reader : Summary.t =
  let to_nadable_single = function
    | H.Id.Function _ as id -> Summary.{ id; kind = Function }
    | H.Id.ClassLike _ as id ->
      H.Read.get_inters reader id
      |> Sequence.find_map ~f:(function
             | H.CustomInterConstraint
                 CustomInterConstraint.{ classish_kind_opt; _ } ->
               let classish_kind =
                 classish_kind_opt
                 |> Option.value_exn
                      ~message:"every class is expected to have a classish_kind"
               in
               Some Summary.{ id; kind = ClassLike classish_kind }
             | _ -> None)
      |> Option.value_exn
           ~message:
             "every class id is expected to have a corresponding CustomInterConstraint"
  in

  let to_nadable = Sequence.map ~f:to_nadable_single in
  let filter_to_syntactically_nadable =
    Sequence.filter
      ~f:
        Summary.(
          function
          | { kind = Function; _ } -> true
          | { kind = ClassLike classish_kind; _ } ->
            is_syntactically_nadable classish_kind)
  in

  let filter_to_no_needs_sdt =
    Sequence.filter ~f:(fun Summary.{ id; _ } ->
        H.Read.get_intras reader id
        |> Sequence.for_all ~f:(function Constraint.NeedsSDT -> false))
  in
  let to_nadable_groups =
    Sequence.bind ~f:(fun nadable ->
        H.Read.get_inters reader nadable.Summary.id
        |> Sequence.filter_map ~f:to_hierarchy_for_final_item
        |> Sequence.map
             ~f:
               (List.map ~f:(fun sid -> to_nadable_single (H.Id.ClassLike sid)))
        |> Sequence.map ~f:(fun nadables -> nadable :: nadables))
  in
  let mk_syntactically_nadable () =
    H.Read.get_keys reader |> to_nadable |> filter_to_syntactically_nadable
  in
  let mk_nadable () = mk_syntactically_nadable () |> filter_to_no_needs_sdt in
  let nadable_groups = mk_nadable () |> to_nadable_groups in

  Summary.
    {
      id_cnt = H.Read.get_keys reader |> Sequence.length;
      syntactically_nadable_cnt = mk_syntactically_nadable () |> Sequence.length;
      nadable_cnt = mk_nadable () |> Sequence.length;
      nadable_groups;
    }
