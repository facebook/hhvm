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
  telemetry: Telemetry.t option;
}

let get_finale_data (server_finale_file : string) : finale_data option =
  try
    let ic = Stdlib.open_in_bin server_finale_file in
    let contents : finale_data = Marshal.from_channel ic in
    Stdlib.close_in ic;
    Some contents
  with
  | _ -> None

let show_finale_data (data : finale_data) : string =
  let (Utils.Callstack stack) = data.stack in
  Printf.sprintf
    "Exit status [%s] %s\n%s\n%s"
    (Exit_status.show data.exit_status)
    (Option.value data.msg ~default:"")
    (Option.value_map data.telemetry ~f:Telemetry.to_string ~default:"")
    (Exception.clean_stack stack)

let hook_upon_clean_exit : (finale_data -> unit) list ref = ref []

let add_hook_upon_clean_exit (hook : finale_data -> unit) : unit =
  hook_upon_clean_exit := hook :: !hook_upon_clean_exit

let exit
    ?(msg : string option)
    ?(telemetry : Telemetry.t option)
    ?(stack : string option)
    (exit_status : Exit_status.t) : 'a =
  let stack =
    Option.value
      ~default:
        (Exception.get_current_callstack_string 99 |> Exception.clean_stack)
      stack
  in
  let server_finale_data =
    { exit_status; msg; stack = Utils.Callstack stack; telemetry }
  in
  List.iter
    (fun hook ->
      try hook server_finale_data with
      | _ -> ())
    !hook_upon_clean_exit;
  Stdlib.exit (Exit_status.exit_code exit_status)
