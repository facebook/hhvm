(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let assert_eq v =
  let ocaml_marshaled = Marshal.to_string v [] in
  let rust_marshaled = Ocamlrep_marshal_ffi.to_string v [] in
  if not (String.equal rust_marshaled ocaml_marshaled) then begin
    Printf.printf
      "OCaml Marshal output does not match Rust ocamlrep_marshal output:\n%!";
    Printf.printf "ocaml:\t%S\n%!" ocaml_marshaled;
    Printf.printf "rust:\t%S\n%!" rust_marshaled
  end

let test_round_trip show (x : 'a) =
  let bytes = Ocamlrep_marshal_ffi.to_string x [] in
  let y : 'a = Ocamlrep_marshal_ffi.from_string bytes 0 in
  let _ = Printf.printf "y = %s\n" (show y) in
  assert (x = y)

let show_pair_int_int = [%derive.show: int * int]

let show_pair_opt_int_string = [%derive.show: int option * string]

let show_float_list = [%derive.show: float list]

let show_float_array x = show_float_list (Array.to_list x)

let () =
  assert_eq 5;
  assert_eq 3.14;
  assert_eq (3, 3);
  assert_eq "a";
  assert_eq (Some 42, "foo");

  test_round_trip string_of_int 5;
  test_round_trip string_of_float 3.14;
  test_round_trip show_pair_int_int (3, 3);
  test_round_trip (Printf.sprintf "%S") "a";
  test_round_trip show_pair_opt_int_string (Some 42, "foo");
  test_round_trip show_float_array (Array.make 3 3.14);

  ()
