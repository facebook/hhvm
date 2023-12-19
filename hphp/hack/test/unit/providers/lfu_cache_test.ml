(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Asserter
open OUnit2

type _ cache_entry =
  | Int_key : int -> int cache_entry
  | String_key : string -> string cache_entry

module Cache_entry = struct
  type 'a t = 'a cache_entry

  type 'a key = 'a t

  type 'a value = 'a

  let compare (type ta tb) (a : ta key) (b : tb key) : int =
    match (a, b) with
    | (Int_key a, Int_key b) -> Int.compare a b
    | (Int_key _, String_key _) -> -1
    | (String_key _, Int_key _) -> 1
    | (String_key a, String_key b) -> String.compare a b

  let hash (type ta) (key : ta key) : int =
    let hsv = Hash.create () in
    let hsv =
      match key with
      | Int_key i -> Hash.fold_int (Hash.fold_int hsv 1) i
      | String_key s -> Hash.fold_string (Hash.fold_int hsv 2) s
    in
    Hash.get_hash_value hsv

  let key_to_log_string : type a. a key -> string =
   fun key ->
    match key with
    | Int_key i -> Printf.sprintf "(Int)%d" i
    | String_key s -> Printf.sprintf "(String)%s" s
end

module Cache = Lfu_cache.Cache (Cache_entry)

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
  let capacity = 10 in
  let cache = Cache.make ~max_size:capacity in
  for i = 1 to 1000 do
    insert_random_entry i cache
  done;
  Int_asserter.assert_leq
    ~actual:(Cache.length cache)
    ~expected:(2 * capacity)
    "Size after many inserts should not be greater than 2 * capacity."

let test_remove _test_ctxt =
  let cache = Cache.make ~max_size:2 in
  Cache.add cache ~key:(Int_key 1) ~value:1;
  Cache.remove cache ~key:(Int_key 1);
  Cache.remove cache ~key:(Int_key 1);
  Cache.remove cache ~key:(String_key "foo");
  Int_asserter.assert_equals
    0
    (Cache.length cache)
    "empty cache should have length 0";
  ()

let test_eviction _test_ctxt =
  let cache = Cache.make ~max_size:1 in
  Cache.add cache ~key:(Int_key 1) ~value:1;
  let _ = Cache.find_or_add cache ~key:(Int_key 1) ~default:(fun () -> None) in
  Cache.add cache ~key:(String_key "foo") ~value:"ab";
  Cache.add cache ~key:(String_key "bar") ~value:"ab";
  (* Frequency is 2 for key `1`, 1 for key 'foo', so before adding 'bar', we should have collected 'foo. *)
  Int_asserter.assert_option_equals
    (Some 1)
    (Cache.find_or_add cache ~key:(Int_key 1) ~default:(fun () -> None))
    "Should have found entry for key `1`, with value 1";
  String_asserter.assert_option_equals
    None
    (Cache.find_or_add cache ~key:(String_key "foo") ~default:(fun () -> None))
    "Should NOT have found entry for key 'foo'";
  String_asserter.assert_option_equals
    (Some "ab")
    (Cache.find_or_add cache ~key:(String_key "bar") ~default:(fun () -> None))
    "Should have found entry for key 'bar', with value 'ab'";
  ()

let () =
  "lfu_cache_test"
  >::: [
         "test_insert" >:: test_insert;
         "test_insert_many" >:: test_insert_many;
         "test_remove" >:: test_remove;
         "test_eviction" >:: test_eviction;
       ]
  |> run_test_tt_main
