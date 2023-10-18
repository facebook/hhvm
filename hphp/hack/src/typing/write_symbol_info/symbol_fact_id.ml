(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type t = int [@@deriving ord]

let next =
  let x = ref 1 in
  fun () ->
    let r = !x in
    x := !x + 1;
    r

let to_json_number i = Hh_json.JSON_Number (string_of_int i)

module Map = Map.Make (struct
  type t = int

  let compare = Int.compare
end)
