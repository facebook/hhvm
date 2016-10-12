(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

type key = Digest.t

module IntVal = struct
  type t = int
  let prefix = Prefix.make()
  let description = "IntVal"
end

let expect ~msg bool =
  if bool then () else begin
    print_endline msg;
    Printexc.(get_callstack 100 |> print_raw_backtrace stderr);
    exit 1
  end

let expect_equals ~name value expected =
  let str x = match x with
    | None -> "None"
    | Some n -> Printf.sprintf "(Some '%d')" n
  in
  expect
    ~msg:(
      Printf.sprintf "Expected key %s to equal %s, got %s"
        name (str expected) (str value)
    )
    (value = expected)

let test_unshelve (module IntHeap: SharedMem.NoCache
                  with type t = int
                   and type key = string
                ) () =
  let key = IntHeap.KeySet.singleton "0" in

  IntHeap.add "0" 0;
  expect_equals "0" (IntHeap.get "0") (Some 0);

  IntHeap.shelve_batch key;
  expect_equals "0" (IntHeap.get "0") None;
  expect_equals "0" (IntHeap.get_shelved "0") (Some 0);

  IntHeap.add "0" 1;
  expect_equals "0" (IntHeap.get "0") (Some 1);

  IntHeap.unshelve_batch key;
  expect_equals "0" (IntHeap.get "0") (Some 0);
  expect_equals "0" (IntHeap.get_shelved "0") None;

  IntHeap.shelve_batch key;
  expect_equals "0" (IntHeap.get "0") None;
  expect_equals "0" (IntHeap.get_shelved "0") (Some 0);

  IntHeap.unshelve_batch key;
  expect_equals "0" (IntHeap.get "0") (Some 0);
  expect_equals "0" (IntHeap.get_shelved "0") None;

  IntHeap.unshelve_batch key;
  expect_equals "0" (IntHeap.get "0") None;
  expect_equals "0" (IntHeap.get_shelved "0") None

let tests () =
  let list = [
    "test_unshelve_no_cache", test_unshelve (
      module SharedMem.NoCache (StringKey) (IntVal)
    );
    "test_unshelve_with_cache", test_unshelve (
      module SharedMem.WithCache (StringKey) (IntVal)
    );
  ] in
  let setup_test (name, test) = name, fun () ->
  let handle = SharedMem.(
      init {
        global_size = 16;
        heap_size = 1024;
        dep_table_pow = 2;
        hash_table_pow = 3;
        shm_dirs = [];
        shm_min_avail = 0;
        log_level = 0;
      }
    ) in
    SharedMem.connect handle ~is_master:true;
    test ();
    true
  in
  List.map setup_test list

let () =
  Unit_test.run_all (tests ())
