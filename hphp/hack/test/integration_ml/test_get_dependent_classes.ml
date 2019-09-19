(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

open Hh_core
module Test = Integration_test_base

let files =
  [
    ("C", "class C {}");
    ("D", "class D extends C {}");
    ("E", "class E extends D {}");
    ("F", "trait F { require extends E; }");
    ("G", "trait G { use F; }");
    ("H", "trait H {}");
    ("I", "class I { use H; }");
    ("J", "interface J {}");
    ("K", "interface K extends J {}");
    ("L", "trait L { require implements K; }");
    ("M", "class :M {}");
    ("N", "class :N { attribute :M; }");
    ("Unrelated", "class Unrelated {}");
  ]
  |> List.map ~f:(fun (name, contents) -> (name ^ ".php", "<?hh\n" ^ contents))

let test () =
  let env = Test.setup_server () in
  let env = Test.setup_disk env files in
  Test.assert_no_errors env;

  let get_classes path =
    match Naming_table.get_file_info env.ServerEnv.naming_table path with
    | None -> SSet.empty
    | Some info -> SSet.of_list @@ List.map info.FileInfo.classes ~f:snd
  in
  let dependent_classes =
    Decl_redecl_service.get_dependent_classes
      None
      ~bucket_size:1
      get_classes
      (SSet.of_list ["\\C"; "\\H"; "\\J"; "\\:M"])
  in
  let expected_dependent_classes =
    List.sort
      ~cmp:String.compare
      [
        "\\C";
        "\\D";
        "\\E";
        "\\F";
        "\\G";
        "\\H";
        "\\I";
        "\\J";
        "\\K";
        "\\L";
        "\\:M";
        "\\:N";
      ]
  in
  List.iter (SSet.elements dependent_classes) ~f:print_endline;

  if SSet.elements dependent_classes <> expected_dependent_classes then
    Test.fail "Missing dependent classes"
