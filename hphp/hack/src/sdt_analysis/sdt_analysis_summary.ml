(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Sdt_analysis_types

let is_syntactically_nadable = function
  | None -> (* is function *) true
  | Some c_kind ->
    Ast_defs.(
      (match c_kind with
      | Cclass _
      | Cinterface
      | Ctrait ->
        true
      | Cenum
      | Cenum_class _ ->
        false))

let calc reader : summary =
  let filter_to_syntactically_nadable =
    Sequence.filter ~f:(fun id ->
        H.Read.get_inters reader id
        |> Sequence.exists ~f:(function
               | H.CustomInterConstraint
                   CustomInterConstraint.{ classish_kind_opt; _ } ->
                 is_syntactically_nadable classish_kind_opt
               | _ -> false))
  in

  let filter_to_no_needs_sdt =
    Sequence.filter ~f:(fun id ->
        H.Read.get_intras reader id
        |> Sequence.for_all ~f:(function Constraint.NeedsSDT -> false))
  in
  let mk_ids () = H.Read.get_keys reader in
  let mk_syntactically_nadable () =
    H.Read.get_keys reader |> filter_to_syntactically_nadable
  in
  let mk_nadable () = mk_syntactically_nadable () |> filter_to_no_needs_sdt in

  {
    id_cnt = mk_ids () |> Sequence.length;
    syntactically_nadable_cnt = mk_syntactically_nadable () |> Sequence.length;
    nadable_cnt = mk_nadable () |> Sequence.length;
    nadables = mk_nadable ();
  }
