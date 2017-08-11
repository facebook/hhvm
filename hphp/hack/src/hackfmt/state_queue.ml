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

(**
 * This kind of defeats the purpose of having a priority queue in the first
 * place, since it degrades worst case performance back to O(n^2), but in
 * practice it leads to a speed up by reducing the branching factor.
 *)
let find_overlap t state =
  let rec aux i =
    if i = t.size
    then false
    else
      match Array.get t.__queue i with
        | None -> failwith "Unexpected null index when finding overlap"
        | Some e ->
          begin match Solve_state.compare_overlap state e with
            | Some s ->
              t.__queue.(i) <- Some s;
              __bubble_down t.__queue t.size (Array.get t.__queue i) i;
              __bubble_up t.__queue i;
              true
            | None -> aux (i + 1)
          end
  in
  aux 0

(* Override PriorityQueue's push *)
let push t state =
  if not (find_overlap t state) then push t state;
