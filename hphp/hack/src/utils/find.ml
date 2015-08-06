(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

(*****************************************************************************)
(* Prelude *)
(*****************************************************************************)

open Core

let fold_files (type t)
    ?max_depth ?(filter=(fun _ -> true))
    (paths: string list) (action: string -> t -> t) (init: t) =
  let rec fold depth acc dir =
    let recurse = max_depth <> Some depth in
    let files = Sys.readdir dir in
    Array.fold_left
      (fun acc file ->
         let file = Filename.concat dir file in
         if Sys.is_directory file then
           if recurse then fold (depth+1) acc file else acc
         else if filter file then action file acc else acc)
      acc files in
  List.fold_left paths ~init ~f:(fold 0)

let iter_files ?max_depth ?filter paths action =
  let paths = List.map paths Path.to_string in
  fold_files ?max_depth ?filter paths (fun file _ -> action file) ()

let find ?max_depth ?filter paths =
  let paths = List.map paths Path.to_string in
  fold_files ?max_depth ?filter paths List.cons []

(*****************************************************************************)
(* Main entry point *)
(*****************************************************************************)

let make_next_files ?filter ?(others=[]) root =
  let done_ = ref false in
  fun () ->
    if !done_ then
      (* see multiWorker.mli, this is the protocol for nextfunc *)
      []
    else
      (done_ := true; find ?filter (root :: others))

