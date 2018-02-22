(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

 (* An implementation of a set of types, using ty_compare for a total order.
  * Typing-rule-equivalent types may get duplicated, as the equality induced
  * by ty_compare does not expand Tvars and type aliases.
 *)
open Typing_defs
module Ty_ = struct
  type t = locl ty
  let compare r1 r2 = ty_compare r1 r2
end

include Set.Make(Ty_)
