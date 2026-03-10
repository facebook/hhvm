(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

type analysis_config = {
  logger_name: string;
  log_line_prefix: string;
  summary_arg_name: string;
  temp_file_prefix: string;
  analysis_display_name: string;
}

type args = {
  hh_distc: string;
  hh_distc_worker: string;
  summary_binary: string;
  root: string;
  pastry: bool;
  quiet: bool;
  extra_args: string list;
}

let fail (msg : string) (code : int) : 'a =
  Printf.eprintf "Error: %s\n" msg;
  exit code

let run_command_capture_stdout (cmd : string) : string * Unix.process_status =
  let ic = Unix.open_process_in cmd in
  let buf = Buffer.create 4096 in
  (try
     while true do
       let line = In_channel.input_line_exn ic in
       Buffer.add_string buf line;
       Buffer.add_char buf '\n'
     done
   with
  | End_of_file -> ());
  let status = Unix.close_process_in ic in
  (Buffer.contents buf, status)

let resolve_path (path : string) : string =
  let (output, status) =
    run_command_capture_stdout
      (Printf.sprintf "realpath %s" (Filename.quote path))
  in
  match status with
  | Unix.WEXITED 0 -> String.strip output
  | _ -> fail (Printf.sprintf "Could not resolve path: %s" path) 1

let extract_logger_lines
    ~(prefix : string) ~(log_file : string) ~(output_file : string) : int =
  let ic = In_channel.create log_file in
  let oc = Out_channel.create output_file in
  let count = ref 0 in
  (try
     while true do
       let line = In_channel.input_line_exn ic in
       if String.is_substring line ~substring:prefix then begin
         Out_channel.output_string oc line;
         Out_channel.output_char oc '\n';
         incr count
       end
     done
   with
  | End_of_file -> ());
  In_channel.close ic;
  Out_channel.close oc;
  !count

let upload_file_to_pastry ~(title : string) (file : string) : string =
  let cmd =
    Printf.sprintf
      "pastry --title %s -q < %s"
      (Filename.quote title)
      (Filename.quote file)
  in
  let (output, status) = run_command_capture_stdout cmd in
  match status with
  | Unix.WEXITED 0 -> String.strip output
  | _ ->
    Printf.eprintf "Warning: pastry upload failed for '%s'\n%!" title;
    "(upload failed)"

let temp_file (prefix : string) (suffix : string) : string =
  Printf.sprintf "/tmp/%s%d%s" prefix (Unix.getpid ()) suffix

let log ~(quiet : bool) (fmt : ('a, Out_channel.t, unit) format) : 'a =
  if quiet then
    Printf.ifprintf Out_channel.stdout fmt
  else
    Printf.printf fmt

let make_usage (config : analysis_config) : string =
  Printf.sprintf
    {|Usage: buck run @//mode/opt //hphp/hack/src/analyze/<name>:run_<name>_analysis -- --root DIR [--pastry] [--quiet] [-- EXTRA_ARGS...]

Type-checks the repo with %s logger enabled via hh_distc (distributed
type checking with Remote Execution), then produces a summary report.

On large repos (e.g. www) the distributed check typically takes ~15-30 minutes.

Options:
  --root DIR     Root directory containing .hhconfig
  --pastry       Upload raw log and summary to Pastry (additive: summary is always printed to stdout)
  --quiet        Only print the summary (no status messages)

Unrecognized arguments are passed through to hh_distc.|}
    config.logger_name

let parse_args_exn (config : analysis_config) : args =
  let usage = make_usage config in
  let hh_distc = ref None in
  let hh_distc_worker = ref None in
  let summary_binary = ref None in
  let root = ref None in
  let pastry = ref false in
  let quiet = ref false in
  let extra = ref [] in
  Arg.parse
    [
      ( "--hh-distc",
        Arg.String (fun s -> hh_distc := Some s),
        " Path to hh_distc binary (injected by buck)" );
      ( "--hh-distc-worker",
        Arg.String (fun s -> hh_distc_worker := Some s),
        " Path to hh_distc worker binary (injected by buck)" );
      ( config.summary_arg_name,
        Arg.String (fun s -> summary_binary := Some s),
        " Path to summary binary (injected by buck)" );
      ("--root", Arg.String (fun s -> root := Some s), " Root directory");
      ("--pastry", Arg.Set pastry, " Upload results to Pastry");
      ("--quiet", Arg.Set quiet, " Only print the summary");
      ( "--",
        Arg.Rest_all (fun args -> extra := args),
        " Pass remaining arguments to hh_distc" );
    ]
    (fun arg -> extra := !extra @ [arg])
    usage;
  let require name r =
    match !r with
    | Some s -> s
    | None ->
      Printf.eprintf "Missing required argument: %s\n%s\n" name usage;
      exit 1
  in
  {
    hh_distc = require "--hh-distc" hh_distc;
    hh_distc_worker = require "--hh-distc-worker" hh_distc_worker;
    summary_binary = require config.summary_arg_name summary_binary;
    root = require "--root" root;
    pastry = !pastry;
    quiet = !quiet;
    extra_args = !extra;
  }

let run (config : analysis_config) : unit =
  let args = parse_args_exn config in
  let root = resolve_path args.root in

  log
    ~quiet:args.quiet
    "Running distributed type check with %s logger via hh_distc...\n%!"
    config.logger_name;
  let extra_args =
    String.concat ~sep:" " (List.map args.extra_args ~f:Filename.quote)
  in
  let stdout_file =
    temp_file (config.temp_file_prefix ^ "distc_stdout_") ".log"
  in
  let check_cmd =
    Printf.sprintf
      {|%s --cmd worker=%s check --root %s --config 'log_levels={"%s":1}' %s >%s|}
      (Filename.quote args.hh_distc)
      (Filename.quote args.hh_distc_worker)
      (Filename.quote root)
      config.logger_name
      extra_args
      (Filename.quote stdout_file)
  in
  let check_cmd =
    if args.quiet then
      check_cmd ^ " 2>/dev/null"
    else
      check_cmd
  in
  let check_exit = Sys.command check_cmd in
  (* Non-zero exit is common (134 = type errors found via signal, etc.).
     Proceed to extract whatever logger entries were written before exit. *)
  if check_exit <> 0 then
    log
      ~quiet:args.quiet
      "hh_distc exited with code %d, checking for partial results...\n%!"
      check_exit;

  (* Extract logger entries from hh_distc stdout *)
  let raw_file = temp_file (config.temp_file_prefix ^ "raw_") ".log" in
  let n =
    extract_logger_lines
      ~prefix:config.log_line_prefix
      ~log_file:stdout_file
      ~output_file:raw_file
  in
  Unix.unlink stdout_file;
  log ~quiet:args.quiet "Extracted %d %s entries\n\n%!" n config.logger_name;
  if n = 0 then begin
    Unix.unlink raw_file;
    fail (Printf.sprintf "No %s entries found" config.logger_name) 4
  end;

  (* Produce summary *)
  let summary_cmd =
    Printf.sprintf "%s %s" args.summary_binary (Filename.quote raw_file)
  in
  let (summary, summary_status) = run_command_capture_stdout summary_cmd in
  (match summary_status with
  | Unix.WEXITED 0 -> ()
  | _ ->
    Printf.eprintf
      "%s summary output:\n%s\n"
      config.analysis_display_name
      summary;
    fail (Printf.sprintf "%s summary failed" config.analysis_display_name) 5);
  Out_channel.output_string Out_channel.stdout summary;
  Out_channel.newline Out_channel.stdout;

  (* Upload to Pastry if requested *)
  if args.pastry then begin
    let summary_file =
      temp_file (config.temp_file_prefix ^ "summary_") ".txt"
    in
    Out_channel.write_all summary_file ~data:summary;
    log ~quiet:args.quiet "Uploading to Pastry...\n%!";
    let raw_url =
      upload_file_to_pastry
        ~title:(config.analysis_display_name ^ " raw log")
        raw_file
    in
    let summary_url =
      upload_file_to_pastry
        ~title:(config.analysis_display_name ^ " summary")
        summary_file
    in
    Printf.printf "\nRaw log:  %s\n" raw_url;
    Printf.printf "Summary:  %s\n" summary_url;
    Unix.unlink summary_file
  end;

  Unix.unlink raw_file
