(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 *)

type key = Opaque_digest.t

module IntVal = struct
  type t = int

  let description = "Test_IntVal"
end

let test_add_remove
    (module IntHeap : Shared_mem.Heap
      with type value = int
       and type key = string)
    () =
  assert (Shared_mem.SMTelemetry.hh_removed_count () = 0);
  IntHeap.add "a" 4;
  assert (Shared_mem.SMTelemetry.hh_removed_count () = 0);
  assert (IntHeap.mem "a");
  IntHeap.remove_batch (IntHeap.KeySet.singleton "a");
  assert (not @@ IntHeap.mem "a");
  assert (Shared_mem.SMTelemetry.hh_removed_count () = 1)

module TestNoCache =
  Shared_mem.Heap
    (Shared_mem.ImmediateBackend (Shared_mem.NonEvictable)) (String_key)
    (IntVal)

let tests () =
  let list = [("test_add_remove", test_add_remove (module TestNoCache))] in
  let setup_test (name, test) =
    ( name,
      fun () ->
        let num_workers = 0 in
        let handle =
          Shared_mem.init
            ~num_workers
            {
              Shared_mem.global_size = 16;
              heap_size = 1024;
              hash_table_pow = 3;
              shm_dirs = [];
              shm_use_sharded_hashtbl = false;
              shm_cache_size = -1;
              shm_min_avail = 0;
              log_level = 0;
              sample_rate = 0.0;
              compression = 0;
            }
        in
        ignore (handle : Shared_mem.handle);
        test ();
        true )
  in
  List.map setup_test list

let () = Unit_test.run_all (tests ())
