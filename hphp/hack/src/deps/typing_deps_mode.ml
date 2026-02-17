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
          Optionally, the in-memory delta is backed by a pre-computed
          Dependency graph stored using a custom file format (.hhdg).
          The optional string is the path to that pre-computed depgraph. *)
  | SaveToDiskMode of {
      graph: string option;
      new_edges_dir: string;
      human_readable_dep_map_dir: string option;
    }
      (** Mode that writes newly discovered edges to binary files on disk
          (one file per disk). Those binary files can then be post-processed
          using a tool of choice.
          The first parameter is (optionally) a path to an existing custom 64-bit
          dependency graph. If it is present, only new edges will be written,
          of not, all edges will be written. *)

let to_opaque_json (t : t) : Yojson.Safe.t =
  let string_ s = `String s in
  let string_opt = function
    | Some s -> `String s
    | None -> `Null
  in
  let opaque opt = Option.map (fun _ -> "<opaque>") opt in
  match t with
  | InMemoryMode base ->
    `Assoc
      [
        ("mode", string_ "InMemoryMode");
        ("props", `Assoc [("base", string_opt (opaque base))]);
      ]
  | SaveToDiskMode { graph; new_edges_dir = _; human_readable_dep_map_dir } ->
    `Assoc
      [
        ("mode", string_ "SaveToDiskMode");
        ( "props",
          `Assoc
            [
              ("graph", string_opt (opaque graph));
              ( "human_readable_dep_map_dir",
                string_opt (opaque human_readable_dep_map_dir) );
            ] );
      ]
  [@@deriving show]
