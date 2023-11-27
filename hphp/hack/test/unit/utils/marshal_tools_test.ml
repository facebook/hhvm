(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

(* This tests ensures that marshaling over an FD sent over Unix Domain
 * Sockets preserves the read position in the FD.
 *
 * Steps:
   * Spawn 2 child processes (1 and 2) with sockets to each (fd1 and fd2).
   * Child 2 sends 2 messages over its FD (message_1 and messsage_2).
   * Parent reads marshaled message from fd2 (should be message_1).
   * Parent sends fd2 to child 1 over fd1.
   * Child 1 process reads a marshaled message from the FD it got (should
   *   resume reading where parent left off, thus should be message_2).
 *)

let int_to_fd (x : int) : Unix.file_descr = Obj.magic x

let fd_to_int (x : Unix.file_descr) : int = Obj.magic x

let payload_message_1 = "Hello parent"

let payload_message_2 = "Hello child 1"

(** Tail call; exits *)
let send_fd_and_wait child_1_pid socket_fd fd_to_be_sent =
  (* Before sending FD to child 1, read a message from it. *)
  let msg : string = Marshal_tools.from_fd_with_preamble fd_to_be_sent in
  if msg <> payload_message_1 then (
    Printf.eprintf "Parent: error reading message. Got %s\n" msg;
    exit 1
  );
  let result = Libancillary.ancil_send_fd socket_fd fd_to_be_sent in
  if result = -1 then (
    Printf.eprintf "Parent: Failed to send fd. Exiting\n";
    let _ = Unix.wait () in
    exit 1
  ) else
    let (_, status) = Unix.waitpid [] child_1_pid in
    match status with
    | Unix.WEXITED 0 ->
      print_endline "Success!";
      exit 0
    | Unix.WEXITED i ->
      Printf.eprintf "Error: Child 1 exited with code %i" i;
      exit 1
    | _ ->
      Printf.eprintf "Error. Unexpected status";
      exit 1

(** Tail call; exits *)
let child_1_process socket_fd =
  (* Receive the fd from parent process which will be used to get messages
   * from child 2. *)
  let upward_fd_2 =
    try Libancillary.ancil_recv_fd socket_fd with
    | Libancillary.Receiving_Fd_Exception err ->
      Printf.eprintf "Child 1: Failed to receive fd: %s. Exiting.\n" err;
      exit 1
  in
  let msg : string = Marshal_tools.from_fd_with_preamble upward_fd_2 in
  (* Reading should resume where parent left off, so should be message_2. *)
  if msg = payload_message_2 then
    exit 0
  else (
    Printf.eprintf "Child 1: Got message: %s\n" msg;
    exit 1
  )

(** Tail call; exits *)
let child_2_process socket_fd =
  Marshal_tools.to_fd_with_preamble socket_fd payload_message_1 |> ignore;
  Marshal_tools.to_fd_with_preamble socket_fd payload_message_2 |> ignore;
  exit 0

(** After forking once, continue as the parent to fork the second child.
 * downward_1_fd is the socket to send data to child 1. *)
let continue_parent child_1_pid downward_fd_1 =
  let (downward_fd_2, upward_fd_2) =
    Unix.socketpair Unix.PF_UNIX Unix.SOCK_STREAM 0
  in
  let child_2_pid = Unix.fork () in
  if child_2_pid = -1 then (
    Printf.eprintf "Parent: Error forking child 2. Exiting with code 1\n";
    exit 1
  ) else if child_2_pid = 0 then (
    (* Child process doesn't use this. *)
    Unix.close downward_fd_2;
    child_2_process upward_fd_2
  ) else
    Unix.close upward_fd_2;
  send_fd_and_wait child_1_pid downward_fd_1 downward_fd_2

let test_ok () =
  (* This socket lets parent send data to child 1. We will be using it to
   * send a file descriptor to child 1. *)
  let (upward_fd_1, downward_fd_1) =
    Unix.socketpair Unix.PF_UNIX Unix.SOCK_STREAM 0
  in
  let child_1_pid = Unix.fork () in
  if child_1_pid = -1 then (
    Printf.eprintf "Owner: Error forking child 1. Exiting with code 1\n";
    exit 1
  ) else if child_1_pid = 0 then (
    (* Child process doesn't use this. *)
    Unix.close downward_fd_1;
    child_1_process upward_fd_1
  ) else (
    (* Parent process doesn't use this. *)
    Unix.close upward_fd_1;
    continue_parent child_1_pid downward_fd_1
  )

let test_err () =
  (* Let's open and close a socket pair *)
  let (upward_fd_1, downward_fd_1) =
    Unix.socketpair Unix.PF_UNIX Unix.SOCK_STREAM 0
  in
  Unix.close upward_fd_1;
  Unix.close downward_fd_1;

  try
    let _upward : Unix.file_descr = Libancillary.ancil_recv_fd upward_fd_1 in
    Printf.eprintf "Expected an exception to be thrown: exiting\n";
    exit 1
  with
  | Libancillary.Receiving_Fd_Exception err ->
    Printf.eprintf "Failed to receive fd: %s; as expected.\n" err;
    ()

let () =
  Printf.eprintf "** Testing error scenario.\n";
  test_err ();
  Printf.eprintf "** Testing success condition.\n";
  test_ok ()
