(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude
open Refactor_sd_types
open Refactor_sd_pretty_printer
module T = Tast
module Solver = Refactor_sd_solver
module Walker = Refactor_sd_walker

exception Refactor_sd_exn = Refactor_sd_exn

let add_ns name =
  if Char.equal name.[0] '\\' then
    name
  else
    "\\" ^ name

let do_
    (upcasted_id : string)
    (options : options)
    (ctx : Provider_context.t)
    (tast : T.program) =
  let empty_typing_env = Tast_env.tast_env_as_typing_env (Tast_env.empty ctx) in
  let upcasted_id = add_ns upcasted_id in
  let upcasted_info = { element_name = upcasted_id } in
  match options.analysis_mode with
  | FlagTargets -> ()
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
    Walker.program upcasted_info ctx tast
    |> SMap.iter print_function_constraints
  | SimplifyConstraints ->
    let print_callable_summary (id : string) (results : refactor_sd_result list)
        : unit =
      Format.printf "Summary for %s:\n" id;
      List.iter results ~f:(fun result ->
          Format.printf "%s\n" (show_refactor_sd_result empty_typing_env result))
    in
    let process_callable id constraints =
      Solver.simplify empty_typing_env constraints |> print_callable_summary id
    in
    Walker.program upcasted_info ctx tast |> SMap.iter process_callable
  | SolveConstraints -> ()

let callable = Walker.callable

let simplify = Solver.simplify

let show_refactor_sd_result = show_refactor_sd_result

let contains_upcast = function
  | Exists_Upcast _ -> true
  | _ -> false

let show_refactor_sd_result = show_refactor_sd_result
