(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_core
open Coverage_level
open Option.Monad_infix
open Reordered_argument_collections
open Utils
open String_utils

module FileInfoStore = GlobalStorage.Make(struct
  type t = FileInfo.t Relative_path.Map.t
end)

(* Count the number of expressions of each kind of Coverage_level. *)
let count_exprs fn type_acc =
  let level_of_type = level_of_type_mapper fn in
  Hashtbl.fold (fun (p, kind) ty acc ->
    let r, lvl = level_of_type (p, ty) in
    let counter = match SMap.get acc kind with
      | Some counter -> counter
      | None -> empty_counter in
    SMap.add acc ~key:kind ~data:(incr_counter lvl (r, p, counter))
  ) type_acc SMap.empty

let accumulate_types fn defs tcopt =
  let type_acc = Hashtbl.create 0 in
  let tcopt = TypecheckerOptions.make_permissive tcopt in
  Typing.with_expr_hook (fun (p, e) ty ->
    let expr_kind_opt = match e with
      | Nast.Array_get _ -> Some "array_get"
      | Nast.Call _ -> Some "call"
      | Nast.Class_get _ -> Some "class_get"
      | Nast.Class_const _ -> Some "class_const"
      | Nast.Lvar _ -> Some "lvar"
      | Nast.New _ -> Some "new"
      | Nast.Obj_get _ -> Some "obj_get"
      | _ -> None in
    Option.iter expr_kind_opt (fun kind ->
      Hashtbl.replace type_acc (p, kind) ty))
    (fun () ->
      ignore (Typing_check_utils.check_defs tcopt fn defs));
  type_acc

let combine v1 v2 =
  SMap.merge ~f:(fun _ cs1 cs2 -> Option.merge cs1 cs2 merge_and_sum) v1 v2

(* Create a trie for a single key. More complicated tries can then be built from
 * path tries using merge_trie* functions *)
let rec mk_path_trie c path  = match path with
  | [] -> Leaf c
  | h::tl -> Node (c, SMap.singleton h (mk_path_trie c tl))

let rec merge_trie x y = match x, y with
  | Leaf x, Leaf y -> Leaf (combine x y)
  | Leaf x, Node (y, c)
  | Node(y, c), Leaf x -> Node (combine x y, c)
  | Node (x, c), Node (y, d) -> Node (combine x y, merge_trie_children c d)

and merge_trie_children x y =
    SMap.merge ~f:(fun _ x y -> merge_trie_opt x y) x y

and merge_trie_opt x y = Option.merge x y (merge_trie)

(* Convert a list of (file_name, map of counts) into a trie. Each
 * internal node of the trie has the sum of counts of all its child nodes. *)
let mk_trie acc fn_counts_l =
  List.fold_left
    ~f:(fun acc (fn, counts) ->
      let path_l = Str.split (Str.regexp Filename.dir_sep) fn in
      let path_trie = Some (mk_path_trie counts path_l) in
      merge_trie_opt acc path_trie)
    ~init:acc fn_counts_l

(* Convert an absolute path to one relative to the given root.
 * Returns None if root is not a prefix of path. *)
let relativize root path =
  let root = Path.to_string root in
  (* If we're provided a file instead of a directory as the path to filter, the
     only valid value is the filename itself. *)
  if FindUtils.is_php root && root = path
  then Some (Filename.basename path)
  else

  (* naive implementation *)
  let root = root ^ Filename.dir_sep in
  if string_starts_with path root
  then
    let root_len = String.length root in
    Some (String.sub path root_len (String.length path - root_len))
  else None

(* Returns a list of (file_name, assoc list of counts) *)
let get_coverage root tcopt neutral fnl  =
  SharedMem.invalidate_caches();
  let files_info = FileInfoStore.load () in
  let file_counts = List.rev_filter_map fnl begin fun fn ->
    relativize root (Relative_path.to_absolute fn) >>= fun relativized_fn ->
    Relative_path.Map.get files_info fn >>= fun defs ->
    let type_acc = accumulate_types fn defs tcopt in
    let counts = count_exprs fn type_acc in
    Some (relativized_fn, counts)
  end in
  mk_trie neutral file_counts

let go_ fn genv env =
  let root = Path.make fn in
  let module RP = Relative_path in
  let next_files = compose
    (fun paths -> paths |> List.map ~f:(RP.create RP.Root) |> Bucket.of_list)
    (genv.ServerEnv.indexer FindUtils.is_php)
  in
  FileInfoStore.store env.ServerEnv.files_info;
  let tcopt = env.ServerEnv.tcopt in
  let result =
    MultiWorker.call
      genv.ServerEnv.workers
      ~job:(get_coverage root tcopt)
      ~neutral:None
      ~merge:merge_trie_opt
      ~next:next_files
  in
  FileInfoStore.clear ();
  result

let go fn genv env =
  try go_ fn genv env
  with Failure _ | Invalid_argument _ ->
    Hh_logger.log "Coverage collection failed!";
    None
