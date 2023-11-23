(*
 * Copyright (c) 2021, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let children_files (file_list : String.t list) =
  let module StringSet = Set.Make (String) in
  let rec children_files_inner
      acc (parent : String.t) (file_list : String.t list) =
    let file_list =
      List.map file_list ~f:(fun s ->
          if String.equal parent "" then
            s
          else
            parent ^ "/" ^ s)
    in
    let on_child acc child =
      if Path.is_directory (Path.make child) then
        children_files_inner acc child (Sys.readdir child |> Array.to_list)
      else
        Set.add acc child
    in
    List.fold file_list ~init:acc ~f:on_child
  in
  let file_set = children_files_inner StringSet.empty "" file_list in
  Set.to_list file_set

let json_array_of_json_strings (l_in : String.t list) =
  let add_json_string acc s = Hh_json.JSON_String s :: acc in
  let list_of_json = List.fold_left l_in ~init:[] ~f:add_json_string in
  Hh_json.JSON_Array list_of_json

(*parse command line arguments *)
let parse_options () =
  let input_files = ref [] in
  let input_dirs = ref [] in
  let anon_fun filename = input_files := filename :: !input_files in
  let output_file = ref "hh_deps_out" in
  let speclist =
    [
      ("-o", Arg.Set_string output_file, "Set output file name");
      ( "-d",
        Arg.String (fun s -> input_dirs := s :: !input_dirs),
        "Declare next string is a directory" );
      ( "--dirs",
        Arg.Rest (fun s -> input_dirs := s :: !input_dirs),
        "All remaining strings are directories" );
    ]
  in
  Arg.parse
    speclist
    anon_fun
    "Get incoming and outgoing dependencies for all specified files, and for all files in specified directories";
  (!input_files, !input_dirs, !output_file)

(**
let get_files_from_dirs input_dirs =

*)

(*Check files and dirs exist, and are correctly labeled as such*)
let validate_paths input_files input_dirs =
  let check_exists =
    Unix.handle_unix_error (fun s ->
        if not (Path.file_exists (Path.make s)) then
          raise (Unix.Unix_error (Unix.EBADF, "check file exists", s)))
  in
  List.iter ~f:check_exists input_files;
  List.iter ~f:check_exists input_dirs;
  let check_is_dir =
    Unix.handle_unix_error (fun s ->
        if not (Path.is_directory (Path.make s)) then
          raise (Unix.Unix_error (Unix.ENOTDIR, "check is directory", s)))
  in
  List.iter ~f:check_is_dir input_dirs;
  let check_not_dir =
    Unix.handle_unix_error (fun s ->
        if Path.is_directory (Path.make s) then
          raise (Unix.Unix_error (Unix.EISDIR, "check not directory", s)))
  in
  List.iter ~f:check_not_dir input_files

let main_deps input_files input_dirs output_file =
  validate_paths input_files input_dirs;
  let all_inputs = List.append input_files input_dirs in
  let all_files = children_files all_inputs in
  Printf.printf "Output File: %s\n%!" output_file;
  List.iter input_files ~f:(fun s -> Printf.printf "Input: %s\n%!" s);
  List.iter input_dirs ~f:(fun s -> Printf.printf "Input dir: %s\n%!" s);
  let out_file_channel = Sys_utils.open_out_no_fail output_file in
  Hh_json.json_to_multiline_output
    out_file_channel
    (json_array_of_json_strings all_files);
  Sys_utils.close_out_no_fail output_file out_file_channel

(* command line driver *)
let () =
  if !Sys.interactive then
    ()
  else
    (* On windows, setting 'binary mode' avoids to output CRLF on
       stdout.  The 'text mode' would not hurt the user in general, but
       it breaks the testsuite where the output is compared to the
       expected one (i.e. in given file without CRLF). *)
    Out_channel.set_binary_mode stdout true;
  let (input_files, input_dirs, output_file) = parse_options () in
  main_deps input_files input_dirs output_file
