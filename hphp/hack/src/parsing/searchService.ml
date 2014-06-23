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
open FuzzySearchService

type search_result = FuzzySearchService.term

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

(* Unlike anything else, we need to look at the class body to extract it's
 * methods so that they can also be searched for *)
  let update_class c acc =
    let prefix = (snd c.Ast.c_name)^"::" in
    List.fold_left begin fun acc elt ->
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
    end acc c.Ast.c_body
  
  (* Called by a worker after the file is parsed *)
  let update fn ast =
    let type_to_keylist, type_to_defmap, defs =
        List.fold_left begin fun (type_to_keylist, type_to_defmap, defs) def ->
      match def with
      | Ast.Fun f ->
          let type_to_keylist, type_to_defmap =
            FuzzySearchService.process_term
                f.Ast.f_name Function type_to_keylist type_to_defmap in
          type_to_keylist, type_to_defmap, defs
      | Ast.Class c ->
          let type_to_keylist, type_to_defmap =
            FuzzySearchService.process_term
                c.Ast.c_name
                (Class c.Ast.c_kind)
                type_to_keylist
                type_to_defmap
          in
          (* Still index methods for regular search *)
          let defs = update_class c defs in
          type_to_keylist, type_to_defmap, defs
      | Ast.Typedef td ->
          let type_to_keylist, type_to_defmap =
            FuzzySearchService.process_term
                td.Ast.t_id Typedef type_to_keylist type_to_defmap in
          type_to_keylist, type_to_defmap, defs
      | Ast.Constant cst ->
          let type_to_keylist, type_to_defmap =
            FuzzySearchService.process_term
                cst.Ast.cst_name Constant type_to_keylist type_to_defmap in
          type_to_keylist, type_to_defmap, defs
      | _ -> type_to_keylist, type_to_defmap, defs
    end (TMap.empty, TMap.empty, []) ast in

    SearchUpdates.add fn defs;
    let keys = List.fold_left begin fun acc def ->
      let key = simplify_key (fst def) in
      SSet.add key acc
    end SSet.empty defs in
    let keys = SSet.elements keys in
    SearchKeys.add fn keys;

    FuzzySearchService.SearchKeyToTermMap.add fn type_to_defmap;
    FuzzySearchService.SearchKeys.add fn type_to_keylist

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

    FuzzySearchService.index_files files;
    (* At this point, users can start searching agian so we should clear the
     * cache that contains the actual results. We don't have to worry
     * about the string->keys list shared memory because it's uncached *)
    SharedMem.invalidate_caches()

  let clear_shared_memory failed_parsing =
    SearchUpdates.remove_batch failed_parsing;
    SearchKeys.remove_batch failed_parsing;
    FuzzySearchService.SearchKeyToTermMap.remove_batch failed_parsing;
    FuzzySearchService.SearchKeys.remove_batch failed_parsing

  (* Note: the score should be able to compare to the scoring in
   * FuzzySearchService so that the results can be merged and the ordering still
   * makes sense. *)
  let rec get_score ?qi:(qi=0) ?ti:(ti=0) ?score:(score=0) term query =
    if (String.length query = String.length term.name) then
      if String.compare query term.name = 0 then 0
      else String.length term.name
    else if (qi >= String.length query || ti >= String.length term.name) then
      score + ti + String.length term.name
    else
      let qc = String.get query qi in
      let tc = String.get term.name ti in
      if Char.compare qc tc = 0 then
        get_score ~qi:(qi+1) ~ti:(ti+1) ~score:(score) term query
      else
        get_score ~qi:(qi+1) ~ti:(ti+1) ~score:(score+1) term query

  let search_query input =
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
          let score =
            if str_starts_with (String.lowercase res.name) str
            then get_score res str
            else (String.length key) * 2
          in
          results := (res, score) :: !results
      end defs
    end files;
    let res = List.sort begin fun a b ->
      (snd a) - (snd b)
    end !results in
    Utils.cut_after 50 res

  let get_type = function
    | "class" -> Some (Class Ast.Cnormal)
    | "function" -> Some Function
    | "constant" -> Some Constant
    | "typedef" -> Some Typedef
    | _ -> None

  let query input type_ =
    let type_ = get_type type_ in
    let res = match type_ with
    | Some _ -> []
    | None -> search_query input in

    let fuzzy_results = FuzzySearchService.query input type_ in
    let res = List.merge begin fun a b ->
      (snd a) - (snd b)
    end fuzzy_results res in
    let res = Utils.cut_after 50 res in
    List.map fst res

end
