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

let finale_file_for_eventual_exit : string option ref = ref None

let set_finale_file_for_eventual_exit (finale_file : string) : unit =
  begin
    try Unix.unlink finale_file with _ -> ()
  end;
  finale_file_for_eventual_exit := Some finale_file

let exit
    ?(msg : string option)
    ?(stack : string option)
    (exit_status : Exit_status.t) : 'a =
  let exit_code = Exit_status.exit_code exit_status in
  match !finale_file_for_eventual_exit with
  | None -> Stdlib.exit exit_code
  | Some finale_file ->
    let stack =
      Option.value
        ~default:
          (Exception.get_current_callstack_string 99 |> Exception.clean_stack)
        stack
    in
    let finale_data = { exit_status; msg; stack = Utils.Callstack stack } in
    let finale_file = finale_file in
    begin
      try
        Sys_utils.with_umask 0o000 (fun () ->
            let oc = Stdlib.open_out_bin finale_file in
            Marshal.to_channel oc finale_data [];
            Stdlib.close_out oc)
      with _ -> ()
    end;
    Stdlib.exit exit_code
