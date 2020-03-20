(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Core_kernel
open Reordered_argument_collections

let db_path_of_ctx (ctx : Provider_context.t) : Naming_sqlite.db_path option =
  Db_path_provider.get_naming_db_path ctx

let not_implemented (backend : Provider_backend.t) =
  failwith
    ("not implemented for backend: " ^ Provider_backend.t_to_string backend)

let attach_name_type (name_type : FileInfo.name_type) (x : 'a) :
    'a * FileInfo.name_type =
  (x, name_type)

let remove_name_type (x : 'a * FileInfo.name_type) : 'a = fst x

let find_symbol_in_context
    ~(ctx : Provider_context.t)
    ~(get_entry_symbols : FileInfo.t -> (FileInfo.id * FileInfo.name_type) list)
    ~(is_symbol : string -> bool) :
    (Relative_path.t * (FileInfo.pos * FileInfo.name_type)) option =
  ctx.Provider_context.entries
  |> Relative_path.Map.filter_map ~f:(fun entry ->
         let file_info = Ast_provider.compute_file_info ctx entry in
         let symbols = get_entry_symbols file_info in
         List.find_map symbols ~f:(fun ((pos, name), kind) ->
             if is_symbol name then
               Some (pos, kind)
             else
               None))
  |> Relative_path.Map.choose_opt

let find_symbol_in_context_with_suppression
    ~(ctx : Provider_context.t)
    ~(get_entry_symbols : FileInfo.t -> (FileInfo.id * FileInfo.name_type) list)
    ~(is_symbol : string -> bool)
    ~(fallback : unit -> (FileInfo.pos * FileInfo.name_type) option) :
    (FileInfo.pos * FileInfo.name_type) option =
  match find_symbol_in_context ~ctx ~get_entry_symbols ~is_symbol with
  | Some (_path, pos) -> Some pos
  | None ->
    (match fallback () with
    | Some (pos, kind) ->
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
        Some (pos, kind)
    | None -> None)

let get_and_cache
    ~(ctx : Provider_context.t)
    ~(name : 'name)
    ~(make_pos_for_kind : 'sqlite_result -> 'pos)
    ~(get_delta_for_kind :
       unit ->
       'pos Provider_backend.Reverse_naming_table_delta.pos_or_deleted SMap.t)
    ~(set_delta_for_kind :
       'pos Provider_backend.Reverse_naming_table_delta.pos_or_deleted SMap.t ->
       unit)
    ~(get_from_sqlite : Naming_sqlite.db_path -> 'name -> 'sqlite_result option)
    =
  let open Provider_backend.Reverse_naming_table_delta in
  let delta = get_delta_for_kind () in
  match (SMap.find_opt delta name, db_path_of_ctx ctx) with
  | (Some Deleted, _) -> None
  | (Some (Pos pos), _) -> Some pos
  | (None, Some db_path) ->
    (match get_from_sqlite db_path name with
    | None -> None
    | Some path ->
      let pos = make_pos_for_kind path in
      let new_delta = SMap.add delta ~key:name ~data:(Pos pos) in
      set_delta_for_kind new_delta;
      Some pos)
  | (None, _) -> None

let get_const_pos (ctx : Provider_context.t) (name : string) :
    FileInfo.pos option =
  let open Option.Monad_infix in
  find_symbol_in_context_with_suppression
    ~ctx
    ~get_entry_symbols:(fun { FileInfo.consts; _ } ->
      List.map consts ~f:(attach_name_type FileInfo.Const))
    ~is_symbol:(String.equal name)
    ~fallback:(fun () ->
      match ctx.Provider_context.backend with
      | Provider_backend.Shared_memory ->
        Naming_heap.Consts.get_pos (db_path_of_ctx ctx) name
        >>| attach_name_type FileInfo.Const
      | Provider_backend.Local_memory { reverse_naming_table_delta; _ } ->
        let open Provider_backend.Reverse_naming_table_delta in
        get_and_cache
          ~ctx
          ~name
          ~make_pos_for_kind:(fun path -> FileInfo.File (FileInfo.Const, path))
          ~get_delta_for_kind:(fun () -> reverse_naming_table_delta.consts)
          ~set_delta_for_kind:(fun consts ->
            reverse_naming_table_delta.consts <- consts)
          ~get_from_sqlite:(fun db_path name ->
            Naming_sqlite.get_const_pos db_path name)
        >>| attach_name_type FileInfo.Const
      | Provider_backend.Decl_service _ as backend -> not_implemented backend)
  >>| remove_name_type

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
  | Provider_backend.Shared_memory ->
    Naming_heap.Consts.remove_batch (db_path_of_ctx ctx) names
  | Provider_backend.Local_memory { reverse_naming_table_delta; _ } ->
    let open Provider_backend.Reverse_naming_table_delta in
    reverse_naming_table_delta.consts <-
      SSet.fold
        names
        ~init:reverse_naming_table_delta.consts
        ~f:(fun name acc -> SMap.add acc ~key:name ~data:Deleted)
  | Provider_backend.Decl_service _ as backend -> not_implemented backend

let get_fun_pos (ctx : Provider_context.t) (name : string) : FileInfo.pos option
    =
  let open Option.Monad_infix in
  find_symbol_in_context_with_suppression
    ~ctx
    ~get_entry_symbols:(fun { FileInfo.funs; _ } ->
      List.map funs ~f:(attach_name_type FileInfo.Fun))
    ~is_symbol:(String.equal name)
    ~fallback:(fun () ->
      match ctx.Provider_context.backend with
      | Provider_backend.Shared_memory ->
        Naming_heap.Funs.get_pos (db_path_of_ctx ctx) name
        >>| attach_name_type FileInfo.Fun
      | Provider_backend.Local_memory { reverse_naming_table_delta; _ } ->
        let open Provider_backend.Reverse_naming_table_delta in
        get_and_cache
          ~ctx
          ~name
          ~make_pos_for_kind:(fun path -> FileInfo.File (FileInfo.Fun, path))
          ~get_delta_for_kind:(fun () -> reverse_naming_table_delta.funs)
          ~set_delta_for_kind:(fun funs ->
            reverse_naming_table_delta.funs <- funs)
          ~get_from_sqlite:(fun db_path name ->
            Naming_sqlite.get_fun_pos db_path ~case_insensitive:false name)
        >>| attach_name_type FileInfo.Fun
      | Provider_backend.Decl_service _ as backend -> not_implemented backend)
  >>| remove_name_type

let fun_exists (ctx : Provider_context.t) (name : string) : bool =
  get_fun_pos ctx name |> Option.is_some

let get_fun_path (ctx : Provider_context.t) (name : string) :
    Relative_path.t option =
  get_fun_pos ctx name |> Option.map ~f:FileInfo.get_pos_filename

let get_fun_canon_name (ctx : Provider_context.t) (name : string) :
    string option =
  let open Option.Monad_infix in
  let canon_name_key = Naming_sqlite.to_canon_name_key name in
  let symbol_opt =
    find_symbol_in_context
      ~ctx
      ~get_entry_symbols:(fun { FileInfo.funs; _ } ->
        List.map funs ~f:(attach_name_type FileInfo.Fun))
      ~is_symbol:(fun symbol_name ->
        String.equal
          (Naming_sqlite.to_canon_name_key symbol_name)
          canon_name_key)
  in
  let compute_symbol_canon_name path =
    Ast_provider.find_fun_in_file ~case_insensitive:true ctx path name
    >>| fun { Aast.f_name = (_, canon_name); _ } -> canon_name
  in

  match symbol_opt with
  | Some (path, _pos) -> compute_symbol_canon_name path
  | None ->
    (match ctx.Provider_context.backend with
    | Provider_backend.Shared_memory ->
      (* NB: as written, this code may return a canon name even when the
        given symbol has been deleted in a context entry. We're relying on
        the caller to have called `remove_fun_batch` on any deleted symbols
        before having called this function. `get_fun_canon_name` is only
        called in some functions in `Naming_global`, which expects the caller
        to have called `Naming_global.remove_decls` already. *)
      Naming_heap.Funs.get_canon_name (db_path_of_ctx ctx) ctx name
    | Provider_backend.Local_memory { reverse_naming_table_delta; _ } ->
      let open Provider_backend.Reverse_naming_table_delta in
      get_and_cache
        ~ctx
        ~name
        ~make_pos_for_kind:(fun path -> FileInfo.File (FileInfo.Fun, path))
        ~get_delta_for_kind:(fun () -> reverse_naming_table_delta.funs)
        ~set_delta_for_kind:(fun _funs ->
          (* Do not cache, since we're looking up case-insensitively. *)
          ())
        ~get_from_sqlite:(fun db_path name ->
          Naming_sqlite.get_fun_pos db_path ~case_insensitive:true name)
      >>= fun pos -> compute_symbol_canon_name (FileInfo.get_pos_filename pos)
    | Provider_backend.Decl_service _ as backend -> not_implemented backend)

let add_fun (ctx : Provider_context.t) (name : string) (pos : FileInfo.pos) :
    unit =
  match ctx.Provider_context.backend with
  | Provider_backend.Shared_memory -> Naming_heap.Funs.add name pos
  | Provider_backend.Local_memory { reverse_naming_table_delta; _ } ->
    let open Provider_backend.Reverse_naming_table_delta in
    reverse_naming_table_delta.funs <-
      SMap.add reverse_naming_table_delta.funs ~key:name ~data:(Pos pos)
  | Provider_backend.Decl_service _ as backend -> not_implemented backend

let remove_fun_batch (ctx : Provider_context.t) (names : SSet.t) : unit =
  match ctx.Provider_context.backend with
  | Provider_backend.Shared_memory ->
    Naming_heap.Funs.remove_batch (db_path_of_ctx ctx) names
  | Provider_backend.Local_memory { reverse_naming_table_delta; _ } ->
    let open Provider_backend.Reverse_naming_table_delta in
    reverse_naming_table_delta.funs <-
      SSet.fold names ~init:reverse_naming_table_delta.funs ~f:(fun name acc ->
          SMap.add acc ~key:name ~data:Deleted)
  | Provider_backend.Decl_service _ as backend -> not_implemented backend

let add_type
    (ctx : Provider_context.t)
    (name : string)
    (pos : FileInfo.pos)
    (kind : Naming_types.kind_of_type) : unit =
  match ctx.Provider_context.backend with
  | Provider_backend.Shared_memory -> Naming_heap.Types.add name (pos, kind)
  | Provider_backend.Local_memory { reverse_naming_table_delta; _ } ->
    let open Provider_backend.Reverse_naming_table_delta in
    reverse_naming_table_delta.types <-
      SMap.add
        reverse_naming_table_delta.types
        ~key:name
        ~data:(Pos (pos, kind))
  | Provider_backend.Decl_service _ as backend -> not_implemented backend

let remove_type_batch (ctx : Provider_context.t) (names : SSet.t) : unit =
  match ctx.Provider_context.backend with
  | Provider_backend.Shared_memory ->
    Naming_heap.Types.remove_batch (db_path_of_ctx ctx) names
  | Provider_backend.Local_memory { reverse_naming_table_delta; _ } ->
    let open Provider_backend.Reverse_naming_table_delta in
    reverse_naming_table_delta.types <-
      SSet.fold names ~init:reverse_naming_table_delta.types ~f:(fun name acc ->
          SMap.add acc ~key:name ~data:Deleted)
  | Provider_backend.Decl_service _ as backend -> not_implemented backend

let get_entry_symbols_for_type { FileInfo.classes; typedefs; record_defs; _ } =
  let classes = List.map classes ~f:(attach_name_type FileInfo.Class) in
  let typedefs = List.map typedefs ~f:(attach_name_type FileInfo.Typedef) in
  let record_defs =
    List.map record_defs ~f:(attach_name_type FileInfo.RecordDef)
  in
  List.concat [classes; typedefs; record_defs]

let kind_to_name_type (kind : Naming_types.kind_of_type) : FileInfo.name_type =
  match kind with
  | Naming_types.TClass -> FileInfo.Class
  | Naming_types.TRecordDef -> FileInfo.RecordDef
  | Naming_types.TTypedef -> FileInfo.Typedef

let name_type_to_kind (name_type : FileInfo.name_type) :
    Naming_types.kind_of_type =
  match name_type with
  | FileInfo.Class -> Naming_types.TClass
  | FileInfo.Typedef -> Naming_types.TTypedef
  | FileInfo.RecordDef -> Naming_types.TRecordDef
  | (FileInfo.Const | FileInfo.Fun) as name_type ->
    failwith
      (Printf.sprintf
         "Unexpected name type %s"
         (FileInfo.show_name_type name_type))

let get_type_pos_and_kind (ctx : Provider_context.t) (name : string) :
    (FileInfo.pos * Naming_types.kind_of_type) option =
  let open Option.Monad_infix in
  find_symbol_in_context_with_suppression
    ~ctx
    ~get_entry_symbols:get_entry_symbols_for_type
    ~is_symbol:(String.equal name)
    ~fallback:(fun () ->
      match ctx.Provider_context.backend with
      | Provider_backend.Shared_memory ->
        Naming_heap.Types.get_pos (db_path_of_ctx ctx) name
        >>| fun (pos, kind) -> (pos, kind_to_name_type kind)
      | Provider_backend.Local_memory { reverse_naming_table_delta; _ } ->
        let open Provider_backend.Reverse_naming_table_delta in
        get_and_cache
          ~ctx
          ~name
          ~make_pos_for_kind:(fun (path, kind) ->
            let name_type = kind_to_name_type kind in
            let pos = FileInfo.File (name_type, path) in
            (pos, kind))
          ~get_delta_for_kind:(fun () -> reverse_naming_table_delta.types)
          ~set_delta_for_kind:(fun types ->
            reverse_naming_table_delta.types <- types)
          ~get_from_sqlite:(fun db_path name ->
            Naming_sqlite.get_type_pos db_path ~case_insensitive:false name)
        >>| fun (pos, kind) -> (pos, kind_to_name_type kind)
      | Provider_backend.Decl_service _ as backend -> not_implemented backend)
  >>| fun (pos, name_type) -> (pos, name_type_to_kind name_type)

let get_type_pos (ctx : Provider_context.t) (name : string) :
    FileInfo.pos option =
  match get_type_pos_and_kind ctx name with
  | Some (pos, _kind) -> Some pos
  | None -> None

let get_type_path (ctx : Provider_context.t) (name : string) :
    Relative_path.t option =
  match get_type_pos_and_kind ctx name with
  | Some (pos, _kind) -> Some (FileInfo.get_pos_filename pos)
  | None -> None

let get_type_path_and_kind (ctx : Provider_context.t) (name : string) :
    (Relative_path.t * Naming_types.kind_of_type) option =
  match get_type_pos_and_kind ctx name with
  | Some (pos, kind) -> Some (FileInfo.get_pos_filename pos, kind)
  | None -> None

let get_type_kind (ctx : Provider_context.t) (name : string) :
    Naming_types.kind_of_type option =
  match get_type_pos_and_kind ctx name with
  | Some (_pos, kind) -> Some kind
  | None -> None

let get_type_canon_name (ctx : Provider_context.t) (name : string) :
    string option =
  let open Option.Monad_infix in
  let canon_name_key = Naming_sqlite.to_canon_name_key name in
  let symbol_opt =
    find_symbol_in_context
      ~ctx
      ~get_entry_symbols:get_entry_symbols_for_type
      ~is_symbol:(fun symbol_name ->
        String.equal
          (Naming_sqlite.to_canon_name_key symbol_name)
          canon_name_key)
  in
  let compute_symbol_canon_name path kind =
    match kind with
    | Naming_types.TClass ->
      Ast_provider.find_class_in_file ~case_insensitive:true ctx path name
      >>| fun { Aast.c_name = (_, canon_name); _ } -> canon_name
    | Naming_types.TTypedef ->
      Ast_provider.find_typedef_in_file ~case_insensitive:true ctx path name
      >>| fun { Aast.t_name = (_, canon_name); _ } -> canon_name
    | Naming_types.TRecordDef ->
      Ast_provider.find_record_def_in_file ~case_insensitive:true ctx path name
      >>| fun { Aast.rd_name = (_, canon_name); _ } -> canon_name
  in

  match symbol_opt with
  | Some (path, (_pos, name_type)) ->
    compute_symbol_canon_name path (name_type_to_kind name_type)
  | None ->
    (match ctx.Provider_context.backend with
    | Provider_backend.Shared_memory ->
      (* NB: as written, this code may return a canon name even when the
    given symbol has been deleted in a context entry. We're relying on
    the caller to have called `remove_fun_batch` on any deleted symbols
    before having called this function. `get_type_canon_name` is only
    called in some functions in `Naming_global`, which expects the caller
    to have called `Naming_global.remove_decls` already. *)
      Naming_heap.Types.get_canon_name (db_path_of_ctx ctx) ctx name
    | Provider_backend.Local_memory { reverse_naming_table_delta; _ } ->
      let open Option.Monad_infix in
      let open Provider_backend.Reverse_naming_table_delta in
      get_and_cache
        ~ctx
        ~name
        ~make_pos_for_kind:(fun (path, kind) ->
          let name_type =
            match kind with
            | Naming_types.TClass -> FileInfo.Class
            | Naming_types.TRecordDef -> FileInfo.RecordDef
            | Naming_types.TTypedef -> FileInfo.Typedef
          in
          let file_info = FileInfo.File (name_type, path) in
          (file_info, kind))
        ~get_delta_for_kind:(fun () -> reverse_naming_table_delta.types)
        ~set_delta_for_kind:(fun types ->
          reverse_naming_table_delta.types <- types)
        ~get_from_sqlite:(fun db_path name ->
          Naming_sqlite.get_type_pos db_path ~case_insensitive:true name)
      >>= fun (pos, kind) ->
      compute_symbol_canon_name (FileInfo.get_pos_filename pos) kind
    | Provider_backend.Decl_service _ as backend -> not_implemented backend)

let get_class_path (ctx : Provider_context.t) (name : string) :
    Relative_path.t option =
  match get_type_path_and_kind ctx name with
  | Some (fn, Naming_types.TClass) -> Some fn
  | Some (_, (Naming_types.TRecordDef | Naming_types.TTypedef))
  | None ->
    None

let add_class (ctx : Provider_context.t) (name : string) (pos : FileInfo.pos) :
    unit =
  add_type ctx name pos Naming_types.TClass

let get_record_def_path (ctx : Provider_context.t) (name : string) :
    Relative_path.t option =
  match get_type_path_and_kind ctx name with
  | Some (fn, Naming_types.TRecordDef) -> Some fn
  | Some (_, (Naming_types.TClass | Naming_types.TTypedef))
  | None ->
    None

let add_record_def
    (ctx : Provider_context.t) (name : string) (pos : FileInfo.pos) : unit =
  add_type ctx name pos Naming_types.TRecordDef

let get_typedef_path (ctx : Provider_context.t) (name : string) :
    Relative_path.t option =
  match get_type_path_and_kind ctx name with
  | Some (fn, Naming_types.TTypedef) -> Some fn
  | Some (_, (Naming_types.TClass | Naming_types.TRecordDef))
  | None ->
    None

let add_typedef (ctx : Provider_context.t) (name : string) (pos : FileInfo.pos)
    : unit =
  add_type ctx name pos Naming_types.TTypedef

let push_local_changes (ctx : Provider_context.t) : unit =
  match ctx.Provider_context.backend with
  | Provider_backend.Shared_memory -> Naming_heap.push_local_changes ()
  | Provider_backend.Local_memory _ -> ()
  | Provider_backend.Decl_service _ as backend -> not_implemented backend

let pop_local_changes (ctx : Provider_context.t) : unit =
  match ctx.Provider_context.backend with
  | Provider_backend.Shared_memory -> Naming_heap.pop_local_changes ()
  | Provider_backend.Local_memory _ -> ()
  | Provider_backend.Decl_service _ as backend -> not_implemented backend
