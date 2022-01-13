(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Shape_analysis_types
open Shape_analysis_pretty_printer
module T = Tast
module Solver = Shape_analysis_solver
module Walker = Shape_analysis_walker

let do_ (options : options) (ctx : Provider_context.t) (tast : T.program) =
  let empty_typing_env = Tast_env.tast_env_as_typing_env (Tast_env.empty ctx) in
  match options.mode with
  | FlagTargets ->
    let log_pos = Format.printf "%a\n" Pos.pp in
    let { expressions_to_modify; hints_to_modify } =
      Walker.collect_analysis_targets ctx tast
    in
    Format.printf "~Expressions~\n";
    List.iter expressions_to_modify ~f:log_pos;
    Format.printf "~Hints~\n";
    List.iter hints_to_modify ~f:log_pos
  | DumpConstraints ->
    let print_function_constraints
        (id : string) (constraints : constraint_ list) : unit =
      Format.printf "Constraints for %s:\n" id;
      constraints
      |> List.map ~f:(show_constraint_ empty_typing_env)
      |> List.sort ~compare:String.compare
      |> List.iter ~f:(Format.printf "%s\n");
      Format.printf "\n"
    in
    Walker.program ctx tast |> SMap.iter print_function_constraints
  | SimplifyConstraints ->
    let print_callable_summary (id : string) (results : shape_result list) :
        unit =
      Format.printf "Summary for %s:\n" id;
      List.iter results ~f:(fun result ->
          Format.printf "%s\n" (show_shape_result empty_typing_env result))
    in
    let process_callable id constraints =
      Solver.simplify empty_typing_env constraints |> print_callable_summary id
    in
    Walker.program ctx tast |> SMap.iter process_callable
  | SolveConstraints -> ()

let callable = Walker.callable

let simplify = Solver.simplify

let show_shape_result = show_shape_result
