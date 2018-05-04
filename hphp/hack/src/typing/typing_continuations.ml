(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Continuations = struct
  type t =
    | Next
    | Continue
    | Break
    | Catch [@@deriving show]

  let compare = Pervasives.compare

  let to_string = function
    | Next -> "Next"
    | Continue -> "Continue"
    | Break -> "Break"
    | Catch -> "Catch"
end

include Continuations

module Map = struct
  let show _ = "<MyMap.Make(Continuations)>";;
  let pp _ _ = Printf.printf "%s\n" "<MyMap.Make(Continuations)>";;
  include MyMap.Make(Continuations)
end
