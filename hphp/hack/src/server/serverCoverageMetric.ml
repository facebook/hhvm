(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Coverage_level
open Coverage_level_defs
open Option.Monad_infix
open Reordered_argument_collections
open String_utils
module Tast = Aast

module NamingTableStore = GlobalStorage.Make (struct
  type t = Naming_table.t
end)

let combine v1 v2 =
  SMap.merge ~f:(fun _ cs1 cs2 -> Option.merge cs1 cs2 merge_and_sum) v1 v2

class count_getter fixme_map =
  object
    inherit [level_stats SMap.t] Tast_visitor.reduce as super

    method zero = SMap.empty

    method plus = combine

    method! on_expr env expr =
      let acc = super#on_expr env expr in
      let ((pos, ty), e) = expr in
      let expr_kind_opt =
        match e with
        | Tast.Array_get _ -> Some "array_get"
        | Tast.Call _ -> Some "call"
        | Tast.Class_get _ -> Some "class_get"
        | Tast.Class_const _ -> Some "class_const"
        | Tast.Lvar _ -> Some "lvar"
        | Tast.New _ -> Some "new"
        | Tast.Obj_get _ -> Some "obj_get"
        | _ -> None
      in
      match expr_kind_opt with
      | None -> acc
      | Some kind ->
        let r = Typing_defs.get_reason ty in
        let (_env, lvl) = level_of_type env fixme_map (pos, ty) in
        let counter =
          match SMap.find_opt acc kind with
          | Some counter -> counter
          | None -> empty_counter
        in
        SMap.add acc ~key:kind ~data:(incr_counter lvl (r, pos, counter))
  end

(* This should likely take in tasts made with type checker options that were
 * made permissive using TypecheckerOptions.make_permissive
 *)
let accumulate_types ctx tast check =
  let fixmes =
    match Fixme_provider.get_hh_fixmes check with
    | Some fixmes -> fixmes
    | None ->
      failwith
        ("HH_FIXMEs not found for path " ^ Relative_path.to_absolute check)
  in
  let cg = new count_getter fixmes in
  cg#go ctx tast

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
  if FindUtils.is_hack root && root = path then
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
