(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module SyntaxError = Full_fidelity_syntax_error
module SyntaxTree =
  Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax)
module Logger = HackfmtEventLogger
module FEnv = Format_env
open Printf
open Boundaries
open Hackfmt_error
open Ocaml_overrides
open FindUtils

type filename = string

type range =
  (* 0-based byte offsets; half-open/inclusive-exclusive *)
  | Byte of Interval.t
  (* 1-based line numbers; inclusive *)
  | Line of Interval.t

type text_source =
  | File of filename
  (* Optional filename for logging. *)
  | Stdin of filename option

let text_source_to_filename = function
  | File filename -> Some filename
  | Stdin filename -> filename

let file_exists path = Option.is_some (Sys_utils.realpath path)

let rec guess_root config start recursion_limit =
  if Path.equal start (Path.parent start) then
    None
  (* Reach fs root, nothing to do. *)
  else if Wwwroot.is_www_directory ~config start then
    Some start
  else if recursion_limit <= 0 then
    None
  else
    guess_root config (Path.parent start) (recursion_limit - 1)

let get_root_for_diff () =
  eprintf "No root specified, trying to guess one\n";
  let config = ".hhconfig" in
  let start_path = Path.make "." in
  let root =
    match guess_root config start_path 50 with
    | None -> start_path
    | Some r -> r
  in
  Wwwroot.assert_www_directory ~config root;
  eprintf "Guessed root: %a\n%!" Path.output root;
  root

let get_root_for_format files =
  let cur = Path.make "." in
  let start_path =
    match files with
    | [] -> cur
    (* just use the first file here. specifying more than one will result in an
     * error during validation *)
    | hd :: _ -> Path.make hd |> Path.dirname
  in
  match guess_root ".hhconfig" start_path 50 with
  | Some p -> p
  | None -> cur

let read_hhconfig path =
  let (_hash, config) = Config_file.parse_hhconfig path in
  ( FEnv.
      {
        add_trailing_commas =
          Config_file.Getters.bool_
            "hackfmt.add_trailing_commas"
            ~default:default.add_trailing_commas
            config;
        indent_width =
          Config_file.Getters.int_
            "hackfmt.indent_width"
            ~default:default.indent_width
            config;
        indent_with_tabs =
          Config_file.Getters.bool_
            "hackfmt.tabs" (* keep consistent with CLI arg name *)
            ~default:default.indent_with_tabs
            config;
        line_width =
          Config_file.Getters.int_
            "hackfmt.line_width"
            ~default:default.line_width
            config;
        format_generated_code =
          Config_file.Getters.bool_
            "hackfmt.format_generated_code"
            ~default:default.format_generated_code
            config;
        version = Config_file.Getters.int_opt "hackfmt.version" config;
      },
    Full_fidelity_parser_env.make
      ?enable_xhp_class_modifier:
        (Config_file.Getters.bool_opt "enable_xhp_class_modifier" config)
      () )

let opt_default opt def =
  match opt with
  | Some v -> v
  | None -> def

module Env = struct
  type t = {
    debug: bool;
    test: bool;
    mutable mode: string option;
    mutable text_source: text_source;
    root: string;
  }
end

let usage =
  sprintf "Usage: %s [--range s e] [filename or read from stdin]\n" Sys.argv.(0)

let parse_options () =
  let files = ref [] in
  let filename_for_logging = ref None in
  let start_char = ref None in
  let end_char = ref None in
  let start_line = ref None in
  let end_line = ref None in
  let at_char = ref None in
  let inplace = ref false in
  let diff = ref false in
  let diff_dry = ref false in
  let check = ref false in
  let debug = ref false in
  let test = ref false in
  (* The following are either inferred from context (cwd and .hhconfig),
   * or via CLI flags, with priority given to the latter. *)
  let cli_indent_width = ref None in
  let cli_indent_with_tabs = ref false in
  let cli_line_width = ref None in
  let cli_root = ref None in
  let cli_format_generated_code = ref false in
  let cli_version = ref None in
  let rec options =
    ref
      [
        ( "--range",
          Arg.Tuple
            [
              Arg.Int (fun x -> start_char := Some x);
              Arg.Int (fun x -> end_char := Some x);
            ],
          "[start end]  Range of character positions to be formatted (1 indexed)"
        );
        ( "--line-range",
          Arg.Tuple
            [
              Arg.Int (fun x -> start_line := Some x);
              Arg.Int (fun x -> end_line := Some x);
            ],
          "[start end]  Range of lines to be formatted (1 indexed, inclusive)"
        );
        ( "--at-char",
          Arg.Int (fun x -> at_char := Some x),
          "[idx]  Format a node ending at the given character" ^ " (0 indexed)"
        );
        ("-i", Arg.Set inplace, " Format file in-place");
        ("--in-place", Arg.Set inplace, " Format file in-place");
        ( "--indent-width",
          Arg.Int (fun x -> cli_indent_width := Some x),
          sprintf
            " Specify the number of spaces per indentation level. Defaults to %d"
            FEnv.(default.indent_width) );
        ( "--line-width",
          Arg.Int (fun x -> cli_line_width := Some x),
          sprintf
            " Specify the maximum length for each line. Defaults to %d"
            FEnv.(default.line_width) );
        ( "--tabs",
          Arg.Set cli_indent_with_tabs,
          " Indent with tabs rather than spaces" );
        ( "--format-generated-code",
          Arg.Set cli_format_generated_code,
          " Enable formatting of generated files and generated sections in "
          ^ "partially-generated files. By default, generated code will be left "
          ^ "untouched." );
        ( "--version",
          Arg.Int (fun x -> cli_version := Some x),
          " For version-gated formatter features, specify the version to use. "
          ^ "Defaults to latest." );
        ( "--diff",
          Arg.Set diff,
          " Format the changed lines in a diff"
          ^ " (example: hg diff | hackfmt --diff)" );
        ( "--root",
          Arg.String (fun x -> cli_root := Some x),
          "[dir]  Specify a root directory for --diff mode, or a directory "
          ^ "containing .hhconfig for standard mode" );
        ( "--diff-dry-run",
          Arg.Set diff_dry,
          " Preview the files that would be overwritten by --diff mode" );
        ( "--check-formatting",
          Arg.Set check,
          " Given a list of filenames, check whether they are formatted and "
          ^ "print the filenames which would be modified by hackfmt -i" );
        ( "--debug",
          Arg.Unit
            (fun () ->
              debug := true;
              options := Hackfmt_debug.init_with_options ()),
          " Print debug statements" );
        ( "--filename-for-logging",
          Arg.String (fun x -> filename_for_logging := Some x),
          " The filename for logging purposes, when providing file contents "
          ^ "through stdin." );
        ("--test", Arg.Set test, " Disable logging");
      ]
  in
  Arg.parse_dynamic options (fun file -> files := file :: !files) usage;
  let range =
    match (!start_char, !end_char, !start_line, !end_line) with
    | (Some s, Some e, None, None) -> Some (Byte (s - 1, e - 1))
    | (None, None, Some s, Some e) -> Some (Line (s, e))
    | (Some _, Some _, Some _, Some _) ->
      raise (InvalidCliArg "Cannot use --range with --line-range")
    | _ -> None
  in
  let root =
    match !cli_root with
    | Some p -> Path.make p
    | None ->
      if !diff then
        get_root_for_diff ()
      else
        get_root_for_format !files
  in
  let hhconfig_path = Path.concat root ".hhconfig" |> Path.to_string in
  let (config, parser_env) =
    if file_exists hhconfig_path then
      read_hhconfig hhconfig_path
    else
      (FEnv.default, Full_fidelity_parser_env.default)
  in
  let config =
    FEnv.
      {
        add_trailing_commas = config.add_trailing_commas;
        indent_width = opt_default !cli_indent_width config.indent_width;
        indent_with_tabs = !cli_indent_with_tabs || config.indent_with_tabs;
        line_width = opt_default !cli_line_width config.line_width;
        format_generated_code =
          !cli_format_generated_code || config.format_generated_code;
        version =
          (if Option.is_some !cli_version then
            !cli_version
          else
            config.version);
      }
  in
  ( ( !files,
      !filename_for_logging,
      range,
      !at_char,
      !inplace,
      !diff,
      !diff_dry,
      !check,
      config ),
    root,
    parser_env,
    (!debug, !test) )

type format_options =
  | Print of {
      text_source: text_source;
      range: range option;
      config: FEnv.t;
    }
  | InPlace of {
      files: filename list;
      config: FEnv.t;
    }
  | Check of {
      files: filename list;
      config: FEnv.t;
    }
  | AtChar of {
      text_source: text_source;
      pos: int;
      config: FEnv.t;
    }
  | Diff of {
      dry: bool;
      config: FEnv.t;
    }

let mode_string format_options =
  match format_options with
  | Print { text_source = File _; range = None; _ } -> "FILE"
  | Print { text_source = File _; range = Some _; _ } -> "RANGE"
  | Print { text_source = Stdin _; range = None; _ } -> "STDIN"
  | Print { text_source = Stdin _; range = Some _; _ } -> "STDIN_RANGE"
  | InPlace _ -> "IN_PLACE"
  | Check _ -> "CHECK_FORMATTING"
  | AtChar _ -> "AT_CHAR"
  | Diff { dry = false; _ } -> "DIFF"
  | Diff { dry = true; _ } -> "DIFF_DRY"

type validate_options_input = {
  text_source: text_source;
  range: range option;
  at_char: int option;
  inplace: bool;
  diff: bool;
  check: bool;
}

let validate_options
    env
    ( files,
      filename_for_logging,
      range,
      at_char,
      inplace,
      diff,
      diff_dry,
      check,
      config ) =
  let fail msg = raise (InvalidCliArg msg) in
  let multifile_allowed = inplace || check in
  let filename =
    match files with
    | [filename] -> Some filename
    | filename :: _ :: _ when multifile_allowed -> Some filename
    | [] -> None
    | _ -> fail "More than one file given"
  in
  let text_source =
    match (filename, filename_for_logging) with
    | (Some _, Some _) ->
      fail "Can't supply both a filename and a filename for logging"
    | (Some filename, None) -> File filename
    | (None, Some filename_for_logging) -> Stdin (Some filename_for_logging)
    | (None, None) -> Stdin None
  in
  let assert_file_exists = function
    | None -> ()
    | Some path ->
      if not (file_exists path) then fail ("No such file or directory: " ^ path)
  in
  assert_file_exists filename;
  assert_file_exists (Some env.Env.root);
  let files =
    if multifile_allowed then
      List.rev_filter files ~f:(fun path ->
          is_hack path
          ||
          (Printf.eprintf
             "Unexpected file extension (probably not a Hack file): %s\n"
             path;
           false))
    else (
      (match text_source with
      | File path
      | Stdin (Some path)
        when not (is_hack path) ->
        fail ("Unexpected file extension (probably not a Hack file): " ^ path)
      | _ -> ());
      files
    )
  in

  (* Let --diff-dry-run imply --diff *)
  let diff = diff || diff_dry in
  match { diff; inplace; text_source; range; at_char; check } with
  | _ when env.Env.debug && diff -> fail "Can't format diff in debug mode"
  | { diff = true; text_source = File _; _ }
  | { diff = true; text_source = Stdin (Some _); _ } ->
    fail "--diff mode expects no files"
  | { diff = true; range = Some _; _ } -> fail "--diff mode expects no range"
  | { diff = true; at_char = Some _; _ } ->
    fail "--diff mode can't format at-char"
  | { diff = true; check = true; _ } ->
    fail "Can't check formatting in --diff mode"
  | { inplace = true; text_source = Stdin _; _ } ->
    fail "Provide a filename to format in-place"
  | { inplace = true; range = Some _; _ } ->
    fail "Can't format a range in-place"
  | { inplace = true; at_char = Some _; _ } ->
    fail "Can't format at-char in-place"
  | { check = true; text_source = Stdin _; _ } ->
    fail "Provide a filename to check formatting"
  | { inplace = true; check = true; _ } ->
    fail "Can't check formatting in-place"
  | { check = true; range = Some _; _ } ->
    fail "Can't check formatting for a range"
  | { check = true; at_char = Some _; _ } ->
    fail "Can't check formatting at-char"
  | { diff = false; inplace = false; range = Some _; at_char = Some _; _ } ->
    fail "--at-char expects no range"
  | { diff = false; inplace = false; at_char = None; check = false; _ } ->
    Print { text_source; range; config }
  | { diff = false; inplace = true; text_source = File _; range = None; _ } ->
    InPlace { files; config }
  | { diff = false; check = true; text_source = File _; range = None; _ } ->
    Check { files; config }
  | { diff = false; inplace = false; range = None; at_char = Some pos; _ } ->
    AtChar { text_source; pos; config }
  | { diff = true; text_source = Stdin None; range = None; _ } ->
    Diff { dry = diff_dry; config }

let read_stdin () =
  let buf = Buffer.create 256 in
  try
    while true do
      Buffer.add_string
        buf
        (Out_channel.(flush stdout);
         In_channel.(input_line_exn stdin));
      Buffer.add_char buf '\n'
    done;
    assert false
  with
  | End_of_file -> Buffer.contents buf

let print_error source_text error =
  let text =
    SyntaxError.to_positioned_string
      error
      (SourceText.offset_to_position source_text)
  in
  Printf.eprintf "%s\n" text

let parse_text ~parser_env source_text =
  let parser_env =
    {
      parser_env with
      Full_fidelity_parser_env.mode =
        Full_fidelity_parser.parse_mode source_text;
    }
  in
  let tree = SyntaxTree.make ~env:parser_env source_text in
  if List.is_empty (SyntaxTree.all_errors tree) then
    tree
  else (
    List.iter (SyntaxTree.all_errors tree) ~f:(print_error source_text);
    raise Hackfmt_error.InvalidSyntax
  )

let read_file filename =
  SourceText.from_file @@ Relative_path.create Relative_path.Dummy filename

let parse ~parser_env text_source =
  let source_text =
    match text_source with
    | File filename -> read_file filename
    | Stdin _ -> SourceText.make Relative_path.default @@ read_stdin ()
  in
  parse_text ~parser_env source_text

let logging_time_taken env logger thunk =
  let start_t = Unix.gettimeofday () in
  let res = thunk () in
  let end_t = Unix.gettimeofday () in
  if not env.Env.test then
    logger
      ~start_t
      ~end_t
      ~mode:env.Env.mode
      ~file:(text_source_to_filename env.Env.text_source)
      ~root:env.Env.root;
  res

(* If the range is a byte range, expand it to line boundaries.
 * If the range is a line range, convert it to a byte range. *)
let expand_or_convert_range ?ranges source_text range =
  match range with
  | Byte range -> expand_to_line_boundaries ?ranges source_text range
  | Line (st, ed) ->
    let line_boundaries =
      match ranges with
      | Some ranges -> ranges
      | None -> get_line_boundaries (SourceText.text source_text)
    in
    let st = max st 1 in
    let ed = min ed (Array.length line_boundaries) in
    (try line_interval_to_offset_range line_boundaries (st, ed) with
    | Invalid_argument msg -> raise (InvalidCliArg msg))

let format ?config ?range ?ranges env tree =
  let source_text = SyntaxTree.text tree in
  match range with
  | None ->
    logging_time_taken env Logger.format_tree_end (fun () ->
        Libhackfmt.format_tree ?config tree)
  | Some range ->
    let range = expand_or_convert_range ?ranges source_text range in
    logging_time_taken env Logger.format_range_end (fun () ->
        let formatted = Libhackfmt.format_range ?config range tree in
        (* This is a bit of a hack to deal with situations where a newline exists
         * in the original source in a location where hackfmt refuses to split,
         * and the range end falls at that newline. It is correct for format_range
         * not to add the trailing newline, but it looks better to add an
         * incorrect newline than to omit it, which would cause the following line
         * (along with its indentation spaces) to be joined with the last line in
         * the range. See test case: binary_expression_range_formatting.php *)
        if Char.equal formatted.[String.length formatted - 1] '\n' then
          formatted
        else
          formatted ^ "\n")

let output ?text_source str =
  let with_out_channel f =
    match text_source with
    | Some (File filename) ->
      let out_channel = Out_channel.create filename in
      f out_channel;
      Out_channel.close out_channel
    | Some (Stdin _)
    | None ->
      f stdout
  in
  with_out_channel (fun out_channel -> fprintf out_channel "%s%!" str)

let format_diff_intervals ?config env intervals tree =
  try
    logging_time_taken env Logger.format_intervals_end (fun () ->
        Libhackfmt.format_intervals ?config intervals tree)
  with
  | Invalid_argument s -> raise (InvalidDiff s)

let debug_print ?range ?config text_source =
  let tree = parse ~parser_env:Full_fidelity_parser_env.default text_source in
  let source_text = SyntaxTree.text tree in
  let range = Option.map range ~f:(expand_or_convert_range source_text) in
  let env = Libhackfmt.env_from_config config in
  let doc =
    Hack_format.transform env (SyntaxTransforms.editable_from_positioned tree)
  in
  let chunk_groups = Chunk_builder.build env doc in
  Hackfmt_debug.debug env ~range source_text doc chunk_groups

let main
    (env : Env.t)
    (options : format_options)
    (parser_env : Full_fidelity_parser_env.t) =
  env.Env.mode <- Some (mode_string options);
  match options with
  | Print { text_source; range; config } ->
    env.Env.text_source <- text_source;
    if env.Env.debug then
      debug_print ?range ~config text_source
    else
      text_source |> parse ~parser_env |> format ?range ~config env |> output
  | InPlace { files; config } ->
    List.iter files ~f:(fun filename ->
        let text_source = File filename in
        env.Env.text_source <- text_source;
        if env.Env.debug then debug_print ~config text_source;
        let source_text = read_file filename in
        let source_text_string = SourceText.text source_text in
        try
          let formatted_text =
            text_source |> parse ~parser_env |> format ~config env
          in
          if String.(source_text_string <> formatted_text) then
            output ~text_source formatted_text
        with
        | InvalidSyntax -> eprintf "Parse error in file: %s\n%!" filename)
  | Check { files; config } ->
    List.iter files ~f:(fun filename ->
        let text_source = File filename in
        env.Env.text_source <- text_source;
        let source_text = read_file filename in
        let source_text_string = SourceText.text source_text in
        try
          let formatted_text =
            text_source |> parse ~parser_env |> format ~config env
          in
          if String.(source_text_string <> formatted_text) then
            printf "%s\n" filename
        with
        | InvalidSyntax -> eprintf "Parse error in file: %s\n%!" filename)
  | AtChar { text_source; pos; config } ->
    env.Env.text_source <- text_source;
    let tree = parse ~parser_env text_source in
    let (range, formatted) =
      try
        logging_time_taken env Logger.format_at_offset_end (fun () ->
            Libhackfmt.format_at_offset ~config tree pos)
      with
      | Invalid_argument s -> raise (InvalidCliArg s)
    in
    if env.Env.debug then debug_print text_source ~range:(Byte range) ~config;
    Printf.printf "%d %d\n" (fst range) (snd range);
    output formatted
  | Diff { dry; config } ->
    let root = Path.make env.Env.root in
    read_stdin ()
    |> Parse_diff.go
    |> List.filter ~f:(fun (rel_path, _intervals) -> is_hack rel_path)
    |> List.filter_map ~f:(fun (rel_path, intervals) ->
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
               |> parse ~parser_env
               |> format_diff_intervals ~config env intervals
             in
             Some (text_source, rel_path, contents)
           with
           (* A parse error isn't a signal that there's something wrong with the
            * diff--there's just something wrong with that file. We can leave that
            * file alone and move on. *)
           | InvalidSyntax ->
             Printf.eprintf "Parse error in file: %s\n%!" rel_path;
             None)
    |> List.iter ~f:(fun (text_source, rel_path, contents) ->
           (* Log this filename in the event of an exception. *)
           env.Env.text_source <- text_source;
           if dry then printf "*** %s\n" rel_path;
           let output_text_source =
             if dry then
               Stdin None
             else
               text_source
           in
           output ~text_source:output_text_source contents)

let () =
  (* no-op, needed at entry point for the Daemon module (used by
     HackfmtEventLogger) to behave correctly *)
  Daemon.check_entry_point ();
  Folly.ensure_folly_init ();

  let (options, root, parser_env, (debug, test)) = parse_options () in
  let env =
    {
      Env.debug;
      test;
      mode = None;
      text_source = Stdin None;
      root = Path.to_string root;
    }
  in
  let start_time = Unix.gettimeofday () in
  if not env.Env.test then Logger.init start_time;

  let (err_msg, exit_code) =
    try
      let options = validate_options env options in
      main env options parser_env;
      (None, 0)
    with
    | exn ->
      let exit_code = get_exception_exit_value exn in
      if exit_code = 255 then Printexc.print_backtrace stderr;
      let err_str = get_error_string_from_exn exn in
      let err_msg =
        match exn with
        | InvalidSyntax -> err_str
        | InvalidCliArg s
        | InvalidDiff s ->
          err_str ^ ": " ^ s
        | _ -> err_str ^ ": " ^ Exn.to_string exn
      in
      (Some err_msg, exit_code)
  in
  (if not env.Env.test then
    let time_taken = Unix.gettimeofday () -. start_time in
    Logger.exit
      ~time_taken
      ~error:err_msg
      ~exit_code:(Some exit_code)
      ~mode:env.Env.mode
      ~file:(text_source_to_filename env.Env.text_source)
      ~root:env.Env.root);

  Option.iter err_msg ~f:(eprintf "%s\n");
  exit exit_code
