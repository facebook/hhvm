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
module Codemod = Shape_analysis_codemod
module JSON = Hh_json

exception Shape_analysis_exn = Shape_analysis_exn

let simplify env constraints =
  Solver.deduce constraints |> Solver.produce_results env

let do_ (options : options) (ctx : Provider_context.t) (tast : T.program) =
  let { mode; verbosity } = options in
  let empty_typing_env = Tast_env.tast_env_as_typing_env (Tast_env.empty ctx) in
  let strip_decorations { constraint_; _ } = constraint_ in
  match mode with
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
        (id : string) (constraints : decorated_constraints) : unit =
      Format.printf "Constraints for %s:\n" id;
      let print_help projection constr_printer constr =
        projection constr
        |> List.sort ~compare:(fun c1 c2 -> Pos.compare c1.hack_pos c2.hack_pos)
        |> List.map ~f:(constr_printer ~verbosity empty_typing_env)
        |> List.iter ~f:(Format.printf "%s\n")
      in
      print_help fst show_decorated_constraint constraints;
      print_help snd show_decorated_inter_constraint constraints;
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
      simplify empty_typing_env constraints |> print_callable_summary id
    in
    Walker.program ctx tast
    |> SMap.map fst
    |> SMap.map (List.map ~f:strip_decorations)
    |> SMap.iter process_callable
  | Codemod ->
    let process_callable constraints =
      simplify empty_typing_env constraints
      |> Codemod.of_results empty_typing_env
    in
    Walker.program ctx tast
    |> SMap.map fst
    |> SMap.map (List.map ~f:strip_decorations)
    |> SMap.map process_callable
    |> SMap.values
    |> JSON.array_ (fun json -> json)
    |> Format.printf "%a" JSON.pp_json
  | SolveConstraints -> ()

let callable = Walker.callable

let show_shape_result = show_shape_result

let is_shape_like_dict = function
  | Shape_like_dict _ -> true
  | _ -> false
