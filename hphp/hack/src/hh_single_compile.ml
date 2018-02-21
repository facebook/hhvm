(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_core
open Sys_utils

module P = Printf
module SyntaxError = Full_fidelity_syntax_error
module SourceText = Full_fidelity_source_text
module Logger = HackcEventLogger

(*****************************************************************************)
(* Types, constants *)
(*****************************************************************************)
type parser =
  | Legacy
  | FFP

type mode =
  | CLI
  | DAEMON

type options = {
  filename         : string;
  fallback         : bool;
  config_list      : string list;
  debug_time       : bool;
  parser           : parser;
  output_file      : string option;
  config_file      : string option;
  quiet_mode       : bool;
  mode             : mode;
  input_file_list  : string option;
  dump_symbol_refs : bool;
  dump_stats       : bool;
  dump_config      : bool;
  log_stats        : bool;
}

type message_handler = Hh_json.json -> string -> unit

type message_handlers = {
  set_config : message_handler;
  compile    : message_handler;
  error      : message_handler;
}

(*****************************************************************************)
(* Debug info refs *)
(*****************************************************************************)

type debug_time = {
  parsing_t: float ref;
  codegen_t: float ref;
  printing_t: float ref;
}

let new_debug_time () =
{
  parsing_t = ref 0.0;
  codegen_t = ref 0.0;
  printing_t = ref 0.0;
}

(*****************************************************************************)
(* Helpers *)
(*****************************************************************************)

let die str =
  prerr_endline str;
  exit 2

let is_file_path_for_evaled_code s =
  let s = Relative_path.to_absolute s in
  String_utils.string_ends_with s ") : eval()'d code"

let print_compiler_version () =
  let open Hh_json in
  let compiler_version_msg = json_to_string @@ JSON_Object
    [ ("type", JSON_String "compiler_version")
    ; ("version", JSON_String (Compiler_id.get_compiler_id ()))
    ] in
  P.printf "%s\n%!" compiler_version_msg

let assert_regular_file filename =
  if not (Sys.file_exists filename) ||
    (Unix.stat filename).Unix.st_kind <> Unix.S_REG
  then raise (Arg.Bad (filename ^ " not a valid file"))

let parse_options () =
  let fn_ref = ref None in
  let want_version = ref false in
  let fallback = ref false in
  let debug_time = ref false in
  let parser = ref FFP in
  let config_list = ref [] in
  let mode = ref CLI in
  let output_file = ref None in
  let config_file = ref None in
  let quiet_mode = ref false in
  let input_file_list = ref None in
  let dump_symbol_refs = ref false in
  let dump_stats = ref false in
  let dump_config = ref false in
  let log_stats = ref true in
  let usage = P.sprintf "Usage: hh_single_compile (%s) filename\n" Sys.argv.(0) in
  let options =
    [ ("--version"
          , Arg.Set want_version
            , " print the version and do nothing"
      );
      ("--fallback"
          , Arg.Set fallback
            , " Enables fallback compilation"
      );
      ("--debug-time"
          , Arg.Set debug_time
            , " Enables debugging logging for elapsed time"
      );
      ("--quiet-mode"
          , Arg.Set quiet_mode
            , " Runs very quietly, and ignore any result if invoked without -o "
              ^ "(lower priority than the debug-time option)"
      );
      ("-v"
          , Arg.String (fun str -> config_list := str :: !config_list)
            , " Configuration: Eval.EnableHipHopSyntax=<value> "
              ^ "or Hack.Lang.IntsOverflowToInts=<value>"
              ^ "\n"
              ^ "\t\tAllows overriding config options passed on a file"
      );
      ("-c"
      , Arg.String (fun str ->
        assert_regular_file str;
        config_file := Some str)
      , " Config file in JSON format"
      );
      ("-o"
          , Arg.String (fun str -> output_file := Some str)
            , " Output file. Creates it if necessary"
      );
      ("--parser"
          , Arg.String
            (function "ffp" -> parser := FFP
              | "legacy" -> parser := Legacy
              | p -> failwith @@ p ^ " is an invalid parser")
            , " Parser: ffp or legacy [def: ffp]"
      );
      ("--daemon"
          , Arg.Unit (fun () -> mode := DAEMON)
            , " Run a daemon which processes Hack source from standard input"
      );
      ("--input-file-list"
          , Arg.String (fun str -> input_file_list := Some str)
            , " read a list of files (one per line) from the file `input-file-list'"
      );
      ("--dump-symbol-refs"
          , Arg.Set dump_symbol_refs
            , " Dump symbol ref sections of HHAS"
      );
      ("--dump-stats"
      , Arg.Set dump_stats
      , " Dump timing stats for functions"
      );
      ("--dump-config"
      , Arg.Set dump_config
      , " Dump configuration settings"
      );
      ("--stop-logging-stats"
      , Arg.Unit (fun () -> log_stats := false)
      , " Stop logging stats"
      );
    ] in
  let options = Arg.align ~limit:25 options in
  Arg.parse options (fun fn -> fn_ref := Some fn) usage;
  if !want_version then (print_compiler_version (); exit 0);
  if !mode = DAEMON then print_compiler_version ();
  let needs_file = Option.is_none !input_file_list in
  let fn =
    if needs_file then
      match !fn_ref with
        | Some fn -> if !mode = CLI then fn else die usage
        | None    -> if !mode = CLI then die usage else read_line ()
    else
      ""
  in
  { filename           = fn
  ; fallback           = !fallback
  ; config_list        = !config_list
  ; debug_time         = !debug_time
  ; parser             = !parser
  ; output_file        = !output_file
  ; config_file        = !config_file
  ; quiet_mode         = !quiet_mode
  ; mode               = !mode
  ; input_file_list    = !input_file_list
  ; dump_symbol_refs   = !dump_symbol_refs
  ; dump_stats         = !dump_stats
  ; dump_config        = !dump_config
  ; log_stats          = !log_stats
  }

let fail_daemon file error =
  let open Hh_json in
  let file = Option.value ~default:"[unknown]" file in
  let msg = json_to_string @@ JSON_Object
    [ ("type", JSON_String "error")
    ; ("file", JSON_String file)
    ; ("error", JSON_String error)
    ] in
  P.printf "%s\n%!" msg;
  die error

let rec dispatch_loop handlers =
  let open Hh_json in
  let open Access in
  let read_message () =
    let line = read_line () in
    let header = json_of_string line in
    let file = get_field_opt (get_string "file") header in
    let bytes = get_field (get_number_int "bytes") (fun _af -> 0) header in
    let is_systemlib = get_field_opt (get_bool "is_systemlib") header in
    Emit_env.set_is_systemlib @@ Option.value ~default:false is_systemlib;
    let body = Bytes.create bytes in begin
    try
      really_input stdin body 0 bytes;
      header, body
    with exc ->
      fail_daemon file ("Exception reading message body: " ^ (Printexc.to_string exc))
    end in
  let header, body = read_message () in
  let msg_type = get_field
    (get_string "type")
    (fun af -> fail_daemon None ("Cannot determine type of message: " ^ af))
    header in
  (match msg_type with
    | "code"   -> handlers.compile header body
    | "error"  -> handlers.error header body
    | "config" -> handlers.set_config header body
    | _        -> fail_daemon None ("Unhandled message type '" ^ msg_type ^ "'"));
  dispatch_loop handlers

let set_stats_if_enabled ~compiler_options =
  if compiler_options.dump_stats then
    Stats_container.set_instance (Some (Stats_container.new_container ()))

let write_stats_if_enabled ~compiler_options =
  if compiler_options.dump_stats then
    match (Stats_container.get_instance ()) with
    | Some s -> Stats_container.write_out ~out:stderr s
    | None -> ()

let parse_text compiler_options popt fn text =
  let () = set_stats_if_enabled ~compiler_options in
  match compiler_options.parser with
  | FFP ->
    let ignore_pos =
      not (Hhbc_options.source_mapping !Hhbc_options.compiler_options) in
    let enable_hh_syntax =
      Hhbc_options.enable_hiphop_syntax !Hhbc_options.compiler_options in
    let php5_compat_mode =
      not (Hhbc_options.enable_uniform_variable_syntax !Hhbc_options.compiler_options) in
    let systemlib_compat_mode = Emit_env.is_systemlib () in
    let env = Full_fidelity_ast.make_env
      ~parser_options:popt
      ~ignore_pos
      ~codegen:true
      ~systemlib_compat_mode
      ~php5_compat_mode
      ~enable_hh_syntax
      fn
    in
    let parser_ret = Full_fidelity_ast.from_text_with_legacy env text in
    let () = write_stats_if_enabled ~compiler_options in
    parser_ret
  | Legacy ->
    Parser_hack.program popt fn text

let parse_file compiler_options popt filename text =
  try
    `ParseResult (Errors.do_ begin fun () ->
      parse_text compiler_options popt filename text
    end)
  with
    (* FFP failed to parse *)
    | Failure s -> `ParseFailure (SyntaxError.make 0 0 s)
    (* FFP generated an error *)
    | SyntaxError.ParserFatal e -> `ParseFailure e

let add_to_time_ref r t0 =
  let t = Unix.gettimeofday () in
  r := !r +. (t -. t0);
  t

let print_debug_time_info filename debug_time =
  let stat = Gc.stat () in
  (P.eprintf "File %s:\n" (Relative_path.to_absolute filename);
  P.eprintf "Parsing: %0.3f s\n" !(debug_time.parsing_t);
  P.eprintf "Codegen: %0.3f s\n" !(debug_time.codegen_t);
  P.eprintf "Printing: %0.3f s\n" !(debug_time.printing_t);
  P.eprintf "MinorWords: %0.3f\n" stat.Gc.minor_words;
  P.eprintf "PromotedWords: %0.3f\n" stat.Gc.promoted_words)

let mode_to_string = function
  | CLI -> "CLI"
  | DAEMON -> "DAEMON"

let log_success compiler_options filename debug_time =
  Logger.success
    ~filename:(Relative_path.to_absolute filename)
    ~parsing_t:!(debug_time.parsing_t)
    ~codegen_t:!(debug_time.codegen_t)
    ~printing_t:!(debug_time.printing_t)
    ~mode:(mode_to_string compiler_options.mode)

let log_fail compiler_options filename exc =
  Logger.fail
    ~filename:(Relative_path.to_absolute filename)
    ~mode:(mode_to_string compiler_options.mode)
    ~exc:(Printexc.to_string exc ^ "\n" ^ Printexc.get_backtrace ())


let do_compile filename compiler_options text fail_or_ast debug_time =
  let t = Unix.gettimeofday () in
  let t = add_to_time_ref debug_time.parsing_t t in
  let hhas_prog =
    match fail_or_ast with
    | `ParseFailure e ->
      let error_t = match SyntaxError.error_type e with
        | SyntaxError.ParseError -> Hhbc_ast.FatalOp.Parse
        | SyntaxError.RuntimeError -> Hhbc_ast.FatalOp.Runtime
      in
      let s = SyntaxError.message e in
      let source_text = SourceText.make filename text in
      let pos =
        SourceText.relative_pos filename source_text
          (SyntaxError.start_offset e) (SyntaxError.end_offset e) in
      Emit_program.emit_fatal_program ~ignore_message:false error_t pos s
    | `ParseResult (errors, parser_return) ->
      let ast = parser_return.Parser_hack.ast in
      List.iter (Errors.get_error_list errors) (fun e ->
        P.eprintf "%s\n%!" (Errors.to_string (Errors.to_absolute e)));
      if Errors.is_empty errors
      then Emit_program.from_ast
        parser_return.Parser_hack.is_hh_file
        (is_file_path_for_evaled_code filename)
        ast
      else Emit_program.emit_fatal_program ~ignore_message:true
        Hhbc_ast.FatalOp.Parse Pos.none "Syntax error"
      in
  let t = add_to_time_ref debug_time.codegen_t t in
  let hhas_text = Hhbc_hhas.to_string
    ~path:filename
    ~dump_symbol_refs:compiler_options.dump_symbol_refs
    hhas_prog in
  ignore @@ add_to_time_ref debug_time.printing_t t;
  if compiler_options.debug_time
  then print_debug_time_info filename debug_time;
  if compiler_options.log_stats
  then log_success compiler_options filename debug_time;
  hhas_text


(*****************************************************************************)
(* Main entry point *)
(*****************************************************************************)

let process_single_source_unit compiler_options popt handle_output handle_exception filename source_text =
  try
    let t = Unix.gettimeofday () in
    let fail_or_ast = parse_file compiler_options popt filename source_text in
    let debug_time = new_debug_time () in
    ignore @@ add_to_time_ref debug_time.parsing_t t;
    let output = do_compile filename compiler_options source_text fail_or_ast debug_time in
    handle_output filename output
  with exc ->
    if compiler_options.log_stats
    then log_fail compiler_options filename exc;
    handle_exception filename exc

let decl_and_run_mode compiler_options popt =
  let open Hh_json in
  let open Access in

  let set_compiler_options config_json =
    let options =
      Hhbc_options.get_options_from_config config_json compiler_options.config_list in
    Hhbc_options.set_compiler_options options in
  let ini_config_json =
    Option.map ~f:json_of_file compiler_options.config_file in

  set_compiler_options ini_config_json;
  let dumped_options = lazy (Hhbc_options.to_string !Hhbc_options.compiler_options) in
  Ident.track_names := true;

  match compiler_options.mode with
    | DAEMON ->
      let handle_output filename output =
        let abs_path = Relative_path.to_absolute filename in
        let bytes = String.length output in
        let msg = json_to_string @@ JSON_Object
          [ ("type", JSON_String "hhas")
          ; ("file", JSON_String abs_path)
          ; ("bytes", int_ bytes)
          ] in
        P.printf "%s\n%s%!" msg output in
      let handle_exception filename exc =
        let abs_path = Relative_path.to_absolute filename in
        let msg = json_to_string @@ JSON_Object
          [ ("type", JSON_String "error")
          ; ("file", JSON_String abs_path)
          ; ("error", JSON_String (Printexc.to_string exc))
          ] in
        P.printf "%s\n%!" msg in
      let handlers =
        { set_config = (fun _header body ->
          let config_json =
            if body = "" then None else Some (json_of_string body) in
          set_compiler_options config_json)
        ; error = (fun header _body ->
          let filename = get_field
            (get_string "file")
            (fun af -> fail_daemon None ("Cannot determine file name of source unit: " ^ af))
            header in
          let error = get_field
            (get_string "error")
            (fun _af -> fail_daemon (Some filename) ("No 'error' field in error message"))
            header in
          fail_daemon (Some filename) ("Error processing " ^ filename ^ ": " ^ error))
        ; compile = (fun header body ->
          let filename = get_field
            (get_string "file")
            (fun af -> fail_daemon None ("Cannot determine file name of source unit: " ^ af))
            header in
          process_single_source_unit
            compiler_options
            popt
            handle_output
            handle_exception
            (Relative_path.create Relative_path.Dummy filename)
            body)
        } in
      dispatch_loop handlers

    | CLI ->
      let handle_exception filename exc =
        if not compiler_options.quiet_mode
        then
          P.eprintf "Error in file %s: %s\n"
            (Relative_path.to_absolute filename)
            (Printexc.to_string exc) in

      let process_single_file handle_output filename =
        let filename = Relative_path.create Relative_path.Dummy filename in
        let abs_path = Relative_path.to_absolute filename in
        process_single_source_unit
          compiler_options popt handle_output handle_exception filename (cat abs_path) in

      let filenames, handle_output = match compiler_options.input_file_list with
        (* List of source files explicitly given *)
        | Some input_file_list ->
          let get_lines_in_file filename =
            let inch = open_in filename in
            let rec go lines =
              try
                let line = input_line inch |> String.trim in
                go (line :: lines)
              with End_of_file -> lines in
            go [] in
          let handle_output _filename output =
            if compiler_options.dump_config then
              Printf.printf "===CONFIG===\n%s\n\n%!" (Lazy.force dumped_options);
            if not compiler_options.quiet_mode then P.printf "%s%!" output
          in get_lines_in_file input_file_list, handle_output

        | None ->
          if Sys.is_directory compiler_options.filename

          (* Compile every file under directory *)
          then
            let files_in_dir =
              let rec go dirs = match dirs with
                | [] -> []
                | dir :: dirs ->
                  let ds, fs = Sys.readdir dir
                |> Array.map (Filename.concat dir)
                |> Array.to_list
                |> List.partition_tf ~f: Sys.is_directory in
                  fs @ go (ds @ dirs) in
              go [compiler_options.filename] in
            let handle_output filename output =
              let abs_path = Relative_path.to_absolute filename in
              if Filename.check_suffix abs_path ".php" then
                let output_file = Filename.chop_suffix abs_path ".php" ^ ".hhas" in
                if Sys.file_exists output_file
                then (
                  if not compiler_options.quiet_mode
                  then P.fprintf stderr "Output file %s already exists\n" output_file)
                else
                  Sys_utils.write_file ~file:output_file output
            in files_in_dir, handle_output

          (* Compile a single file *)
          else
            let handle_output _filename output =
              match compiler_options.output_file with
                | Some output_file ->
                  Sys_utils.write_file ~file:output_file output
                | None ->
                  if compiler_options.dump_config then
                    Printf.printf "===CONFIG===\n%s\n\n%!" (Lazy.force dumped_options);
                  if not compiler_options.quiet_mode then P.printf "%s%!" output
            in [compiler_options.filename], handle_output

      (* Actually execute the compilation(s) *)
      in List.iter filenames (process_single_file handle_output)

let main_hack opts =
  let popt = ParserOptions.default in
  let start_time = Unix.gettimeofday () in
  if opts.log_stats then Logger.init start_time;
  let _handle = SharedMem.init GlobalConfig.default_sharedmem_config in
  let tmp_hhi = Path.concat (Path.make Sys_utils.temp_dir_name) "hhi" in
  Hhi.set_hhi_root_for_unit_test tmp_hhi;
  decl_and_run_mode opts popt

(* command line driver *)
let _ =
  Printexc.record_backtrace true;
  try
    if ! Sys.interactive
    then ()
    else
    (* On windows, setting 'binary mode' avoids to output CRLF on
       stdout.  The 'text mode' would not hurt the user in general, but
       it breaks the testsuite where the output is compared to the
       expected one (i.e. in given file without CRLF). *)
      set_binary_mode_out stdout true;
    let options = parse_options () in
    main_hack options
  with exc ->
    Printexc.get_backtrace () |> prerr_endline;
    die (Printexc.to_string exc)
