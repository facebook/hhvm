(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Option = Base.Option

type finale_data = {
  exit_status: Exit_status.t;
  msg: string option;
  stack: Utils.callstack;
}

let show_finale_data (data : finale_data) : string =
  let (Utils.Callstack stack) = data.stack in
  Printf.sprintf
    "Exit status [%s] %s\n%s"
    (Exit_status.show data.exit_status)
    (Option.value data.msg ~default:"")
    (Exception.clean_stack stack)

type server_specific_files = {
  server_finale_file: string;
  server_progress_file: string;
  server_receipt_to_monitor_file: string;
}

let server_specific_files : server_specific_files option ref = ref None

let prepare_server_specific_files
    ~server_finale_file ~server_progress_file ~server_receipt_to_monitor_file :
    unit =
  server_specific_files :=
    Some
      {
        server_finale_file;
        server_progress_file;
        server_receipt_to_monitor_file;
      };
  (try Unix.unlink server_finale_file with _ -> ());
  (try Unix.unlink server_progress_file with _ -> ());
  (try Unix.unlink server_receipt_to_monitor_file with _ -> ());
  ()

let exit
    ?(msg : string option)
    ?(stack : string option)
    (exit_status : Exit_status.t) : 'a =
  let exit_code = Exit_status.exit_code exit_status in
  match !server_specific_files with
  | None -> Stdlib.exit exit_code
  | Some
      {
        server_finale_file;
        server_progress_file;
        server_receipt_to_monitor_file;
      } ->
    (try Unix.unlink server_progress_file with _ -> ());
    (try Unix.unlink server_receipt_to_monitor_file with _ -> ());
    let stack =
      Option.value
        ~default:
          (Exception.get_current_callstack_string 99 |> Exception.clean_stack)
        stack
    in
    let server_finale_data =
      { exit_status; msg; stack = Utils.Callstack stack }
    in
    begin
      try
        Sys_utils.with_umask 0o000 (fun () ->
            let oc = Stdlib.open_out_bin server_finale_file in
            Marshal.to_channel oc server_finale_data [];
            Stdlib.close_out oc)
      with _ -> ()
    end;
    Stdlib.exit exit_code
