(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 *)

(* @generated
   regenerate: buck2 run fbcode//glean/schema/gen:gen-schema  -- --ocaml fbcode/hphp/hack/src/typing/write_symbol_info/schema --dir DEST_DIR *)

open Hh_json


type t = int [@@deriving ord]

let next =
  let x = ref 1 in
  fun () ->
    let r = !x in
    x := !x + 1;
    r

let to_json_number i = JSON_Number (string_of_int i)

module Map = Map.Make (struct
  type t = int

  let compare = Int.compare
end)


