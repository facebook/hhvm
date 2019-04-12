(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Sys_utils

module P = Printf
module SyntaxError = Full_fidelity_syntax_error
module SourceText = Full_fidelity_source_text
module SyntaxTree = Full_fidelity_syntax_tree
  .WithSyntax(Full_fidelity_positioned_syntax)
module Lex = Full_fidelity_lexer
module Logger = HackcEventLogger

(*****************************************************************************)
(* Types, constants *)
(*****************************************************************************)
type mode =
  | CLI
  | DAEMON

type options = {
  filename          : string;
  fallback          : bool;
  config_list       : string list;
  debug_time        : bool;
  output_file       : string option;
  config_file       : string option;
  quiet_mode        : bool;
  mode              : mode;
  input_file_list   : string option;
  dump_symbol_refs  : bool;
  dump_stats        : bool;
  dump_config       : bool;
  extract_facts     : bool;
  log_stats         : bool;
  for_debugger_eval : bool;
}

type message_handler = Hh_json.json -> string -> unit

type message_handlers = {
  set_config : message_handler;
  compile    : message_handler;
  facts      : message_handler;
  parse      : message_handler;
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
  let config_list = ref [] in
  let mode = ref CLI in
  let output_file = ref None in
  let config_file = ref None in
  let quiet_mode = ref false in
  let input_file_list = ref None in
  let dump_symbol_refs = ref false in
  let extract_facts = ref false in
  let dump_stats = ref false in
  let dump_config = ref false in
  let log_stats = ref false in
  let for_debugger_eval = ref false in
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
      ("--facts"
          , Arg.Set extract_facts
          , "Extract facts from the source code."
      );
      ("-v"
          , Arg.String (fun str -> config_list := str :: !config_list)
            , " Configuration: Eval.EnableHipHopSyntax=<value> "
              ^ "or Hack.Lang.EnableConcurrent=<value>"
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
      ("--enable-logging-stats"
      , Arg.Unit (fun () -> log_stats := true)
      , " Starts logging stats"
      );
      ("--stop-logging-stats"
      , Arg.Unit (fun () -> log_stats := false)
      , " Stop logging stats"
      );
      ("--for-debugger-eval"
      , Arg.Unit (fun () -> for_debugger_eval := true)
      , " Mutate the program as if we're in the debugger repl"
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
        | None    -> if !mode = CLI then die usage else Caml.read_line ()
    else
      ""
  in
  { filename           = fn
  ; fallback           = !fallback
  ; config_list        = !config_list
  ; debug_time         = !debug_time
  ; output_file        = !output_file
  ; config_file        = !config_file
  ; quiet_mode         = !quiet_mode
  ; mode               = !mode
  ; input_file_list    = !input_file_list
  ; dump_symbol_refs   = !dump_symbol_refs
  ; dump_stats         = !dump_stats
  ; dump_config        = !dump_config
  ; log_stats          = !log_stats
  ; extract_facts      = !extract_facts
  ; for_debugger_eval  = !for_debugger_eval
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
    let line = Caml.read_line () in
    let header = json_of_string line in
    let file = get_field_opt (get_string "file") header in
    let bytes = get_field (get_number_int "bytes") (fun _af -> 0) header in
    let is_systemlib = get_field_opt (get_bool "is_systemlib") header in
    Emit_env.set_is_systemlib @@ Option.value ~default:false is_systemlib;
    let body = Bytes.create bytes in begin
    try
      Caml.really_input Caml.stdin body 0 bytes;
      header, body
    with exc ->
      fail_daemon file ("Exception reading message body: " ^ (Caml.Printexc.to_string exc))
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
    | "facts"  -> handlers.facts header body
    | "parse"  -> handlers.parse header body
    | _        -> fail_daemon None ("Unhandled message type '" ^ msg_type ^ "'"));
  dispatch_loop handlers

let set_stats_if_enabled ~compiler_options =
  if compiler_options.dump_stats then
    Stats_container.set_instance (Some (Stats_container.new_container ()))

let write_stats_if_enabled ~compiler_options =
  if compiler_options.dump_stats then
    match (Stats_container.get_instance ()) with
    | Some s -> Stats_container.write_out ~out:Caml.stderr s
    | None -> ()

let parse_text compiler_options popt fn text =
  let () = set_stats_if_enabled ~compiler_options in
  let ignore_pos =
    not (Hhbc_options.source_mapping !Hhbc_options.compiler_options) in
  let enable_hh_syntax =
    Hhbc_options.enable_hiphop_syntax !Hhbc_options.compiler_options in
  let enable_xhp =
    Hhbc_options.enable_xhp !Hhbc_options.compiler_options in
  let php5_compat_mode =
    not (Hhbc_options.enable_uniform_variable_syntax !Hhbc_options.compiler_options) in
  let hacksperimental =
    Hhbc_options.hacksperimental !Hhbc_options.compiler_options in
  let lower_coroutines =
    Hhbc_options.enable_coroutines !Hhbc_options.compiler_options in
  let pocket_universes =
    Hhbc_options.enable_pocket_universes !Hhbc_options.compiler_options in
  let systemlib_compat_mode = Emit_env.is_systemlib () in
  let popt = ParserOptions.setup_pocket_universes popt pocket_universes in
  let env = Full_fidelity_ast.make_env
    ~parser_options:popt
    ~ignore_pos
    ~codegen:true
    ~fail_open:false
    ~systemlib_compat_mode
    ~php5_compat_mode
    ~enable_hh_syntax
    ~enable_xhp
    ~hacksperimental
    ~keep_errors:false
    ~lower_coroutines
    fn
  in
  let source_text = SourceText.make fn text in
  let { Full_fidelity_ast.ast; Full_fidelity_ast.is_hh_file; _ } =
    Full_fidelity_ast.from_text env source_text in
  let () = write_stats_if_enabled ~compiler_options in
  let ast = if pocket_universes then
      Pocket_universes.translate ast
    else ast in
  (ast, is_hh_file)

let parse_file compiler_options popt filename text =
  try
    `ParseResult (Errors.do_ begin fun () ->
      parse_text compiler_options popt filename text
    end)
  with
    (* FFP failed to parse *)
    | Failure s -> `ParseFailure ((SyntaxError.make 0 0 s), Pos.none)
    (* FFP generated an error *)
    | SyntaxError.ParserFatal (e, p) -> `ParseFailure (e, p)

let add_to_time_ref r t0 =
  let t = Unix.gettimeofday () in
  r := !r +. (t -. t0);
  t

let print_debug_time_info filename debug_time =
  let stat = Caml.Gc.stat () in
  (P.eprintf "File %s:\n" (Relative_path.to_absolute filename);
  P.eprintf "Parsing: %0.3f s\n" !(debug_time.parsing_t);
  P.eprintf "Codegen: %0.3f s\n" !(debug_time.codegen_t);
  P.eprintf "Printing: %0.3f s\n" !(debug_time.printing_t);
  P.eprintf "MinorWords: %0.3f\n" stat.Caml.Gc.minor_words;
  P.eprintf "PromotedWords: %0.3f\n" stat.Caml.Gc.promoted_words)

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

let log_fail compiler_options filename exc ~stack =
  Logger.fail
    ~filename:(Relative_path.to_absolute filename)
    ~mode:(mode_to_string compiler_options.mode)
    ~exc:(Caml.Printexc.to_string exc ^ "\n" ^ stack)

let do_compile filename compiler_options popt fail_or_ast debug_time =
  let t = Unix.gettimeofday () in
  let t = add_to_time_ref debug_time.parsing_t t in
  let hhas_prog =
    match fail_or_ast with
    | `ParseFailure (e, pos) ->
      let error_t = match SyntaxError.error_type e with
        | SyntaxError.ParseError -> Hhbc_ast.FatalOp.Parse
        | SyntaxError.RuntimeError -> Hhbc_ast.FatalOp.Runtime
      in
      let s = SyntaxError.message e in
      Emit_program.emit_fatal_program ~ignore_message:false error_t pos s
    | `ParseResult (errors, (ast, is_hh_file)) ->
      List.iter (Errors.get_error_list errors) (fun e ->
        P.eprintf "%s\n%!" (Errors.to_string (Errors.to_absolute e)));
      if Errors.is_empty errors
      then
        let is_js_file = Filename.check_suffix (Relative_path.to_absolute filename) "js" in
        Emit_program.from_ast
          ~is_hh_file
          ~is_js_file
          ~is_evaled:(is_file_path_for_evaled_code filename)
          ~for_debugger_eval:(compiler_options.for_debugger_eval)
          ~popt
          ast
      else Emit_program.emit_fatal_program ~ignore_message:true
        Hhbc_ast.FatalOp.Parse Pos.none "Syntax error"
      in
  let t = add_to_time_ref debug_time.codegen_t t in
  let hhas = Hhbc_hhas.to_segments
    ~path:filename
    ~dump_symbol_refs:compiler_options.dump_symbol_refs
    hhas_prog in
  ignore @@ add_to_time_ref debug_time.printing_t t;
  if compiler_options.debug_time
  then print_debug_time_info filename debug_time;
  if compiler_options.log_stats
  then log_success compiler_options filename debug_time;
  hhas

let extract_facts ?pretty ~filename ~source_root text =
  let json_facts = match Hackc_parse_delegator.extract_facts filename source_root with
    | Some result -> Some result
    | None ->
      let enable_hh_syntax =
        Hhbc_options.enable_hiphop_syntax !Hhbc_options.compiler_options in
      let enable_xhp =
        Hhbc_options.enable_xhp !Hhbc_options.compiler_options in
      Facts_parser.extract_as_json
        ~php5_compat_mode:true
        ~hhvm_compat_mode:true
        ~force_hh:enable_hh_syntax
        ~enable_xhp
        ~filename ~text
  in
  (* return empty string if file has syntax errors *)
  Option.value_map ~default:"" ~f:(Hh_json.json_to_string ?pretty) json_facts
  |> fun x -> [x]

let parse_hh_file filename body =
  let file = Relative_path.create Relative_path.Dummy filename in
  let source_text = SourceText.make file body in
  let syntax_tree = SyntaxTree.make source_text in
  let json = SyntaxTree.to_json syntax_tree in
  [Hh_json.json_to_string json]

(*****************************************************************************)
(* Main entry point *)
(*****************************************************************************)

let make_popt () =
  let open Hhbc_options in
  let co = !compiler_options in
  ParserOptions.make
    ~auto_namespace_map:(aliased_namespaces co)
    ~disallow_execution_operator:(phpism_disallow_execution_operator co)
    ~enable_concurrent:(enable_concurrent co)
    ~enable_await_as_an_expression:(enable_await_as_an_expression co)
    ~disable_nontoplevel_declarations:(phpism_disable_nontoplevel_declarations co)
    ~disable_static_closures:(phpism_disable_static_closures co)
    ~enable_hh_syntax_for_hhvm:(enable_hiphop_syntax co)
    ~enable_stronger_await_binding:(enable_stronger_await_binding co)
    ~disable_lval_as_an_expression:(disable_lval_as_an_expression co)
    ~disable_instanceof:(phpism_disable_instanceof co)

let process_single_source_unit compiler_options
  handle_output handle_exception filename source_text source_root =
  try
    let popt = make_popt () in
    let debug_time = new_debug_time () in
    let t = Unix.gettimeofday () in
    let output =
      if compiler_options.extract_facts
      then extract_facts ~pretty:true ~filename ~source_root source_text
      else begin
        let fail_or_ast =
          match Hackc_parse_delegator.parse_file filename source_text source_root with
          | Some fail_or_ast -> fail_or_ast
          | None -> parse_file compiler_options popt filename source_text
        in
        ignore @@ add_to_time_ref debug_time.parsing_t t;
        do_compile filename compiler_options popt fail_or_ast debug_time
      end in
    handle_output filename output debug_time
  with exc ->
    let stack = Caml.Printexc.get_backtrace () in
    if compiler_options.log_stats
    then log_fail compiler_options filename exc ~stack;
    handle_exception filename exc

let decl_and_run_mode compiler_options =
  let open Hh_json in
  let open Access in

  let print_and_flush_strings strings =
    List.iter ~f:(P.printf "%s") strings;
    P.printf "%!" in

  let set_compiler_options config_json =
    let options =
      Hhbc_options.get_options_from_config
        config_json
        ~init:!Hhbc_options.compiler_options
        ~config_list:compiler_options.config_list
    in
    Hhbc_options.set_compiler_options options in
  let ini_config_json =
    Option.map ~f:json_of_file compiler_options.config_file in

  set_compiler_options ini_config_json;
  let dumped_options = lazy (Hhbc_options.to_string !Hhbc_options.compiler_options) in
  Ident.track_names := true;

  match compiler_options.mode with
    | DAEMON ->
      let handle_output filename output debug_time =
        let abs_path = Relative_path.to_absolute filename in
        let bytes =
          List.fold ~f:(fun len s -> len + String.length s) ~init:0 output in
        let msg =
          [ ("type", JSON_String "success")
          ; ("file", JSON_String abs_path)
          ; ("bytes", int_ bytes)
          ] in
        let msg =
          if Hhbc_options.enable_perf_logging !Hhbc_options.compiler_options
          then
            let json_microsec t = int_ @@ int_of_float @@ t *. 1000000.0 in
               ("parsing_time", json_microsec !(debug_time.parsing_t))
            :: ("codegen_time", json_microsec !(debug_time.codegen_t))
            :: ("printing_time", json_microsec !(debug_time.printing_t))
            :: msg
          else msg
        in
        P.printf "%s\n" (json_to_string @@ JSON_Object msg);
        print_and_flush_strings output in
      let handle_exception filename exc =
        let abs_path = Relative_path.to_absolute filename in
        let msg = json_to_string @@ JSON_Object
          [ ("type", JSON_String "error")
          ; ("file", JSON_String abs_path)
          ; ("error", JSON_String (Caml.Printexc.to_string exc))
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
          let for_debugger_eval = get_field
            (get_bool "for_debugger_eval")
            (fun af -> fail_daemon None ("for_debugger_eval flag missing: " ^ af))
            header in
          let old_config = !Hhbc_options.compiler_options in
          let config_overrides = get_field
            (get_obj "config_overrides")
            (fun _af -> JSON_Object [])
            header in
          let source_root = get_field_opt (get_string "root") header in
          set_compiler_options (Some config_overrides);
          let compiler_options = { compiler_options with for_debugger_eval } in
          let result = process_single_source_unit
            compiler_options
            handle_output
            handle_exception
            (Relative_path.create Relative_path.Dummy filename)
            body
            source_root in
          Hhbc_options.set_compiler_options old_config;
          result)
        ; facts = (fun header body -> (
          (* if body is empty - read file from disk *)
          let filename = get_field
            (get_string "file")
            (fun af -> fail_daemon None ("Cannot determine file name of source unit: " ^ af))
            header in
          let source_root = get_field_opt (get_string "root") header in
          let body =
            if String.length body = 0
            then Sys_utils.cat filename
            else body in
          let path = Relative_path.create Relative_path.Dummy filename in
          handle_output
            path
            (extract_facts ~filename:path ~source_root body))
            (new_debug_time ()))
        ; parse = (fun header body -> (
          let filename = get_field
            (get_string "file")
            (fun af -> fail_daemon None ("Cannot determine file name of source unit: " ^ af))
            header in
          let body =
            if String.length body = 0
            then Sys_utils.cat filename
            else body in
          handle_output
            (Relative_path.create Relative_path.Dummy filename)
            (parse_hh_file filename body))
            (new_debug_time ()))
        } in
      dispatch_loop handlers

    | CLI ->
      let handle_exception filename exc =
        if not compiler_options.quiet_mode
        then
          P.eprintf "Error in file %s: %s\n"
            (Relative_path.to_absolute filename)
            (Caml.Printexc.to_string exc) in

      let process_single_file handle_output filename =
        let filename = Relative_path.create Relative_path.Dummy filename in
        let abs_path = Relative_path.to_absolute filename in
        process_single_source_unit
          compiler_options handle_output handle_exception filename (cat abs_path) None in

      let filenames, handle_output = match compiler_options.input_file_list with
        (* List of source files explicitly given *)
        | Some input_file_list ->
          let get_lines_in_file filename =
            let inch = Caml.open_in filename in
            let rec go lines =
              match Caml.input_line inch with
              | line -> go (Caml.String.trim line :: lines)
              | exception End_of_file -> lines in
            go [] in
          let handle_output _filename output _debug_time =
            if compiler_options.dump_config then
              Printf.printf "===CONFIG===\n%s\n\n%!" (Lazy.force dumped_options);
            if not compiler_options.quiet_mode then
              print_and_flush_strings output
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
                |> Array.map ~f:(Filename.concat dir)
                |> Array.to_list
                |> List.partition_tf ~f: Sys.is_directory in
                  fs @ go (ds @ dirs) in
              go [compiler_options.filename] in
            let handle_output filename output _debug_time =
              let abs_path = Relative_path.to_absolute filename in
              if Filename.check_suffix abs_path ".php" then
                let output_file = Filename.chop_suffix abs_path ".php" ^ ".hhas" in
                if Sys.file_exists output_file
                then (
                  if not compiler_options.quiet_mode
                  then Caml.Printf.fprintf Caml.stderr "Output file %s already exists\n" output_file)
                else
                  Sys_utils.write_strings_to_file ~file:output_file output
            in files_in_dir, handle_output

          (* Compile a single file *)
          else
            let handle_output _filename output _debug_time =
              match compiler_options.output_file with
                | Some output_file ->
                  Sys_utils.write_strings_to_file ~file:output_file output
                | None ->
                  if compiler_options.dump_config then
                    Printf.printf "===CONFIG===\n%s\n\n%!" (Lazy.force dumped_options);
                  if not compiler_options.quiet_mode then
                    print_and_flush_strings output
            in [compiler_options.filename], handle_output

      (* Actually execute the compilation(s) *)
      in List.iter filenames (process_single_file handle_output)

let main_hack opts =
  let start_time = Unix.gettimeofday () in
  if opts.log_stats then Logger.init start_time;
  decl_and_run_mode opts

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
      Caml.set_binary_mode_out Caml.stdout true;
    let handle = SharedMem.init ~num_workers:0 GlobalConfig.default_sharedmem_config in
    ignore (handle: SharedMem.handle);
    let options = parse_options () in
    main_hack options
  with exc ->
    let stack = Caml.Printexc.get_backtrace () in
    prerr_endline stack;
    die (Caml.Printexc.to_string exc)
