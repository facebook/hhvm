(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = { next_id: int }

let make () = { next_id = 0 }

let make_span t =
  let id = t.next_id in
  ({ next_id = t.next_id + 1 }, { Span.id })
