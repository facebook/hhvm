(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

let const_exists (_ctx : Provider_context.t) (name : string) : bool =
  Naming_heap.Consts.is_defined name

let get_const_path (_ctx : Provider_context.t) (name : string) :
    Relative_path.t option =
  Naming_heap.Consts.get_filename name

let get_const_pos (_ctx : Provider_context.t) (name : string) :
    FileInfo.pos option =
  Naming_heap.Consts.get_pos name

let add_const (_ctx : Provider_context.t) (name : string) (pos : FileInfo.pos) :
    unit =
  Naming_heap.Consts.add name pos

let remove_const_batch (_ctx : Provider_context.t) (names : SSet.t) : unit =
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

let get_class_path (name : string) : Relative_path.t option =
  match Naming_heap.Types.get_filename_and_kind name with
  | Some (fn, Naming_types.TClass) -> Some fn
  | Some (_, (Naming_types.TRecordDef | Naming_types.TTypedef))
  | None ->
    None

let add_class (name : string) (pos : FileInfo.pos) : unit =
  add_type name pos Naming_types.TClass

let get_record_def_path (name : string) : Relative_path.t option =
  match Naming_heap.Types.get_filename_and_kind name with
  | Some (fn, Naming_types.TRecordDef) -> Some fn
  | Some (_, (Naming_types.TClass | Naming_types.TTypedef))
  | None ->
    None

let add_record_def (name : string) (pos : FileInfo.pos) : unit =
  add_type name pos Naming_types.TRecordDef

let get_typedef_path (name : string) : Relative_path.t option =
  match Naming_heap.Types.get_filename_and_kind name with
  | Some (fn, Naming_types.TTypedef) -> Some fn
  | Some (_, (Naming_types.TClass | Naming_types.TRecordDef))
  | None ->
    None

let add_typedef (name : string) (pos : FileInfo.pos) : unit =
  add_type name pos Naming_types.TTypedef

let push_local_changes () : unit = Naming_heap.push_local_changes ()

let pop_local_changes () : unit = Naming_heap.pop_local_changes ()
