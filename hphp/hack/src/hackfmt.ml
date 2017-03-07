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

open Core
open Printf
open Hackfmt_error

let usage = sprintf
  "Usage: %s [--range s e] [filename or read from stdin]\n" Sys.argv.(0)

let parse_options () =
  let files = ref [] in
  let start_char = ref None in
  let end_char = ref None in
  let inplace = ref false in
  let diff = ref false in
  let root = ref None in
  let diff_dry = ref false in
  let debug = ref false in

  let rec options = ref [
    "--range",
      Arg.Tuple ([
        Arg.Int (fun x -> start_char := Some x);
        Arg.Int (fun x -> end_char := Some x);
      ]),
      "[start end]  Range of character positions to be formatted (1 indexed)";

    "-i", Arg.Set inplace, " Format file in-place";
    "--in-place", Arg.Set inplace, " Format file in-place";

    "--diff",
      Arg.Set diff,
      " Format the changed lines in a diff" ^
      " (example: hg diff | hackfmt --diff)";

    "--root", Arg.String (fun x -> root := Some x),
      "[dir]  Specify a root directory for --diff mode";

    "--diff-dry-run", Arg.Set diff_dry,
      " Preview the files that would be overwritten by --diff mode";

    "--debug",
      Arg.Unit (fun () ->
        debug := true;
        options := Hackfmt_debug.init_with_options (); ),
      " Print debug statements";
  ] in
  Arg.parse_dynamic options (fun file -> files := file :: !files) usage;
  let range =
    match !start_char, !end_char with
    | Some s, Some e -> Some (s - 1, e - 1)
    | _ -> None
  in
  !files, range, !inplace, !diff, !root, !diff_dry, !debug

let file_exists path = Option.is_some (Sys_utils.realpath path)

type range = int * int
type root = string
type dry = bool
type filename = string
type debug = bool

type format_mode =
  | Print of filename option * range option * debug
  | InPlace of filename
  | Diff of root option * dry

let validate_options (files, range, inplace, diff, root, diff_dry, debug) =
  let fail msg = raise (InvalidCliArg msg) in
  let filename =
    match files with
    | [filename] -> Some filename
    | [] -> None
    | _ -> fail "More than one file given"
  in
  let assert_file_exists = function
    | None -> ()
    | Some path ->
      if not (file_exists path) then
        fail ("No such file or directory: " ^ path)
  in
  assert_file_exists filename;
  assert_file_exists root;

  (* Let --diff-dry-run imply --diff *)
  let diff = diff || diff_dry in

  match diff, inplace, filename, range with
  | _ when debug && diff -> fail "Can't format diff in debug mode"
  | _ when debug && inplace -> fail "Can't format in-place in debug mode"

  | true, _, Some _, _ -> fail "--diff mode expects no files"
  | true, _, _, Some _ -> fail "--diff mode expects no range"

  (* TODO: Allow formatting a range in-place. *)
  | _, true, _, Some _ -> fail "Can't format a range in-place"
  | _, true, None, _ -> fail "Provide a filename to format in-place"

  | false, false, _, _ -> Print (filename, range, debug)
  | false, true, Some filename, None -> InPlace filename
  | true, _, None, None -> Diff (root, diff_dry)

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

let parse filename =
  let source_text =
    match filename with
    | Some fn ->
      SourceText.from_file @@ Relative_path.create Relative_path.Dummy fn
    | None ->
      SourceText.make @@ read_stdin ()
  in
  let syntax_tree = SyntaxTree.make source_text in
  let editable = Full_fidelity_editable_syntax.from_tree syntax_tree in
  source_text, syntax_tree, editable

let get_char_ranges lines =
  let chars_seen = ref 0 in
  Array.of_list @@ List.map lines (fun line -> begin
    let line_start = !chars_seen in
    let line_end = line_start + String.length line + 1 in
    chars_seen := line_end;
    line_start, line_end
  end)

let format_chunk_groups ?range ?ranges (source_text, syntax_tree, editable) =
  let start_char, end_char =
    match range with
    | None -> (0, SourceText.length source_text)
    | Some (start_char, end_char) ->
      (* Ensure that 0 <= start_char <= end_char <= source_text length *)
      let start_char = max start_char 0 in
      let end_char = max start_char end_char in
      let end_char = min end_char (SourceText.length source_text) in
      let start_char = min start_char end_char in
      (* Ensure that start_char and end_char fall on line boundaries *)
      let ranges = match ranges with
        | Some ranges -> ranges
        | None -> get_char_ranges
          (SourceText.text source_text |> String_utils.split_on_newlines)
      in
      Array.fold_left (fun (st, ed) (line_start, line_end) ->
        let st = if st > line_start && st < line_end then line_start else st in
        let ed = if ed > line_start && ed < line_end then line_end else ed in
        st, ed
      ) (start_char, end_char) ranges
  in
  Hack_format.format_node editable start_char end_char

let format ?range ?ranges parsed_file =
  Line_splitter.solve @@ format_chunk_groups ?range ?ranges parsed_file

let output ?filename str =
  let out_channel =
    match filename with
    | Some filename -> open_out filename
    | None -> stdout
  in
  fprintf out_channel "%s%!" str;
  if Option.is_some filename then close_out out_channel

let rec guess_root config start recursion_limit =
  if start = Path.parent start then None (* Reach fs root, nothing to do. *)
  else if Wwwroot.is_www_directory ~config start then Some start
  else if recursion_limit <= 0 then None
  else guess_root config (Path.parent start) (recursion_limit - 1)

let get_root = function
  | Some root -> Path.make root
  | None ->
    eprintf "No root specified, trying to guess one\n";
    let config = ".hhconfig" in
    let start_path = Path.make "." in
    let root = match guess_root config start_path 50 with
      | None -> start_path
      | Some r -> r in
    Wwwroot.assert_www_directory ~config root;
    eprintf "Guessed root: %a\n%!" Path.output root;
    root

let line_interval_to_char_range char_ranges (start_line, end_line) =
  if start_line > end_line then
    raise (InvalidDiff (sprintf
      "Illegal line interval: %d,%d" start_line end_line));
  if start_line < 1 || start_line > Array.length char_ranges ||
     end_line < 1 || end_line > Array.length char_ranges then
      raise (InvalidDiff (sprintf
        "Can't format line interval %d,%d in file with %d lines"
        start_line end_line (Array.length char_ranges)));
  let start_char, _ = char_ranges.(start_line - 1) in
  let _, end_char = char_ranges.(end_line - 1) in
  start_char, end_char

let format_intervals intervals parsed_file =
  let source_text, _, _ = parsed_file in
  let lines = SourceText.text source_text |> String_utils.split_on_newlines in
  let ranges = get_char_ranges lines in
  let formatted_intervals = ref (List.map intervals (fun interval ->
    let range = line_interval_to_char_range ranges interval in
    interval, format ~range ~ranges parsed_file
  )) in
  let buf = Buffer.create @@ SourceText.length source_text + 256 in
  let add_line line = Buffer.add_string buf line; Buffer.add_char buf '\n' in
  List.iteri lines (fun idx line ->
    match !formatted_intervals with
    | [] -> add_line line
    | ((st, ed), str) :: tl ->
      if idx < st - 1 then
        add_line line
      else if idx = ed - 1 then begin
        formatted_intervals := tl;
        Buffer.add_string buf str
      end
  );
  Buffer.contents buf

let main = function
  | Print (filename, range, debug) ->
    if debug then
      let parsed_file = parse filename in
      let source_text, syntax_tree, _ = parsed_file in
      let chunk_groups = format_chunk_groups ?range parsed_file in
      Hackfmt_debug.debug source_text syntax_tree chunk_groups
    else
      parse filename
      |> format ?range
      |> output
  | InPlace filename ->
    parse (Some filename)
    |> format
    |> output ~filename
  | Diff (root, dry) ->
    let root = get_root root in
    let diff = read_stdin () in
    let parsed_diff = Parse_diff.go diff in
    let formatted_files = List.map parsed_diff (fun (rel_path, intervals) ->
      let filename = Path.concat root rel_path |> Path.to_string in
      if not (file_exists filename) then
        raise (InvalidDiff ("No such file or directory: " ^ rel_path));
      let contents = parse (Some filename) |> format_intervals intervals in
      rel_path, filename, contents
    ) in
    List.iter formatted_files (fun (rel_path, filename, contents) ->
      if dry then printf "*** %s\n" rel_path;
      let filename = if dry then None else Some filename in
      output ?filename contents
    )

let () =
  try
    parse_options ()
    |> validate_options
    |> main
  with exn ->
    let exit_code = get_exception_exit_value exn in
    let err_str = get_error_string_from_exit_value exit_code in
    let msg = match exn with
      | Failure s
      | UnsupportedSyntax s
      | InvalidCliArg s
      | InvalidDiff s ->
        err_str ^ ": " ^ s
      | InvalidSyntax
      | _ ->
        err_str
    in
    eprintf "%s\n" msg;
    exit exit_code
