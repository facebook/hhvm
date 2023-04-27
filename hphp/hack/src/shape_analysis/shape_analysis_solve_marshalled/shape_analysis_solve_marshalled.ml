(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
(**
  * This module as much as possible is just IO and
    doesn't know anything about the shapes of data.

    These restrictions are intended to facilitate adapting for future analyses
*)
open Hh_prelude

module SA = Shape_analysis
module SAF = Shape_analysis_files
module SAC = Shape_analysis_codemod

let usage_message =
  {|
  Utility to read shape analysis constraints from a directory and output codemod information
  in JSON Lines format in the same directory as the constraints.

  USAGE:
   $THIS_EXECUTABLE\
    --constraints-dir [directory containing .dmpc files with shape analysis constraints] For example: '/tmp/shape_analysis_constraints'] \
    --source-root [directory that relative paths in constraints are relative to]
    --grain [The granularity of solving. One of 'callable' (does not use HIPS) or 'source-file' (uses HIPS)]
    --atomic (Flag. If provided, then group codemod directives. Described in D40008464.)

   To observe progress, append the following, where $CONSTRAINTS_DIR matches --constraints-dir:
    & while true;
    do echo entries written: $(cat $CONSTRAINTS_DIR/*.jsonl | wc -l) &&
    sleep 10
    done
  |}

type grain =
  | Callable
  | SourceFile

type args = {
  constraints_dir: string;
  source_root: string;
  grain: grain;
  atomic: bool;
}

let parse_args () : args =
  let constraints_dir = ref "" in
  let source_root = ref "" in
  let grain_str = ref "" in
  let should_show_help = ref false in
  let atomic = ref false in
  let args_spec =
    [
      ( "--constraints-dir",
        Arg.Set_string constraints_dir,
        "directory containing constraints" );
      ( "--source-root",
        Arg.Set_string source_root,
        "Constraints contain positions, which contain paths that are relative to the source root."
      );
      ( "--grain",
        Arg.Set_string grain_str,
        "The granularity of constraint solving. One of 'callable' or 'source-file'"
      );
      ( "--atomic",
        Arg.Set atomic,
        "If provided, then group codemod directives. Described in D40008464." );
      ("--help", Arg.Set should_show_help, "show help help");
    ]
  in
  let exit_bad_args () =
    print_endline usage_message;
    exit 2
  in
  let parse_grain = function
    | "callable" -> Callable
    | "source-file" -> SourceFile
    | _ -> exit_bad_args ()
  in

  Arg.parse args_spec ignore usage_message;
  if !should_show_help then exit_bad_args ();
  let unref_string s =
    match !s with
    | "" ->
      print_endline usage_message;
      exit 2
    | s -> s
  in
  {
    constraints_dir = unref_string constraints_dir;
    source_root = unref_string source_root;
    grain = parse_grain @@ unref_string grain_str;
    atomic = !atomic;
  }

let print_codemods formatter =
  List.iter ~f:(fun json ->
      Format.fprintf formatter "%s\n" @@ Hh_json.json_to_string json)

let par_iter (items : 'i list) ~(f : 'i -> unit) : unit =
  let orig_pid = Unix.getpid () in
  let rec loop pids = function
    | [] -> pids
    | h :: t ->
      (match Unix.fork () with
      | 0 ->
        f h;
        pids
      | pid -> loop (pid :: pids) t)
  in
  let pids = loop [] items in
  if Unix.getpid () = orig_pid then
    List.iter pids ~f:(fun pid ->
        match snd @@ Unix.waitpid [] pid with
        | Unix.WEXITED 0 -> ()
        | Unix.WEXITED c ->
          failwith @@ Format.sprintf "subprocess %d exited with code %d\n" pid c
        | Unix.WSIGNALED s ->
          failwith
          @@ Format.sprintf "subprocess %d killed with signal %d\n" pid s
        | Unix.WSTOPPED s ->
          failwith
          @@ Format.sprintf "subprocess %d stopped with signal %d\n" pid s)
  else
    exit 0

let mutate_global_state source_root =
  Relative_path.set_path_prefix Relative_path.Root @@ Path.make source_root;
  let num_workers = Sys_utils.nbr_procs in
  let (_ : SharedMem.handle) =
    SharedMem.init SharedMem.default_config ~num_workers
  in
  ()

(**
It's OK to use an empty typing env because
`Shape_analysis_codemod` only uses the env
for printing, using `Typing_print.full env shape_ty`,
which in turn only uses env.genv.tcopt.tco_type_printer_fuel
*)
let env =
  Tast_env.tast_env_as_typing_env
    (Tast_env.empty
    @@ Provider_context.empty_for_debugging
         ~popt:ParserOptions.default
         ~tcopt:TypecheckerOptions.default
         ~deps_mode:(Typing_deps_mode.InMemoryMode None))

let () =
  let { constraints_dir; source_root; grain; atomic } = parse_args () in
  mutate_global_state source_root;

  let (read_entries, solve) =
    match grain with
    | SourceFile ->
      (SAF.read_entries_by_source_file, SA.shape_results_using_hips)
    | Callable -> (SAF.read_entries_by_callable, SA.shape_results_no_hips)
  in

  let solve_and_write constraints_file =
    let out_channel =
      constraints_file
      |> Filename.chop_extension
      |> Format.sprintf "%s.jsonl"
      |> Out_channel.create
    in
    let formatter = Format.formatter_of_out_channel out_channel in
    read_entries ~constraints_file
    |> Sequence.map ~f:(SAC.codemods_of_entries env ~solve ~atomic)
    |> Sequence.iter ~f:(print_codemods formatter);
    Out_channel.close out_channel
  in
  Sys.readdir constraints_dir
  |> Array.to_list
  |> List.filter ~f:(String.is_suffix ~suffix:SAF.constraints_file_extension)
  |> List.map ~f:(Filename.concat constraints_dir)
  |> par_iter ~f:solve_and_write;
  Format.printf "wrote .jsonl files to %s\n" constraints_dir
