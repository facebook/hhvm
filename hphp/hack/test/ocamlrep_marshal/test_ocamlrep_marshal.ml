(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

external ocamlrep_marshal_to_string : 'a -> Marshal.extern_flags list -> string
  = "ocamlrep_marshal_output_value_to_string"

let assert_eq v =
  let ocaml_marshaled = Marshal.to_string v [] in
  let rust_marshaled = ocamlrep_marshal_to_string v [] in
  if not (String.equal rust_marshaled ocaml_marshaled) then begin
    Printf.printf
      "OCaml Marshal output does not match Rust ocamlrep_marshal output:\n%!";
    Printf.printf "ocaml:\t%S\n%!" ocaml_marshaled;
    Printf.printf "rust:\t%S\n%!" rust_marshaled
  end

let () =
  assert_eq 5;
  assert_eq 3.14;
  assert_eq (3, 3);
  assert_eq "a";
  assert_eq (Some 42, "foo");
  ()
