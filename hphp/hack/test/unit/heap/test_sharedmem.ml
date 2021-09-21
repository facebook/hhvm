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

  let description = "Test_IntVal"
end

module Capacity = struct
  let capacity = 1000
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

let test_local_changes
    (module IntHeap : SharedMem.Heap with type value = int and type key = string)
    () =
  let expect_value ~name expected =
    expect_equals ~name (IntHeap.get name) expected;
    let mem = expected <> None in
    let str =
      if mem then
        "be"
      else
        "not be"
    in
    expect
      ~msg:(Printf.sprintf "Expected key %s to %s a member" name str)
      (IntHeap.mem name = mem)
  in
  let expect_add key value =
    IntHeap.add key value;
    expect_value ~name:key (Some value)
  in
  let expect_remove key =
    IntHeap.remove_batch (IntHeap.KeySet.singleton key);
    expect_value ~name:key None
  in
  let test () =
    expect_value ~name:"Filled" (Some 0);
    expect_value ~name:"Empty" None;

    IntHeap.LocalChanges.push_stack ();

    expect_value ~name:"Filled" (Some 0);
    expect_value ~name:"Empty" None;

    (* For local changes we allow overriding values *)
    expect_add "Filled" 1;
    expect_add "Empty" 2;

    expect_remove "Filled";
    expect_remove "Empty";

    expect_add "Filled" 1;
    expect_add "Empty" 2;

    IntHeap.LocalChanges.pop_stack ();
    expect_value ~name:"Filled" (Some 0);
    expect_value ~name:"Empty" None;

    (* Commit changes are reflected in the shared memory *)
    IntHeap.LocalChanges.push_stack ();

    expect_add "Filled" 1;
    expect_add "Empty" 2;

    IntHeap.LocalChanges.commit_all ();
    IntHeap.LocalChanges.pop_stack ();
    expect_value ~name:"Filled" (Some 1);
    expect_value ~name:"Empty" (Some 2);

    IntHeap.LocalChanges.push_stack ();

    expect_remove "Filled";
    expect_remove "Empty";

    IntHeap.LocalChanges.pop_stack ();
    expect_value ~name:"Filled" (Some 1);
    expect_value ~name:"Empty" (Some 2);

    IntHeap.LocalChanges.push_stack ();

    expect_remove "Filled";
    expect_remove "Empty";

    IntHeap.LocalChanges.commit_all ();
    IntHeap.LocalChanges.pop_stack ();
    expect_value ~name:"Filled" None;
    expect_value ~name:"Empty" None;

    IntHeap.LocalChanges.push_stack ();

    expect_add "Filled" 0;
    expect_add "Empty" 2;
    IntHeap.LocalChanges.commit_batch (IntHeap.KeySet.singleton "Filled");
    IntHeap.LocalChanges.revert_batch (IntHeap.KeySet.singleton "Empty");
    expect_value ~name:"Filled" (Some 0);
    expect_value ~name:"Empty" None;

    IntHeap.LocalChanges.pop_stack ();
    expect_value ~name:"Filled" (Some 0);
    expect_value ~name:"Empty" None
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

module type HeapWithLocalCache =
    module type of
      SharedMem.HeapWithLocalCache (SharedMem.ImmediateBackend) (StringKey)
        (IntVal)
        (Capacity)

let test_cache_behavior (module IntHeap : HeapWithLocalCache) () =
  let expect_cache_size expected =
    let seq_len = Seq.fold_left (fun x _ -> x + 1) 0 in
    let actual =
      seq_len @@ snd @@ IntHeap.Cache.get_telemetry_items_and_keys ()
    in
    expect
      ~msg:(Printf.sprintf "Expected cache size of %d, got %d" expected actual)
      (expected = actual)
  in
  expect_cache_size 0;

  (* Fill the L1 cache. *)
  for i = 1 to 1000 do
    IntHeap.add (Printf.sprintf "%d" i) i;
    expect_cache_size i
  done;

  (* Make sure the L1 cache does not grow past capacity - L2 will. *)
  for i = 1001 to 2000 do
    IntHeap.add (Printf.sprintf "%d" i) i;
    expect_cache_size (1000 + (i - 1000))
  done

(* Cannot test beyond this point. We don't know how many unique keys are
   in the combined cache. *)

module TestNoCache =
  SharedMem.Heap (SharedMem.ImmediateBackend) (StringKey) (IntVal)

(* We shall not mix compressions, so create 2 separate caches  *)
module TestWithCacheLz4 =
  SharedMem.HeapWithLocalCache (SharedMem.ImmediateBackend) (StringKey) (IntVal)
    (Capacity)
module TestWithCacheZstd =
  SharedMem.HeapWithLocalCache (SharedMem.ImmediateBackend) (StringKey) (IntVal)
    (Capacity)

let tests () =
  let zstd_compression_with_default_level = 3 in
  let lz4_compression = 0 in
  let list =
    [
      ( "test_local_changes_no_cache",
        test_local_changes (module TestNoCache),
        lz4_compression );
      ( "test_local_changes_with_cache",
        test_local_changes (module TestWithCacheLz4),
        lz4_compression );
      ( "test_local_changes_with_cache_zstd",
        test_local_changes (module TestWithCacheZstd),
        zstd_compression_with_default_level );
      ( "test_cache_behavior",
        test_cache_behavior (module TestWithCacheLz4),
        lz4_compression );
      ( "test_cache_behavior_zstd",
        test_cache_behavior (module TestWithCacheZstd),
        zstd_compression_with_default_level );
    ]
  in
  let setup_test (name, test, compression) =
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
              shm_use_sharded_hashtbl = false;
              shm_min_avail = 0;
              log_level = 0;
              sample_rate = 0.0;
              compression;
            }
        in
        ignore (handle : SharedMem.handle);
        test ();
        true )
  in
  List.map setup_test list

let () = Unit_test.run_all (tests ())
