(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

module Id = struct
  type t = int [@@deriving ord]

  let zero = 0

  let next = ( + ) 1
end

type chunk_kind =
  | Top_level
  | Stmt
  | Non_hack of { cell_type: string }
[@@deriving ord, eq]

(** A chunk has the information we need to map between
* some Hack code and a cell for ipynb notebook format
*)
type t = {
  id: Id.t;
  chunk_kind: chunk_kind;
  contents: string;
}
[@@deriving ord]

let wrap_in_comment = Printf.sprintf "/*\n%s\n*/"

let to_hack { chunk_kind; id; contents } : string =
  let (cell_type, body) =
    match chunk_kind with
    | Top_level
    | Stmt ->
      ("code", contents)
    | Non_hack { cell_type } -> (cell_type, wrap_in_comment contents)
  in
  Printf.sprintf
    "//@bento-cell:{\"id\": %d, \"cell_type\": \"%s\"}\n%s\n"
    id
    cell_type
    body
