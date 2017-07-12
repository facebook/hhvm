(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module SpecialFlowInstr = struct
  open Hhbc_ast
  type t = instruct_special_flow

  let compare_ints x y =
    if x > y then 1 else if x < y then -1 else 0

  let compare x y =
    match (x, y) with
    (* permit only one ret* in the list *)
    (* RetC > RetV > Continue > Break *)
    | SF_RetC _, SF_RetC _
    | SF_RetV _, SF_RetV _ -> 0
    | SF_RetC _, _ -> 1
    | _, SF_RetC _ -> -1
    | SF_RetV _, _ -> 1
    | _, SF_RetV _ -> -1
    | (Continue _, Break _) -> 1
    | (Break _, Continue _) -> -1
    | (Continue (l1, o1), Continue (l2, o2))
    | (Break (l1, o1, _), Break (l2, o2, _)) ->
      if l1 <> l2 then compare_ints l1 l2
      else compare_ints o1 o2
end

include Unique_list.WithValue (SpecialFlowInstr)
