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
module EditableSyntax = Full_fidelity_editable_syntax
module SourceText = Full_fidelity_source_text
module Logger = HackfmtEventLogger

open Core
open Printf
open Hackfmt_error

type filename = string
type range = int * int
type text_source =
  | File of filename
  | Stdin of filename option (* Optional filename for logging. *)

let text_source_to_filename = function
  | File filename -> Some filename
  | Stdin filename -> filename

module Env = struct
  type t = {
    debug: bool;
    test: bool;
    mutable mode: string option;
    mutable text_source: text_source;
    mutable root: string;
  }
end

let usage = sprintf
  "Usage: %s [--range s e] [filename or read from stdin]\n" Sys.argv.(0)

let parse_options () =
  let files = ref [] in
  let filename_for_logging = ref None in
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
      "[idx]  Format a node ending at the given character" ^
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

    "--filename-for-logging",
      Arg.String (fun x -> filename_for_logging := Some x),
      " The filename for logging purposes, when providing file contents " ^
      "through stdin.";

    "--test", Arg.Set test, " Disable logging";
  ] in
  Arg.parse_dynamic options (fun file -> files := file :: !files) usage;
  let range =
    match !start_char, !end_char with
    | Some s, Some e -> Some (s - 1, e - 1)
    | _ -> None
  in
  (!files, !filename_for_logging, range, !at_char, !inplace, !diff, !root,
    !diff_dry),
  (!debug, !test)

let file_exists path = Option.is_some (Sys_utils.realpath path)

module PrintOptions = struct
  type t = {
    text_source: text_source;
    range: range option;
  }
end

module InPlaceOptions = struct
  type t = {
    filename: filename;
  }
end

module AtCharOptions = struct
  type t = {
    text_source: text_source;
    pos: int;
  }
end

module DiffOptions = struct
  type t = {
    root: string option;
    dry: bool;
  }
end
type format_options =
  | Print of PrintOptions.t
  | InPlace of InPlaceOptions.t
  | AtChar of AtCharOptions.t
  | Diff of DiffOptions.t

let mode_string format_options =
  match format_options with
  | Print {PrintOptions.text_source = File _; range = None; _} -> "FILE"
  | Print {PrintOptions.text_source = File _; range = Some _; _} -> "RANGE"
  | Print {PrintOptions.text_source = Stdin _; range = None; _} -> "STDIN"
  | Print {PrintOptions.text_source = Stdin _; range = Some _; _} -> "STDIN_RANGE"
  | InPlace _ -> "IN_PLACE"
  | AtChar _ -> "AT_CHAR"
  | Diff {DiffOptions.dry = true; _} -> "DIFF"
  | Diff {DiffOptions.dry = false; _} -> "DIFF_DRY"

type validate_options_input = {
  text_source : text_source;
  range : range option;
  at_char : int option;
  inplace : bool;
  diff : bool;
}

let validate_options env
  (files, filename_for_logging, range, at_char, inplace, diff, root, diff_dry) =
  let fail msg = raise (InvalidCliArg msg) in
  let filename =
    match files with
    | [filename] -> Some filename
    | [] -> None
    | _ -> fail "More than one file given"
  in
  let text_source = match filename, filename_for_logging with
    | Some _, Some _ ->
      fail "Can't supply both a filename and a filename for logging"
    | Some filename, None -> File filename
    | None, Some filename_for_logging -> Stdin (Some filename_for_logging)
    | None, None -> Stdin None
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

  match {diff; inplace; text_source; range; at_char} with
  | _ when env.Env.debug && diff -> fail "Can't format diff in debug mode"

  | {diff = true; text_source = File _; _}
  | {diff = true; text_source = Stdin (Some _); _} -> fail "--diff mode expects no files"
  | {diff = true; range = Some _; _} -> fail "--diff mode expects no range"
  | {diff = true; at_char = Some _; _} -> fail "--diff mode can't format at-char"

  | {inplace = true; text_source = Stdin _; _} -> fail "Provide a filename to format in-place"
  | {inplace = true; range = Some _; _} -> fail "Can't format a range in-place"
  | {inplace = true; at_char = Some _; _} -> fail "Can't format at-char in-place"

  | {diff = false; inplace = false; range = Some _; at_char = Some _; _} ->
    fail "--at-char expects no range"

  | {diff = false; inplace = false; at_char = None; _} ->
    Print {PrintOptions.text_source; range}
  | {diff = false; inplace = true; text_source = File filename; range = None; _} ->
    InPlace {InPlaceOptions.filename}
  | {diff = false; inplace = false; range = None; at_char = Some pos; _} ->
    AtChar {AtCharOptions.text_source; pos}
  | {diff = true; text_source = Stdin None; range = None; _} ->
    Diff {DiffOptions.root; dry = diff_dry}

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

let parse text_source =
  let source_text =
    match text_source with
    | File filename ->
      SourceText.from_file @@ Relative_path.create Relative_path.Dummy filename
    | Stdin _ ->
      SourceText.make @@ read_stdin ()
  in
  let tree = SyntaxTree.make source_text in
  if List.is_empty (SyntaxTree.all_errors tree)
    then tree
    else raise Hackfmt_error.InvalidSyntax

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

let get_split_boundaries solve_states =
  List.fold solve_states
    ~init:[]
    ~f:begin fun boundaries solve_state ->
      let chunks = Solve_state.chunks solve_state in
      match chunks with
      | [] -> boundaries
      | hd :: tl ->
        let boundaries, last_range = List.fold tl
          ~init:(boundaries, Chunk.get_range hd)
          ~f:begin fun (boundaries, curr_range) chunk ->
            let chunk_range = Chunk.get_range chunk in
            if Solve_state.has_split_before_chunk solve_state ~chunk
              then curr_range :: boundaries, chunk_range
              else boundaries, (fst curr_range, snd chunk_range)
          end
        in
        last_range :: boundaries
    end
  |> List.rev

let expand_to_split_boundaries boundaries range =
  List.fold boundaries ~init:range ~f:(fun (st, ed) (b_st, b_ed) ->
    let st = if st > b_st && st < b_ed then b_st else st in
    let ed = if ed > b_st && ed < b_ed then b_ed else ed in
    st, ed
  )

let format ?range ?ranges tree =
  let source_text = SyntaxTree.text tree in
  let range =
    Option.map range (expand_to_line_boundaries ?ranges source_text)
  in
  tree
  |> EditableSyntax.from_tree
  |> Hack_format.transform
  |> Chunk_builder.build
  |> Line_splitter.solve ?range (SourceText.text source_text)

let output ?text_source str =
  let with_out_channel f =
    match text_source with
    | Some (File filename) ->
      let out_channel = open_out filename in
      f out_channel;
      close_out out_channel
    | Some (Stdin _)
    | None -> f stdout
  in
  with_out_channel (fun out_channel -> fprintf out_channel "%s%!" str)

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

let format_intervals intervals tree =
  let source_text = SyntaxTree.text tree in
  let text = SourceText.text source_text in
  let lines = String_utils.split_on_newlines text in
  let chunk_groups =
    tree
    |> EditableSyntax.from_tree
    |> Hack_format.transform
    |> Chunk_builder.build
  in
  let solve_states = Line_splitter.find_solve_states
    (SourceText.text source_text) chunk_groups in
  let line_ranges = get_char_ranges lines in
  let split_boundaries = get_split_boundaries solve_states in
  let ranges = intervals
    |> List.map ~f:(line_interval_to_char_range line_ranges)
    |> List.map ~f:(expand_to_split_boundaries split_boundaries)
    |> Interval.union_list
    |> List.sort ~cmp:Interval.comparator
  in
  let formatted_intervals = List.map ranges (fun range ->
    range, Line_splitter.print ~range solve_states
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

let format_at_char tree pos =
  let source_text = SyntaxTree.text tree in
  let chunk_groups =
    tree
    |> EditableSyntax.from_tree
    |> Hack_format.transform
    |> Chunk_builder.build in
  let module PS = Full_fidelity_positioned_syntax in
  (* Grab the node which is the direct parent of the token at pos *)
  let token, node = match PS.parentage (PS.from_tree tree) pos with
    | token :: node :: tl -> token, node
    | _ -> invalid_arg "Invalid offset" in
  (* Format up to the end of the token at the given offset. *)
  let pos = PS.end_offset token in
  (* Our ranges are half-open, so the range end is the token end offset + 1. *)
  let ed = pos + 1 in
  (* Take a half-open range which starts at the beginning of the parent node *)
  let range = (PS.start_offset node, ed) in
  (* Expand the start offset to the nearest line boundary in the original
   * source, since we want to add a leading newline before the node we're
   * formatting if one should be there and isn't already present. *)
  let range = (fst (expand_to_line_boundaries source_text range), ed) in
  (* Find a solution for that range, then expand the range start to a split
   * boundary (a line boundary in the formatted source). *)
  let solve_states = Line_splitter.find_solve_states
    ~range (SourceText.text source_text) chunk_groups in
  let split_boundaries = get_split_boundaries solve_states in
  let range = (fst (expand_to_split_boundaries split_boundaries range), ed) in
  (* Produce the formatted text *)
  let formatted = Line_splitter.print solve_states ~range in
  (* We don't want a trailing newline here (in as-you-type-formatting, it would
   * place the cursor on the next line), but the Line_splitter always prints one
   * at the end of a formatted range. So, we remove it. *)
  range, String.sub formatted 0 (String.length formatted - 1)

let debug_print ?range text_source =
  let tree = parse text_source in
  let source_text = SyntaxTree.text tree in
  let range = Option.map range (expand_to_line_boundaries source_text) in
  let fmt_node = Hack_format.transform (EditableSyntax.from_tree tree) in
  let chunk_groups = Chunk_builder.build fmt_node in
  Hackfmt_debug.debug ~range source_text tree fmt_node chunk_groups

let main (env: Env.t) (options: format_options) =
  env.Env.mode <- Some (mode_string options);
  match options with
  | Print {PrintOptions.text_source; range} ->
    env.Env.text_source <- text_source;
    if env.Env.debug then
      debug_print ?range text_source
    else
      text_source
        |> parse
        |> format ?range
        |> output
  | InPlace {InPlaceOptions.filename} ->
    let text_source = File filename in
    env.Env.text_source <- text_source;
    if env.Env.debug then debug_print text_source;
    text_source
      |> parse
      |> format
      |> output ~text_source
  | AtChar {AtCharOptions.text_source; pos} ->
    env.Env.text_source <- text_source;
    let tree = parse text_source in
    let range, formatted =
      try format_at_char tree pos with
      | Invalid_argument s -> raise (InvalidCliArg s) in
    if env.Env.debug then debug_print text_source ~range;
    Printf.printf "%d %d\n" (fst range) (snd range);
    output formatted;
  | Diff {DiffOptions.root; dry} ->
    let root = get_root root in
    env.Env.root <- Path.to_string root;
    read_stdin ()
      |> Parse_diff.go
      |> List.filter_map ~f:begin fun (rel_path, intervals) ->
        (* We intentionally raise an exception here instead of printing a
         * message and moving on--if a file is missing, it may be a signal that
         * this diff is out of date and may lead us to format unexpected ranges
         * (typically diffs will be directly piped from `hg diff`, and thus
         * won't be out of date).
         *
         * Similarly, InvalidDiff exceptions thrown by format_diff_intervals
         * (caused by out-of-bounds line numbers, etc) will cause us to bail
         * before writing to any files. *)
        let filename = Path.to_string (Path.concat root rel_path) in
        if not (file_exists filename) then
          raise (InvalidDiff ("No such file or directory: " ^ rel_path));
        (* Store the name of the file we're working with, so if we encounter an
         * exception, this filename will be the one that is logged. *)
        let text_source = File filename in
        env.Env.text_source <- text_source;
        try
          let contents =
            text_source
            |> parse
            |> format_diff_intervals intervals in
          Some (text_source, rel_path, contents)
        with
        (* A parse error isn't a signal that there's something wrong with the
         * diff--there's just something wrong with that file. We can leave that
         * file alone and move on. *)
        | InvalidSyntax ->
          Printf.eprintf "Parse error in file: %s\n%!" rel_path;
          None
      end
      |> List.iter ~f:begin fun (text_source, rel_path, contents) ->
        (* Log this filename in the event of an exception. *)
        env.Env.text_source <- text_source;
        if dry then printf "*** %s\n" rel_path;
        let output_text_source = if dry then Stdin None else text_source in
        output ~text_source:output_text_source contents
      end

let () =
  let options, (debug, test) = parse_options () in
  let env = { Env.
    debug;
    test;
    mode = None;
    text_source = Stdin None;
    root = Sys.getcwd ();
  } in

  let start_time = Unix.gettimeofday () in
  if not env.Env.test then Logger.init start_time;

  try
    let options = validate_options env options in
    main env options;

    let time_taken = Unix.gettimeofday () -. start_time in
    if not env.Env.test then
      Logger.exit
        ~time_taken
        ~error:None
        ~exit_code:None
        ~mode:env.Env.mode
        ~file:(text_source_to_filename env.Env.text_source)
        ~root:env.Env.root;
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
    if not env.Env.test then
      Logger.exit
        ~time_taken
        ~error:(Some msg)
        ~exit_code:(Some exit_code)
        ~mode:env.Env.mode
        ~file:(text_source_to_filename env.Env.text_source)
        ~root:env.Env.root;
    eprintf "%s\n" msg;
    exit exit_code
