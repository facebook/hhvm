(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Core
open Sys_utils

module P = Printf

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
  filename        : string;
  fallback        : bool;
  config_list     : string list;
  debug_time      : bool;
  parser          : parser;
  output_file     : string option;
  config_file     : string option;
  quiet_mode      : bool;
  mode            : mode;
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
  let oc = stderr in
  output_string oc str;
  close_out oc;
  exit 2

let parse_options () =
  let fn_ref = ref None in
  let fallback = ref false in
  let debug_time = ref false in
  let parser = ref FFP in
  let config_list = ref [] in
  let mode = ref CLI in
  let output_file = ref None in
  let config_file = ref None in
  let quiet_mode = ref false in
  let usage = P.sprintf "Usage: %s filename\n" Sys.argv.(0) in
  let options =
    [ ("--fallback"
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
      , Arg.String (fun str -> config_file := Some str)
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
      )
    ] in
  let options = Arg.align ~limit:25 options in
  Arg.parse options (fun fn -> fn_ref := Some fn) usage;
  let fn = match !fn_ref with
    | Some fn -> if !mode == CLI then fn else die usage
    | None -> if !mode == CLI then die usage else read_line () in
  { filename    = fn
  ; fallback    = !fallback
  ; config_list = !config_list
  ; debug_time  = !debug_time
  ; parser      = !parser
  ; output_file = !output_file
  ; config_file = !config_file
  ; quiet_mode  = !quiet_mode
  ; mode        = !mode
  }

let load_file_stdin () =
  let _ = read_line () in (* md5 *)
  let len = read_int () in
  let code = Bytes.create len in
  let _ = really_input stdin code 0 len in
  (* TODO: Read config file from stdin and call Hh_json.json_of_string *)
  Hh_json.JSON_Null, code

let load_file file =
  let abs_fn = Relative_path.to_absolute file in
  let content = cat abs_fn in
  content

let parse_text compiler_options popt fn text =
  match compiler_options.parser with
  | FFP ->
    let ignore_pos =
      not (Hhbc_options.source_mapping !Hhbc_options.compiler_options) in
    Full_fidelity_ast.from_text_with_legacy
      ~parser_options:popt
      ~ignore_pos
      ~suppress_output:true
      fn text
  | Legacy ->
    Parser_hack.program popt fn text

let parse_file compiler_options popt filename text =
  try
    Some (Errors.do_ begin fun () ->
      parse_text compiler_options popt filename text
    end)
  with Failure _ -> None


let hhvm_unix_call config filename =
  P.printf "compiling: %s\n" filename;
  let readme, writeme = Unix.pipe () in
  let prog_name = "/usr/local/hphpi/bin/hhvm" in
  let options =
    List.concat_map ("Eval.DumpHhas=1" :: config) (fun s -> ["-v"; s]) in
  let params = Array.of_list ([prog_name] @ options @ [filename]) in
  let _ =
    Unix.create_process prog_name params Unix.stdin writeme Unix.stderr in
  Unix.close writeme;
  let in_channel = Unix.in_channel_of_descr readme in
  let rec aux acc : string list =
    try
      aux @@ input_line in_channel :: acc
    with End_of_file -> List.rev acc
  in
  let result =
    List.fold_left (aux []) ~f:(fun acc line -> acc ^ line ^ "\n") ~init:"" in
  Unix.close readme;
  result

let add_to_time_ref r t0 =
  let t = Unix.gettimeofday () in
  r := !r +. (t -. t0);
  t

let print_debug_time_info filename debug_time =
  P.eprintf "File %s:\n" (Relative_path.to_absolute filename);
  P.eprintf "Parsing: %0.3f s\n" !(debug_time.parsing_t);
  P.eprintf "Codegen: %0.3f s\n" !(debug_time.codegen_t);
  P.eprintf "Printing: %0.3f s\n" !(debug_time.printing_t)

let do_compile filename compiler_options opt_ast debug_time =
  let t = Unix.gettimeofday () in
  let t = add_to_time_ref debug_time.parsing_t t in
  let hhas_prog =
    match opt_ast with
    | None ->
      Hhas_program.emit_fatal_program ~ignore_message:true
        Hhbc_ast.FatalOp.Parse Pos.none "Syntax error"
    | Some (errors, parser_return, _) ->
      let is_hh_file =
        Option.value_map parser_return.Parser_hack.file_mode
          ~default:false ~f:(fun v -> v <> FileInfo.Mphp)
      in
      let ast = parser_return.Parser_hack.ast in
      List.iter (Errors.get_error_list errors) (fun e ->
        Printf.printf "%s\n" (Errors.to_string (Errors.to_absolute e)));
      if Errors.is_empty errors
      then Hhas_program.from_ast is_hh_file ast
      else Hhas_program.emit_fatal_program ~ignore_message:true
        Hhbc_ast.FatalOp.Parse Pos.none "Syntax error"
      in
  let t = add_to_time_ref debug_time.codegen_t t in
  let hhas_text = Hhbc_hhas.to_string hhas_prog in
  ignore @@ add_to_time_ref debug_time.printing_t t;
  if compiler_options.debug_time
  then print_debug_time_info filename debug_time;
  hhas_text

let load_config_and_file compiler_options filename =
  match compiler_options.mode with
  | CLI ->
    Option.map ~f:Hh_json.json_of_file compiler_options.config_file,
    load_file filename
  | DAEMON ->
    (* TODO: Read config file from stdin *)
    let _config, file = load_file_stdin () in
    None, file

(*****************************************************************************)
(* Main entry point *)
(*****************************************************************************)

let process_single_file compiler_options popt filename outputfile =
  try
    let t = Unix.gettimeofday () in
    let config, text = load_config_and_file compiler_options filename in
    let options =
      Hhbc_options.get_options_from_config config compiler_options.config_list
    in
    Hhbc_options.set_compiler_options options;
    let opt_ast = parse_file compiler_options popt filename text in
    let debug_time = new_debug_time () in
    ignore @@ add_to_time_ref debug_time.parsing_t t;
    let text = do_compile filename compiler_options opt_ast debug_time in
    if compiler_options.mode = DAEMON then
      Printf.printf "%i\n%!" (String.length text);
    match outputfile with
    | None ->
      if not compiler_options.quiet_mode
      then P.printf "%s%!" text
      else ()
    | Some outputfile -> Sys_utils.write_file ~file:outputfile text
  with e ->
    if not compiler_options.quiet_mode
    then begin
      if compiler_options.mode = DAEMON then
        Printf.printf "ERROR: %s\n%!" (Printexc.to_string e)
      else
        let f = Relative_path.to_absolute filename in
        Printf.eprintf "Error in file %s: %s\n" f (Printexc.to_string e)
    end
    else ()

let compile_files_recursively compiler_options f =
  let rec loop dirs = begin
    match dirs with
    | [] -> ()
    | dir::dirs ->
      let compile_file = fun p ->
        if Filename.check_suffix p ".php" then
          let outputfile =
            let f = Filename.chop_suffix p ".php" in
            f ^ ".hhas"
          in
          if Sys.file_exists outputfile then
            if not compiler_options.quiet_mode
            then P.fprintf stderr "Output file %s already exists\n" outputfile
            else ()
          else begin
            f (Relative_path.create Relative_path.Dummy p) (Some outputfile)
          end
      in
      let ds, fs =
        Sys.readdir dir
        |> Array.map (Filename.concat dir)
        |> Array.to_list
        |> List.partition_tf ~f: Sys.is_directory
      in
      List.iter fs compile_file;
      loop (ds @ dirs)
  end
  in loop [compiler_options.filename]

let decl_and_run_mode compiler_options popt =
  Local_id.track_names := true;
  Ident.track_names := true;
  let process_single_file = process_single_file compiler_options popt in
  if compiler_options.mode = DAEMON then
    let rec process_next fn = begin
      let fname = Relative_path.create Relative_path.Dummy fn in
      process_single_file fname None;
      let filename = read_line () in
      process_next filename
    end in
      process_next compiler_options.filename
  else if Sys.is_directory compiler_options.filename then
    compile_files_recursively compiler_options process_single_file
  else
    let filename =
      Relative_path.create Relative_path.Dummy compiler_options.filename in
    process_single_file
      filename
      compiler_options.output_file

let main_hack opts =
  let popt = ParserOptions.default in
  EventLogger.init EventLogger.Event_logger_fake 0.0;
  let _handle = SharedMem.init GlobalConfig.default_sharedmem_config in
  let tmp_hhi = Path.concat (Path.make Sys_utils.temp_dir_name) "hhi" in
  Hhi.set_hhi_root_for_unit_test tmp_hhi;
  decl_and_run_mode opts popt

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
    Unix.handle_unix_error main_hack options
