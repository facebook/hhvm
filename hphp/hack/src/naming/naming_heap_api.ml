(**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

let get_class id =
  match Naming_heap.TypeIdHeap.get id with
  | None
  | Some (_, `Typedef) -> None
  | Some (p, `Class) ->
    let fn = Pos.filename p in
    match Parser_heap.find_class_in_file fn id with
    | None -> None
    | Some class_ ->
      Some (Naming.class_
        (TypecheckerOptions.make_permissive TypecheckerOptions.default)
        class_)

let get_fun id =
  match Naming_heap.FunPosHeap.get id with
  | None -> None
  | Some p ->
    let fn = Pos.filename p in
    match Parser_heap.find_fun_in_file fn id with
    | None -> None
    | Some fun_ ->
      Some (Naming.fun_
        (TypecheckerOptions.make_permissive TypecheckerOptions.default)
        fun_)
