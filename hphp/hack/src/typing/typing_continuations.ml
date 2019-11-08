(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

module Continuations = struct
  type t =
    | Next
    | Continue
    | Break
    | Catch
    | Do
    | Exit
    | Fallthrough
    | Finally
    | Goto of string
  [@@deriving eq, ord, show]

  let to_string = function
    | Next -> "Next"
    | Continue -> "Continue"
    | Break -> "Break"
    | Catch -> "Catch"
    | Do -> "Do"
    | Exit -> "Exit"
    | Fallthrough -> "Fallthrough"
    | Finally -> "Finally"
    | Goto l -> "Goto " ^ l
end

include Continuations

module Map = struct
  let show _ = "<WrappedMap.Make(Continuations)>"

  let pp _ _ = Printf.printf "%s\n" "<WrappedMap.Make(Continuations)>"

  include WrappedMap.Make (Continuations)
end
