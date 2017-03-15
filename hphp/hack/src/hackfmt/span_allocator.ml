(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type t = {
  next_id: int;
}

let make () = { next_id = 0 }

let make_span t cost =
  let id = t.next_id in
  { next_id = t.next_id + 1 }, { Span.id ; cost = cost; }
