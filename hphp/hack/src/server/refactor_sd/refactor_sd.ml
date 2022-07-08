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
module Walker = Refactor_sd_walker

exception Refactor_sd_exn = Refactor_sd_exn

let add_ns name =
  if Char.equal name.[0] '\\' then
    name
  else
    "\\" ^ name

let do_
    (function_name : string)
    (options : options)
    (ctx : Provider_context.t)
    (tast : T.program) =
  let empty_typing_env = Tast_env.tast_env_as_typing_env (Tast_env.empty ctx) in
  let function_name = add_ns function_name in
  match options.mode with
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
    Walker.program function_name ctx tast
    |> SMap.iter print_function_constraints
  | SimplifyConstraints
  | SolveConstraints ->
    ()

let callable = Walker.callable

let show_refactor_sd_result = show_refactor_sd_result

let contains_upcast = function
  | Exists_Upcast -> true
  | _ -> false

let show_refactor_sd_result = show_refactor_sd_result
