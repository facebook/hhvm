(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core
module Sys = Stdlib.Sys
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
  let module_name = String.chop_suffix_exn in_basename ~suffix:".ml" in
  let module_name = convert_module_name module_name in
  log "Converting %s" module_name;
  let oxidized_module =
    Utils.with_log_indent (fun () ->
        Output.with_output_context ~module_name (fun () ->
            let _env =
              Convert_toplevel_phrase.(
                List.fold phrases ~f:toplevel_phrase ~init:Env.empty)
            in
            ()))
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
  write filename contents;
  if Sys.command (sprintf "%S %S" env.rustfmt filename) <> 0 then
    failwith ("Could not format Rust output in " ^ filename);
  let contents = read filename in
  let contents =
    try Signed_source.sign_file contents with
    | Signed_source.Token_not_found -> contents
  in
  write filename contents

let make_header regen_command =
  match regen_command with
  | None -> header
  | Some cmd -> header ^ regen_instructions ^ cmd

let convert_files env out_dir files regen_command =
  ignore (Sys.command (sprintf "rm -f %S/*.rs" out_dir));
  let header = make_header regen_command in
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

let convert_single_file env filename regen_command =
  with_tempfile @@ fun out_filename ->
  let (_, oxidized_module) = oxidize filename in
  let src = Stringify.stringify oxidized_module in
  write_format_and_sign env out_filename src;
  let header = make_header regen_command in
  printf "%s\n%s" header (read out_filename)

let parse_types_file filename =
  let lines = ref [] in
  let ic = Stdlib.open_in filename in
  (try
     while true do
       lines := Stdlib.input_line ic :: !lines
     done;
     Stdlib.close_in ic
   with
  | End_of_file -> Stdlib.close_in ic);
  List.filter_map !lines ~f:(fun name ->
      (* Ignore comments beginning with '#' *)
      let name =
        match String.index name '#' with
        | Some idx -> String.sub name ~pos:0 ~len:idx
        | None -> name
      in
      (* Strip whitespace *)
      let name = String.strip name in
      if String.is_substring name ~substring:"::" then
        Some name
      else (
        if String.(name <> "") then
          failwith
            (Printf.sprintf
               "Failed to parse line in types file %S: %S"
               filename
               name);
        None
      ))

let parse_extern_types_file filename =
  parse_types_file filename
  |> List.fold ~init:SMap.empty ~f:(fun map name ->
         try
           (* Map the name with the crate prefix stripped (since we do not expect to see
              the crate name in our OCaml source) to the fully-qualified name. *)
           let coloncolon_idx = String.substr_index_exn name ~pattern:"::" in
           let after_coloncolon_idx = coloncolon_idx + 2 in
           assert (Char.(name.[after_coloncolon_idx] <> ':'));
           let name_without_crate =
             String.subo name ~pos:after_coloncolon_idx
           in
           SMap.add map ~key:name_without_crate ~data:name
         with
         | _ ->
           if String.(name <> "") then
             failwith
               (Printf.sprintf
                  "Failed to parse line in extern types file %S: %S"
                  filename
                  name);
           map)

let parse_owned_types_file filename = SSet.of_list (parse_types_file filename)

let parse_copy_types_file filename = SSet.of_list (parse_types_file filename)

let usage =
  "Usage: buck run hphp/hack/src/hh_oxidize -- [out_directory] [target_files]
       buck run hphp/hack/src/hh_oxidize -- [target_file]"

type mode =
  | File of {
      file: string;
      regen_command: string option;
    }
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
  let mode = ref Configuration.ByBox in
  let extern_types_file = ref None in
  let owned_types_file = ref None in
  let copy_types_file = ref None in
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
      ( "--by-ref",
        Arg.Unit (fun () -> mode := Configuration.ByRef),
        " Use references instead of Box, slices instead of Vec and String" );
      ( "--extern-types-file",
        Arg.String (fun s -> extern_types_file := Some s),
        " Use the types listed in this file rather than assuming all types"
        ^ " are defined within the set of files being oxidized" );
      ( "--owned-types-file",
        Arg.String (fun s -> owned_types_file := Some s),
        " Do not add a lifetime parameter to the types listend in this file"
        ^ " (when --by-ref is enabled)" );
      ( "--copy-types-file",
        Arg.String (fun s -> copy_types_file := Some s),
        " Do not use references for the types listed in this file"
        ^ " (when --by-ref is enabled)" );
    ]
  in
  Arg.parse options (fun file -> files := file :: !files) usage;
  let extern_types =
    match !extern_types_file with
    | None -> Configuration.(default.extern_types)
    | Some filename -> parse_extern_types_file filename
  in
  let owned_types =
    match !owned_types_file with
    | None -> Configuration.(default.owned_types)
    | Some filename -> parse_owned_types_file filename
  in
  let copy_types = Option.map !copy_types_file ~f:parse_copy_types_file in
  Configuration.set
    { Configuration.mode = !mode; extern_types; owned_types; copy_types };
  let rustfmt_path = Option.value !rustfmt_path ~default:"rustfmt" in
  let regen_command = !regen_command in
  match !files with
  | [] ->
    eprintf "%s\n" usage;
    exit 1
  | [file] ->
    if Option.is_some !out_dir then
      failwith "Cannot set output directory in single-file mode";
    { mode = File { file; regen_command }; rustfmt_path }
  | files ->
    let out_dir =
      match !out_dir with
      | Some d -> d
      | None ->
        failwith "Cannot convert multiple files without output directory"
    in
    { mode = Files { out_dir; files; regen_command }; rustfmt_path }

let () =
  let { mode; rustfmt_path } = parse_args () in
  let env = { rustfmt = rustfmt_path } in
  match mode with
  | File { file; regen_command } -> convert_single_file env file regen_command
  | Files { out_dir; files; regen_command } ->
    convert_files env out_dir files regen_command
