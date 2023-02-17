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
  let print_constraints id constraints =
    let open DecoratedConstraint in
    Format.printf "Constraints for %s:\n" id;
    Set.elements constraints
    |> List.sort ~compare:(fun c1 c2 -> Pos.compare c1.hack_pos c2.hack_pos)
    |> List.iter ~f:(fun constr ->
           Format.printf "%s\n" (PP.decorated_constraint ~verbosity constr));
    Format.printf "\n%!"
  in
  match command with
  | Options.DumpConstraints ->
    Walker.program () ctx tast |> SMap.iter print_constraints
