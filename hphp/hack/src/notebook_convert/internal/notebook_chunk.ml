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
[@@deriving eq, ord]

(** A chunk has the information we need to map between
* some Hack code and a cell for ipynb notebook format
*)
type t = {
  id: Id.t;
  chunk_kind: chunk_kind;
  contents: string;
}
[@@deriving ord]

let metadata_pattern = {|^ *//@bento-cell:\(.*\)|}

let metadata_regexp = Str.regexp metadata_pattern

let is_chunk_start_comment s = Str.string_match metadata_regexp s 0

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

type metadata = {
  metadata_id: Id.t;
  metadata_cell_type: string;
}

(** Assumes `comment` has already been checked against [is_chunk_start_comment] *)
let metadata_of_comment_exn (comment : string) : metadata =
  let obj =
    let json_string =
      if Str.string_match metadata_regexp comment 0 then
        Str.matched_group 1 comment
      else
        failwith
          (Printf.sprintf
             "Should be unreachable. Failed to read metadata from comment. Got: \"%s\". Expected string matching: %s"
             comment
             metadata_pattern)
    in
    json_string |> Hh_json.json_of_string |> Hh_json.get_object_exn
  in
  let find_exn = List.Assoc.find_exn ~equal:String.equal in
  let metadata_id = Hh_json.get_number_int_exn (find_exn obj "id") in
  let metadata_cell_type = Hh_json.get_string_exn (find_exn obj "cell_type") in
  { metadata_id; metadata_cell_type }

let strip_comment_wrapper (hack : string) : string =
  let stripped = String.strip hack in
  String.sub
    stripped
    ~pos:2
    ~len:(String.length stripped - 4 (* length of "/*" + "*/"  *))

let of_hack_exn
    ~(comment : string) ~(is_from_toplevel_statements : bool) (hack : string) :
    t =
  let { metadata_id = id; metadata_cell_type } =
    metadata_of_comment_exn comment
  in
  if is_from_toplevel_statements then
    { id; chunk_kind = Stmt; contents = hack }
  else if String.equal "code" metadata_cell_type then
    { id; chunk_kind = Top_level; contents = hack }
  else
    {
      id;
      chunk_kind = Non_hack { cell_type = metadata_cell_type };
      contents = strip_comment_wrapper hack;
    }
