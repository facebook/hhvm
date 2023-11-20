(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(* CAUTION: This type must be kept in sync with typing_deps.rs *)
(** Which dependency graph format are we using? *)
type t =
  | InMemoryMode of string option
      (** Keep track of newly discovered edges in an in-memory delta.
        *
        * Optionally, the in-memory delta is backed by a pre-computed
        * dependency graph stored using a custom file format.
        *)
  | SaveToDiskMode of {
      graph: string option;
      new_edges_dir: string;
      human_readable_dep_map_dir: string option;
    }
      (** Mode that writes newly discovered edges to binary files on disk
        * (one file per disk). Those binary files can then be post-processed
        * using a tool of choice.
        *
        * The first parameter is (optionally) a path to an existing custom 64-bit
        * dependency graph. If it is present, only new edges will be written,
        * of not, all edges will be written. *)

let to_opaque_json (t : t) : Hh_json.json =
  let open Hh_json in
  let opaque opt = Option.map (fun _ -> "<opaque>") opt in
  match t with
  | InMemoryMode base ->
    JSON_Object
      [
        ("mode", string_ "InMemoryMode");
        ("props", JSON_Object [("base", opt_string_to_json (opaque base))]);
      ]
  | SaveToDiskMode { graph; new_edges_dir = _; human_readable_dep_map_dir } ->
    JSON_Object
      [
        ("mode", string_ "SaveToDiskMode");
        ( "props",
          JSON_Object
            [
              ("graph", opt_string_to_json (opaque graph));
              ( "human_readable_dep_map_dir",
                opt_string_to_json (opaque human_readable_dep_map_dir) );
            ] );
      ]
  [@@deriving show]
