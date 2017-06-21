(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

 (* An implementation of a set of types, checking for equality by ty_equal.
 *)
open Typing_defs
module Ty_ = struct
  type t = locl ty
  let compare r1 r2 =
    if ty_equal r1 r2 then 0
    else begin
    let x1 = Hashtbl.hash r1 in
    let x2 = Hashtbl.hash r2 in
    x1 - x2
    end
end

include Set.Make(Ty_)
