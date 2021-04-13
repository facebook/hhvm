(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Coverage_level_defs
open Option.Monad_infix
open Reordered_argument_collections
open String_utils

module NamingTableStore = GlobalStorage.Make (struct
  type t = Naming_table.t
end)

open ServerCoverageMetricUtils

(* Create a trie for a single key. More complicated tries can then be built from
 * path tries using merge_trie* functions *)
let rec mk_path_trie c path =
  match path with
  | [] -> Leaf c
  | h :: tl -> Node (c, SMap.singleton h (mk_path_trie c tl))

let rec merge_trie x y =
  match (x, y) with
  | (Leaf x, Leaf y) -> Leaf (combine x y)
  | (Leaf x, Node (y, c))
  | (Node (y, c), Leaf x) ->
    Node (combine x y, c)
  | (Node (x, c), Node (y, d)) -> Node (combine x y, merge_trie_children c d)

and merge_trie_children x y =
  SMap.merge ~f:(fun _ x y -> merge_trie_opt x y) x y

and merge_trie_opt x y = Option.merge x y merge_trie

(* Convert a list of (file_name, map of counts) into a trie. Each
 * internal node of the trie has the sum of counts of all its child nodes. *)
let mk_trie acc fn_counts_l =
  List.fold_left
    ~f:(fun acc (fn, counts) ->
      let path_l = Str.split (Str.regexp Filename.dir_sep) fn in
      let path_trie = Some (mk_path_trie counts path_l) in
      merge_trie_opt acc path_trie)
    ~init:acc
    fn_counts_l

(* Convert an absolute path to one relative to the given root.
 * Returns None if root is not a prefix of path. *)
let relativize root path =
  let root = Path.to_string root in
  (* If we're provided a file instead of a directory as the path to filter, the
     only valid value is the filename itself. *)
  if FindUtils.is_hack root && String.equal root path then
    Some (Filename.basename path)
  else
    (* naive implementation *)
    let root = root ^ Filename.dir_sep in
    if string_starts_with path root then
      let root_len = String.length root in
      Some (String.sub path root_len (String.length path - root_len))
    else
      None

(* Returns a list of (file_name, assoc list of counts) *)
let get_coverage root ctx neutral fnl =
  SharedMem.invalidate_caches ();
  let naming_table = NamingTableStore.load () in
  let file_counts =
    List.rev_filter_map fnl (fun fn ->
        relativize root (Relative_path.to_absolute fn) >>= fun relativized_fn ->
        Naming_table.get_file_info naming_table fn >>= fun defs ->
        let (tast, _) = Typing_check_utils.type_file ctx fn defs in
        let type_acc = accumulate_types ctx tast fn in
        Some (relativized_fn, type_acc))
  in
  mk_trie neutral file_counts

let go_ fn genv env =
  let root = Path.make fn in
  let module RP = Relative_path in
  let next_files =
    MultiWorker.next
      genv.ServerEnv.workers
      (Naming_table.get_files env.ServerEnv.naming_table)
  in
  NamingTableStore.store env.ServerEnv.naming_table;
  let ctx = Provider_utils.ctx_from_server_env env in
  let result =
    MultiWorker.call
      genv.ServerEnv.workers
      ~job:(get_coverage root ctx)
      ~neutral:None
      ~merge:merge_trie_opt
      ~next:next_files
  in
  NamingTableStore.clear ();
  result

let go fn genv env =
  try go_ fn genv env with
  | Failure _
  | Invalid_argument _ ->
    Hh_logger.log "Coverage collection failed!";
    None
