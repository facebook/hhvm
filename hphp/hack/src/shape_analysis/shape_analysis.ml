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
module SAF = Shape_analysis_files
module JSON = Hh_json
module Inter_shape = Hips_solver.Inter (Shape_analysis_hips.Intra_shape)

exception Shape_analysis_exn = Shape_analysis_exn

let simplify env constraints =
  Solver.deduce constraints |> Solver.produce_results env

let strip_decoration_of_lists
    ((intra_dec_list, inter_dec_list) : decorated_constraints) :
    any_constraint list =
  (DecoratedConstraintSet.elements intra_dec_list
  |> List.map ~f:(fun { constraint_; _ } -> HT.Intra constraint_))
  @ (DecoratedInterConstraintSet.elements inter_dec_list
    |> List.map ~f:(fun { constraint_; _ } -> HT.Inter constraint_))

let process_errors_out (constraints, errors) =
  if not (List.is_empty errors) then Printf.eprintf "\nErrors:\n";
  let print_error err = Printf.eprintf "%s\n" (Error.show err) in
  List.iter ~f:print_error errors;
  constraints

let intra_constraints_of_any_constraints =
  List.filter_map ~f:(function
      | HT.Intra intra_constr -> Some intra_constr
      | HT.Inter _ -> None)

let strip_decorations { constraint_; _ } = constraint_

let any_constraints_of_decorated_constraints decorated_constraints =
  let intra_constraints =
    fst decorated_constraints
    |> DecoratedConstraintSet.elements
    |> List.map ~f:strip_decorations
    |> List.map ~f:(fun c -> HT.Intra c)
  in
  let inter_constraints =
    snd decorated_constraints
    |> DecoratedInterConstraintSet.elements
    |> List.map ~f:strip_decorations
    |> List.map ~f:(fun c -> HT.Inter c)
  in
  List.append intra_constraints inter_constraints

let analyse (constraints : any_constraint list SMap.t) ~verbose :
    any_constraint list SMap.t =
  constraints |> Inter_shape.analyse ~verbose |> function
  | Inter_shape.Convergent constr_map -> constr_map
  | Inter_shape.Divergent constr_map -> constr_map

let shape_results_using_hips_internal ~verbose tenv entries =
  entries
  |> List.map ~f:(fun ConstraintEntry.{ id; constraints; _ } ->
         (id, constraints))
  |> SMap.of_list
  |> analyse ~verbose
  |> SMap.map intra_constraints_of_any_constraints
  |> SMap.map (simplify tenv)

let shape_results_no_hips tenv entries =
  let simplify ConstraintEntry.{ id; constraints; _ } =
    let shape_results =
      constraints |> intra_constraints_of_any_constraints |> simplify tenv
    in
    (id, shape_results)
  in
  entries |> List.map ~f:simplify |> SMap.of_list

let read_constraints ~constraints_dir : ConstraintEntry.t list =
  let read_one constraints_file =
    SAF.read_entries_by_source_file ~constraints_file |> Sequence.hd_exn
  in
  Sys.readdir constraints_dir
  |> Array.to_list
  |> List.filter ~f:(String.is_suffix ~suffix:SAF.constraints_file_extension)
  |> List.hd_exn
  |> Filename.concat constraints_dir
  |> read_one

let do_ (options : options) (ctx : Provider_context.t) (tast : T.program) =
  let { command; mode; verbosity } = options in
  let verbose = verbosity > 0 in
  let empty_typing_env = Tast_env.tast_env_as_typing_env (Tast_env.empty ctx) in

  let source_file = Relative_path.create Relative_path.Dummy "dummy.php" in

  let dump_marshalled_constraints ~constraints_dir =
    Sys_utils.mkdir_p ~skip_mocking:true constraints_dir;
    Walker.program mode ctx tast
    |> SMap.iter (fun id (decorated_constraints, errors) ->
           let constraints = strip_decoration_of_lists decorated_constraints in
           let error_count = List.length errors in
           let constraint_entry =
             ConstraintEntry.{ source_file; id; constraints; error_count }
           in
           SAF.write_constraints ~constraints_dir ~worker:0 constraint_entry;
           SAF.flush ())
  in
  let solve_marshalled_constraints ~constraints_dir =
    let print_callable_summary (id : string) (results : shape_result list) :
        unit =
      Format.printf "Summary after closing and simplifying for %s:\n" id;
      List.iter results ~f:(fun result ->
          Format.printf "%s\n" (show_shape_result empty_typing_env result))
    in
    read_constraints ~constraints_dir
    |> shape_results_using_hips_internal empty_typing_env ~verbose
    |> SMap.iter print_callable_summary
  in
  let fresh_constraints_dir () =
    (* enables test parallelization *)
    let pid = Unix.getpid () in
    let secs = int_of_float @@ Unix.time () in
    Format.sprintf "/tmp/shape_analysis_constraints-%d-%d" secs pid
  in
  let get_constraint_entries () =
    (* we persist and unpersist entries to exercise persistence code in tests and share logic *)
    let constraints_dir = fresh_constraints_dir () in
    Sys_utils.rm_dir_tree ~skip_mocking:true constraints_dir;
    dump_marshalled_constraints ~constraints_dir;
    read_constraints ~constraints_dir
  in
  match command with
  | DumpConstraints ->
    let print_function_constraints
        (id : string)
        ((intra_constraints, inter_constraints) : decorated_constraints) : unit
        =
      Format.printf "Constraints for %s:\n" id;
      DecoratedConstraintSet.elements intra_constraints
      |> List.sort ~compare:(fun c1 c2 -> Pos.compare c1.hack_pos c2.hack_pos)
      |> List.iter ~f:(fun constr ->
             Format.printf
               "%s\n"
               (show_decorated_constraint ~verbosity empty_typing_env constr));
      DecoratedInterConstraintSet.elements inter_constraints
      |> List.sort ~compare:(fun c1 c2 -> Pos.compare c1.hack_pos c2.hack_pos)
      |> List.iter ~f:(fun constr ->
             Format.printf
               "%s\n"
               (show_decorated_inter_constraint
                  ~verbosity
                  empty_typing_env
                  constr));
      Format.printf "\n"
    in
    Walker.program mode ctx tast
    |> SMap.map process_errors_out
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
    get_constraint_entries ()
    |> List.map ~f:(fun ConstraintEntry.{ id; constraints; _ } ->
           (id, constraints))
    |> SMap.of_list
    |> analyse ~verbose
    |> SMap.iter print_function_constraints
  | DumpDerivedConstraints ->
    let print_function_constraints
        (id : string) (intra_constraints : constraint_ list) : unit =
      Format.printf "Derived constraints for %s:\n" id;
      intra_constraints
      |> Solver.deduce
      |> List.map ~f:(show_constraint empty_typing_env)
      |> List.iter ~f:(Format.printf "%s\n");
      Format.printf "\n"
    in
    let process_entry ConstraintEntry.{ id; constraints; _ } =
      intra_constraints_of_any_constraints constraints
      |> print_function_constraints id
    in
    get_constraint_entries () |> List.iter ~f:process_entry
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
    Walker.program mode ctx tast
    |> SMap.map (fun rs ->
           process_errors_out rs
           |> fst
           |> DecoratedConstraintSet.elements
           |> List.map ~f:strip_decorations)
    |> SMap.iter process_callable
  | Codemod ->
    get_constraint_entries ()
    |> Codemod.codemods_of_entries
         empty_typing_env
         ~solve:shape_results_no_hips
         ~atomic:true
    |> JSON.array_ Fn.id
    |> Format.printf "%a" JSON.pp_json
  | DumpMarshalledConstraints { constraints_dir } ->
    dump_marshalled_constraints ~constraints_dir
  | SolveConstraints ->
    let constraints_dir = fresh_constraints_dir () in
    Sys_utils.rm_dir_tree ~skip_mocking:true constraints_dir;
    dump_marshalled_constraints ~constraints_dir;
    solve_marshalled_constraints ~constraints_dir

let callable = Walker.callable

let show_shape_result = show_shape_result

let is_shape_like_dict = function
  | Shape_like_dict _ -> true
  | _ -> false

let shape_results_using_hips : solve_entries =
  shape_results_using_hips_internal ~verbose:false
