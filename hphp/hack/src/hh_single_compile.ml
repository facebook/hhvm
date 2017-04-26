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
open String_utils
open Sys_utils

(*****************************************************************************)
(* Types, constants *)
(*****************************************************************************)
type parser =
  | Legacy
  | FFP

type options = {
  filename    : string;
  fallback    : bool;
  config      : string list;
  debug_time  : bool;
  parser      : parser;
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
  let config = ref [] in
  let usage = Printf.sprintf "Usage: %s filename\n" Sys.argv.(0) in
  let options =
    [ ("--fallback"
      , Arg.Set fallback
      , " Enables fallback compilation"
      );
      ("--debug-time"
      , Arg.Set debug_time
      , " Enables debugging logging for elapsed time"
      );
      ("-v"
      , Arg.String (fun str -> config := str :: !config)
      , " Configuration: Eval.EnableHipHopSyntax=<value> or Hack.Lang.IntsOverflowToInts=<value>"
      );
      ("--parser"
      , Arg.String
        (function "ffp" -> parser := FFP
                | "legacy" -> parser := Legacy
                | p -> failwith @@ p ^ " is an invalid parser")
      , " Parser: ffp or legacy [def: ffp]"
      )
    ] in
  let options = Arg.align ~limit:25 options in
  Arg.parse options (fun fn -> fn_ref := Some fn) usage;
  let fn = match !fn_ref with
    | Some fn -> fn
    | None -> die usage in
  { filename = fn
  ; fallback = !fallback
  ; config = !config
  ; debug_time = !debug_time
  ; parser = !parser
  }

(* This allows one to fake having multiple files in one file. This
 * is used only in unit test files.
 * Indeed, there are some features that require mutliple files to be tested.
 * For example, newtype has a different meaning depending on the file.
 *)
let rec make_files = function
  | [] -> []
  | Str.Delim header :: Str.Text content :: rl ->
      let pattern = Str.regexp "////" in
      let header = Str.global_replace pattern "" header in
      let pattern = Str.regexp "[ ]*" in
      let filename = Str.global_replace pattern "" header in
      (filename, content) :: make_files rl
  | _ -> assert false

(* We have some hacky "syntax extensions" to have one file contain multiple
 * files, which can be located at arbitrary paths. This is useful e.g. for
 * testing lint rules, some of which activate only on certain paths. It's also
 * useful for testing abstract types, since the abstraction is enforced at the
 * file boundary.
 * Takes the path to a single file, returns a map of filenames to file contents.
 *)
let file_to_files file =
  let abs_fn = Relative_path.to_absolute file in
  let content = cat abs_fn in
  let delim = Str.regexp "////.*" in
  if Str.string_match delim content 0
  then
    let contentl = Str.full_split delim content in
    let files = make_files contentl in
    List.fold_left ~f: begin fun acc (sub_fn, content) ->
      let file =
        Relative_path.create Relative_path.Dummy (abs_fn^"--"^sub_fn) in
      Relative_path.Map.add acc ~key:file ~data:content
    end ~init: Relative_path.Map.empty files
  else if string_starts_with content "// @directory " then
    let contentl = Str.split (Str.regexp "\n") content in
    let first_line = List.hd_exn contentl in
    let regexp = Str.regexp ("^// @directory *\\([^ ]*\\) \
      *\\(@file *\\([^ ]*\\)*\\)?") in
    let has_match = Str.string_match regexp first_line 0 in
    assert has_match;
    let dir = Str.matched_group 1 first_line in
    let file_name =
      try
        Str.matched_group 3 first_line
      with
        Not_found -> abs_fn in
    let file = Relative_path.create Relative_path.Dummy (dir ^ file_name) in
    let content = String.concat "\n" (List.tl_exn contentl) in
    Relative_path.Map.singleton file content
  else
    Relative_path.Map.singleton file content

let parse_name compiler_options popt files_contents =
  Errors.do_ begin fun () ->
    let parsed_files =
      if compiler_options.parser = FFP
      then Relative_path.Map.mapi
        ( Full_fidelity_ast.from_text_with_legacy
            ~parser_options:popt
            ~ignore_pos:true
        )
        files_contents
      else Relative_path.Map.mapi (Parser_hack.program popt) files_contents
    in

    let files_info =
      Relative_path.Map.mapi begin fun fn parsed_file ->
        let {Parser_hack.file_mode; comments; ast; _} = parsed_file in
        Parser_heap.ParserHeap.add fn (ast, Parser_heap.Full);
        let funs, classes, typedefs, consts = Ast_utils.get_defs ast in
        { FileInfo.
          file_mode; funs; classes; typedefs; consts; comments = Some comments;
          consider_names_just_for_autoload = false }, ast
      end parsed_files in

    files_info
  end

let hhvm_unix_call config filename =
  Printf.printf "compiling: %s\n" filename;
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
  Printf.fprintf stderr "File %s:\n" (Relative_path.to_absolute filename);
  Printf.fprintf stderr "Parsing: %0.3f s\n" !(debug_time.parsing_t);
  Printf.fprintf stderr "Codegen: %0.3f s\n" !(debug_time.codegen_t);
  Printf.fprintf stderr "Printing: %0.3f s\n" !(debug_time.printing_t)

let do_compile filename compiler_options opts files_info debug_time = begin
  let nyi_regexp = Str.regexp "\\(.\\|\n\\)*NYI" in
  let get_nast_from_fileinfo tcopt fn (fileinfo, ast) =
    (* Functions *)
    let funs = fileinfo.FileInfo.funs in
    let parse_function (_, fun_) =
        Parser_heap.find_fun_in_file ~full:true tcopt fn fun_ in
    let parsed_functions = List.filter_map funs parse_function in
    (* Classes *)
    let classes = fileinfo.FileInfo.classes in
    let parse_class (_, class_) =
        Parser_heap.find_class_in_file ~full:true tcopt fn class_ in
    let parsed_classes = List.filter_map classes parse_class in
    (* Typedefs *)
    let typedefs = fileinfo.FileInfo.typedefs in
    let parse_typedef (_, typedef_) =
        Parser_heap.find_typedef_in_file ~full:true tcopt fn typedef_ in
    let parsed_typedefs = List.filter_map typedefs parse_typedef in
    (parsed_functions, parsed_classes, parsed_typedefs, ast) in
  let f_fold fn fileinfo text = begin
    let t = Unix.gettimeofday () in
    let ast = get_nast_from_fileinfo opts fn fileinfo in
    let t = add_to_time_ref debug_time.parsing_t t in
    let options = Hhbc_options.get_options_from_config
      compiler_options.config in
    Emit_expression.set_compiler_options options;
    let hhas_prog = Hhas_program.from_ast ast in
    let t = add_to_time_ref debug_time.codegen_t t in
    let hhas_text = Hhbc_hhas.to_string hhas_prog in
    let text =
      if compiler_options.fallback && Str.string_match nyi_regexp hhas_text 0
      then text ^
        hhvm_unix_call compiler_options.config (Relative_path.to_absolute fn)
      else text ^ hhas_text
    in
    ignore @@ add_to_time_ref debug_time.printing_t t;
    text
  end in
  let hhas_text = Relative_path.Map.fold files_info ~f:f_fold ~init:"" in
  if compiler_options.debug_time then print_debug_time_info filename debug_time;
  hhas_text
end

(*****************************************************************************)
(* Main entry point *)
(*****************************************************************************)

let process_single_file compiler_options popt tcopt filename outputfile =
  try
    let t = Unix.gettimeofday () in
    let files_contents = file_to_files filename in
    let _, files_info, _ = parse_name compiler_options popt files_contents in
    let debug_time = new_debug_time() in
    ignore @@ add_to_time_ref debug_time.parsing_t t;
    let text =
      do_compile filename compiler_options tcopt files_info debug_time in
    match outputfile with
    | None -> Printf.printf "%s" text
    | Some outputfile -> Sys_utils.write_file ~file:outputfile text
  with e ->
    let f = Relative_path.to_absolute filename in
    Printf.fprintf stderr "Error in file %s: %s\n" f (Printexc.to_string e)

let compile_files_recursively dir f =
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
            Printf.fprintf stderr "Output file %s already exists\n" outputfile
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
  in loop [dir]

let decl_and_run_mode compiler_options popt tcopt =
  Local_id.track_names := true;
  Ident.track_names := true;
  let process_single_file = process_single_file compiler_options popt tcopt in
  if Sys.is_directory compiler_options.filename then
    compile_files_recursively compiler_options.filename process_single_file
  else
    let filename =
      Relative_path.create Relative_path.Dummy compiler_options.filename in
    process_single_file filename None

let main_hack opts =
  let popt = ParserOptions.default in
  let tcopt = TypecheckerOptions.default in
  EventLogger.init EventLogger.Event_logger_fake 0.0;
  let _handle = SharedMem.init GlobalConfig.default_sharedmem_config in
  let tmp_hhi = Path.concat (Path.make Sys_utils.temp_dir_name) "hhi" in
  Hhi.set_hhi_root_for_unit_test tmp_hhi;
  decl_and_run_mode opts popt tcopt

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
