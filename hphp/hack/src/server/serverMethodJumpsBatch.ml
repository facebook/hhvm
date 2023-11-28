(*
 * Copyright (c) 2017, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE fn in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

let get_ancestors_single ctx class_ ~filter =
  let class_ = MethodJumps.add_ns class_ in
  let class_ = Decl_provider.get_class ctx class_ |> Decl_entry.to_option in
  Option.map class_ ~f:(fun c ->
      MethodJumps.get_ancestor_classes_and_methods ctx c ~filter [])

let get_ancestors_multiple ctx acc classes ~filter =
  let result =
    List.concat
      (List.filter_map classes ~f:(fun class_ ->
           get_ancestors_single ctx class_ ~filter))
  in
  result :: acc

let parallel_helper ctx workers classes filter =
  MultiWorker.call
    workers
    ~job:(get_ancestors_multiple ctx ~filter)
    ~neutral:[]
    ~merge:List.rev_append
    ~next:(MultiWorker.next workers classes)

(* Entry Point *)
let go :
    Provider_context.t ->
    MultiWorker.worker list option ->
    Decl_provider.type_key list ->
    ServerCommandTypes.Method_jumps.filter ->
    ServerCommandTypes.Method_jumps.result list =
 fun ctx workers classes filter ->
  (* Sort and dedup identical queries *)
  let deduped =
    List.remove_consecutive_duplicates
      ~equal:String.( = )
      (List.sort ~compare:String.compare classes)
  in
  let results =
    if List.length deduped < 10 then
      get_ancestors_multiple ctx [] deduped ~filter
    else
      parallel_helper ctx workers deduped filter
  in
  List.concat results
