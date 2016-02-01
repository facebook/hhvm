(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Core

let payload = "Hello"

let test_basic_no_timeout () =
  Timeout.with_timeout
    ~timeout:1
    ~on_timeout:(fun _ -> false)
    ~do_:begin fun _ ->
      true
    end

let test_basic_with_timeout () =
  try
    Timeout.with_timeout
      ~timeout:1
      ~on_timeout:(fun _ -> true)
      ~do_:begin fun timeout ->
        let _ = Unix.select [] [] [] 2.0 in
        false
      end
  with
  | Timeout.Timeout -> true

(** The main method of the forked child process which will feed
 * data to the oc channel in two chunks split up by "delay" seconds. *)
let data_producer delay oc =
  let _ = Unix.select [] [] [] delay in
  Daemon.to_channel oc payload

let data_producer_entry =
  Daemon.register_entry_point
    "data_producer_main"
    (fun (delay: float) ((_ic: unit Daemon.in_channel), oc) ->
       data_producer delay oc)

(** Child sends data after a 0.5 second delay. Parent read timeout is
 * 1 second. *)
let test_input_within_timeout () =
  let handle = Daemon.spawn
      ~channel_mode:`socket ~log_mode:Daemon.Parent_streams
      data_producer_entry 0.5 in
  let ic, _ = handle.Daemon.channels in
  Timeout.with_timeout
    ~timeout:1
    ~on_timeout:(fun _ -> false)
    ~do_:begin fun timeout ->
      let result: string = Daemon.from_channel ~timeout ic in
      assert (result = payload);
      true
    end

(** Child sends data after a 3 second delay. Parent read timeout is
 * 2 seconds. *)
let test_input_exceeds_timeout () =
  let handle = Daemon.spawn
      ~channel_mode:`socket data_producer_entry 3.0 in
  let ic, _ = handle.Daemon.channels in
  try
    Timeout.with_timeout
      ~timeout:2
      ~on_timeout:(fun _ -> true)
      ~do_:begin fun timeout ->
        let _result: string = Daemon.from_channel ~timeout ic in
        false
      end
  with
  | Timeout.Timeout -> true

(**
 * We want to use up a lot of CPU time without any I/O operations like
 * Select and without sleeping.
 *
 * Computing Collatz's conjecture is very slow and won't be optimized
 * away by the compiler like a trivial loop and won't integer
 * overflow. *)
let rec collatz_time num count =
  if num = 1 then
    count
  else if (num mod 2) = 0
  then (collatz_time (num / 2) (count + 1))
  else (collatz_time ((num * 3) + 1) (count + 1))

let collatz_accum (t, accum) num =
  let res = collatz_time num 0 in
  Timeout.check_timeout t;
  (t, accum + res)

(** This probably will take weeks to compute. *)
let really_long_computation t =
  let _, total = List.fold (List.range 1000 100000000) ~init:(t, 0)
      ~f:collatz_accum in
  total

(** Child process that wraps an infinite spin loop with a timeout. *)
let slow_computation_with_timeout_entry =
  Daemon.register_entry_point
    "slow_computation_with_timeout"
    begin fun (timeout: int) ((_ic: unit Daemon.in_channel),
                              (oc_: unit Daemon.out_channel)) ->
      try
        Timeout.with_timeout
          ~timeout
          ~on_timeout:(fun _ -> exit 0)
          ~do_:begin fun timeout ->
            let total = really_long_computation timeout in
            if total < 0 then
              exit 2
            else
              exit 1
          end
      with
      | Timeout.Timeout -> exit 0
    end

(** Forks a child that should exit after 2 seconds. *)
let test_timeout_no_input () =
  let handle = Daemon.spawn
      ~channel_mode:`socket ~log_mode:Daemon.Parent_streams
      slow_computation_with_timeout_entry 2 in
  let _ = Unix.select [] [] [] 2.2 in
  let pid = handle.Daemon.pid in
  let result = match Unix.waitpid [Unix.WNOHANG] pid with
    | 0, _ -> false
    | _, Unix.WEXITED 0-> true
    | _ -> false
  in
  if not result then
    Unix.kill pid Sys.sigkill;
  result

(** A process with a Timeout block where most of the Timeout time is consumed
 * by I/O operations. *)
let slow_computation_after_io_entry =
  Daemon.register_entry_point
    "slow_computation_after_io"
    begin fun (timeout: int) ((ic: string Daemon.in_channel),
                              (oc: unit Daemon.out_channel)) ->
      try
        Timeout.with_timeout
          ~timeout
          ~on_timeout:(fun _ -> exit 0)
          ~do_:begin fun timeout ->
            (** Read from ic, which is sent by parent process after a pause. *)
            let result: string = Daemon.from_channel ~timeout ic in
            assert (result = payload);
            let total = really_long_computation timeout in
            if total < 0 then
              exit 2
            else
              exit 1
          end
      with
      | Timeout.Timeout -> exit 0
    end

(** This tests that timeouts from strictly CPU work still happen after the Alarm
 * is paused by IO.
 *
 * See also Timeout.pause_alarm_then_io *)
let test_timeout_after_input () =
  (** This process times out after 2 seconds and exits. *)
  let handle = Daemon.spawn
      ~channel_mode:`socket ~log_mode:Daemon.Parent_streams
      slow_computation_after_io_entry 2 in
  let (_ic, oc) = handle.Daemon.channels in
  (** Wait 1.1 seconds before sending data on the in channel. *)
  let _ = Unix.select [] [] [] 1.1 in
  Daemon.to_channel oc ~flush:true payload;
  (** 0.9 seconds left in the timeout, but its rounded by alarms being integers
   * so give it slightly more time. *)
  let _ = Unix.select [] [] [] 1.1 in
  let pid = handle.Daemon.pid in
  let result = match Unix.waitpid [Unix.WNOHANG] pid with
    | 0, _ -> false
    | _, Unix.WEXITED 0 -> true
    | _ -> false
  in
  if not result then
    Unix.kill pid Sys.sigkill;
  result

let tests = [
  "test_basic_no_timeout", test_basic_no_timeout;
  "test_basic_with_timeout", test_basic_with_timeout;
  "test_input_within_timeout", test_input_within_timeout;
  "test_input_exceeds_timeout", test_input_exceeds_timeout;
  "test_timeout_no_input", test_timeout_no_input;
  "test_timeout_after_input", test_timeout_after_input;
]

let () =
  Daemon.check_entry_point (); (* this call might not return *)
  Unit_test.run_all tests
