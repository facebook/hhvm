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

exception Refactor_sd_exn = Refactor_sd_exn

let do_
    (_function_name : string)
    (options : options)
    (ctx : Provider_context.t)
    (_tast : T.program) =
  let empty_typing_env = Tast_env.tast_env_as_typing_env (Tast_env.empty ctx) in
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
    print_function_constraints "test printing" [Subset (Variable 1, Variable 2)]
  | SimplifyConstraints
  | SolveConstraints ->
    ()

let show_refactor_sd_result = show_refactor_sd_result

let contains_upcast = function
  | Exists_Upcast -> true
  | _ -> false

let show_refactor_sd_result = show_refactor_sd_result
