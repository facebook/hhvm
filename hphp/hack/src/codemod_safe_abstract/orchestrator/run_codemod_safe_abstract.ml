(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let usage =
  {|
  --root $DIR
where $DIR has a .hhconfig file
Codemod runner for iterative safe abstract analysis.
A "what-if" analysis that modifies all-WWW locally.
It adds the attribute and runs `hh` in a loop until there are no more errors.
Note that it adds a few temporary commits called 'Iterate' as it goes, so you
can see what changed in each iteration and how many iterations there were.
|}

type args = {
  codemod_safe_abstract: string;
  hh_distc: string;
  worker: string;
  root: string;
}

let parse_args_exn () : args =
  let codemod_safe_abstract = ref None in
  let hh_distc = ref None in
  let worker = ref None in
  let root = ref None in

  let () =
    Arg.parse
      [
        ( "--codemod-safe-abstract",
          Arg.String (fun s -> codemod_safe_abstract := Some s),
          "Path to codemod_safe_abstract binary" );
        ( "--hh_distc",
          Arg.String (fun s -> hh_distc := Some s),
          "Path to hh_distc binary" );
        ( "--worker",
          Arg.String (fun s -> worker := Some s),
          "Path to worker binary" );
        ("--root", Arg.String (fun s -> root := Some s), "Root directory path");
      ]
      (fun _ -> ())
      usage
  in
  let get_value opt =
    match !opt with
    | Some s -> s
    | None ->
      Printf.eprintf "%s\n" usage;
      exit 1
  in
  let codemod_safe_abstract = get_value codemod_safe_abstract in
  let hh_distc = get_value hh_distc in
  let worker = get_value worker in
  let root = get_value root in
  { codemod_safe_abstract; hh_distc; worker; root }

let fail (message : string) (code : int) : 'a =
  Printf.eprintf "%s\n" message;
  exit code

let run_command_in_dir ~(dir : string) ~(cmd : string) : int =
  let orig_dir = Sys.getcwd () in
  Sys.chdir dir;
  let result = Sys.command cmd in
  Sys.chdir orig_dir;
  result

let run_command_capture_output ~(dir : string) ~(cmd : string) :
    string * Unix.process_status =
  let orig_dir = Sys.getcwd () in
  Sys.chdir dir;
  let ic = Unix.open_process_in cmd in
  let rec read_all (acc : string list) : string list =
    match In_channel.input_line ic with
    | Some line -> read_all (line :: acc)
    | None -> List.rev acc
  in
  let lines = read_all [] in
  let output = String.concat ~sep:"\n" lines in
  let status = Unix.close_process_in ic in
  Sys.chdir orig_dir;
  (output, status)

let check_working_directory_clean (root : string) : bool =
  let (output, _) = run_command_capture_output ~dir:root ~cmd:"sl st" in
  String.is_empty (String.strip output)

let get_commit_id (root : string) : string =
  let (output, _) = run_command_capture_output ~dir:root ~cmd:"sl id" in
  String.strip output

let run_hh_check (args : args) (root : string) (round : int) : unit =
  let commit = get_commit_id root in
  let errors_file = Printf.sprintf "/tmp/codemod-sa-%s-%d-out" commit round in
  let hh_cmd =
    Printf.sprintf
      "%s --cmd worker=%s check --root %s --json --show-warnings --config needs_concrete=true > %s"
      args.hh_distc
      args.worker
      root
      errors_file
  in
  let hh_result = run_command_in_dir ~dir:root ~cmd:hh_cmd in

  (* hh_distc uses return code of 2 if there are errors, which is expected *)
  if hh_result <> 0 && hh_result <> 2 then fail "hh command failed" 4;

  let codemod_cmd =
    Printf.sprintf
      "%s --errors %s --root %s"
      args.codemod_safe_abstract
      errors_file
      root
  in
  let codemod_result = Sys.command codemod_cmd in
  if codemod_result <> 0 then fail "codemod failed" 3

let has_changes (root : string) : bool =
  let (output, _) = run_command_capture_output ~dir:root ~cmd:"sl st" in
  not (String.is_empty (String.strip output))

let commit_changes (root : string) (round : int) : unit =
  let commit_msg = Printf.sprintf "safe_abstract_codemod round %d" round in
  let commit_cmd = Printf.sprintf "sl commit -m \"%s\"" commit_msg in
  let result = run_command_in_dir ~dir:root ~cmd:commit_cmd in
  if result <> 0 then fail "commit failed" 5

let fold_commits (root : string) (final_round : int) : unit =
  if final_round > 1 then (
    Printf.printf "Folding %d commits into a single commit...\n" final_round;
    (* Fold the last N commits that were created by the codemod *)
    let fold_cmd =
      Printf.sprintf
        "sl fold --from -r .~%d -m \"Apply safe_abstract_codemod - %d rounds\""
        (final_round - 1)
        final_round
    in
    let result = run_command_in_dir ~dir:root ~cmd:fold_cmd in
    if result <> 0 then fail "fold commits failed" 6
  ) else if final_round = 1 then
    Printf.printf "Only 1 commit made, no folding needed.\n"

let main () : unit =
  let args = parse_args_exn () in

  if not (check_working_directory_clean args.root) then
    fail "working directory must be clean" 2;

  let round = ref 0 in

  let commit = get_commit_id args.root in
  Printf.printf "round %d (commit %s)\n%!" !round commit;
  run_hh_check args args.root !round;

  while has_changes args.root do
    commit_changes args.root !round;
    incr round;
    let commit = get_commit_id args.root in
    Printf.printf "round %d (commit %s)\n%!" !round commit;
    run_hh_check args args.root !round
  done;

  (*
  We use version control to help us see when there are no more changes.
  But we only want a single commit at the end, so we fold.
  *)
  fold_commits args.root !round;

  Printf.printf "done (%d rounds)\n" !round

let () = main ()
