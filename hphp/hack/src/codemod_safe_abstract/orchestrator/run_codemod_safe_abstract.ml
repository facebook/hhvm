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
It checkpoints every successful rewrite round before starting the next check,
then folds those commits after reaching a fixed point.
|}

type args = {
  codemod_safe_abstract: string;
  hh_distc: string;
  worker: string;
  root: string;
  artifacts_dir: string option;
  keep_all_errors: bool;
}

let parse_args_exn () : args =
  let codemod_safe_abstract = ref None in
  let hh_distc = ref None in
  let worker = ref None in
  let root = ref None in
  let artifacts_dir = ref None in
  let keep_all_errors = ref false in

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
        ( "--artifacts-dir",
          Arg.String (fun s -> artifacts_dir := Some s),
          "Directory for raw hh_distc JSON; defaults to a unique /tmp directory"
        );
        ( "--keep-all-errors",
          Arg.Set keep_all_errors,
          "Retain intermediate raw JSON, not only the initial and final rounds"
        );
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
  {
    codemod_safe_abstract;
    hh_distc;
    worker;
    root;
    artifacts_dir = !artifacts_dir;
    keep_all_errors = !keep_all_errors;
  }

let fail (message : string) (code : int) : 'a =
  Printf.eprintf "%s\n" message;
  exit code

let run_command_in_dir ~(dir : string) ~(cmd : string) : int =
  let orig_dir = Sys.getcwd () in
  Exn.protect
    ~f:(fun () ->
      Sys.chdir dir;
      Sys.command cmd)
    ~finally:(fun () -> Sys.chdir orig_dir)

let run_command_capture_output ~(dir : string) ~(cmd : string) :
    string * Unix.process_status =
  let orig_dir = Sys.getcwd () in
  Exn.protect
    ~f:(fun () ->
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
      (output, status))
    ~finally:(fun () -> Sys.chdir orig_dir)

let command_succeeded (status : Unix.process_status) : bool =
  match status with
  | Unix.WEXITED 0 -> true
  | Unix.WEXITED _
  | Unix.WSIGNALED _
  | Unix.WSTOPPED _ ->
    false

let prepare_artifacts_dir (requested : string option) : string =
  match requested with
  | None ->
    let (marker, channel) =
      Filename_unix.open_temp_file "codemod-safe-abstract-" ".artifacts"
    in
    Out_channel.close channel;
    Sys.remove marker;
    Unix.mkdir marker 0o700;
    marker
  | Some path ->
    begin
      try Unix.mkdir path 0o700 with
      | Unix.Unix_error (Unix.EEXIST, _, _) -> ()
    end;
    begin
      match (Unix.stat path).Unix.st_kind with
      | Unix.S_DIR -> path
      | _ ->
        fail (Printf.sprintf "artifacts path is not a directory: %s" path) 10
    end

let summary_rewrites (line : string) : string option =
  if String.is_prefix line ~prefix:"SAFE_ABSTRACT_SUMMARY\t" then
    String.split line ~on:'\t'
    |> List.find_map ~f:(fun field ->
           String.chop_prefix field ~prefix:"rewrites=")
  else
    None

let run_codemod_and_get_rewrites (cmd : string) : int =
  let input = Unix.open_process_in (cmd ^ " 2>&1") in
  let summaries = ref [] in
  let rec consume_output () =
    match In_channel.input_line input with
    | None -> ()
    | Some line ->
      Printf.printf "%s\n%!" line;
      Option.iter (summary_rewrites line) ~f:(fun value ->
          summaries := value :: !summaries);
      consume_output ()
  in
  consume_output ();
  let status = Unix.close_process_in input in
  if not (command_succeeded status) then fail "codemod failed" 3;
  match !summaries with
  | [rewrites] -> begin
    try Stdlib.int_of_string rewrites with
    | Failure _ -> fail "codemod emitted an invalid rewrite count" 3
  end
  | _ -> fail "codemod must emit exactly one summary" 3

let check_working_directory_clean (root : string) : bool =
  let (output, status) =
    run_command_capture_output
      ~dir:root
      ~cmd:
        "sl status --reason 'run isolated Safe Abstract analysis - sl help status'"
  in
  if not (command_succeeded status) then fail "sl status failed" 8;
  String.is_empty (String.strip output)

let get_commit_id (root : string) : string =
  let (output, status) =
    run_command_capture_output
      ~dir:root
      ~cmd:"sl id --reason 'run isolated Safe Abstract analysis - sl help id'"
  in
  if not (command_succeeded status) then fail "sl id failed" 9;
  String.strip output

let run_hh_check
    (args : args) (artifacts_dir : string) (root : string) (round : int) :
    int * string =
  let commit = get_commit_id root in
  let errors_file =
    Filename.concat
      artifacts_dir
      (Printf.sprintf "codemod-sa-%s-%d-out" commit round)
  in
  let partial_errors_file = errors_file ^ ".partial" in
  if Sys.file_exists errors_file || Sys.file_exists partial_errors_file then
    fail
      (Printf.sprintf "refusing to overwrite analysis artifact: %s" errors_file)
      10;
  let hh_cmd =
    Printf.sprintf
      "%s --cmd worker=%s check --root %s --json --show-warnings --config needs_concrete=true --config needs_concrete_override_check=1 --config warnings_generated_files= > %s"
      (Filename.quote args.hh_distc)
      (Filename.quote args.worker)
      (Filename.quote root)
      (Filename.quote partial_errors_file)
  in
  let hh_result = run_command_in_dir ~dir:root ~cmd:hh_cmd in

  (* hh_distc uses return code of 2 if there are errors, which is expected *)
  if hh_result <> 0 && hh_result <> 2 then fail "hh command failed" 4;
  Unix.rename partial_errors_file errors_file;

  let codemod_cmd =
    Printf.sprintf
      "%s --errors %s --root %s"
      (Filename.quote args.codemod_safe_abstract)
      (Filename.quote errors_file)
      (Filename.quote root)
  in
  (run_codemod_and_get_rewrites codemod_cmd, errors_file)

let has_changes (root : string) : bool =
  let (output, status) =
    run_command_capture_output
      ~dir:root
      ~cmd:
        "sl status --reason 'run isolated Safe Abstract analysis - sl help status'"
  in
  if not (command_succeeded status) then fail "sl status failed" 8;
  not (String.is_empty (String.strip output))

let commit_changes (root : string) (round : int) : unit =
  let commit_msg = Printf.sprintf "safe_abstract_codemod round %d" round in
  let commit_cmd =
    Printf.sprintf
      "sl commit -m %s --reason 'run isolated Safe Abstract analysis - sl help commit'"
      (Filename.quote commit_msg)
  in
  let result = run_command_in_dir ~dir:root ~cmd:commit_cmd in
  if result <> 0 then fail "commit failed" 5

let fold_commits (root : string) (final_round : int) : unit =
  if final_round > 1 then (
    Printf.printf "Folding %d commits into a single commit...\n" final_round;
    (* Fold the last N commits that were created by the codemod *)
    let fold_cmd =
      Printf.sprintf
        "sl fold --from -r .~%d -m %s --reason 'run isolated Safe Abstract analysis - sl help fold'"
        (final_round - 1)
        (Filename.quote
           (Printf.sprintf
              "Apply safe_abstract_codemod - %d rounds"
              final_round))
    in
    let result = run_command_in_dir ~dir:root ~cmd:fold_cmd in
    if result <> 0 then fail "fold commits failed" 6
  ) else if final_round = 1 then
    Printf.printf "Only 1 commit made, no folding needed.\n"

let main () : unit =
  let args = parse_args_exn () in

  if not (check_working_directory_clean args.root) then
    fail "working directory must be clean" 2;

  let artifacts_dir = prepare_artifacts_dir args.artifacts_dir in
  Printf.printf "artifacts directory: %s\n%!" artifacts_dir;
  let round = ref 0 in
  let fixed_point = ref false in

  while not !fixed_point do
    let commit = get_commit_id args.root in
    Printf.printf "round %d (commit %s)\n%!" !round commit;
    let (rewrites, errors_file) =
      run_hh_check args artifacts_dir args.root !round
    in
    let changes = has_changes args.root in
    if Bool.( <> ) changes (rewrites > 0) then
      fail
        (Printf.sprintf
           "rewrite/status mismatch: rewrites=%d, working_directory_changed=%b"
           rewrites
           changes)
        10;
    if changes then begin
      commit_changes args.root !round;
      if !round > 0 && not args.keep_all_errors then begin
        Sys.remove errors_file;
        Printf.printf "removed intermediate artifact: %s\n%!" errors_file
      end;
      incr round
    end else
      fixed_point := true
  done;

  (*
  We use version control to help us see when there are no more changes.
  But we only want a single commit at the end, so we fold.
  *)
  fold_commits args.root !round;

  Printf.printf "done (%d rounds)\n" !round;
  Printf.printf "SAFE_ABSTRACT_RUN_SUCCESS\n%!"

let () = main ()
