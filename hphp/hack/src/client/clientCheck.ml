(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open ClientEnv
open ClientExceptions
open ClientUtils

let get_list_files (args:client_check_env): string list =
  let ic, oc = connect args.root in
  ServerMsg.cmd_to_channel oc ServerMsg.LIST_FILES;
  let res = ref [] in
  try
    while true do
      res := (input_line ic) :: !res
    done;
    assert false
  with End_of_file -> !res

let print_all ic =
  try
    while true do
      Printf.printf "%s\n" (input_line ic);
    done
  with End_of_file -> ()

let expand_path file =
  let path = Path.mk_path file in
  if Path.file_exists path
  then Path.string_of_path path
  else
    let file = Filename.concat (Sys.getcwd()) file in
    let path = Path.mk_path file in
    if Path.file_exists path
    then Path.string_of_path path
    else begin
      Printf.printf "File not found\n";
      exit 2
    end

let rec main args retries =
  let has_timed_out = match args.timeout with
    | None -> false
    | Some t -> Unix.time() > t
  in if has_timed_out
  then begin
      Printf.fprintf stderr "Error: hh_client hit timeout, giving up!\n%!";
      exit 7
  end else try
    match args.mode with
    | MODE_LIST_FILES ->
      let infol = get_list_files args in
      List.iter (Printf.printf "%s\n") infol
    | MODE_SKIP ->
      let ic, oc = connect args.root in
      let command = ServerMsg.SKIP in
      ServerMsg.cmd_to_channel oc command;
      Printf.printf "No errors!\n"; flush stdout;
    | MODE_COLORING file ->
        let file = expand_path file in
        let ic, oc = connect args.root in
        let command = ServerMsg.PRINT_TYPES file in
        ServerMsg.cmd_to_channel oc command;
        let pos_type_l = Marshal.from_channel ic in
        ClientColorFile.go file args.output_json pos_type_l;
        exit 0
    | MODE_FIND_CLASS_REFS name ->
        let ic, oc = connect args.root in
        let command = ServerMsg.FIND_REFS (ServerMsg.Class name) in
        ServerMsg.cmd_to_channel oc command;
        let results = Marshal.from_channel ic in
        ClientFindRefs.go results args.output_json;
        exit 0     
    | MODE_FIND_REFS name ->
        let ic, oc = connect args.root in
        let pieces = Str.split (Str.regexp "::") name in
        let action =
          try
            match pieces with
            | class_name :: method_name :: _ ->
                ServerMsg.Method (class_name, method_name)
            | method_name :: _ -> ServerMsg.Function method_name
            | _ -> raise Exit
          with _ -> Printf.fprintf stderr "Invalid input\n"; exit 1 in
        let command = ServerMsg.FIND_REFS action in
        ServerMsg.cmd_to_channel oc command;
        let results = Marshal.from_channel ic in
        ClientFindRefs.go results args.output_json;
        exit 0
    | MODE_IDENTIFY_FUNCTION arg ->
      let tpos = Str.split (Str.regexp ":") arg in
      let line, char =
        try
          match tpos with
          | [line; char] ->
              int_of_string line, int_of_string char
          | _ -> raise Exit
        with _ ->
          Printf.fprintf stderr "Invalid position\n"; exit 1
      in
      let ic, oc = connect args.root in
      let content = ClientUtils.read_stdin_to_string () in
      let command = ServerMsg.IDENTIFY_FUNCTION (content, line, char) in
      ServerMsg.cmd_to_channel oc command;
      print_all ic
    | MODE_SHOW_TYPES file ->
        Printf.printf "option disabled (sorry!)";
        exit 0
    | MODE_TYPE_AT_POS arg ->
      let tpos = Str.split (Str.regexp ":") arg in
      let fn, line, char =
        try
          match tpos with
          | [filename; line; char] ->
              filename, int_of_string line, int_of_string char
          | _ -> raise Exit
        with _ ->
          Printf.fprintf stderr "Invalid position\n"; exit 1
      in
      let fn = expand_path fn in
      let ic, oc = connect args.root in
      ServerMsg.cmd_to_channel oc (ServerMsg.INFER_TYPE (fn, line, char));
      let (_, ty) = Marshal.from_channel ic in
      print_endline ty
    | MODE_AUTO_COMPLETE ->
      let ic, oc = connect args.root in
      let content = ClientUtils.read_stdin_to_string () in
      let command = ServerMsg.AUTOCOMPLETE content in
      ServerMsg.cmd_to_channel oc command;
      print_all ic
    | MODE_OUTLINE ->
      let content = ClientUtils.read_stdin_to_string () in
      let ic, oc = connect args.root in
      let command = ServerMsg.OUTLINE content in
      ServerMsg.cmd_to_channel oc command;
      let results = Marshal.from_channel ic in
      ClientOutline.go results args.output_json;
      exit 0
    | MODE_STATUS -> ClientCheckStatus.check_status args
    | MODE_VERSION ->
      Printf.printf "%s\n" (Build_id.build_id_ohai);
    | MODE_SAVE_STATE filename ->
        let ic, oc = connect args.root in
        ServerMsg.cmd_to_channel oc (ServerMsg.SAVE_STATE filename);
        let response = input_line ic in
        Printf.printf "%s\n" response;
        flush stdout
    | MODE_SHOW classname ->
        let ic, oc = connect args.root in
        ServerMsg.cmd_to_channel oc (ServerMsg.SHOW classname);
        print_all ic
    | MODE_UNSPECIFIED -> assert false
  with
  | Server_initializing ->
      let init_msg = "hh_server still initializing. If it was "^
                     "just started this can take some time." in
      if args.retry_if_init
      then begin
        Printf.fprintf stderr "%s Retrying...\n" init_msg;
        flush stderr;
        Unix.sleep(1);
        main args retries
      end else begin
        Printf.fprintf stderr "%s Try again...\n" init_msg;
        flush stderr;
      end
  | Server_cant_connect ->
      if retries > 1
      then begin
        Printf.fprintf stderr "Error: could not connect to hh_server, retrying...\n";
        flush stderr;
        Unix.sleep(1);
        main args (retries-1)
      end else begin
        Printf.fprintf stderr "Error: could not connect to hh_server, giving up!\n";
        flush stderr;
        exit 3
      end
  | Server_busy ->
      if retries > 1
      then begin
        Printf.fprintf stderr "Error: hh_server is busy, retrying...\n";
        flush stderr;
        Unix.sleep(1);
        main args (retries-1)
      end else begin
        Printf.fprintf stderr "Error: hh_server is busy, giving up!\n";
        flush stderr;
        exit 4;
      end
  | Server_missing ->
      if args.autostart
      then begin
        if retries > 1
        then begin
          Unix.sleep(3);
          main args (retries-1)
        end else begin
          Printf.fprintf stderr "The server will be ready in a few seconds (a couple of minutes if your files are cold)!\n";
          flush stderr;
          exit 6;
        end
      end else begin
        Printf.fprintf stderr "Error: no hh_server running. Either start hh_server yourself or run hh_client without --autostart-server false\n%!";
        exit 6;
      end
  | Server_directory_mismatch ->
      if retries > 1
      then begin
        Unix.sleep(3);
        main args (retries-1)
      end else begin
        Printf.fprintf stderr "The server will be ready in a few seconds (a couple of minutes if your files are cold)!\n";
        flush stderr;
        exit 6;
      end
  | _ ->
      if retries > 1
      then begin
        Printf.fprintf stderr "Error: hh_server disconnected or crashed, retrying...\n";
        flush stderr;
        Unix.sleep(1);
        main args (retries-1)
      end else begin
        Printf.fprintf stderr "Error: hh_server disconnected or crashed, giving up!\n";
        flush stderr;
        exit 5;
      end
