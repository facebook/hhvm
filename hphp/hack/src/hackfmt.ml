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
module Logger = HackfmtEventLogger

open Core
open Printf
open Hackfmt_error

type env = {
  debug: bool;
  test: bool;
  mutable mode: string option;
  mutable file: string option;
  mutable root: string;
}

let usage = sprintf
  "Usage: %s [--range s e] [filename or read from stdin]\n" Sys.argv.(0)

let parse_options () =
  let files = ref [] in
  let start_char = ref None in
  let end_char = ref None in
  let at_char = ref None in
  let inplace = ref false in
  let diff = ref false in
  let root = ref None in
  let diff_dry = ref false in
  let debug = ref false in
  let test = ref false in

  let rec options = ref [
    "--range",
      Arg.Tuple ([
        Arg.Int (fun x -> start_char := Some x);
        Arg.Int (fun x -> end_char := Some x);
      ]),
      "[start end]  Range of character positions to be formatted (1 indexed)";

    "--at-char",
      Arg.Int (fun x -> at_char := Some x),
      "[idx]  Format the smallest possible range around this character" ^
      " (0 indexed)";

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

    "--test", Arg.Set test, " Disable logging";
  ] in
  Arg.parse_dynamic options (fun file -> files := file :: !files) usage;
  let range =
    match !start_char, !end_char with
    | Some s, Some e -> Some (s - 1, e - 1)
    | _ -> None
  in
  (!files, range, !at_char, !inplace, !diff, !root, !diff_dry),
  (!debug, !test)

let file_exists path = Option.is_some (Sys_utils.realpath path)

type range = int * int
type root = string
type dry = bool
type filename = string

type format_options =
  | Print of filename option * range option
  | InPlace of filename
  | AtChar of filename option * int
  | Diff of root option * dry

let mode_string format_options =
  match format_options with
  | Print (Some _, None) -> "FILE"
  | Print (Some _, Some _) -> "RANGE"
  | Print (None, None) -> "STDIN"
  | Print (None, Some _) -> "STDIN_RANGE"
  | InPlace _ -> "IN_PLACE"
  | AtChar (_, _) -> "AT_CHAR"
  | Diff (_, false) -> "DIFF"
  | Diff (_, true) -> "DIFF_DRY"

let validate_options env
    (files, range, at_char, inplace, diff, root, diff_dry) =
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

  match diff, inplace, filename, range, at_char with
  | _ when env.debug && diff -> fail "Can't format diff in debug mode"

  | true, _, Some _, _, _ -> fail "--diff mode expects no files"
  | true, _, _, Some _, _ -> fail "--diff mode expects no range"
  | true, _, _, _, Some _ -> fail "--diff mode can't format at-char"

  | _, true, None, _, _ -> fail "Provide a filename to format in-place"
  | _, true, _, Some _, _ -> fail "Can't format a range in-place"
  | _, true, _, _, Some _ -> fail "Can't format at-char in-place"

  | false, false, _, Some _, Some _ -> fail "--at-char expects no range"

  | false, false, _, _, None -> Print (filename, range)
  | false, true, Some filename, None, _ -> InPlace filename
  | false, false, _, None, Some pos -> AtChar (filename, pos)
  | true, _, None, None, _ -> Diff (root, diff_dry)

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
  if not (List.is_empty (SyntaxTree.all_errors syntax_tree)) then
    raise Hackfmt_error.InvalidSyntax;
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

let expand_to_line_boundaries ?ranges source_text range =
  let start_char, end_char = range in
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

let format ?range ?ranges parsed_file =
  let source_text, _, editable = parsed_file in
  let range =
    Option.map range (expand_to_line_boundaries ?ranges source_text)
  in
  editable
  |> Hack_format.transform
  |> Chunk_builder.build
  |> Line_splitter.solve ?range

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
    invalid_arg (sprintf
      "Illegal line interval: %d,%d" start_line end_line);
  if start_line < 1 || start_line > Array.length char_ranges ||
     end_line < 1 || end_line > Array.length char_ranges then
      invalid_arg (sprintf
        "Can't format line interval %d,%d in file with %d lines"
        start_line end_line (Array.length char_ranges));
  let start_char, _ = char_ranges.(start_line - 1) in
  let _, end_char = char_ranges.(end_line - 1) in
  start_char, end_char

let format_intervals intervals parsed_file =
  let source_text, _, editable = parsed_file in
  let text = SourceText.text source_text in
  let lines = String_utils.split_on_newlines text in
  let chunk_groups = Hack_format.transform editable |> Chunk_builder.build in
  let line_ranges = get_char_ranges lines in
  let ranges = intervals
    |> List.map ~f:(line_interval_to_char_range line_ranges)
    |> List.map ~f:(expand_to_line_boundaries ~ranges:line_ranges source_text)
    |> Interval.union_list
    |> List.sort ~cmp:Interval.comparator
  in
  let formatted_intervals = List.map ranges (fun range ->
    range, Line_splitter.solve ~range chunk_groups
  ) in
  let length = SourceText.length source_text in
  let buf = Buffer.create (length + 256) in
  let chars_seen = ref 0 in
  List.iter formatted_intervals (fun ((st, ed), formatted) ->
    Buffer.add_string buf (String.sub text !chars_seen (st - !chars_seen));
    Buffer.add_string buf formatted;
    chars_seen := ed;
  );
  Buffer.add_string buf (String.sub text !chars_seen (length - !chars_seen));
  Buffer.contents buf

let format_diff_intervals intervals parsed_file =
  try format_intervals intervals parsed_file with
  | Invalid_argument s -> raise (InvalidDiff s)

let debug_print ?range filename =
  let source_text, syntax_tree, editable = parse filename in
  let range = Option.map range (expand_to_line_boundaries source_text) in
  let fmt_node = Hack_format.transform editable in
  let chunk_groups = Chunk_builder.build fmt_node in
  Hackfmt_debug.debug ~range source_text syntax_tree fmt_node chunk_groups

let main (env: env) (options: format_options) =
  env.mode <- Some (mode_string options);
  match options with
  | Print (filename, range) ->
    env.file <- filename;
    if env.debug then
      debug_print ?range filename
    else
      filename
        |> parse
        |> format ?range
        |> output
  | InPlace filename ->
    let filename = Some filename in
    env.file <- filename;
    if env.debug then debug_print filename;
    filename
      |> parse
      |> format
      |> output ?filename
  | AtChar (filename, pos) ->
    let _, _, editable = parse filename in
    let chunk_groups = editable
      |> Hack_format.transform
      |> Chunk_builder.build
    in
    let range = List.fold chunk_groups ~init:None ~f:(fun range cg ->
      let st, ed = Chunk_group.get_char_range cg in
      if st <= pos && ed > pos then Some (st, ed) else range
    ) in
    (match range with
    | None -> raise (InvalidCliArg "No formattable text at given position")
    | Some (st, ed) ->
      Printf.printf "%d %d\n" st ed;
      chunk_groups
      |> Line_splitter.solve ~range:(st, ed)
      |> output
    )
  | Diff (root, dry) ->
    let root = get_root root in
    env.root <- Path.to_string root;
    read_stdin ()
      |> Parse_diff.go
      |> List.map ~f:(fun (rel_path, intervals) ->
        env.file <- Some rel_path;
        let filename = Path.to_string (Path.concat root rel_path) in
        if not (file_exists filename) then
          raise (InvalidDiff ("No such file or directory: " ^ rel_path));
        let contents = Some filename
          |> parse
          |> format_diff_intervals intervals
        in
        rel_path, filename, contents
      )
      |> List.iter ~f:(fun (rel_path, filename, contents) ->
        env.file <- Some rel_path;
        if dry then printf "*** %s\n" rel_path;
        let filename = if dry then None else Some filename in
        output ?filename contents
      )

let () =
  let options, (debug, test) = parse_options () in
  let env = {debug; test; mode = None; file = None; root = Sys.getcwd ()} in

  let start_time = Unix.gettimeofday () in
  if not env.test then Logger.init start_time;

  try
    let options = validate_options env options in
    main env options;

    let time_taken = Unix.gettimeofday () -. start_time in
    if not env.test then
      Logger.exit time_taken None None env.mode env.file env.root;
  with exn ->
    let exit_code = get_exception_exit_value exn in
    if exit_code = 255 then Printexc.print_backtrace stderr;
    let err_str = get_error_string_from_exn exn in
    let msg = match exn with
      | InvalidSyntax ->
        err_str
      | InvalidCliArg s
      | InvalidDiff s ->
        err_str ^ ": " ^ s
      | _ ->
        err_str ^ ": " ^ (Printexc.to_string exn)
    in
    let time_taken = Unix.gettimeofday () -. start_time in
    if not env.test then
      Logger.exit time_taken (Some msg) (Some exit_code)
        env.mode env.file env.root;
    eprintf "%s\n" msg;
    exit exit_code
