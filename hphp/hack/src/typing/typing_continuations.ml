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
    | Catch
    | Do
    | Exit
    | Fallthrough
    | Finally [@@deriving show, enum]

  (* build the list of all continuations *)
  let all =
    let n_cont = max + 1 in
    let rec build_all i conts =
      if (i < 0) then conts else
      let cont = match (of_enum i) with
        | Some cont -> [cont]
        | None -> [] in
      let conts = cont @ conts in
      build_all (i - 1) conts in
    build_all (n_cont - 1) []

  let compare = Pervasives.compare

  let to_string = function
    | Next -> "Next"
    | Continue -> "Continue"
    | Break -> "Break"
    | Catch -> "Catch"
    | Do -> "Do"
    | Exit -> "Exit"
    | Fallthrough -> "Fallthrough"
    | Finally -> "Finally"
end

include Continuations

module Map = struct
  let show _ = "<MyMap.Make(Continuations)>";;
  let pp _ _ = Printf.printf "%s\n" "<MyMap.Make(Continuations)>";;
  include MyMap.Make(Continuations)
end
