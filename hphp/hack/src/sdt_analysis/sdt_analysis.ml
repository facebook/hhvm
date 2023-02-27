(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Sdt_analysis_types
open Hh_prelude
module Walker = Sdt_analysis_walker
module PP = Sdt_analysis_pretty_printer

let do_ (options : Options.t) (ctx : Provider_context.t) (tast : Tast.program) =
  let Options.{ command; verbosity } = options in
  let print_intra_constraints (_id_kind, sid) intra_constraints =
    if not @@ HashSet.is_empty intra_constraints then (
      let open DecoratedConstraint in
      Format.printf "Intraprocedural Constraints for %s:\n" sid;
      intra_constraints
      |> HashSet.to_list
      |> List.sort ~compare:(fun c1 c2 -> Pos.compare c1.hack_pos c2.hack_pos)
      |> List.iter ~f:(fun constr ->
             Format.printf "%s\n" (PP.decorated_constraint ~verbosity constr));
      Format.printf "\n%!"
    )
  in
  let print_inter_constraints (_id_kind, sid) inter_constraints =
    if not @@ HashSet.is_empty inter_constraints then
      Format.printf "Interprocedural Constraints for %s:\n" sid;
    inter_constraints
    |> HashSet.to_list
    |> List.iter ~f:(fun constr ->
           Format.printf "  %s\n" (H.show_inter_constraint_ constr));
    Format.printf "\n%!"
  in
  let writer = H.Write.create () in
  let tast_handler = Walker.tast_handler writer in
  (Tast_visitor.iter_with [tast_handler])#go ctx tast;
  match command with
  | Options.DumpConstraints ->
    let reader = H.Write.debug_dump writer in
    H.Read.get_keys reader
    |> Sequence.to_list
    |> List.sort ~compare:H.compare_id
    |> List.iter ~f:(fun id ->
           print_intra_constraints id @@ H.Read.get_intras reader id;
           print_inter_constraints id @@ H.Read.get_inters reader id)
  | Options.SolveConstraints ->
    let reader = H.Write.solve writer in
    H.Read.get_keys reader
    |> Sequence.iter ~f:(fun id ->
           print_intra_constraints id @@ H.Read.get_intras reader id)
