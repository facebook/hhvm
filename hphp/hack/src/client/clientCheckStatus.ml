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

module C = TtyColor

let print_reason_color ~(first:bool) ((p, s): Pos.t * string) =
  let line, start, end_ = Pos.info_pos p in
  let err_clr  = if first then C.Bold C.Red else C.Normal C.Green in
  let file_clr = if first then C.Bold C.Red else C.Normal C.Red in
  let line_clr = C.Normal C.Yellow in
  let col_clr  = C.Normal C.Cyan in

  let to_print = [
    (file_clr,           p.Pos.pos_file);
    (C.Normal C.Default, ":");
    (line_clr,           string_of_int line);
    (C.Normal C.Default, ":");
    (col_clr,            string_of_int start);
    (C.Normal C.Default, ",");
    (col_clr,            string_of_int end_);
    (C.Normal C.Default, ": ");
    (err_clr,            s);
    (C.Normal C.Default, "\n");
  ] in

  if not first then Printf.printf "  " else ();
  if Unix.isatty Unix.stdout
  then
    C.print to_print
  else
    let strings = List.map (fun (_,x) -> x) to_print in
    List.iter (Printf.printf "%s") strings

let print_error_color (e:Errors.error) =
  let e = Errors.to_list e in
  print_reason_color ~first:true (List.hd e);
  List.iter (print_reason_color ~first:false) (List.tl e)

module type CHECKER_UTILS = sig
  val start_server : client_check_env -> unit
  val server_name : string
end

module HackCheckerUtils : CHECKER_UTILS = struct
  let start_server args = ClientStart.start_server args.root
  let server_name = "hh_server"
end

module type STATUS_CHECKER = sig
  val check_status : (ClientEnv.client_check_env -> in_channel * out_channel)
    -> client_check_env -> unit
end

module StatusChecker (CheckerUtils : CHECKER_UTILS) : STATUS_CHECKER = struct
  let check_status connect (args:client_check_env) =
    Sys.set_signal Sys.sigalrm (Sys.Signal_handle (fun _ -> raise Server_busy));
    ignore(Unix.alarm 6);

    let name = CheckerUtils.server_name in

    (* Check if a server is up *)
    if not (ClientUtils.server_exists args.root)
    then begin
      ignore (Unix.alarm 0);
      if args.autostart
      then
        (* fork the server and raise an exception *)
        CheckerUtils.start_server args;
      raise Server_missing
    end;
    let ic, oc = connect args in
    ServerMsg.cmd_to_channel oc (ServerMsg.STATUS args.root);
    let response = ServerMsg.response_from_channel ic in
    ignore (Unix.alarm 0);
    match response with
    | ServerMsg.SERVER_OUT_OF_DATE ->
      if args.autostart
      then Printf.printf "%s is outdated, going to launch a new one.\n" name
      else Printf.printf "%s is outdated, killing it.\n" name;
      flush stdout;
      raise Server_missing
    | ServerMsg.NO_ERRORS ->
      ServerError.print_errorl args.output_json [] stdout;
      exit 0
    | ServerMsg.ERRORS e ->
      if args.output_json || args.from <> ""
      then ServerError.print_errorl args.output_json e stdout
      else List.iter print_error_color e;
      exit 2
    | ServerMsg.DIRECTORY_MISMATCH d ->
      Printf.printf "%s is running on a different directory.\n" name;
      Printf.printf "server_root: %s, client_root: %s\n"
        (Path.string_of_path d.ServerMsg.server)
        (Path.string_of_path d.ServerMsg.client);
      flush stdout;
      raise Server_directory_mismatch
    | ServerMsg.SERVER_DYING ->
      Printf.printf "Server has been killed for %s\n"
        (Path.string_of_path args.root);
      exit 2
    | ServerMsg.PONG ->
        Printf.printf "Why on earth did the server respond with a pong?\n%!";
        exit 2
end

module HackStatusChecker = StatusChecker (HackCheckerUtils)
let check_status = HackStatusChecker.check_status
