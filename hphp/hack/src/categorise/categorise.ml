(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open PHPError
open ScubaQuery

type args =
  {
    targets : string list;
    errors : int;
    days : int;
  }

let analyse_directories _ = ()  (* TODO: write function *)

let analyse_files { targets; errors; days; } =
  let now = int_of_float (Unix.time ()) in
  let day_length = 60 * 60 * 24 in
  let analyse_file file =  (* TODO: sensible checks for file existence, and empty scuba result *)
    let q = new_php_error_query ()
         |> add_filter (normfilter "error_file" Contains [file])
         |> add_time_range (now - days * day_length, now)
         |> add_order_by ("time", Desc)
         |> add_limit errors in
    let column_names, rows = query_scuba q in
    let php_errors = format_results column_names rows in
    List.iter list_stack_trace_by_mode php_errors in
  List.iter analyse_file targets

let parse_args () =
  let usage_msg = "Given a file, analyse related PHP runtime errors" in
  let is_directory = ref false in (* TODO: proper documentation *)
  let errors = ref 1 in
  let days = ref 7 in
  let targets = ref [] in
  let templates = ref None in
  let specs = [
    "-d",
      Arg.Set is_directory,
      "Analyse directories rather than files";
    "--errors",
      Arg.Set_int errors,
      "Number of errors to analyse, most recent first (default 1)";
    "--days",
      Arg.Set_int days,
      "Number of days to check data for (default 7)";
    "--error-templates",
      Arg.String (fun s -> templates := Some s),
      "File of provided error templates (one regexp per line)";
  ] in
  Arg.parse specs (fun x -> targets := x :: !targets) usage_msg;
  set_www_root ();
  let args =
    { targets      = List.rev !targets;
      errors       = !errors;
      days         = !days;
    } in
  if !targets = [] then analyse_and_bucket !templates !days !errors else
    if !is_directory
    then analyse_directories args
    else analyse_files args

let () = parse_args ()
