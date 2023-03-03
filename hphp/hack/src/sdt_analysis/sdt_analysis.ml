(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Sdt_analysis_types
open Hh_prelude
module IntraWalker = Sdt_analysis_intra_walker
module InterWalker = Sdt_analysis_inter_walker
module TastHandler = Sdt_analysis_tast_handler
module PP = Sdt_analysis_pretty_printer

let exit_if_incorrect_tcopt ctx : unit =
  let tcopt = Provider_context.get_tcopt ctx in
  let has_correct_options =
    TypecheckerOptions.(
      enable_sound_dynamic tcopt
      && tast_under_dynamic tcopt
      && pessimise_builtins tcopt
      && everything_sdt tcopt)
  in
  if not has_correct_options then (
    Out_channel.output_string
      stderr
      {|sdt analysis can only be used with certain options.
    If running with hh_single_type_check, pass flags `--implicit-pess --tast_under_dynamic.
    If running with hh, pass these flags or set the corresponding .hhconfig options:
        --config enable_sound_dynamic_type=true \
        --config tast_under_dynamic=true \
        --config pessimise_builtins=true \
        --config everything_sdt=true \
|};
    exit 2
  )

let create_handler ctx : Tast_visitor.handler =
  exit_if_incorrect_tcopt ctx;
  let writer = H.Write.create () in
  TastHandler.create_handler ctx writer

let parse_command ~command ~verbosity ~on_bad_command =
  let command =
    match Sdt_analysis_options.parse_command command with
    | Some command -> command
    | None ->
      on_bad_command "invalid SDT analysis mode";
      failwith "unreachable"
  in
  Sdt_analysis_options.mk ~verbosity ~command

let sid_of_id = function
  | H.Class sid -> sid
  | H.Function sid -> sid

let do_tast
    (options : Options.t) (ctx : Provider_context.t) (tast : Tast.program) =
  let Options.{ command; verbosity } = options in
  exit_if_incorrect_tcopt ctx;
  let print_decorated_intra_constraints id decorated_constraints =
    let sid = sid_of_id id in
    Format.printf "Intraprocedural Constraints for %s:\n" sid;
    decorated_constraints
    |> List.sort ~compare:(fun c1 c2 -> Pos.compare c1.hack_pos c2.hack_pos)
    |> List.iter ~f:(fun constr ->
           Format.printf
             "%s\n"
             (PP.decorated ~show:Constraint.show ~verbosity constr));
    Format.printf "\n%!"
  in
  let print_intra_constraints id (intra_constraints : Constraint.t HashSet.t) =
    let sid = sid_of_id id in
    if not @@ HashSet.is_empty intra_constraints then (
      Format.printf "Intraprocedural Constraints for %s:\n" sid;
      intra_constraints
      |> HashSet.to_list
      |> List.sort ~compare:Constraint.compare
      |> List.iter ~f:(fun constr ->
             Format.printf "%s\n" (Constraint.show constr));
      Format.printf "\n%!"
    )
  in
  let print_decorated_inter_constraints
      id (decorated_inter_constraints : H.inter_constraint_ decorated list) =
    let sid = sid_of_id id in
    if not @@ List.is_empty decorated_inter_constraints then begin
      Format.printf "Interprocedural Constraints for %s:\n" sid;
      decorated_inter_constraints
      |> List.sort ~compare:(compare_decorated H.compare_inter_constraint_)
      |> List.iter ~f:(fun constr ->
             Format.printf
               "  %s\n"
               (PP.decorated ~show:H.show_inter_constraint_ ~verbosity constr));
      Format.printf "\n%!"
    end
  in
  match command with
  | Options.DumpConstraints ->
    IntraWalker.program ctx tast |> IdMap.iter print_decorated_intra_constraints;
    InterWalker.program ctx tast |> IdMap.iter print_decorated_inter_constraints
  | Options.SolveConstraints ->
    let writer = H.Write.create () in
    let tast_handler = TastHandler.create_handler ctx writer in
    (Tast_visitor.iter_with [tast_handler])#go ctx tast;
    let reader = H.Write.solve writer in
    H.Read.get_keys reader
    |> Sequence.iter ~f:(fun id ->
           print_intra_constraints id @@ H.Read.get_intras reader id)

let do_ ~command ~verbosity ~on_bad_command =
  let opts = parse_command ~command ~on_bad_command ~verbosity in
  (fun () -> do_tast opts)
