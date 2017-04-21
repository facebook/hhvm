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

let test_local_changes (
    module IntHeap: SharedMem.NoCache
      with type t = int
       and type key = string
  ) () =
  let expect_value ~name expected =
    expect_equals ~name (IntHeap.get name) expected;
    let mem = expected <> None in
    let str = if mem then "be" else "not be" in
    expect
      ~msg:(
        Printf.sprintf "Expected key %s to %s a member" name str
      )
      (IntHeap.mem name = mem)
  in

  let expect_add key value =
    IntHeap.add key value;
    expect_value key (Some value)
  in
  let expect_remove key =
    IntHeap.remove_batch (IntHeap.KeySet.singleton key);
    expect_value key None
  in

  let test () =
    expect_value "Filled" (Some 0);
    expect_value "Empty" None;

    IntHeap.LocalChanges.push_stack ();
    begin
      expect_value "Filled" (Some 0);
      expect_value "Empty" None;

      (* For local changes we allow overriding values *)
      expect_add "Filled" 1;
      expect_add "Empty" 2;

      expect_remove "Filled";
      expect_remove "Empty";

      expect_add "Filled" 1;
      expect_add "Empty" 2
    end;
    IntHeap.LocalChanges.pop_stack ();
    expect_value "Filled" (Some 0);
    expect_value "Empty" None;

    (* Commit changes are reflected in the shared memory *)
    IntHeap.LocalChanges.push_stack ();
    begin
      expect_add "Filled" 1;
      expect_add "Empty" 2
    end;
    IntHeap.LocalChanges.commit_all ();
    IntHeap.LocalChanges.pop_stack ();
    expect_value "Filled" (Some 1);
    expect_value "Empty" (Some 2);

    IntHeap.LocalChanges.push_stack ();
    begin
      expect_remove "Filled";
      expect_remove "Empty"
    end;
    IntHeap.LocalChanges.pop_stack ();
    expect_value "Filled" (Some 1);
    expect_value "Empty" (Some 2);

    IntHeap.LocalChanges.push_stack ();
    begin
      expect_remove "Filled";
      expect_remove "Empty"
    end;
    IntHeap.LocalChanges.commit_all ();
    IntHeap.LocalChanges.pop_stack ();
    expect_value "Filled" None;
    expect_value "Empty" None;

    IntHeap.LocalChanges.push_stack ();
    begin
      expect_add "Filled" 0;
      expect_add "Empty" 2;
      IntHeap.LocalChanges.commit_batch (IntHeap.KeySet.singleton "Filled");
      IntHeap.LocalChanges.revert_batch (IntHeap.KeySet.singleton "Empty");
      expect_value "Filled" (Some 0);
      expect_value "Empty" None
    end;
    IntHeap.LocalChanges.pop_stack ();
    expect_value "Filled" (Some 0);
    expect_value "Empty" None
  in

  IntHeap.add "Filled" 0;
  test ();

  IntHeap.(remove_batch @@ KeySet.singleton "Filled");
  IntHeap.add "Empty" 2;
  IntHeap.LocalChanges.push_stack ();
  IntHeap.add "Filled" 0;
  IntHeap.(remove_batch @@ KeySet.singleton "Empty");
  test ();
  IntHeap.LocalChanges.pop_stack ()

module TestNoCache = SharedMem.NoCache (StringKey) (IntVal)
module TestWithCache = SharedMem.WithCache (StringKey) (IntVal)

let tests () =
  let list = [
    "test_local_changes_no_cache", test_local_changes (module TestNoCache);
    "test_local_changes_with_cache", test_local_changes (module TestWithCache);
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
