(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Hh_core

type test_env = {
  (* What callbacks were called and when *)
  callbacks_trace : (string * float) list;
}

module TestScheduler = Scheduler.Make(struct type t = test_env end)

let empty_test_env () = {
  callbacks_trace = [];
}

let record_trace name =
  (fun env -> {
       callbacks_trace = (name, Unix.gettimeofday ()):: env.callbacks_trace
     })

(* Keep running the function until ~steps callbacks have executed *)
let schedule_run_until name ~steps ~priority =
  TestScheduler.wait_for_fun
    ~priority
    (fun env -> (List.length env.callbacks_trace < steps))
    (record_trace name)

let schedule_wait_for_channel name fd ~priority =
  TestScheduler.wait_for_channel
    fd
    (fun env ->
       let payload =  Marshal_tools.from_fd_with_preamble fd in
       record_trace (name ^ ":" ^payload) env
    )
    ~priority

let run_and_expect_trace env trace =
  let env = TestScheduler.wait_and_run_ready env in
  List.iter2_exn env.callbacks_trace trace
    ~f:(fun (x, _) y -> if x <> y then raise Exit);
  env

let test_fun_wait env =
  schedule_run_until "fun" ~steps:3 ~priority:0;
  ignore (run_and_expect_trace env ["fun"; "fun"; "fun"]);
  true

let test_fun_wait_once env =
  TestScheduler.wait_for_fun
    (fun _ -> true)
    (record_trace "fun")
    ~once:true
    ~priority:0;
  ignore (run_and_expect_trace env ["fun"]);
  true

let test_channel_wait env =
  let fd_in, fd_out = Unix.pipe () in

  schedule_wait_for_channel "channel" fd_in ~priority:0;
  let env = run_and_expect_trace env [] in
  Marshal_tools.to_fd_with_preamble fd_out "msg1";
  let env = run_and_expect_trace env ["channel:msg1"] in
  Marshal_tools.to_fd_with_preamble fd_out "msg2";
  let env = run_and_expect_trace env ["channel:msg2"; "channel:msg1"] in
  Marshal_tools.to_fd_with_preamble fd_out "msg3";
  TestScheduler.stop_waiting_for_channel fd_in;
  ignore (run_and_expect_trace env ["channel:msg2"; "channel:msg1"]);
  true

let test_priorities env =
  schedule_run_until "fun1" ~steps:1 ~priority:1;
  schedule_run_until "fun2" ~steps:1 ~priority:0;
  schedule_run_until "fun3" ~steps:1 ~priority:2;
  ignore (run_and_expect_trace env ["fun3"; "fun1"; "fun2"]);
  true

let test_fun_and_channel env =
  let fd_in, _ = Unix.pipe () in

  schedule_run_until "fun" ~steps:1 ~priority:1;
  schedule_wait_for_channel "channel" fd_in ~priority:0;

  let t = Unix.gettimeofday () in
  let env = TestScheduler.wait_and_run_ready env in
  (match env.callbacks_trace with
   (* Check that function that was immediately ready did not block
    * because of the channel *)
   | ["fun", when_] -> when_ -. t < 0.1
   | _ ->  false)

let tests = List.map [
    "test_fun_wait", test_fun_wait;
    "test_fun_wait_once", test_fun_wait_once;
    "test_channel_wait", test_channel_wait;
    "test_priorities", test_priorities;
    "test_fun_and_channel", test_fun_and_channel
  ] begin fun (name, f) ->
    let f' = begin fun () ->
      TestScheduler.reset ();
      let env = (empty_test_env ()) in
      f env
    end in
    (name, f')
  end

let () =
  Unit_test.run_all tests
