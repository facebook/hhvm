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
  | Hack
  | Non_hack of { cell_type: string }
[@@deriving eq, ord]

module Hh_json = struct
  include Hh_json

  (** don't include JSON fields when sorting below *)
  let compare_json _ _ = -1
end

(** A chunk has the information we need to map between
* some Hack code and a cell for ipynb notebook format
*)
type t = {
  id: Id.t;
  chunk_kind: chunk_kind;
  contents: string;
  cell_bento_metadata: Hh_json.json option;
}
[@@deriving ord]

let metadata_pattern = {|^ *//@bento-cell:\(.*\)|}

let metadata_regexp = Str.regexp metadata_pattern

let is_chunk_start_comment s = Str.string_match metadata_regexp s 0

let bento_cell_end_comment = "//@bento-cell-end"

let cell_end_regexp = Str.regexp ({|^ *|} ^ bento_cell_end_comment)

let is_chunk_end_comment s = Str.string_match cell_end_regexp s 0

let non_hack_prefix = "/*@non_hack:\n"

let non_hack_suffix = "\n*/"

let strip_non_hack_comment (code : string) =
  if
    String.is_prefix code ~prefix:non_hack_prefix
    && String.is_suffix code ~suffix:non_hack_suffix
  then
    String.sub
      code
      ~pos:(String.length non_hack_prefix)
      ~len:
        (String.length code
        - String.length non_hack_prefix
        - String.length non_hack_suffix)
  else
    code

let wrap_in_comment : string -> string = Printf.sprintf "/*@non_hack:\n%s\n*/"

let to_hack { chunk_kind; id; contents; cell_bento_metadata } : string =
  let (cell_type, body) =
    match chunk_kind with
    | Hack -> ("code", contents)
    | Non_hack { cell_type } -> (cell_type, wrap_in_comment contents)
  in
  let cell_bento_metadata_list =
    match cell_bento_metadata with
    | Some cell_bento_metadata -> [("cell_bento_metadata", cell_bento_metadata)]
    | None -> []
  in
  let json_string =
    Hh_json.json_to_string
      ~sort_keys:true
      Hh_json.(
        JSON_Object
          (cell_bento_metadata_list
          @ [
              ("id", JSON_Number (string_of_int id));
              ("cell_type", JSON_String cell_type);
            ]))
  in
  Printf.sprintf
    "//@bento-cell:%s\n%s\n%s\n"
    json_string
    body
    bento_cell_end_comment

type metadata = {
  metadata_id: Id.t;
  metadata_cell_type: string;
  cell_bento_metadata: Hh_json.json option;
}

exception Not_found of string

(** Assumes `comment` has already been checked against [is_chunk_start_comment] *)
let metadata_of_comment (comment : string) :
    (metadata, Notebook_convert_error.t) result =
  match
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
    let find_exn obj key =
      try List.Assoc.find_exn ~equal:String.equal obj key with
      | _ -> raise (Not_found key)
    in
    let metadata_id = Hh_json.get_number_int_exn (find_exn obj "id") in
    let metadata_cell_type =
      Hh_json.get_string_exn (find_exn obj "cell_type")
    in
    let cell_bento_metadata =
      List.Assoc.find obj "cell_bento_metadata" ~equal:String.equal
    in
    { metadata_id; metadata_cell_type; cell_bento_metadata }
  with
  | exception Not_found key ->
    Error
      (Notebook_convert_error.Invalid_input
         (Printf.sprintf "missing key '%s'" key))
  | exception Hh_json.Syntax_error msg ->
    Error (Notebook_convert_error.Invalid_input msg)
  | metadata -> Ok metadata

(** remove leading `/*\n` and trailing `\n*/`. These were inserted
* by older versions  of the conversion script for non-hack cells.
* We also handle the newer format of `/*@non_hack:\n` ... `\n*/`
*)
let strip_legacy_comment_wrapper (hack : string) : string =
  let stripped = String.strip hack in
  String.sub
    stripped
    ~pos:3 (* length of "\n/*" *)
    ~len:(String.length stripped - 5 (* length of "\n/*" + length of "*/"  *))

let of_hack ~(comment : string) (hack : string) :
    (t, Notebook_convert_error.t) result =
  Result.map
    (metadata_of_comment comment)
    ~f:(fun { metadata_id = id; metadata_cell_type; cell_bento_metadata } ->
      let hack = strip_non_hack_comment hack in
      if String.equal "code" metadata_cell_type then
        { id; chunk_kind = Hack; contents = hack; cell_bento_metadata }
      else
        {
          id;
          chunk_kind = Non_hack { cell_type = metadata_cell_type };
          contents = strip_legacy_comment_wrapper hack;
          cell_bento_metadata;
        })
