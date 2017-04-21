(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
*)

let test_heapsize_simple () =
  let wordsize = Sys.word_size / 8 in
  let pair = (1,2) in
  (* Have to be careful: Caml shares constant pairs *)
  let nonshared = [(1,2);(3,4)] in
  let shared = [pair;pair] in
  let str1 = "0123456" in
  let str2 = "01234567" in
  (* We expect a two-element pair to use 3 words *)
  let c1 = SharedMem.value_size (Obj.repr pair) / wordsize in
  (* We expect a two element list of two-element pairs to use 12 words *)
  let c2 = SharedMem.value_size (Obj.repr nonshared) / wordsize in
  (* But if the pair is shared then only 9 words *)
  let c3 = SharedMem.value_size (Obj.repr shared) / wordsize in
  (* We expect strings to use a word and then words filled with characters,
     including null termination *)
  let c4 = SharedMem.value_size (Obj.repr str1) / wordsize in
  let c4expected = 2 + (String.length str1) / wordsize in
  let c5 = SharedMem.value_size (Obj.repr str2) / wordsize in
  let c5expected = 2 + (String.length str2) / wordsize in
  Printf.printf "c1 = %d c2 = %d c3 = %d c4 = %d c5 = %d\n" c1 c2 c3 c4 c5;
  c1 == 3 &&
  c2 == 12 &&
  c3 == 9 &&
  c4 == c4expected &&
  c5 == c5expected

let tests = [
  "test_heapsize_simple", test_heapsize_simple;
]

let () =
  Daemon.check_entry_point (); (* this call might not return *)
  Unit_test.run_all tests
