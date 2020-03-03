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

let fun_exists (name : string) : bool = Naming_heap.Funs.is_defined name

let get_fun_path (name : string) : Relative_path.t option =
  Naming_heap.Funs.get_filename name

let get_fun_pos (name : string) : FileInfo.pos option =
  Naming_heap.Funs.get_pos name

let get_fun_canon_name (ctx : Provider_context.t) (name : string) :
    string option =
  Naming_heap.Funs.get_canon_name ctx name

let add_fun (name : string) (pos : FileInfo.pos) : unit =
  Naming_heap.Funs.add name pos

let remove_fun_batch (names : SSet.t) : unit =
  Naming_heap.Funs.remove_batch names

let add_type
    (name : string) (pos : FileInfo.pos) (kind : Naming_types.kind_of_type) :
    unit =
  Naming_heap.Types.add name (pos, kind)

let remove_type_batch (names : SSet.t) : unit =
  Naming_heap.Types.remove_batch names

let get_type_pos (name : string) : FileInfo.pos option =
  match Naming_heap.Types.get_pos name with
  | Some (pos, _kind) -> Some pos
  | None -> None

let get_type_path (name : string) : Relative_path.t option =
  Naming_heap.Types.get_filename name

let get_type_pos_and_kind (name : string) :
    (FileInfo.pos * Naming_types.kind_of_type) option =
  Naming_heap.Types.get_pos name

let get_type_path_and_kind (name : string) :
    (Relative_path.t * Naming_types.kind_of_type) option =
  Naming_heap.Types.get_filename_and_kind name

let get_type_kind (name : string) : Naming_types.kind_of_type option =
  Naming_heap.Types.get_kind name

let get_type_canon_name (ctx : Provider_context.t) (name : string) :
    string option =
  Naming_heap.Types.get_canon_name ctx name
