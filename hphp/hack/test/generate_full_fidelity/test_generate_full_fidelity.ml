(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Full_fidelity_schema

let verify_file template =
  let file = open_in template.filename in
  let result = really_input_string file (in_channel_length file) in
  close_in file;

  let expected = generate_string template in
  if result <> expected then (
    Printf.printf
      "Run `buck run hphp/hack/src:generate_full_fidelity` and include changed files in your commit\n";
    exit 1
  )

let () = List.iter verify_file Generate_full_fidelity_data.templates
