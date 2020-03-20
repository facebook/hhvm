(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Asserter
open OUnit2

type _ cache_entry =
  | Int_key : int -> int cache_entry
  | String_key : string -> string cache_entry

module Cache_entry = struct
  type 'a t = 'a cache_entry

  type 'a key = 'a t

  type 'a value = 'a

  let get_size : type a. key:a key -> value:a value -> Lru_cache.size =
   fun ~key ~value ->
    match key with
    | Int_key _ -> 1
    | String_key _ -> String.length value

  let key_to_log_string : type a. a key -> string =
   fun key ->
    match key with
    | Int_key i -> Printf.sprintf "(Int)%d" i
    | String_key s -> Printf.sprintf "(String)%s" s
end

module Cache = Lru_cache.Cache (Cache_entry)

let test_insert _test_ctxt =
  let cache = Cache.make ~max_size:2 in
  Int_asserter.assert_equals
    0
    (Cache.length cache)
    "empty cache should have length 0";
  Cache.add cache ~key:(Int_key 1) ~value:1;
  Int_asserter.assert_equals
    1
    (Cache.length cache)
    "cache after adding an element should have length 1";

  Cache.add cache ~key:(Int_key 1) ~value:2;
  Int_asserter.assert_option_equals
    (Some 2)
    (Cache.find_or_add cache ~key:(Int_key 1) ~default:(fun () -> None))
    "Key should have been overwritten with value 2"

let test_insert_many _test_ctxt =
  let insert_random_entry i cache =
    if Random.bool () then
      Cache.add cache ~key:(Int_key i) ~value:1
    else
      Cache.add cache ~key:(String_key (string_of_int i)) ~value:"a"
  in
  let cache = Cache.make ~max_size:10 in
  for i = 1 to 1000 do
    insert_random_entry i cache
  done;
  Int_asserter.assert_equals
    10
    (Cache.length cache)
    "Size after many inserts should be maxed out"

let test_remove _test_ctxt =
  let cache = Cache.make ~max_size:2 in
  Cache.add cache ~key:(Int_key 1) ~value:1;
  Cache.remove cache ~key:(Int_key 1);
  Cache.remove cache ~key:(Int_key 1);
  Cache.remove cache ~key:(String_key "foo");
  let telemetry = Cache.get_telemetry ~key:"t" cache (Telemetry.create ()) in
  Int_asserter.assert_equals
    0
    (Cache.length cache)
    "empty cache should have length 0";
  Bool_asserter.assert_equals
    true
    (Telemetry_test_utils.is_absent telemetry "t")
    "removed should emit no telemetry";
  ()

let test_eviction_oversized _test_ctxt =
  let cache = Cache.make ~max_size:2 in
  Cache.add cache ~key:(Int_key 1) ~value:1;
  (* The value "ab" has size 2, so we should overflow max cache size. We should
  evict the first key. *)
  Cache.add cache ~key:(String_key "foo") ~value:"ab";
  String_asserter.assert_option_equals
    (Some "ab")
    (Cache.find_or_add cache ~key:(String_key "foo") ~default:(fun () -> None))
    "Should have found entry for key 'foo'";
  Int_asserter.assert_option_equals
    None
    (Cache.find_or_add cache ~key:(Int_key 1) ~default:(fun () -> None))
    "Should NOT have found entry for key 1"

let test_eviction_lru _test_ctxt =
  let cache = Cache.make ~max_size:2 in
  Cache.add cache ~key:(Int_key 1) ~value:1;
  Cache.add cache ~key:(Int_key 2) ~value:2;

  (* Touch key 2. *)
  Int_asserter.assert_option_equals
    (Some 2)
    (Cache.find_or_add cache ~key:(Int_key 2) ~default:(fun () -> None))
    "Should have successfully touched key 2";

  (* Should evict key 1. *)
  Cache.add cache ~key:(Int_key 3) ~value:3;
  Int_asserter.assert_equals
    2
    (Cache.length cache)
    "full cache should have length 2";
  Int_asserter.assert_option_equals
    None
    (Cache.find_or_add cache ~key:(Int_key 1) ~default:(fun () -> None))
    "key 1 should have been evicted"

let test_telemetry _test_ctxt =
  let cache = Cache.make ~max_size:2 in
  let telemetry = Cache.get_telemetry ~key:"t" cache (Telemetry.create ()) in
  Bool_asserter.assert_equals
    true
    (Telemetry_test_utils.is_absent telemetry "t")
    "init telemetry should be absent";

  Cache.add cache ~key:(Int_key 1) ~value:1;
  Cache.add cache ~key:(Int_key 2) ~value:2;
  Cache.add cache ~key:(Int_key 3) ~value:3;

  let telemetry = Cache.get_telemetry ~key:"t" cache (Telemetry.create ()) in
  Int_asserter.assert_equals
    2
    (Telemetry_test_utils.int_exn telemetry "t.total_size")
    "post-add size should be two";
  Int_asserter.assert_equals
    2
    (Telemetry_test_utils.int_exn telemetry "t.length")
    "post-add length should be two";
  Int_asserter.assert_equals
    1
    (Telemetry_test_utils.int_exn telemetry "t.num_evictions")
    "post-add one eviction hass occurred";
  Bool_asserter.assert_equals
    true
    (Telemetry_test_utils.float_exn telemetry "t.time_spent" > 0.0)
    "post-add should have spent >0 time on cache operations so far";

  Cache.reset_telemetry cache;

  let telemetry = Cache.get_telemetry ~key:"t" cache (Telemetry.create ()) in
  Int_asserter.assert_equals
    2
    (Telemetry_test_utils.int_exn telemetry "t.total_size")
    "post-reset size should be two";
  Int_asserter.assert_equals
    2
    (Telemetry_test_utils.int_exn telemetry "t.length")
    "post-reset length should be two";
  Int_asserter.assert_equals
    0
    (Telemetry_test_utils.int_exn telemetry "t.num_evictions")
    "post-reset no evictions";
  Bool_asserter.assert_equals
    true
    (Telemetry_test_utils.float_exn telemetry "t.time_spent" = 0.0)
    "post-reset should have 0 time spent";

  ()

let () =
  "lru_cache_test"
  >::: [
         "test_insert" >:: test_insert;
         "test_insert_many" >:: test_insert_many;
         "test_remove" >:: test_remove;
         "test_eviction_oversized" >:: test_eviction_oversized;
         "test_eviction_lru" >:: test_eviction_lru;
         "test_telemetry" >:: test_telemetry;
       ]
  |> run_test_tt_main
