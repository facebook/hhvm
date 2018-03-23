(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)
open Hhas_parser
open Diff

module Loc = Srcloc_stats
module Log = Semdiff_logging

type options = {
  files : string * string;
}

let die str =
  let oc = stderr in
  output_string oc str;
  close_out oc;
  exit 2

let parse_options () =
  let purpose = "Hhas differencing tool" in
  let usage =
    Printf.sprintf "%s\nUsage: %s file1 file2\n" purpose Sys.argv.(0)
  in
  let options =
    [ ("--verbose"
      , Arg.Int (fun i ->
                  if i < 0 || 3 < i
                  then raise (Arg.Bad "Verbosity level has to be 0, 1, 2, or 3")
                  else Log.verbosity_level := i)
        (* Change the default logging level in Semdiff_logging.ml *)
      , " Set verbosity level 0, 1, 2 or 3 [default: 2]
                  0: Displays nothing
                  1: Only displays differences on STDOUT
                  2: Also displays differences and debugging information on STDOUT
                  3: Also displays full trace on STDOUT"
      );
      ("--laxunset",
       Arg.Unit (fun () -> Rhl.lax_unset := true),
       " Ignore finalizer ordering effects of Unset instructions"
      );
      ("--hidesim",
       Arg.Unit (fun () -> Log.hide_sim := true),
       " hide similarity information"
      );
      ("--hidesize",
       Arg.Unit (fun () -> Log.hide_size := true),
       " hide size information"
      );
      ("--hidedist",
       Arg.Unit (fun () -> Log.hide_dist := true),
       " hide size information"
      );
      ("--hideassm",
       Arg.Unit (fun () -> Log.hide_assm := true),
       " hide assumed & todo information"
      );
      ("--srcloc",
       Arg.Unit (fun () -> Hhas_parser_actions.check_srcloc := true),
       " Check line numbers in srcloc directives"
      );
    ] in
  let options = Arg.align ~limit:25 options in
  let files = ref [] in
  Arg.parse options (fun file -> files := file::!files) usage;
  match !files with
  | [x; y] ->
    (* !files is in reverse order, so swap the files to get the order back *)
    { files = (y, x) }
  | _ -> die usage

let parse_file program_parser filename =
  let channel = open_in filename in (* TODO: error handling *)

  let lexer = Lexing.from_channel channel in
  let prog =
    try program_parser lexer
      with Parsing.Parse_error -> (
        Printf.eprintf "Parsing of file failed\n";
        raise Parsing.Parse_error
        )
  in
  close_in channel;
  prog

let print_srcloc_stats () =
  Log.print_default "{";
  Log.print_set_json ~name:"mismatched_srcloc"
    !Loc.mismatch_loc_instrs;
  Log.print_set_json ~name:"missing_on_lhs_srcloc"
    !Loc.missing_loc_on_left_instrs;
  Log.print_set_json ~name:"missing_on_rhs_srcloc" ~trailing_coma:false
    !Loc.missing_loc_on_right_instrs;
  Log.print_default "}"

let srcloc_are_the_same () =
  SSet.is_empty !Loc.mismatch_loc_instrs
    && SSet.is_empty !Loc.missing_loc_on_left_instrs
    (* we don't care about missing on right as rhs corresponds to HHVM *)

let run options =
  let program_parser = program Hhas_lexer.read in
  let prog1 = parse_file program_parser (fst options.files) in
  let prog2 = parse_file program_parser (snd options.files) in

  let d, (s, e) = program_comparer.comparer prog1 prog2 in
  let all_the_same =
    if !Hhas_parser_actions.check_srcloc then begin
      print_srcloc_stats ();
      srcloc_are_the_same ()
    end else begin
      let similarity = (100.0 *. (1.0 -. float_of_int d /. float_of_int (s+1))) in
      if not !Log.hide_dist
      then
        Log.print_default @@ Printf.sprintf "Distance = %.d" d;
      if not !Log.hide_sim
      then
        Log.print_default @@
        Printf.sprintf "Similarity = %.2f" similarity;
      if not !Log.hide_size
      then
        Log.print_default @@ Printf.sprintf "Size = %d" s;
      if d <> 0
      then Log.print_default ~level:1 @@ Printf.sprintf "Edits = \n";
      Log.print_edit_sequence ~level:1 e;
      d = 0
    end in
  all_the_same

(* command line driver *)
let _ =
  if ! Sys.interactive
  then ()
  else
    (* On windows, setting 'binary mode' avoids to output CRLF on
       stdout.  The 'text mode' would not hurt the user in general, but
       it breaks the testsuite where the output is compared to the
       expected one (i.e. in given file without CRLF). *)
    set_binary_mode_out stdout true;
    let options = parse_options () in
    (* TODO: double-check what we should do about the return codes *)
    let all_the_same = Unix.handle_unix_error run options in
    exit (if all_the_same then 0 else 1)
