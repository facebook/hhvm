(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module SolveStateKey = struct
  type t = Solve_state.t
  let compare = Solve_state.compare
end

include PriorityQueue.Make(SolveStateKey)

let make item =
  let pq = make_empty 7 in
  push pq item;
  pq
