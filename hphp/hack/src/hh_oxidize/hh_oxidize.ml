(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Printf
open Reordered_argument_collections
open Utils

type env = { rustfmt: string }

let header =
  "// Copyright (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the \"hack\" directory of this source tree.
//
// "
  ^ Signed_source.signing_token

let regen_instructions = "
//
// To regenerate this file, run:
//   "

let parse filename =
  let ic = In_channel.create filename in
  let lexbuf = Lexing.from_channel ic in
  let phrases = Parse.use_file lexbuf in
  In_channel.close ic;
  phrases

let oxidize filename =
  let phrases = parse filename in
  let in_basename = Filename.basename filename in
  let module_name = String.chop_suffix_exn in_basename ".ml" in
  let module_name = convert_module_name module_name in
  log "Converting %s" module_name;
  let oxidized_module =
    Utils.with_log_indent (fun () ->
        Output.with_output_context ~module_name (fun () ->
            List.iter phrases Convert_toplevel_phrase.toplevel_phrase))
  in
  (module_name, oxidized_module)

let read filename =
  let ic = In_channel.create filename in
  let contents = In_channel.input_all ic in
  In_channel.close ic;
  contents

let write filename contents =
  let oc = Out_channel.create filename in
  fprintf oc "%s%!" contents;
  Out_channel.close oc

let write_format_and_sign env filename contents =
  log "Writing %s" filename;
  write filename contents;
  if Sys.command (sprintf "%S %S" env.rustfmt filename) <> 0 then
    failwith ("Could not format Rust output in " ^ filename);
  let contents = read filename in
  let contents =
    (try Signed_source.sign_file contents with Caml.Not_found -> contents)
  in
  write filename contents

let convert_files env out_dir files regen_command =
  ignore (Sys.command (sprintf "rm %S/*.rs" out_dir));
  let header =
    match regen_command with
    | None -> header
    | Some cmd -> header ^ regen_instructions ^ cmd
  in
  let modules = files |> List.map ~f:oxidize |> SMap.of_list in
  let () =
    modules
    |> SMap.map ~f:Stringify.stringify
    |> SMap.iter ~f:(fun name src ->
           let src = sprintf "%s\n\n%s" header src in
           let out_filename = Filename.concat out_dir (name ^ ".rs") in
           write_format_and_sign env out_filename src)
  in
  let manifest_filename = Filename.concat out_dir "mod.rs" in
  let module_names = SMap.ordered_keys modules in
  let manifest_mods =
    map_and_concat module_names ~f:(sprintf "pub mod %s;") ~sep:"\n"
  in
  let manifest = header ^ "\n\n" ^ manifest_mods in
  write_format_and_sign env manifest_filename manifest

let convert_single_file env filename =
  with_tempfile @@ fun out_filename ->
  let (_, oxidized_module) = oxidize filename in
  let src = Stringify.stringify oxidized_module in
  write_format_and_sign env out_filename src;
  printf "%s" (read out_filename)

let usage =
  "Usage: buck run hphp/hack/src/hh_oxidize -- [out_directory] [target_files]
       buck run hphp/hack/src/hh_oxidize -- [target_file]"

type mode =
  | File of string
  | Files of {
      out_dir: string;
      files: string list;
      regen_command: string option;
    }

type options = {
  mode: mode;
  rustfmt_path: string;
}

let parse_args () =
  let out_dir = ref None in
  let regen_command = ref None in
  let rustfmt_path = ref None in
  let files = ref [] in
  let options =
    [
      ( "--out-dir",
        Arg.String (fun s -> out_dir := Some s),
        " Output directory for conversion of multiple files" );
      ( "--regen-command",
        Arg.String (fun s -> regen_command := Some s),
        " Include this command in file headers" );
      ( "--rustfmt-path",
        Arg.String (fun s -> rustfmt_path := Some s),
        " Path to rustfmt binary used to format output" );
    ]
  in
  Arg.parse options (fun file -> files := file :: !files) usage;
  let rustfmt_path = Option.value !rustfmt_path ~default:"rustfmt" in
  match !files with
  | [] ->
    eprintf "%s\n" usage;
    exit 1
  | [file] ->
    if Option.is_some !out_dir then
      failwith "Cannot set output directory in single-file mode";
    if Option.is_some !regen_command then
      failwith "Cannot set regen command in single-file mode";
    { mode = File file; rustfmt_path }
  | files ->
    let out_dir =
      match !out_dir with
      | Some d -> d
      | None ->
        failwith "Cannot convert multiple files without output directory"
    in
    let regen_command = !regen_command in
    { mode = Files { out_dir; files; regen_command }; rustfmt_path }

let () =
  let { mode; rustfmt_path } = parse_args () in
  let env = { rustfmt = rustfmt_path } in
  match mode with
  | File file -> convert_single_file env file
  | Files { out_dir; files; regen_command } ->
    convert_files env out_dir files regen_command
