(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module C = Convert_ty
module H = Hack_ty
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

type test_convert_ty_case =
  {name: string;
   input: T.at_ty;
   strict: C.result;
   loose: C.result;
  }

let test_convert_ty () =
  let cases = [
    {name = "simple";
     input = T.ATstring;
     strict = Common.Left H.Hstring;
     loose = Common.Left H.Hstring;
    };
    {name = "mixed";
     input = T.ATmixed;
     strict = Common.Right "";
     loose = Common.Left H.Hmixed;
    };
    (* TODO enable these tests (and write more), testing currently unimplemented
     * behavior. *)
    (*
    {name = "nullable mixed";
     input = T.ATcomposite [T.ATmixed; T.ATnull];
     strict = Common.Left H.Hmixed;
     loose = Common.Left H.Hmixed;
    };
    {name = "multiple reduce";
     input = T.ATcomposite [T.ATint; T.ATnull; T.ATfloat];
     strict = Common.Left (H.Hnullable H.Hnum);
     loose = Common.Left (H.Hnullable H.Hnum);
    };
    *)
  ] in
  let check_single name output expected = match output, expected with
    | Common.Left e1, Common.Left e2 when e1 = e2 -> ()
    | Common.Right _, Common.Right _ -> ()
    | _ -> raise (Test_failure name) in
  let check {name; input; strict; loose} =
    check_single (name ^ " (strict)") (C.convert C.Strict input) strict;
    check_single (name ^ " (loose)") (C.convert C.Loose input) loose;
    ()
  in
  List.iter check cases

let test () =
  test_at_parse ();
  test_convert_ty ();
  print_endline "Success!"

let () = test ()
