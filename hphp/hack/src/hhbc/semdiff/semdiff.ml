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

type options = {
  files : string * string;
  similarity : bool;
}

let die str =
  let oc = stderr in
  output_string oc str;
  close_out oc;
  exit 2

let parse_options () =
  let similarity = ref false in
  let purpose = "Hhas differencing tool" in
  let usage =
    Printf.sprintf "%s\nUsage: %s file1 file2\n" purpose Sys.argv.(0)
  in
  let options =
    [ ("--similarity"
      , Arg.Set similarity
      , " Only displays the similarity percentage on STDOUT"
      );
    ] in
  let options = Arg.align ~limit:25 options in
  let files = ref [] in
  Arg.parse options (fun file -> files := file::!files) usage;
  match !files with
  | [x; y] ->
    { files = (x, y)
    ; similarity = !similarity
    }
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

let run options =
  let program_parser = program Hhas_lexer.read in
  let prog1 = parse_file program_parser (fst options.files) in
  let prog2 = parse_file program_parser (snd options.files) in

  let (d,(s,e)) = program_comparer.comparer prog1 prog2 in
  let similarity = (100.0 *. (1.0 -. float_of_int d /. float_of_int (s+1))) in
  if options.similarity
  then Printf.printf "%.2f" similarity
  else (Printf.printf
    "distance = %d\nsize = %d\nsimilarity =%.2f%%\nedits=\n%s" d s similarity e;
    print_endline defaultstring) (* make sure the colors are back to normal *)

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
    Unix.handle_unix_error run options
