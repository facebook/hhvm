(**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Utils

type search_result_type =
  | Class of Ast.class_kind
  | Method of bool * string
  | ClassVar of bool * string
  | Function
  | Typedef
  | Constant

type search_result = {
    pos        : Pos.t;
    name       : string;
    result_type: search_result_type;
  }

(* Shared memory for workers to put lists of pairs of keys and results
  * of our index. Indexed on file name. Cached because we read from here
  * every time the user searches *)
module SearchUpdates = SharedMem.WithCache(struct
  type t = (string * search_result) list
  let prefix = Prefix.make()
end)
(* Maps file name to a list of keys that the file has results for *)
(* This is only read once per update, so cache gives us no advantage *)
module SearchKeys = SharedMem.NoCache(struct
  type t = string list
  let prefix = Prefix.make()
end)

(* function that shortens the keys stored in the trie to make things faster *)
let simplify_key key =
  let key =
    try
      (* Testing showed this max length gave us some indexing time back
       * without making search feel any slower *)
      String.sub key 0 6
    with Invalid_argument _ ->
      key 
  in
  try
    (* stuff after the colon is class members and they're all probably
     * in the same file anyways *)
    let i = String.index key ':' in
    String.sub key 0 i
  with Not_found ->
    key

(* Module used to rank results. Provides a comparison function for sorting *)
module Ranking = struct

  (* Class and functions are the most interesting results, so move them
   * to the top. Then, typedefs. Methods and constants are last *)
  let search_result_type_compare a b =
    let rank_search_result_type type_ = match type_ with
      | Class _
      | Function -> 4
      | Typedef -> 3
      | _ -> 2 in
    (rank_search_result_type b) - (rank_search_result_type a)
  
  (* look for results that exactly match the query *)
  let search_result_name_compare query a_name b_name =
    let a_name = Utils.strip_ns a_name in
    let b_name = Utils.strip_ns b_name in
    if a_name <> b_name
    then begin
      if a_name = query then -1
      else if b_name = query then 1
      else 0
    end else
    0
  
  let result_compare query a b =
    (* First compare the result to the query to move any exact match
     * to the top *)
    let name_compare =
      search_result_name_compare query (snd a).name (snd b).name in
    if name_compare <> 0
    then name_compare
    else
      (* case insensitive comparison of result name to query *)
      let lower_name_compare =
        search_result_name_compare query
          (String.lowercase (snd a).name)
          (String.lowercase (snd b).name)
      in
      if lower_name_compare <> 0
      then lower_name_compare
      else
        (* rank based on the result type *)
        let type_compare =
          search_result_type_compare (snd a).result_type (snd b).result_type in
        if type_compare <> 0
        then type_compare
        else
          (* finally, just string compare the keys *)
          String.compare (fst a) (fst b)
end

module WorkerApi = struct

  (* cleans off namespace and colon at the start of xhp name because the
   * user will want to search for xhp classes without typing a : at
   * the start of every search *)
  let clean_key key =
    if (String.length key) > 0
    then
      let key = String.lowercase (Utils.strip_ns key) in
      if (String.length key) > 0 && key.[0] = ':'
      then String.sub key 1 (String.length key - 1)
      else key
    else key

  let result_from_id id res_type =
    {
      pos         = fst id;
      name        = snd id;
      result_type = res_type;
    }

  let result_and_key_from_id id res_type =
    clean_key (snd id), result_from_id id res_type

  let uppercase_filter str =
    let result = ref [] in
    String.iter begin fun c ->
      if Char.lowercase c <> c
      then result := c :: !result
    end str;
    let i = ref (List.length !result) in
    let filtered_str = String.create !i in
    List.iter begin fun c ->
      String.set filtered_str (!i - 1) c;
      decr i
    end !result;
    filtered_str

  let add_cid_result c acc =
    let name = (snd c.Ast.c_name) in
    (* camel name is the name filtered for the starting letters of camelcase
     * For example, FooBarClass's camel name is FBC *)
    let camel_name = clean_key (uppercase_filter name) in
    let name = clean_key name in
    let c_result = result_from_id c.Ast.c_name (Class c.Ast.c_kind) in
    let acc =
      (* only add the camel name if this result wouldn't have already
       * been covered by the normal name *)
      if camel_name <> "" && not (str_starts_with name camel_name)
      then (camel_name, c_result) :: acc
      else acc
    in
    (name, c_result) :: acc

(* Unlike anything else, we need to look at the class body to extract it's
 * methods so that they can also be searched for *)
  let update_class c acc =
    let prefix = (snd c.Ast.c_name)^"::" in
    let acc = add_cid_result c acc in
    let acc = List.fold_left begin fun acc elt ->
      match elt with
      | Ast.Method m -> let id = m.Ast.m_name in
          let name = prefix^(snd id) in
          let is_static = List.mem Ast.Static m.Ast.m_kind in
          let res =  {
                        pos         = fst id;
                        name        = snd id;
                        result_type =
                          Method (is_static,
                                  (Utils.strip_ns (snd c.Ast.c_name)))
                     } in
          (clean_key (snd id), res) :: (clean_key name, res) :: acc
      (* Monocle did this, but it seems like this is mostly noise.
       * Disabling for now. We'll see if people complain *)
      (*| Ast.ClassVars (kind, hint, vars) ->
          let is_static = List.mem Ast.Static kind in
          List.fold_left begin fun acc var ->
            let id = fst var in
            let name = prefix^(snd id) in
            let res =  {
                          pos         = fst id;
                          name        = snd id;
                          result_type =
                            ClassVar (is_static,
                                    (Utils.strip_ns (snd c.Ast.c_name)))
                       } in
            (clean_key (snd id), res) :: (clean_key name, res) :: acc        
          end acc vars*)
      | _ -> acc
    end acc c.Ast.c_body in
    acc
  
  (* Called by a worker after the file is parsed *)
  let update fn ast =
   let defs = List.fold_left begin fun acc def ->
      match def with
      | Ast.Fun f -> (result_and_key_from_id f.Ast.f_name Function) :: acc
      | Ast.Class c -> update_class c acc
      | Ast.Typedef td ->
          (result_and_key_from_id td.Ast.t_id Typedef) ::acc
      | Ast.Constant cst ->
          (result_and_key_from_id cst.Ast.cst_name Constant) :: acc
      | _ -> acc
    end [] ast in
    SearchUpdates.add fn defs;
    let keys = List.fold_left begin fun acc def ->
      let key = simplify_key (fst def) in
      SSet.add key acc
    end SSet.empty defs in
    let keys = SSet.elements keys in
    SearchKeys.add fn keys
end

module MasterApi = struct

  (* what keys a specific file currently has results for *)
  (* 10000 is a number that seemed reasonable for the amount of files
   * in a codebase *)
  let removal_index = Hashtbl.create 1000
  
  (* Hashtable the names of files with results for a string key *)
  let main_index = Hashtbl.create 1000
  
  (* trie used to store ONLY KEYS to give a typeahead feeling for searching *)
  let trie = Trie.create()

  let lookup str =
   try
     Hashtbl.find main_index str
   with Not_found ->
     []

  let replace key value =
    if value = [] then Hashtbl.remove main_index key
    else Hashtbl.replace main_index key value
  
  (* Insert the selected filename into the hashtable, and the trie
   * Takes in a set of keys that are currently empty that we plan to delete
   * from the trie. Removes those keys from the set if we inserted on them *)
  let insert_fn fn key empty_keys =
    let current_val = lookup key in
    replace key (fn :: current_val);
    if current_val == [] && not (SSet.mem key empty_keys)
    then begin
      (* We don't actually store useful stuff in the trie. Just
       * whether a key exists or not *)
      Trie.add trie key ()
        ~if_exist: (fun _ _ -> ()) (* this can probably throw an exception *)
        ~transform: (fun _ -> ())
    end;
    SSet.remove key empty_keys 
  
  (* Remove this filename from the hashtable at this key, if it
   * exists. Accumulates keys have no values anymore
   *  (so we can remove them from the trie) *)
  let remove_fn fn key empty_keys =
    let current_val = lookup key in
    let new_val = List.fold_left begin fun acc file ->
      if file= fn
      then acc
      else (file :: acc)
    end [] current_val in
    replace key new_val;
    if new_val == [] then SSet.add key empty_keys else empty_keys
  
  let process_file fn =
    let old_keys =
      try Hashtbl.find removal_index fn
      with Not_found ->
        [] (* This will happen when we haven't seen this file before *)
    in
    let new_keys =
      try SearchKeys.find_unsafe fn
      with Not_found -> [] (* This shouldn't actually happen *)
    in
    (* Compute diff between old and new keys for this file*)
    let old_keys_set = List.fold_left begin fun acc file ->
      SSet.add file acc
    end SSet.empty old_keys in
    let new_keys_set = List.fold_left begin fun acc file ->
      SSet.add file acc
    end SSet.empty new_keys in
    let to_add = SSet.diff new_keys_set old_keys_set in
    let to_remove = SSet.diff old_keys_set new_keys_set in
    
    let removed_keys = SSet.fold (remove_fn fn) to_remove SSet.empty in
    Hashtbl.replace removal_index fn new_keys;
    let removed_keys = SSet.fold (insert_fn fn) to_add removed_keys in
    (* removed keys now contains any keys that we removed and didn't
     * add again for this file change. So we remove from the trie *)
    SSet.iter (Trie.remove trie) removed_keys
   
  (* Called by the master process when there is new information in
   * shared memory for us to index *)
  let update_search_index files php_files =
    let files = List.fold_left begin fun acc file ->
      SSet.add file acc
    end php_files files in
    SSet.iter process_file files;
    (* At this point, users can start searching agian so we should clear the
     * cache that contains the actual results. We don't have to worry
     * about the string->keys list shared memory because it's uncached *)
    SharedMem.invalidate_caches()
  
  let query input =
    let str = String.lowercase (Utils.strip_ns input) in
    let short_key = simplify_key str in
    (* get all the keys beneath short_key in the trie *)
    let keys =
      try Trie.find_prefix_limit 25 trie short_key (fun k _ -> k)
      with Not_found -> []
    in
    (* Get set of filenames that contain results for those keys *)
    let files = List.fold_left begin fun acc key ->
      let filenames = Hashtbl.find main_index key in
      List.fold_left begin fun acc fn ->
        SSet.add fn acc
      end acc filenames
    end SSet.empty keys in
    let results = ref [] in
    (* for every file, look in shared memory for all the results the file
     * contains. anything where the key starts with the full search
     * term is a match *)
    SSet.iter begin fun fn ->
      let defs =
        try SearchUpdates.find_unsafe fn
        with Not_found -> []
      in
      List.iter begin fun (key, res) ->
        (* Now we're comparing results in shared memory. These keys
         * have not been simplified, so we check if they start with
         * the full search term *)
        if str_starts_with key str then
        results := (key, res) :: !results
      end defs
    end files;
    let res = List.sort (Ranking.result_compare input) !results in
    let res = Utils.cut_after 50 res in
    List.map snd res
end
