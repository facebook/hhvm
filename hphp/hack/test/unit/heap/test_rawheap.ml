(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

type key = Digest.t

module IntVal = struct
  type t = int

  let prefix = Prefix.make ()

  let description = "Test_IntVal"
end

let expect ~msg bool =
  if bool then
    ()
  else (
    print_endline msg;
    Printexc.(get_callstack 100 |> print_raw_backtrace stderr);
    exit 1
  )

let expect_equals ~name value expected =
  let str x =
    match x with
    | None -> "None"
    | Some n -> Printf.sprintf "(Some '%d')" n
  in
  expect
    ~msg:
      (Printf.sprintf
         "Expected key %s to equal %s, got %s"
         name
         (str expected)
         (str value))
    (value = expected)

let test_heap_handle () =
  let handle = SharedMem.get_handle () in
  expect ~msg:"" SharedMem.(handle.h_heap_size = 409600);
  expect ~msg:"" SharedMem.(handle.h_global_size = 16);
  (* no-op *)
  SharedMem.connect handle ~worker_id:0

let test_serialize_deserialize () =
  let test_val kind x =
    let entry = SharedMem.serialize_raw x in
    let y = SharedMem.deserialize_raw entry in
    expect
      ~msg:
        (Format.sprintf "%s value not preserved via serialize/deserialize" kind)
      (x = y)
  in
  test_val "int" 55;
  test_val "list of ints" [1; 3; 5];
  test_val "option of string" (Some "foobar")

let test_add_raw_get_raw () =
  let test_key_val key value =
    let entry = SharedMem.serialize_raw value in
    let () = SharedMem.add_raw key entry in
    let entry2 = SharedMem.get_raw key in
    let value2 = SharedMem.deserialize_raw entry2 in
    expect ~msg:(Format.sprintf "%s add/get failed" key) (value = value2)
  in
  test_key_val "foo" "bar";
  test_key_val "baz" [1; 4; 6];
  test_key_val "bar" (Some "option")

let tests () =
  let list =
    [
      ("test_heap_handle", test_heap_handle);
      ("test_serialize_deserialize", test_serialize_deserialize);
      ("test_add_raw_get_raw", test_add_raw_get_raw);
    ]
  in
  let setup_test (name, test) =
    ( name,
      fun () ->
        let num_workers = 0 in
        let handle =
          SharedMem.init
            ~num_workers
            {
              SharedMem.global_size = 16;
              heap_size = 409600;
              dep_table_pow = 2;
              hash_table_pow = 12;
              shm_dirs = [];
              shm_min_avail = 0;
              log_level = 0;
              sample_rate = 0.0;
              compression = 0;
            }
        in
        ignore (handle : SharedMem.handle);
        test ();
        true )
  in
  List.map setup_test list

let () = Unit_test.run_all (tests ())
