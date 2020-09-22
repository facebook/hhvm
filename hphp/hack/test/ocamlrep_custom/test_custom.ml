(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

type drop_test (* abstract type for the Rust DropTest type *)

type ref_cell (* abstract type for the Rust Rc<RefCell<bool>> type *)

external drop_test_new : unit -> drop_test = "test_custom_drop_test_new"

external drop_test_custom_ref_count : drop_test -> int
  = "test_custom_drop_test_custom_ref_count"

external drop_test_get_cell : drop_test -> ref_cell
  = "test_custom_drop_test_get_cell"

external drop_test_cell_is_dropped : ref_cell -> bool
  = "test_custom_drop_test_cell_is_dropped"

let test_is_dropped_on_gc () =
  let drop_test = ref (Some (drop_test_new ())) in
  let cell = drop_test_get_cell (Option.value_exn !drop_test) in

  assert (Int.equal (drop_test_custom_ref_count (Option.value_exn !drop_test)) 1);
  assert (not @@ drop_test_cell_is_dropped cell);

  Gc.full_major ();
  assert (Int.equal (drop_test_custom_ref_count (Option.value_exn !drop_test)) 1);
  assert (not @@ drop_test_cell_is_dropped cell);

  drop_test := None;
  Gc.full_major ();
  assert (drop_test_cell_is_dropped cell);

  ()

type boxed_int (* abstract tyep for Rust BoxedInt type *)

external boxed_int_register : unit -> unit = "test_custom_boxed_int_register"

external boxed_int_new : int -> boxed_int = "test_custom_boxed_int_new"

external boxed_int_equal : boxed_int -> boxed_int -> bool
  = "test_custom_boxed_int_equal"

let test_boxed_int_serialize () =
  boxed_int_register ();

  let x = boxed_int_new 1 in
  assert (not @@ boxed_int_equal x (boxed_int_new 2));
  assert (boxed_int_equal x (boxed_int_new 1));

  let x_serialized = Marshal.to_bytes x [] in
  let x_deserialized = Marshal.from_bytes x_serialized 0 in
  Gc.full_major ();

  assert (boxed_int_equal x x_deserialized);
  assert (not @@ boxed_int_equal x_deserialized (boxed_int_new 2));
  Gc.full_major ();

  ()

let test_cases = [test_is_dropped_on_gc; test_boxed_int_serialize]

let main () = List.iter test_cases ~f:(fun test -> test ())

let () = main ()
