(**
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

open Core_kernel

module TLazyHeap = Typing_lazy_heap

let get_ancestors_single class_ ~filter =
  let class_ = MethodJumps.add_ns class_ in
  let class_ = TLazyHeap.get_class class_ in
  Option.map class_ ~f:(fun c ->
    MethodJumps.get_ancestor_classes_and_methods c ~filter []
  )

let get_ancestors_multiple acc classes ~filter =
  let result = List.concat (List.filter_map
    classes ~f:(fun class_ -> get_ancestors_single class_ ~filter))
  in result :: acc

let parallel_helper workers classes filter =
  MultiWorker.call
    workers
    ~job:(get_ancestors_multiple ~filter)
    ~neutral:[]
    ~merge:List.rev_append
    ~next:(MultiWorker.next workers classes)

(* Entry Point *)
let go:
  MultiWorker.worker list option ->
  Typing_heap.Classes.key list ->
  ServerCommandTypes.Method_jumps.filter ->
  ServerCommandTypes.Method_jumps.result list =
fun workers classes filter ->
  (* Sort and dedup identical queries *)
  let deduped =
    List.remove_consecutive_duplicates ~equal:(=)
      (List.sort ~compare classes)
  in
  let results =
    if (List.length deduped) < 10
    then get_ancestors_multiple [] deduped filter
    else parallel_helper workers deduped filter
  in
  List.concat results
