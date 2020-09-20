(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let run (f : unit -> unit) : unit -> bool =
 fun () ->
  f ();
  true

let go tests =
  if Array.length Sys.argv != 2 then
    failwith "Expecting exactly one test name"
  else
    let test_name = Sys.argv.(1) in
    let tests = List.filter (fun (name, _test) -> test_name = name) tests in
    match tests with
    | [] -> failwith (Printf.sprintf "Test named '%s' was not found!" test_name)
    | [test] -> Unit_test.run_all [test]
    | _ :: _ :: _ ->
      failwith
        (Printf.sprintf "More than one test named '%s' was found!" test_name)
