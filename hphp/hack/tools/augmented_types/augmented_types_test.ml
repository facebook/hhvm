(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module T = At_ty

exception Test_failure of string

let test_at_parse () =
  let cases = [
    "string", T.ATstring;
    "string|Klass", T.ATcomposite [T.ATstring; T.ATclass "Klass"];
    "*int[]", T.ATvariadic (T.ATarray (T.ATint));
    "int|string[]", T.ATcomposite [T.ATint; T.ATarray T.ATstring];
    "(int|string)[]", T.ATarray (T.ATcomposite [T.ATint; T.ATstring]);
    "(int|string)|bool", T.ATcomposite [T.ATint; T.ATstring; T.ATbool];
  ] in
  let check (input, output) =
    if (At_parse.parse input) <> output then
    raise (Test_failure input)
    else ()
  in
  List.iter check cases

let test () =
  test_at_parse ();
  print_endline "Success!"

let () = test ()
