(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
module P = Printf
module SyntaxError = Full_fidelity_syntax_error
module SourceText = Full_fidelity_source_text
module SyntaxTree =
  Full_fidelity_syntax_tree.WithSyntax (Full_fidelity_positioned_syntax)
module Logger = HackcEventLogger

(*****************************************************************************)
(* Types, constants *)
(*****************************************************************************)
type mode =
  | CLI
  | DAEMON

type options = {
  filename: string;
  fallback: bool;
  config_list: string list;
  debug_time: bool;
  output_file: string option;
  config_file: string option;
  quiet_mode: bool;
  mode: mode;
  input_file_list: string option;
  dump_symbol_refs: bool;
  dump_config: bool;
  extract_facts: bool;
  log_stats: bool;
  for_debugger_eval: bool;
  disable_toplevel_elaboration: bool;
}

type message_handler = Hh_json.json -> string -> unit

type message_handlers = {
  set_config: message_handler;
  compile: message_handler;
  facts: message_handler;
  parse: message_handler;
  error: message_handler;
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
  { parsing_t = ref 0.0; codegen_t = ref 0.0; printing_t = ref 0.0 }

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
  Hh_json.(
    let compiler_version_msg =
      json_to_string
      @@ JSON_Object
           [
             ("type", JSON_String "compiler_version");
             ("version", JSON_String (Compiler_id.get_compiler_id ()));
           ]
    in
    P.printf "%s\n%!" compiler_version_msg)

let assert_regular_file filename =
  if
    (not (Sys.file_exists filename))
    || (Unix.stat filename).Unix.st_kind <> Unix.S_REG
  then
    raise (Arg.Bad (filename ^ " not a valid file"))

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
  let dump_config = ref false in
  let log_stats = ref false in
  let for_debugger_eval = ref false in
  let disable_toplevel_elaboration = ref false in
  let usage =
    P.sprintf "Usage: hh_single_compile (%s) filename\n" Sys.argv.(0)
  in
  let options =
    [
      ("--version", Arg.Set want_version, " print the version and do nothing");
      ("--fallback", Arg.Set fallback, " Enables fallback compilation");
      ( "--debug-time",
        Arg.Set debug_time,
        " Enables debugging logging for elapsed time" );
      ( "--quiet-mode",
        Arg.Set quiet_mode,
        " Runs very quietly, and ignore any result if invoked without -o "
        ^ "(lower priority than the debug-time option)" );
      ("--facts", Arg.Set extract_facts, "Extract facts from the source code.");
      ( "-v",
        Arg.String (fun str -> config_list := str :: !config_list),
        " Configuration: Server.Port=<value> "
        ^ "\n"
        ^ "\t\tAllows overriding config options passed on a file" );
      ( "-c",
        Arg.String
          (fun str ->
            assert_regular_file str;
            config_file := Some str),
        " Config file in JSON format" );
      ( "-o",
        Arg.String (fun str -> output_file := Some str),
        " Output file. Creates it if necessary" );
      ( "--daemon",
        Arg.Unit (fun () -> mode := DAEMON),
        " Run a daemon which processes Hack source from standard input" );
      ( "--input-file-list",
        Arg.String (fun str -> input_file_list := Some str),
        " read a list of files (one per line) from the file `input-file-list'"
      );
      ( "--dump-symbol-refs",
        Arg.Set dump_symbol_refs,
        " Dump symbol ref sections of HHAS" );
      ("--dump-config", Arg.Set dump_config, " Dump configuration settings");
      ( "--enable-logging-stats",
        Arg.Unit (fun () -> log_stats := true),
        " Starts logging stats" );
      ( "--stop-logging-stats",
        Arg.Unit (fun () -> log_stats := false),
        " Stop logging stats" );
      ( "--for-debugger-eval",
        Arg.Unit (fun () -> for_debugger_eval := true),
        " Mutate the program as if we're in the debugger repl" );
      ( "--disable-toplevel-elaboration",
        Arg.Unit (fun () -> disable_toplevel_elaboration := true),
        "Disable toplevel definition elaboration" );
    ]
  in
  let options = Arg.align ~limit:25 options in
  Arg.parse options (fun fn -> fn_ref := Some fn) usage;
  if !want_version then (
    print_compiler_version ();
    exit 0
  );
  if !mode = DAEMON then print_compiler_version ();
  let needs_file = Option.is_none !input_file_list in
  let fn =
    if needs_file then
      match !fn_ref with
      | Some fn ->
        if !mode = CLI then
          fn
        else
          die usage
      | None ->
        if !mode = CLI then
          die usage
        else
          Caml.read_line ()
    else
      ""
  in
  {
    filename = fn;
    fallback = !fallback;
    config_list = !config_list;
    debug_time = !debug_time;
    output_file = !output_file;
    config_file = !config_file;
    quiet_mode = !quiet_mode;
    mode = !mode;
    input_file_list = !input_file_list;
    dump_symbol_refs = !dump_symbol_refs;
    dump_config = !dump_config;
    log_stats = !log_stats;
    extract_facts = !extract_facts;
    for_debugger_eval = !for_debugger_eval;
    disable_toplevel_elaboration = !disable_toplevel_elaboration;
  }

let fail_daemon file error =
  Hh_json.(
    let file = Option.value ~default:"[unknown]" file in
    let msg =
      json_to_string
      @@ JSON_Object
           [
             ("type", JSON_String "error");
             ("file", JSON_String file);
             ("error", JSON_String error);
           ]
    in
    P.printf "%s\n%!" msg;
    die error)

let rec dispatch_loop handlers =
  Hh_json.(
    Access.(
      let read_message () =
        let line = Caml.read_line () in
        let header = json_of_string line in
        let file = get_field_opt (get_string "file") header in
        let bytes = get_field (get_number_int "bytes") (fun _af -> 0) header in
        let body = Bytes.create bytes in
        try
          Caml.really_input Caml.stdin body 0 bytes;
          (header, Bytes.to_string body)
        with exc ->
          fail_daemon
            file
            ("Exception reading message body: " ^ Caml.Printexc.to_string exc)
      in
      let (header, body) = read_message () in
      let msg_type =
        get_field
          (get_string "type")
          (fun af ->
            fail_daemon None ("Cannot determine type of message: " ^ af))
          header
      in
      (match msg_type with
      | "code" -> handlers.compile header body
      | "error" -> handlers.error header body
      | "config" -> handlers.set_config header body
      | "facts" -> handlers.facts header body
      | "parse" -> handlers.parse header body
      | _ -> fail_daemon None ("Unhandled message type '" ^ msg_type ^ "'"));
      dispatch_loop handlers))

let print_debug_time_info filename debug_time =
  let stat = Caml.Gc.stat () in
  P.eprintf "File %s:\n" (Relative_path.to_absolute filename);
  P.eprintf "Parsing: %0.3f s\n" !(debug_time.parsing_t);
  P.eprintf "Codegen: %0.3f s\n" !(debug_time.codegen_t);
  P.eprintf "Printing: %0.3f s\n" !(debug_time.printing_t);
  P.eprintf "MinorWords: %0.3f\n" stat.Caml.Gc.minor_words;
  P.eprintf "PromotedWords: %0.3f\n" stat.Caml.Gc.promoted_words

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

let do_compile
    ~is_systemlib
    ~config_jsons
    (compiler_options : options)
    filename
    source_text
    debug_time =
  let env =
    Compile.
      {
        is_systemlib;
        filepath = filename;
        is_evaled = is_file_path_for_evaled_code filename;
        for_debugger_eval = compiler_options.for_debugger_eval;
        dump_symbol_refs = compiler_options.dump_symbol_refs;
        config_list = compiler_options.config_list;
        config_jsons;
        disable_toplevel_elaboration =
          compiler_options.disable_toplevel_elaboration;
      }
  in
  let ret = Compile.from_text source_text env in
  (debug_time.parsing_t := Compile.(ret.parsing_t));
  (debug_time.codegen_t := Compile.(ret.codegen_t));
  (debug_time.printing_t := Compile.(ret.printing_t));
  if compiler_options.debug_time then print_debug_time_info filename debug_time;
  if compiler_options.log_stats then
    log_success compiler_options filename debug_time;
  Compile.(ret.bytecode_segments, Some ret.hhbc_options)

let extract_facts ~compiler_options ~config_jsons ~filename text =
  let co =
    Hhbc_options.apply_config_overrides_statelessly
      compiler_options.config_list
      config_jsons
  in
  ( [
      Hhbc_options.(
        Facts_parser.extract_as_json_string
          ~php5_compat_mode:true
          ~hhvm_compat_mode:true
          ~disable_nontoplevel_declarations:
            (phpism_disable_nontoplevel_declarations co)
          ~disable_legacy_soft_typehints:(disable_legacy_soft_typehints co)
          ~allow_new_attribute_syntax:(allow_new_attribute_syntax co)
          ~disable_legacy_attribute_syntax:(disable_legacy_attribute_syntax co)
          ~enable_xhp_class_modifier:(enable_xhp_class_modifier co)
          ~disable_xhp_element_mangling:(disable_xhp_element_mangling co)
          ~filename
          ~text
        |> Option.value ~default:"");
    ],
    Some co )

let parse_hh_file ~config_jsons ~compiler_options filename body =
  let co =
    Hhbc_options.apply_config_overrides_statelessly
      compiler_options.config_list
      config_jsons
  in
  Hhbc_options.(
    let file = Relative_path.create Relative_path.Dummy filename in
    let source_text = SourceText.make file body in
    let mode = Full_fidelity_parser.parse_mode source_text in
    let env =
      Full_fidelity_parser_env.make
        ~codegen:true
        ~php5_compat_mode:true
        ~hhvm_compat_mode:true
        ~disable_nontoplevel_declarations:
          (phpism_disable_nontoplevel_declarations co)
        ~disable_legacy_soft_typehints:(disable_legacy_soft_typehints co)
        ~allow_new_attribute_syntax:(allow_new_attribute_syntax co)
        ~disable_legacy_attribute_syntax:(disable_legacy_attribute_syntax co)
        ~enable_xhp_class_modifier:(enable_xhp_class_modifier co)
        ~disable_xhp_element_mangling:(disable_xhp_element_mangling co)
        ?mode
        ()
    in
    let syntax_tree = SyntaxTree.make ~env source_text in
    let json = SyntaxTree.to_json syntax_tree in
    ([Hh_json.json_to_string json], Some co))

(*****************************************************************************)
(* Main entry point *)
(*****************************************************************************)

let process_single_source_unit
    ~is_systemlib
    ~config_jsons
    compiler_options
    handle_output
    handle_exception
    filename
    source_text =
  try
    let debug_time = new_debug_time () in
    let (output, hhbc_options) =
      if compiler_options.extract_facts then
        extract_facts ~compiler_options ~config_jsons ~filename source_text
      else
        do_compile
          ~is_systemlib
          ~config_jsons
          compiler_options
          filename
          source_text
          debug_time
    in
    handle_output filename output hhbc_options debug_time
  with exc ->
    let stack = Caml.Printexc.get_backtrace () in
    if compiler_options.log_stats then
      log_fail compiler_options filename exc ~stack;
    handle_exception filename exc

let decl_and_run_mode compiler_options =
  Hh_json.(
    Access.(
      let print_and_flush_strings strings =
        List.iter ~f:(P.printf "%s") strings;
        P.printf "%!"
      in
      (* list of pending config JSONs *)
      let config_jsons = ref [] in
      let add_config (config_json : string option) =
        config_jsons := config_json :: !config_jsons
      in
      let pop_config () =
        config_jsons :=
          match !config_jsons with
          | _ :: old_config_jsons -> old_config_jsons
          | _ -> !config_jsons
      in
      let get_config_jsons () = List.filter_map ~f:(fun x -> x) !config_jsons in
      let ini_config_json : string option =
        Option.map
          ~f:(fun path -> (try Sys_utils.cat path with _ -> ""))
          compiler_options.config_file
      in
      add_config ini_config_json;
      Ident.track_names := true;

      match compiler_options.mode with
      | DAEMON ->
        let add_config_overrides header =
          let config_overrides =
            get_field
              (get_obj "config_overrides")
              (fun _af -> JSON_Object [])
              header
            |> Hh_json.json_to_string
          in
          add_config (Some config_overrides)
        in
        let get_filename_and_path (header : json) =
          let filename =
            get_field
              (get_string "file")
              (fun af ->
                fail_daemon
                  None
                  ("Cannot determine file name of source unit: " ^ af))
              header
          in
          (filename, Relative_path.create Relative_path.Dummy filename)
        in
        let body_or_file_contents (body : string) (filename : string) =
          (* if body is empty - read file from disk *)
          if String.length body = 0 then
            Sys_utils.cat filename
          else
            body
        in
        let handle_output filename output hhbc_options debug_time =
          let abs_path = Relative_path.to_absolute filename in
          let bytes =
            List.fold ~f:(fun len s -> len + String.length s) ~init:0 output
          in
          let msg =
            [
              ("type", JSON_String "success");
              ("file", JSON_String abs_path);
              ("bytes", int_ bytes);
            ]
          in
          let log_extern_compiler_perf =
            (match hhbc_options with
            | Some opts -> opts
            | None ->
              Hhbc_options.apply_config_overrides_statelessly
                compiler_options.config_list
                (get_config_jsons ()))
            |> Hhbc_options.log_extern_compiler_perf
          in
          let msg =
            if log_extern_compiler_perf then
              let json_microsec t = int_ @@ int_of_float @@ (t *. 1000000.0) in
              ("parsing_time", json_microsec !(debug_time.parsing_t))
              :: ("codegen_time", json_microsec !(debug_time.codegen_t))
              :: ("printing_time", json_microsec !(debug_time.printing_t))
              :: msg
            else
              msg
          in
          P.printf "%s\n" (json_to_string @@ JSON_Object msg);
          print_and_flush_strings output
        in
        let handle_exception filename exc =
          let abs_path = Relative_path.to_absolute filename in
          let msg =
            json_to_string
            @@ JSON_Object
                 [
                   ("type", JSON_String "error");
                   ("file", JSON_String abs_path);
                   ("error", JSON_String (Caml.Printexc.to_string exc));
                 ]
          in
          P.printf "%s\n%!" msg
        in
        let handlers =
          {
            set_config =
              (fun _header body ->
                let config_json =
                  if body = "" then
                    None
                  else
                    Some body
                in
                add_config config_json);
            error =
              (fun header _body ->
                let (filename, _) = get_filename_and_path header in
                let error =
                  get_field
                    (get_string "error")
                    (fun _af ->
                      fail_daemon
                        (Some filename)
                        "No 'error' field in error message")
                    header
                in
                fail_daemon
                  (Some filename)
                  ("Error processing " ^ filename ^ ": " ^ error));
            compile =
              (fun header body ->
                let (_, path) = get_filename_and_path header in
                let is_systemlib =
                  get_field_opt (get_bool "is_systemlib") header
                  |> Option.value ~default:false
                in
                let for_debugger_eval =
                  get_field
                    (get_bool "for_debugger_eval")
                    (fun af ->
                      fail_daemon None ("for_debugger_eval flag missing: " ^ af))
                    header
                in
                add_config_overrides header;
                let compiler_options =
                  { compiler_options with for_debugger_eval }
                in
                let result =
                  process_single_source_unit
                    ~is_systemlib
                    ~config_jsons:(get_config_jsons ())
                    compiler_options
                    handle_output
                    handle_exception
                    path
                    body
                in
                pop_config ();
                result);
            facts =
              (fun header body ->
                (let (filename, path) = get_filename_and_path header in
                 let body = body_or_file_contents body filename in
                 add_config_overrides header;
                 let (output, hhbc_options) =
                   extract_facts
                     ~compiler_options
                     ~config_jsons:(get_config_jsons ())
                     ~filename:path
                     body
                 in
                 let result = handle_output path output hhbc_options in
                 pop_config ();
                 result)
                  (new_debug_time ()));
            parse =
              (fun header body ->
                (let (filename, path) = get_filename_and_path header in
                 let body = body_or_file_contents body filename in
                 add_config_overrides header;
                 let (output, hhbc_options) =
                   parse_hh_file
                     ~config_jsons:(get_config_jsons ())
                     ~compiler_options
                     filename
                     body
                 in
                 let result = handle_output path output hhbc_options in
                 pop_config ();
                 result)
                  (new_debug_time ()));
          }
        in
        dispatch_loop handlers
      | CLI ->
        let dumped_options =
          lazy
            ( Hhbc_options.apply_config_overrides_statelessly
                compiler_options.config_list
                (get_config_jsons ())
            |> Hhbc_options.to_string )
        in
        let handle_output _filename output _hhbc_options _debug_time =
          if compiler_options.dump_config then
            Printf.printf "===CONFIG===\n%s\n\n%!" (Lazy.force dumped_options);
          if not compiler_options.quiet_mode then print_and_flush_strings output
        in
        let handle_exception filename exc =
          if not compiler_options.quiet_mode then (
            let stack = Caml.Printexc.get_backtrace () in
            prerr_endline stack;
            P.eprintf
              "Error in file %s: %s\n"
              (Relative_path.to_absolute filename)
              (Caml.Printexc.to_string exc)
          )
        in
        let process_single_file handle_output filename =
          let filename = Relative_path.create Relative_path.Dummy filename in
          (* let abs_path = Relative_path.to_absolute filename in *)
          let files = Multifile.file_to_file_list filename in
          List.iter files ~f:(fun (filename, content) ->
              process_single_source_unit
                ~is_systemlib:false
                ~config_jsons:(get_config_jsons ())
                compiler_options
                handle_output
                handle_exception
                filename
                content)
        in
        let (filenames, handle_output) =
          match compiler_options.input_file_list with
          (* List of source files explicitly given *)
          | Some input_file_list ->
            let get_lines_in_file filename =
              let inch = Caml.open_in filename in
              let rec go lines =
                match Caml.input_line inch with
                | line -> go (Caml.String.trim line :: lines)
                | exception End_of_file -> lines
              in
              go []
            in
            (get_lines_in_file input_file_list, handle_output)
          | None ->
            if
              Sys.is_directory compiler_options.filename
              (* Compile every file under directory *)
            then
              let files_in_dir =
                let rec go dirs =
                  match dirs with
                  | [] -> []
                  | dir :: dirs ->
                    let (ds, fs) =
                      Sys.readdir dir
                      |> Array.map ~f:(Filename.concat dir)
                      |> Array.to_list
                      |> List.partition_tf ~f:Sys.is_directory
                    in
                    fs @ go (ds @ dirs)
                in
                go [compiler_options.filename]
              in
              let handle_output filename output _hhbc_options _debug_time =
                let abs_path = Relative_path.to_absolute filename in
                if Filename.check_suffix abs_path ".php" then
                  let output_file =
                    Filename.chop_suffix abs_path ".php" ^ ".hhas"
                  in
                  if Sys.file_exists output_file then (
                    if not compiler_options.quiet_mode then
                      Caml.Printf.fprintf
                        Caml.stderr
                        "Output file %s already exists\n"
                        output_file
                  ) else
                    Sys_utils.write_strings_to_file ~file:output_file output
              in
              (files_in_dir, handle_output)
            (* Compile a single file *)
            else
              let handle_output =
                match compiler_options.output_file with
                | Some output_file ->
                  fun _filename output _hhbc_options _debug_time ->
                    Sys_utils.write_strings_to_file ~file:output_file output
                | _ -> handle_output
              in
              ([compiler_options.filename], handle_output)
          (* Actually execute the compilation(s) *)
        in
        List.iter filenames (process_single_file handle_output)))

let main_hack opts =
  let start_time = Unix.gettimeofday () in
  if opts.log_stats then Logger.init start_time;
  decl_and_run_mode opts

(* command line driver *)
let () =
  Printexc.record_backtrace true;
  try
    if !Sys.interactive then
      ()
    else
      (* On windows, setting 'binary mode' avoids to output CRLF on
       stdout.  The 'text mode' would not hurt the user in general, but
       it breaks the testsuite where the output is compared to the
       expected one (i.e. in given file without CRLF). *)
      Caml.set_binary_mode_out Caml.stdout true;
    let handle =
      SharedMem.init ~num_workers:0 GlobalConfig.empty_sharedmem_config
    in
    ignore (handle : SharedMem.handle);
    let options = parse_options () in
    main_hack options
  with exc ->
    let stack = Caml.Printexc.get_backtrace () in
    prerr_endline stack;
    die (Caml.Printexc.to_string exc)
