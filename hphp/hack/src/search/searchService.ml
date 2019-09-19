(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_core

module Make (S : SearchUtils.Searchable) = struct
  module Fuzzy = FuzzySearchService.Make (S)
  module Trie = TrieSearchService.Make (S)
  module AutocompleteTrie = TrieSearchService.Make (S)

  module WorkerApi = struct
    let update fn trie_defs fuzzy_defs autocomplete_defs =
      Trie.WorkerApi.update fn trie_defs;
      AutocompleteTrie.WorkerApi.update fn autocomplete_defs;
      Fuzzy.update fn fuzzy_defs

    let process_trie_term = Trie.WorkerApi.process_term_for_search

    let process_autocomplete_term = Trie.WorkerApi.process_term

    let process_fuzzy_term = Fuzzy.process_term
  end

  module MasterApi = struct
    (* Called by the master process when there is new information in
     * shared memory for us to index *)
    let update_search_index ~fuzzy files =
      Trie.MasterApi.index_files files;
      AutocompleteTrie.MasterApi.index_files files;
      if fuzzy then Fuzzy.index_files files;

      (* At this point, users can start searching again so we should clear the
       * cache that contains the actual results. We don't have to worry
       * about the string->keys list shared memory because it's uncached *)
      SharedMem.invalidate_caches ()

    let clear_shared_memory fns =
      Trie.SearchUpdates.remove_batch fns;
      Trie.SearchKeys.remove_batch fns;
      AutocompleteTrie.SearchUpdates.remove_batch fns;
      AutocompleteTrie.SearchKeys.remove_batch fns;
      Fuzzy.SearchKeyToTermMap.remove_batch fns;
      Fuzzy.SearchKeys.remove_batch fns

    let query
        ~(fuzzy : bool)
        (workers : MultiWorker.worker list option)
        (input : string)
        (type_ : Fuzzy.TMap.key option) =
      let is_fuzzy_indexed =
        match type_ with
        | Some ty -> List.mem S.fuzzy_types ty
        | None -> true
      in
      let trie_results =
        match (type_, is_fuzzy_indexed) with
        | (Some _, false)
        | (None, _) ->
          Trie.MasterApi.search_query input type_
        | (Some _, true) when not fuzzy ->
          Trie.MasterApi.search_query input type_
        | _ -> []
      in
      let fuzzy_results =
        if not fuzzy then
          []
        else
          match (type_, is_fuzzy_indexed) with
          | (Some _, true)
          | (None, _) ->
            Fuzzy.query workers input type_
          | _ -> []
      in
      let res =
        List.merge fuzzy_results trie_results ~cmp:(fun a b -> snd a - snd b)
      in
      let res = List.take res 50 in
      List.map res fst
  end
end
