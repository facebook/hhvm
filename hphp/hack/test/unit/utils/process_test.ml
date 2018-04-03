(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

open Process

(* check that B is not shadowed when we set A.
 *
 * set B=1 outside and then run:
 *
 * case "$B" in
 *   1) exit 0;;
 *   * ) exit 1;;
 * esac
 *
 * except with * and ) next to each other
 *
 * *)
let test_proc_env () =
  let () = Unix.putenv "B" "1" in
  let proc_t = exec "bash" ~env:["A=1"] ["-c"; "case \"$B\" in 1) exit 0;; *) exit 1;; esac"] in
  let proc_stat_ref = proc_t.Process_types.process_status in
  let proc_stat = !proc_stat_ref in
  match proc_stat with
  | Process_types.Process_aborted _ -> failwith "process aborted"
  | Process_types.Process_running pid -> begin
      match Unix.waitpid [] pid with
      | caught_pid, Unix.WEXITED(status) -> begin
          match caught_pid = pid, status with
          | false, _ -> failwith "impossible: caught wrong child process"
          | true, 0 -> true (* success! *)
          | true, _ -> failwith "child exited abnormally"
      end
      | _ -> failwith "child was killed by signal or something"
  end
  | _ -> failwith "process failed for a different reason"


let tests = [
  "test_proc_env", test_proc_env;
]

let () =
  Unit_test.run_all tests
