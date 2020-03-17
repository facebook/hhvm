(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Reordered_argument_collections

let not_implemented (backend : Provider_backend.t) =
  failwith
    ("not implemented for backend: " ^ Provider_backend.t_to_string backend)

let find_symbol_in_context
    ~(ctx : Provider_context.t)
    ~(get_entry_symbols : FileInfo.t -> FileInfo.id list)
    ~(is_symbol : string -> bool)
    ~(fallback : unit -> FileInfo.pos option) : FileInfo.pos option =
  let ctx_pos_opt =
    ctx.Provider_context.entries
    |> Relative_path.Map.filter_map ~f:(fun entry ->
           let file_info = Ast_provider.compute_file_info ctx entry in
           let symbols = get_entry_symbols file_info in
           List.find_map symbols ~f:(fun (pos, name) ->
               if is_symbol name then
                 Some pos
               else
                 None))
    |> Relative_path.Map.choose_opt
  in
  match ctx_pos_opt with
  | Some (_path, pos) -> Some pos
  | None ->
    (match fallback () with
    | Some pos ->
      (* We've just checked to see if the given symbol was in `ctx`, and
      returned it if present. If we've gotten to this point, and we're
      reporting that the symbol is in a file in `ctx`, but we didn't already
      find it in `ctx`, that means that the file content in `ctx` has deleted
      the symbol. So we should suppress this returned position and return
      `None` instead. *)
      let path = FileInfo.get_pos_filename pos in
      if Relative_path.Map.mem ctx.Provider_context.entries path then
        None
      else
        Some pos
    | None -> None)

let get_and_cache
    ~(name : 'name)
    ~(make_pos_for_kind : Relative_path.t -> 'pos)
    ~(get_delta_for_kind :
       unit ->
       'pos Provider_backend.Reverse_naming_table_delta.pos_or_deleted SMap.t)
    ~(set_delta_for_kind :
       'pos Provider_backend.Reverse_naming_table_delta.pos_or_deleted SMap.t ->
       unit)
    ~(get_from_sqlite : 'name -> Relative_path.t option) =
  let open Provider_backend.Reverse_naming_table_delta in
  let delta = get_delta_for_kind () in
  match SMap.find_opt delta name with
  | Some Deleted -> None
  | Some (Pos pos) -> Some pos
  | None when Naming_sqlite.is_connected () ->
    (match get_from_sqlite name with
    | None -> None
    | Some path ->
      let pos = make_pos_for_kind path in
      let new_delta = SMap.add delta ~key:name ~data:(Pos pos) in
      set_delta_for_kind new_delta;
      Some pos)
  | None -> None

let get_const_pos (ctx : Provider_context.t) (name : string) :
    FileInfo.pos option =
  find_symbol_in_context
    ~ctx
    ~get_entry_symbols:(fun { FileInfo.consts; _ } -> consts)
    ~is_symbol:(String.equal name)
    ~fallback:(fun () ->
      match ctx.Provider_context.backend with
      | Provider_backend.Shared_memory -> Naming_heap.Consts.get_pos name
      | Provider_backend.Local_memory { reverse_naming_table_delta; _ } ->
        let open Provider_backend.Reverse_naming_table_delta in
        get_and_cache
          ~name
          ~make_pos_for_kind:(fun path -> FileInfo.File (FileInfo.Const, path))
          ~get_delta_for_kind:(fun () -> reverse_naming_table_delta.consts)
          ~set_delta_for_kind:(fun consts ->
            reverse_naming_table_delta.consts <- consts)
          ~get_from_sqlite:(fun name -> Naming_sqlite.get_const_pos name)
      | Provider_backend.Decl_service _ as backend -> not_implemented backend)

let const_exists (ctx : Provider_context.t) (name : string) : bool =
  get_const_pos ctx name |> Option.is_some

let get_const_path (ctx : Provider_context.t) (name : string) :
    Relative_path.t option =
  get_const_pos ctx name |> Option.map ~f:FileInfo.get_pos_filename

let add_const (ctx : Provider_context.t) (name : string) (pos : FileInfo.pos) :
    unit =
  match ctx.Provider_context.backend with
  | Provider_backend.Shared_memory -> Naming_heap.Consts.add name pos
  | Provider_backend.Local_memory { reverse_naming_table_delta; _ } ->
    let open Provider_backend.Reverse_naming_table_delta in
    reverse_naming_table_delta.consts <-
      SMap.add reverse_naming_table_delta.consts ~key:name ~data:(Pos pos)
  | Provider_backend.Decl_service _ as backend -> not_implemented backend

let remove_const_batch (ctx : Provider_context.t) (names : SSet.t) : unit =
  match ctx.Provider_context.backend with
  | Provider_backend.Shared_memory -> Naming_heap.Consts.remove_batch names
  | Provider_backend.Local_memory { reverse_naming_table_delta; _ } ->
    let open Provider_backend.Reverse_naming_table_delta in
    reverse_naming_table_delta.consts <-
      SSet.fold
        names
        ~init:reverse_naming_table_delta.consts
        ~f:(fun name acc -> SMap.add acc ~key:name ~data:Deleted)
  | Provider_backend.Decl_service _ as backend -> not_implemented backend

let fun_exists (_ctx : Provider_context.t) (name : string) : bool =
  Naming_heap.Funs.is_defined name

let get_fun_path (_ctx : Provider_context.t) (name : string) :
    Relative_path.t option =
  Naming_heap.Funs.get_filename name

let get_fun_pos (_ctx : Provider_context.t) (name : string) :
    FileInfo.pos option =
  Naming_heap.Funs.get_pos name

let get_fun_canon_name (ctx : Provider_context.t) (name : string) :
    string option =
  Naming_heap.Funs.get_canon_name ctx name

let add_fun (_ctx : Provider_context.t) (name : string) (pos : FileInfo.pos) :
    unit =
  Naming_heap.Funs.add name pos

let remove_fun_batch (_ctx : Provider_context.t) (names : SSet.t) : unit =
  Naming_heap.Funs.remove_batch names

let add_type
    (_ctx : Provider_context.t)
    (name : string)
    (pos : FileInfo.pos)
    (kind : Naming_types.kind_of_type) : unit =
  Naming_heap.Types.add name (pos, kind)

let remove_type_batch (_ctx : Provider_context.t) (names : SSet.t) : unit =
  Naming_heap.Types.remove_batch names

let get_type_pos (_ctx : Provider_context.t) (name : string) :
    FileInfo.pos option =
  match Naming_heap.Types.get_pos name with
  | Some (pos, _kind) -> Some pos
  | None -> None

let get_type_path (_ctx : Provider_context.t) (name : string) :
    Relative_path.t option =
  Naming_heap.Types.get_filename name

let get_type_pos_and_kind (_ctx : Provider_context.t) (name : string) :
    (FileInfo.pos * Naming_types.kind_of_type) option =
  Naming_heap.Types.get_pos name

let get_type_path_and_kind (_ctx : Provider_context.t) (name : string) :
    (Relative_path.t * Naming_types.kind_of_type) option =
  Naming_heap.Types.get_filename_and_kind name

let get_type_kind (_ctx : Provider_context.t) (name : string) :
    Naming_types.kind_of_type option =
  Naming_heap.Types.get_kind name

let get_type_canon_name (ctx : Provider_context.t) (name : string) :
    string option =
  Naming_heap.Types.get_canon_name ctx name

let get_class_path (_ctx : Provider_context.t) (name : string) :
    Relative_path.t option =
  match Naming_heap.Types.get_filename_and_kind name with
  | Some (fn, Naming_types.TClass) -> Some fn
  | Some (_, (Naming_types.TRecordDef | Naming_types.TTypedef))
  | None ->
    None

let add_class (ctx : Provider_context.t) (name : string) (pos : FileInfo.pos) :
    unit =
  add_type ctx name pos Naming_types.TClass

let get_record_def_path (_ctx : Provider_context.t) (name : string) :
    Relative_path.t option =
  match Naming_heap.Types.get_filename_and_kind name with
  | Some (fn, Naming_types.TRecordDef) -> Some fn
  | Some (_, (Naming_types.TClass | Naming_types.TTypedef))
  | None ->
    None

let add_record_def
    (ctx : Provider_context.t) (name : string) (pos : FileInfo.pos) : unit =
  add_type ctx name pos Naming_types.TRecordDef

let get_typedef_path (_ctx : Provider_context.t) (name : string) :
    Relative_path.t option =
  match Naming_heap.Types.get_filename_and_kind name with
  | Some (fn, Naming_types.TTypedef) -> Some fn
  | Some (_, (Naming_types.TClass | Naming_types.TRecordDef))
  | None ->
    None

let add_typedef (ctx : Provider_context.t) (name : string) (pos : FileInfo.pos)
    : unit =
  add_type ctx name pos Naming_types.TTypedef

let push_local_changes (_ctx : Provider_context.t) : unit =
  Naming_heap.push_local_changes ()

let pop_local_changes (_ctx : Provider_context.t) : unit =
  Naming_heap.pop_local_changes ()
