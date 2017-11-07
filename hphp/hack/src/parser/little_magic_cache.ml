(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type ('k, 'v) cache = Empty | Full of {mutable key : 'k; mutable value : 'v}
type ('k, 'v) t = ('k, 'v) cache ref

let populate cache f k = match !cache with
  | Empty -> cache := Full {key = k; value = f k}
  | Full c ->
      if k != c.key then begin
        c.key <- k;
        c.value <- f k;
      end

let from_cache cache = match !cache with
  | Empty -> failwith (
    "The impossible happened! " ^
    "Attempting to call `from_cache' on an empty cache when it's statically known to be non-empty"
  )
  | Full c -> c.value

let memoize cache f k = populate cache f k; from_cache cache

let make () = ref Empty
