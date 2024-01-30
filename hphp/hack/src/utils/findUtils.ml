(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

(*****************************************************************************)
(* The file extensions we are interested in *)
(*****************************************************************************)

let extensions =
  [
    ".php";
    (* normal php file *)
    ".phpt";
    (* our php template or test files *)
    ".hack";
    (* strict-mode files (Hack) *)
    ".hackpartial";
    (* partial-mode files (Hack) *)
    ".hck";
    (* open source hack: bikeshed entry *)
    ".hh";
    (* open source hack: biekshed entry *)
    ".hhi";
    (* interface files only visible to the type checker *)
    ".xhp";
    (* XHP extensions *)
  ]

let is_dot_file path =
  let filename = Filename.basename path in
  String.length filename > 0 && Char.equal filename.[0] '.'

let is_hack path =
  (not (is_dot_file path))
  && List.exists extensions ~f:(Filename.check_suffix path)

(* Returns whether one of the ancestral directories of path has the given
 * name. *)
let rec has_ancestor path ancestor_name =
  let dirname = Filename.dirname path in
  if String.equal dirname path then
    (* Terminal condition *)
    false
  else if String.equal (Filename.basename dirname) ancestor_name then
    true
  else
    has_ancestor dirname ancestor_name

let file_filter f =
  (* Filter the relative path *)
  let f = Relative_path.strip_root_if_possible f |> Option.value ~default:f in
  is_hack f && not (FilesToIgnore.should_ignore f)

let path_filter f = Relative_path.suffix f |> file_filter

let post_watchman_filter_from_fully_qualified_raw_updates
    ~(root : Path.t) ~(raw_updates : SSet.t) : Relative_path.Set.t =
  let root = Path.to_string root in
  (* Because of symlinks, we can have updates from files that aren't in
   * the .hhconfig directory *)
  let updates =
    SSet.filter (fun p -> String.is_prefix p ~prefix:root) raw_updates
  in
  let updates = Relative_path.(relativize_set Root updates) in
  Relative_path.Set.filter updates ~f:(fun update ->
      file_filter (Relative_path.to_absolute update))

(* Hash file name and return true for [sample_rate] fraction of hashes *)
let sample_filter ~sample_rate x =
  Float.(
    float (Base.String.hash (Relative_path.suffix x) mod 1000000)
    <= sample_rate *. 1000000.0)
