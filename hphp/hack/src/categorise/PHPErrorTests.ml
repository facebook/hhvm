(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open ScubaQuery
open PHPError

 (** TESTING **)

let print_rows rows =
  let row_strings =
    List.map (fun r -> let row = List.map string_of_table_entry r in
                       String.concat "  ;  " row) rows in
  let rows_string = String.concat "\n\n" row_strings in
  print_endline rows_string

let print_stacks stack_traces =
  let print_stack stack =
    let stack = List.map stl_to_string stack in
    let stack_string = String.concat ";\n" stack in
    print_endline (stack_string^"\n") in
  List.iter print_stack stack_traces

(* TODO: more thorough testing *)

(* prints all stack traces, line-by-line *)
let () =
  set_www_root ();
  let now = int_of_float (Unix.time ()) in
  let day = 60*60*24 in
  let query = new_php_error_query ()
    |> add_filter (normfilter "error_string" Contains ["must be of type"])
    |> add_time_range (now - 7*day, now)
    |> add_limit 20 in
  let column_names, rows = query_scuba query in
  let php_errors = format_results column_names rows in
  let stack_traces = List.map (fun { stack; _ } -> stack) php_errors in
  print_endline (String.concat "  ;  " column_names);
  print_rows rows;
  print_stacks stack_traces;
  List.iter list_stack_trace_by_mode php_errors
