(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module SyntaxError = Full_fidelity_syntax_error
module SyntaxTree = Full_fidelity_syntax_tree
module SourceText = Full_fidelity_source_text

let usage = Printf.sprintf
  "Usage: %s [--range s e] [filename or read from stdin]\n" Sys.argv.(0)

let parse_and_print (source_text, out_channel, start_char, end_char, debug) =
  let syntax_tree = SyntaxTree.make source_text in
  let editable = Full_fidelity_editable_syntax.from_tree syntax_tree in

  let chunk_groups = Hack_format.format_node editable start_char end_char in
  if debug
  then Hackfmt_debug.debug source_text syntax_tree chunk_groups
  else
    let formatted_string = Line_splitter.solve chunk_groups in
    Printf.fprintf out_channel "%s" formatted_string

let read_stdin () =
  let buf = Buffer.create 256 in
  try
    while true do
      Buffer.add_string buf (read_line());
      Buffer.add_char buf '\n';
    done;
    assert false
  with End_of_file ->
    Buffer.contents buf

let parse_options () =
  let filename = ref None in
  let start_char = ref None in
  let end_char = ref None in
  let debug = ref false in
  let inplace = ref false in

  let rec options = ref [
    "--debug",
      Arg.Unit (fun () ->
        debug := true;
        options := Hackfmt_debug.init_with_options (); ),
      " Print debug statements";
    "--range",
      Arg.Tuple ([
        Arg.Int (fun x -> start_char := Some x);
        Arg.Int (fun x -> end_char := Some x);
      ]),
      "<start end> Range of character positions to be formatted, (1 indexed)";
    "-i", Arg.Set inplace, "Format file in-place";
    "--in-place", Arg.Set inplace, "Format file in-place";
  ] in
  Arg.parse_dynamic options (fun fn -> filename := Some fn) usage;

  if !inplace then
    if Option.is_none !filename then
      raise (Failure "Provide a filename when formatting in-place")
    (* TODO: Allow editing a range in-place. *)
    else if Option.is_some !start_char || Option.is_some !end_char then
      raise (Failure "Can't format a range in-place")
    else if !debug then
      raise (Failure "Can't format in-place in debug mode");

  let source_text = match !filename with
    | Some fn ->
      SourceText.from_file @@ Relative_path.create Relative_path.Dummy fn
    | None ->
      SourceText.make @@ read_stdin ()
  in

  (* TODO: verify that start_c and end_c fall on line boundries *)
  let start_c = Option.value_map !start_char ~default:0 ~f:(fun i -> i - 1) in
  let end_c = Option.value_map !end_char
    ~default:(SourceText.length source_text) ~f:(fun i -> i - 1) in

  let out_channel = match !filename with
    | Some fn when !inplace -> open_out fn
    | _ -> stdout
  in

  parse_and_print (source_text, out_channel, start_c, end_c, !debug)

let () = try parse_options () with
  | exn -> exit (Hackfmt_error.get_exception_exit_value exn)
