(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let const_exists (name : string) : bool = Naming_heap.Consts.is_defined name

let get_const_path (name : string) : Relative_path.t option =
  Naming_heap.Consts.get_filename name

let get_const_pos (name : string) : FileInfo.pos option =
  Naming_heap.Consts.get_pos name

let add_const (name : string) (pos : FileInfo.pos) : unit =
  Naming_heap.Consts.add name pos

let remove_const_batch (names : SSet.t) : unit =
  Naming_heap.Consts.remove_batch names
