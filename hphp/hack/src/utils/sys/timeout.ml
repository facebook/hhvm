(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core
module Unix = Caml_unix

type timings = {
  start_time: float;
  deadline_time: float;  (** caller-supplied deadline *)
  timeout_time: float;  (** actual time we raised the timeout *)
}

let show_timings (t : timings) : string =
  Printf.sprintf
    "{start=%s deadline=%s timeout=%s}"
    (Utils.timestring t.start_time)
    (Utils.timestring t.deadline_time)
    (Utils.timestring t.timeout_time)

let pp_timings fmt t = Format.fprintf fmt "Timeout.timings%s" (show_timings t)

type t = int

exception
  Timeout of {
    exn_id: int;
    timeout_time: float;
    deadline_time: float;
  }

let () =
  Stdlib.Printexc.register_printer (fun exn ->
      match exn with
      | Timeout t ->
        Some
          (Printf.sprintf
             "Timeout.Timeout(id=%d timeout=%s deadline=%s)"
             t.exn_id
             (Utils.timestring t.timeout_time)
             (Utils.timestring t.deadline_time))
      | _ -> None)

(* The IDs are used to tell the difference between timeout A timing out and timeout B timing out.
 * So they only really need to be unique between any two active timeouts in the same process. *)
let id_counter = ref 0

let mk_id () =
  incr id_counter;
  !id_counter

let with_timeout ~timeout ~on_timeout ~do_ =
  let start_time = Unix.gettimeofday () in
  let id = mk_id () in
  let callback () =
    raise
      (Timeout
         {
           exn_id = id;
           timeout_time = Unix.gettimeofday ();
           deadline_time = start_time +. float_of_int timeout;
         })
  in
  try
    let timer = Timer.set_timer ~interval:(float_of_int timeout) ~callback in
    let ret =
      try do_ id with
      | exn ->
        let e = Exception.wrap exn in
        (* Any uncaught exception will cancel the timeout *)
        Timer.cancel_timer timer;
        Exception.reraise e
    in
    Timer.cancel_timer timer;
    ret
  with
  | Timeout { exn_id; timeout_time; deadline_time } when exn_id = id ->
    on_timeout { start_time; timeout_time; deadline_time }

let check_timeout _ = ()

(** Channel *)

type in_channel = Stdlib.in_channel * int option

let ignore_timeout f ?timeout:_ (ic, _pid) = f ic

let input_line = ignore_timeout Stdlib.input_line

let input_value_with_workaround ic =
  (* OCaml 4.03.0 changed the behavior of input_value to no longer
   * throw End_of_file when the pipe has closed. We can simulate that
   * behavior, however, by trying to read a byte afterwards, which WILL
   * raise End_of_file if the pipe has closed
   * http://caml.inria.fr/mantis/view.php?id=7142 *)
  try Stdlib.input_value ic with
  | Failure msg as exn ->
    let e = Exception.wrap exn in
    if String.equal msg "input_value: truncated object" then
      Stdlib.input_char ic |> ignore;
    Exception.reraise e

let input_value = ignore_timeout input_value_with_workaround

let open_in name = (Stdlib.open_in name, None)

let close_in (ic, _) = Stdlib.close_in ic

let close_in_noerr (ic, _) = Stdlib.close_in_noerr ic

let in_channel_of_descr fd = (Unix.in_channel_of_descr fd, None)

let descr_of_in_channel (ic, _) = Unix.descr_of_in_channel ic

let open_process cmd args =
  let (child_in_fd, out_fd) = Unix.pipe () in
  let (in_fd, child_out_fd) = Unix.pipe () in
  Unix.set_close_on_exec in_fd;
  Unix.set_close_on_exec out_fd;
  let pid =
    Unix.(
      create_process
        (Exec_command.to_string cmd)
        args
        child_in_fd
        child_out_fd
        stderr)
  in
  Unix.close child_out_fd;
  Unix.close child_in_fd;
  let ic = (Unix.in_channel_of_descr in_fd, Some pid) in
  let oc = Unix.out_channel_of_descr out_fd in
  (ic, oc)

let open_process_in cmd args =
  let (child_in_fd, out_fd) = Unix.pipe () in
  let (in_fd, child_out_fd) = Unix.pipe () in
  Unix.set_close_on_exec in_fd;
  Unix.set_close_on_exec out_fd;
  Unix.close out_fd;
  let pid =
    Unix.(
      create_process
        (Exec_command.to_string cmd)
        args
        child_in_fd
        child_out_fd
        stderr)
  in
  Unix.close child_out_fd;
  Unix.close child_in_fd;
  let ic = (Unix.in_channel_of_descr in_fd, Some pid) in
  ic

let close_process_in (ic, pid) =
  match pid with
  | None -> invalid_arg "Timeout.close_process_in"
  | Some pid ->
    Stdlib.close_in ic;
    snd (Sys_utils.waitpid_non_intr [] pid)

let read_process ~timeout ~on_timeout ~reader cmd args =
  let (ic, oc) = open_process cmd args in
  let terminate_common () =
    close_in ic;
    Out_channel.close oc;
    ()
  in
  let on_timeout x =
    terminate_common ();
    on_timeout x
  in
  with_timeout ~timeout ~on_timeout ~do_:(fun timeout ->
      try reader timeout ic oc with
      | exn ->
        terminate_common ();
        Exception.reraise (Exception.wrap exn))

let open_connection ?timeout:_ sockaddr =
  (* timeout isn't used in this Alarm_timeout implementation, but is used in Select_timeout *)
  let (ic, oc) = Unix.open_connection sockaddr in
  ((ic, None), oc)

let shutdown_connection (ic, _) = Unix.shutdown_connection ic

let is_timeout_exn id = function
  | Timeout { exn_id; _ } -> exn_id = id
  | _ -> false

let read_connection ~timeout ~on_timeout ~reader sockaddr =
  with_timeout ~timeout ~on_timeout ~do_:(fun timeout ->
      let (tic, oc) = open_connection ~timeout sockaddr in
      try reader timeout tic oc with
      | exn ->
        let e = Exception.wrap exn in
        Out_channel.close oc;
        Exception.reraise e)
