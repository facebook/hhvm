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

module Tempfile = struct

  let mkdtemp () =
    let tmp_dir = Sys_utils.temp_dir_name in
    let tmp_dir = Path.make tmp_dir in
    let name = Random_id.(short_string_with_alphabet alphanumeric_alphabet) in
    let tmp_dir = Path.concat tmp_dir name in
    let () = Unix.mkdir (Path.to_string tmp_dir) 0o740 in
    tmp_dir

  let with_tempdir g =
    let dir = mkdtemp () in
    let f = (fun () -> g dir) in
    Utils.try_finally ~f ~finally:(fun () ->
      Sys_utils.rm_dir_tree (Path.to_string dir))

end;;

exception Expected_throw_missing
exception Thrown_exception_mismatched of (exn * exn)

let expect_throws e f = fun () ->
  try
    let _ = f () in
    Printf.eprintf "Error. Did not throw expected: %s\n" (Printexc.to_string e);
    false
  with | err ->
    if e <> err then
      let () = Printf.eprintf "Error. Expected exn: %s. But got : %s\n"
      (Printexc.to_string e) (Printexc.to_string err) in
      false
    else
      true

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
