(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module ContBreak = struct
  open Hhbc_ast
  type t = instruct_special_flow

  let compare_ints x y =
    if x > y then 1 else if x < y then -1 else 0

  let compare x y =
    match (x, y) with
    | (Continue _, Break _) -> 1
    | (Break _, Continue _) -> -1
    | (Continue (l1, _), Continue (l2, _)) when (l1 <> l2) ->
      compare_ints l1 l2
    | (Break (l1, _), Break (l2, _)) when (l1 <> l2) ->
      compare_ints l1 l2
    | (Continue (_, o1), Continue (_, o2))
    | (Break (_, o1), Break (_, o2)) ->
      compare_ints o1 o2
end

include Unique_list.WithValue (ContBreak)
