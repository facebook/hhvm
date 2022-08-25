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
module Inter_shape = Hips_solver.Inter (Shape_analysis_hips.Intra_shape)

exception Shape_analysis_exn = Shape_analysis_exn

let simplify env constraints =
  Solver.deduce constraints |> Solver.produce_results env

let strip_decoration_of_lists
    ((intra_dec_list, inter_dec_list) : decorated_constraints) :
    any_constraint list =
  List.map ~f:(fun { constraint_; _ } -> HT.Intra constraint_) intra_dec_list
  @ List.map ~f:(fun { constraint_; _ } -> HT.Inter constraint_) inter_dec_list

let process_errors_out map =
  let process_errors _id (_, errors) =
    if not (List.is_empty errors) then Printf.eprintf "\nErrors:\n";
    let print_error err = Printf.eprintf "%s\n" (Error.show err) in
    List.iter ~f:print_error errors
  in
  SMap.iter process_errors map;
  SMap.map (fun (constraints, _errors) -> constraints) map

let do_ (options : options) (ctx : Provider_context.t) (tast : T.program) =
  let { mode; verbosity } = options in
  let empty_typing_env = Tast_env.tast_env_as_typing_env (Tast_env.empty ctx) in
  let strip_decorations { constraint_; _ } = constraint_ in
  let analyse (dec_map : decorated_constraints SMap.t) :
      any_constraint list SMap.t =
    SMap.map strip_decoration_of_lists dec_map |> Inter_shape.analyse
    |> function
    | Inter_shape.Convergent constr_map -> constr_map
    | Inter_shape.Divergent constr_map -> constr_map
  in
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
    Walker.program ctx tast
    |> process_errors_out
    |> SMap.iter print_function_constraints
  | CloseConstraints ->
    let print_function_constraints
        (id : string) (any_constraints_list : any_constraint list) : unit =
      Format.printf "Constraints after closing for %s:\n" id;
      List.map
        ~f:(function
          | HT.Intra intra_constr ->
            show_constraint empty_typing_env intra_constr
          | HT.Inter inter_constr ->
            show_inter_constraint empty_typing_env inter_constr)
        any_constraints_list
      |> List.iter ~f:(Format.printf "%s\n");
      Format.printf "\n"
    in
    Walker.program ctx tast
    |> process_errors_out
    |> analyse
    |> SMap.iter print_function_constraints
  | DumpDerivedConstraints ->
    let print_function_constraints
        (id : string) ((intra_constraints, _) : decorated_constraints) : unit =
      Format.printf "Derived constraints for %s:\n" id;
      intra_constraints
      |> List.map ~f:(fun c -> c.constraint_)
      |> Solver.deduce
      |> List.map ~f:(show_constraint empty_typing_env)
      |> List.iter ~f:(Format.printf "%s\n");
      Format.printf "\n"
    in
    Walker.program ctx tast
    |> process_errors_out
    |> SMap.iter print_function_constraints
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
    |> process_errors_out
    |> SMap.map fst
    |> SMap.map (List.map ~f:strip_decorations)
    |> SMap.iter process_callable
  | Codemod ->
    let process_callable constraints =
      simplify empty_typing_env constraints
      |> Codemod.of_results empty_typing_env
    in
    Walker.program ctx tast
    |> process_errors_out
    |> SMap.map fst
    |> SMap.map (List.map ~f:strip_decorations)
    |> SMap.map process_callable
    |> SMap.values
    |> JSON.array_ (fun json -> json)
    |> Format.printf "%a" JSON.pp_json
  | SolveConstraints ->
    let print_callable_summary (id : string) (results : shape_result list) :
        unit =
      Format.printf "Summary after closing and simplifying for %s:\n" id;
      List.iter results ~f:(fun result ->
          Format.printf "%s\n" (show_shape_result empty_typing_env result))
    in
    let process_callable id constraints =
      simplify empty_typing_env constraints |> print_callable_summary id
    in
    Walker.program ctx tast
    |> process_errors_out
    |> analyse
    |> SMap.map
         (List.filter_map ~f:(function
             | HT.Intra intra_constr -> Some intra_constr
             | HT.Inter _ -> None))
    |> SMap.iter process_callable

let callable = Walker.callable

let show_shape_result = show_shape_result

let is_shape_like_dict = function
  | Shape_like_dict _ -> true
  | _ -> false
