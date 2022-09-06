(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

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
  assert (Poly.equal x y)

let show_pair_int_int = [%derive.show: int * int]

let show_pair_opt_int_string = [%derive.show: int option * string]

let show_float_list = [%derive.show: float list]

let show_float_array x = show_float_list (Array.to_list x)

(* A type of non-empty trees of strings. *)
type tree = [
  |`Node of string * tree list
] [@@ocamlformat "disable"]

(* [print tree] produces a rendering of [tree]. *)
let rec print_tree
          ?(pad : (string * string)= ("", ""))
          (tree : tree) : string list =
  let pd, pc = pad in
  match tree with
  | `Node (tag, cs) ->
     let n = List.length cs - 1 in
     Printf.sprintf "%s%s" pd tag :: List.concat (Caml.List.mapi (
         fun i c ->
         let pad =
           (pc ^ (if i = n then "`-- " else "|-- "),
            pc ^ (if i = n then "    " else "|   ")) in
         print_tree ~pad c
       ) cs) [@@ocamlformat "disable"]

(* [show_tree] produces a string of [t]. *)
let show_tree t =
  Printf.sprintf "\n%s\n" (String.concat ~sep:"\n" (print_tree t))

(* An example tree. *)
let tree =
  `Node ("."
        , [
            `Node ("S", [
                      `Node ("T", [
                                `Node ("U", [])]);
                      `Node ("V", [])])
          ;  `Node ("W", [])
          ]) [@@ocamlformat "disable"]

let test_sharing () =
  let s = "str" in
  let inner = (s, s) in
  let outer = (inner, inner) in
  begin
    let marshaled = Ocamlrep_marshal_ffi.to_string outer [] in
    match Ocamlrep_marshal_ffi.from_string marshaled 0 with
    | (((s1, s2) as tup1), tup2) ->
      assert (phys_equal tup1 tup2);
      assert (phys_equal s1 s2);
      ()
  end;
  let marshaled = Ocamlrep_marshal_ffi.to_string outer [Marshal.No_sharing] in
  match Ocamlrep_marshal_ffi.from_string marshaled 0 with
  | (((s1, s2) as tup1), (s3, s4)) as tup2 ->
    assert (not (phys_equal tup1 tup2));
    assert (not (phys_equal s1 s2));
    assert (not (phys_equal s2 s3));
    assert (not (phys_equal s3 s4));
    ()

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
  test_round_trip show_float_array (Caml.Array.make 3 3.14);
  test_round_trip show_tree tree;

  test_sharing ();

  ()
