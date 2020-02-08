(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

let expect file stack =
  let matches s = String_utils.string_starts_with s file in
  if List.exists matches (String_utils.split_on_newlines stack) then
    ()
  else
    let () =
      Printf.eprintf "expected '%s' but didn't find it:\n%s" file stack
    in
    failwith (Printf.sprintf "didn't find '%s'" file)

let test_current_stack () =
  let stack =
    Exception.get_current_callstack_string 99 |> Exception.clean_stack
  in
  expect "test/unit/utils/exception_test.ml" stack;
  expect "test/unit/unit_test.ml" stack;
  true

let bar b =
  let _ = Printf.sprintf "%s" "no inlining" in
  Exception_test_b.intentional_raise b;
  let _ = Printf.sprintf "%s" "no inlining" in
  ()

let foo () =
  let _ = Printf.sprintf "%s" "no inlining" in
  let () = bar true in
  let _ = Printf.sprintf "%s" "no inlining" in
  ()

let test_exception_stack () =
  try
    foo ();
    failwith "Expected foo to throw an exception"
  with e ->
    let e = Exception.wrap e in
    let stack = Exception.get_backtrace_string e |> Exception.clean_stack in
    expect "test/unit/utils/exception_test_b.ml" stack;
    expect "test/unit/utils/exception_test.ml" stack;
    true

let tests =
  [
    ("test_current_stack", test_current_stack);
    ("test_exception_stack", test_exception_stack);
  ]

let () = Unit_test.run_all tests
