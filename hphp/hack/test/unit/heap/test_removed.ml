(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
*)

type key = OpaqueDigest.t

module IntVal = struct
  type t = int
  let prefix = Prefix.make()
  let description = "IntVal"
  let use_sqlite_fallback () = false
end

let test_add_remove (
    module IntHeap: SharedMem.NoCache
      with type t = int
       and type key = string
  ) () =
    assert (SharedMem.hh_removed_count () = 0);
    IntHeap.add "a" 4;
    assert (SharedMem.hh_removed_count () = 0);
    assert (IntHeap.mem "a");
    IntHeap.remove_batch (IntHeap.KeySet.singleton "a");
    assert (not @@ IntHeap.mem "a");
    assert (SharedMem.hh_removed_count () = 1)

module TestNoCache = SharedMem.NoCache (StringKey) (IntVal)

let tests () =
  let list = [
    "test_add_remove", test_add_remove (module TestNoCache);
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
