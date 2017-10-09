(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(*
This is a tiny, super-cheap memoizer for pure functions (of one parameter) which
are frequently called twice in a row with the same argument.

This happens a lot in the lexer; that's in the inner loop of the parser, and
so is extremely performance-sensitive.
*)

type ('k, 'v) t = {
  key : 'k ref;
  value : 'v ref;
}

let memoize cache f =
  fun k ->
    (* Note that we are deliberately using cheap reference inequality here. *)
    if !(cache.key) != k then begin
      cache.key := k;
      cache.value := f k
    end;
    !(cache.value)

(*
Note that the cache needs to be "seeded" with valid initial values. This
might seem like a strange requirement; why not something like

type ('k, 'v) t = { pair : ('k * 'v) option ref; }

or

type ('k, 'v) c = Empty | Pair of ('k * 'v)
type ('k, 'v) t = { cache : ('k, 'v) c ref }

???

I tried; when we do that, we have to do a type match in the memoizer, and that
is slow enough that it introduces something like a 10% perf regression in the
parser! It is very strange that this match is so slow.  But initializing to
a default value is a small price to pay.
*)
let make k v = {
  key = ref k;
  value = ref v;
}
