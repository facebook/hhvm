(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

type t = NoCost | Base | Assignment | SimpleMemberSelection

let get_cost t = match t with
  | NoCost -> 0
  | Base -> 1
  | Assignment -> 2
  | SimpleMemberSelection -> 3
