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

let usage = Printf.sprintf "Usage: %s filename\n" Sys.argv.(0)

let parse_and_print (source_text, start_char, end_char) =
  let syntax_tree = SyntaxTree.make source_text in

  let editable = Full_fidelity_editable_syntax.from_tree syntax_tree in
  let chunk_groups = Hack_format.format_node editable start_char end_char in
  let result = Line_splitter.solve chunk_groups in
  Printf.printf "%s" result;
  ()

let parse_options () =
  let filename = ref None in
  let start_char = ref None in
  let end_char = ref None in

  let options = [
    "--range",
      Arg.Tuple ([
        Arg.Int (fun x -> start_char := Some x);
        Arg.Int (fun x -> end_char := Some x);
      ]),
      "<start end> Range of character positions to be formatted, (1 indexed)";
  ] in
  Arg.parse options (fun fn -> filename := Some fn) usage;
  let source_text = match !filename with
    | Some fn ->
      SourceText.from_file @@ Relative_path.create Relative_path.Dummy fn
    | None ->
      raise (Failure "Exepected a file name")
  in

  let start_c = Option.value_map !start_char ~default:0 ~f:(fun i -> i - 1) in
  let end_c = Option.value_map !end_char
    ~default:(SourceText.length source_text) ~f:(fun i -> i - 1) in

  source_text, start_c, end_c

let () = parse_and_print @@ parse_options ()
