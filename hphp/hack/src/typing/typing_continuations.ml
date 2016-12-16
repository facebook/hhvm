(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

module Continuations = struct
  type t =
    | Next
    | Continue
    | Break
    | Catch

  let compare = Pervasives.compare

  let to_string = function
    | Next -> "Next"
    | Continue -> "Continue"
    | Break -> "Break"
    | Catch -> "Catch"
end

include Continuations

module Map = MyMap.Make(Continuations)
