(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_core

module Test = Integration_test_base

let files = [
  "C", "class C {}";
  "D", "class D extends C {}";
  "E", "class E extends D {}";
  "F", "trait F { require extends E; }";
  "G", "trait G { use F; }";

  "H", "trait H {}";
  "I", "class I { use H; }";

  "J", "interface J {}";
  "K", "interface K extends J {}";
  "L", "trait L { require implements K; }";

  "M", "class :M {}";
  "N", "class :N { attribute :M; }";

  "Unrelated", "class Unrelated {}";
] |> List.map ~f:begin fun (name, contents) ->
  (name ^ ".php", "<?hh\n" ^ contents)
end

let () =
  let env = Test.setup_server () in
  let env = Test.setup_disk env files in

  Test.assert_no_errors env;

  let dependent_classes = Decl_redecl_service.get_dependent_classes
    None
    ~bucket_size:1
    env.ServerEnv.files_info
    (SSet.of_list ["\\C"; "\\H"; "\\J"; "\\:M";])
  in

  let expected_dependent_classes = List.sort String.compare
    ["\\C"; "\\D"; "\\E"; "\\F"; "\\G"; "\\H";
      "\\I"; "\\J"; "\\K"; "\\L"; "\\:M"; "\\:N";]
  in

  List.iter (SSet.elements dependent_classes) ~f:print_endline;

  if SSet.elements dependent_classes <> expected_dependent_classes then
    Test.fail "Missing dependent classes"
