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

let run (name, f) =
  Printf.printf "Running %s ... %!" name;
  let result = try f () with
    | e ->
      let () = Printf.printf "Exception %s\n" (Printexc.to_string e) in
      let () = Printf.printf "Backtrace %s\n" (Printexc.get_backtrace ()) in
      false
  in
  (if result
  then Printf.printf "ok\n%!"
  else Printf.printf "fail\n%!");
  result

let run_all (tests: (string * (unit -> bool)) list ) =
  exit (if List.for_all tests run then 0 else 1)
