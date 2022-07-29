(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t =
  | Next
  | Continue
  | Break
  | Catch
  | Do
  | Exit
  | Fallthrough
  | Finally
[@@deriving eq, ord, show]

let all = [Next; Continue; Break; Catch; Do; Exit; Fallthrough; Finally]

let to_string = function
  | Next -> "Next"
  | Continue -> "Continue"
  | Break -> "Break"
  | Catch -> "Catch"
  | Do -> "Do"
  | Exit -> "Exit"
  | Fallthrough -> "Fallthrough"
  | Finally -> "Finally"
