(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

let last_result = ref None

let regexp s =
  (* In js_of_ocaml, the regex library follows javascript syntax instead
   * of the Ocaml rules. This is a hack to convert our existing regexes.
   * (It covers all our current regexes, but not every possible regex). *)
  let s = Regexp.global_replace (Regexp.regexp "\\\\\\(") s "(" in
  let s = Regexp.global_replace (Regexp.regexp "\\\\\\)") s ")" in
  Regexp.regexp s

let string_match re s n =
  let result = Regexp.string_match re s n in
  last_result := result;
  result <> None

let matched_group n s =
  match !last_result with
  | None -> raise Not_found
  | Some result -> 
      match Regexp.matched_group result n with
      | None -> raise Not_found
      | Some x -> x
