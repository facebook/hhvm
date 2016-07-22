(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Utils
module Sys = Sys_ext
module CE = Common_exns
module Parser_hack = Parser_hack_ext

let output_file_type = ref FileInfo.HhFile

let parse_options () =
  let src = ref None
  and dest = ref None
  and usage = Printf.sprintf
    "Usage: %s source [destination] [-output-type php|hack]\n"
    Sys.argv.(0) in

  let fail_with_usage error_str = begin
    prerr_endline error_str;
    prerr_endline usage;
    exit 1
  end in

  let parse_arg s = begin
    match !Arg.current with
      | 1 -> src := Some s;
      | 2 -> dest := Some s;
      | _ -> fail_with_usage "This program only accepts two arguments";
  end in

  let output_type = function
  | "php" -> output_file_type := FileInfo.PhpFile
  | "hack" -> output_file_type := FileInfo.HhFile
  | _ -> fail_with_usage "output_type must be php or hack" in

  let is_valid_src f =
    Sys.file_exists f &&
    (Sys.has_extension f ".php" || Sys.has_extension f ".hh") in

  let speclist = [
    ("-d", Arg.Set debug, "Enables debug mode");
    ("-output-type", Arg.String output_type,
      "Select the output type. (valid values: php, hack). Defaults to hack.")
  ] in
  Arg.parse speclist parse_arg usage;

  if !Arg.current < 2 then fail_with_usage
    "This program requires a source file and an [optional] destination file";

  let src = unsafe_opt !src in
  if not (is_valid_src src) then fail_with_usage
   "This program requires a valid php or hack source file";
  match !dest with
    | Some f when Sys.file_exists f -> fail_with_usage
      "Destination file already exists";
    | _ -> ();
  (src, !dest)

let try_dump_unparsed ast =
  try
    dn (Unparsed.dump (Unparser.unparse_internal ast));
    ()
  with _ -> ()

let _ =
  try
    let (src, dest) = parse_options () in
    let _handle = SharedMem.init GlobalConfig.default_sharedmem_config in
    let {Parser_hack.ast; _} =
      Parser_hack.parse_or_die Relative_path.(create Dummy src) in
    if !debug then try_dump_unparsed ast;
    if !debug then dn (Debug.dump_ast (Ast.AProgram ast));

    (*
      the hack parser currently automatically adds a type hint to the return type
      of a constructor. So we strip it out here. The correct solution would be to
      modify the parser, but for now this is an acceptable workaround.
    *)
    let ast = if !output_file_type = FileInfo.PhpFile
              then Erase_types.map ast
              else ast in

    let unparsed = Unparser.unparse !output_file_type (Path.make src) ast in

    match dest with
    | Some f -> Sys.write_file unparsed f
    | None -> print_string unparsed
  with e -> Sys.die (CE.flatten_error e)
