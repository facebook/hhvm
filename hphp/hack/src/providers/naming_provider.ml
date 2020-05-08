(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Reordered_argument_collections

let writes_enabled = ref true

let db_path_of_ctx (ctx : Provider_context.t) : Naming_sqlite.db_path option =
  ctx |> Provider_context.get_backend |> Db_path_provider.get_naming_db_path

let not_implemented (backend : Provider_backend.t) =
  failwith
    ("not implemented for backend: " ^ Provider_backend.t_to_string backend)

let attach_name_type_to_tuple (name_type, path) =
  (FileInfo.File (name_type, path), name_type)

let attach_name_type (name_type : FileInfo.name_type) (x : 'a) :
    'a * FileInfo.name_type =
  (x, name_type)

let remove_name_type (x : 'a * FileInfo.name_type) : 'a = fst x

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

let find_symbol_in_context
    ~(ctx : Provider_context.t)
    ~(get_entry_symbols : FileInfo.t -> (FileInfo.id * FileInfo.name_type) list)
    ~(is_symbol : string -> bool) :
    (Relative_path.t * (FileInfo.pos * FileInfo.name_type)) option =
  Provider_context.get_entries ctx
  |> Relative_path.Map.filter_map ~f:(fun entry ->
         let file_info =
           Ast_provider.compute_file_info
             ~popt:(Provider_context.get_popt ctx)
             ~entry
         in
         let symbols = get_entry_symbols file_info in
         List.find_map symbols ~f:(fun ((pos, name), kind) ->
             if is_symbol name then
               Some (pos, kind)
             else
               None))
  |> Relative_path.Map.choose_opt

let is_path_in_ctx ~(ctx : Provider_context.t) (path : Relative_path.t) : bool =
  Relative_path.Map.mem (Provider_context.get_entries ctx) path

let is_pos_in_ctx ~(ctx : Provider_context.t) (pos : FileInfo.pos) : bool =
  is_path_in_ctx ~ctx (FileInfo.get_pos_filename pos)

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
    | Some (pos, name_type) ->
      (* If fallback said it thought the symbol was in ctx, but we definitively
      know that it isn't, then the answer is None. *)
      if is_pos_in_ctx ~ctx pos then
        None
      else
        Some (pos, name_type)
    | None -> None)

let get_and_cache
    ~(ctx : Provider_context.t)
    ~(name : 'name)
    ~(cache :
       Provider_backend.Reverse_naming_table_delta.pos_or_deleted SMap.t ref)
    ~(fallback :
       Naming_sqlite.db_path ->
       Provider_backend.Reverse_naming_table_delta.pos option) :
    Provider_backend.Reverse_naming_table_delta.pos option =
  let open Provider_backend.Reverse_naming_table_delta in
  match SMap.find_opt !cache name with
  | Some Deleted -> None
  | Some (Pos (name_type, path)) -> Some (name_type, path)
  | None ->
    (match Option.bind (db_path_of_ctx ctx) ~f:fallback with
    | None -> None
    | Some (name_type, path) ->
      cache := SMap.add !cache ~key:name ~data:(Pos (name_type, path));
      Some (name_type, path))

let get_const_pos (ctx : Provider_context.t) (name : string) :
    FileInfo.pos option =
  let open Option.Monad_infix in
  find_symbol_in_context_with_suppression
    ~ctx
    ~get_entry_symbols:(fun { FileInfo.consts; _ } ->
      List.map consts ~f:(attach_name_type FileInfo.Const))
    ~is_symbol:(String.equal name)
    ~fallback:(fun () ->
      match Provider_context.get_backend ctx with
      | Provider_backend.Shared_memory ->
        Naming_heap.Consts.get_pos (db_path_of_ctx ctx) name
        >>| attach_name_type FileInfo.Const
      | Provider_backend.Local_memory
          { Provider_backend.reverse_naming_table_delta; _ } ->
        let open Provider_backend.Reverse_naming_table_delta in
        get_and_cache
          ~ctx
          ~name
          ~cache:reverse_naming_table_delta.consts
          ~fallback:(fun db_path ->
            Naming_sqlite.get_const_pos db_path name
            |> Option.map ~f:(fun path -> (FileInfo.Const, path)))
        >>| attach_name_type_to_tuple
      | Provider_backend.Decl_service _ as backend -> not_implemented backend)
  >>| remove_name_type

let const_exists (ctx : Provider_context.t) (name : string) : bool =
  get_const_pos ctx name |> Option.is_some

let get_const_path (ctx : Provider_context.t) (name : string) :
    Relative_path.t option =
  get_const_pos ctx name |> Option.map ~f:FileInfo.get_pos_filename

let add_const
    (backend : Provider_backend.t) (name : string) (pos : FileInfo.pos) : unit =
  if !writes_enabled then
    match backend with
    | Provider_backend.Shared_memory -> Naming_heap.Consts.add name pos
    | Provider_backend.Local_memory
        { Provider_backend.reverse_naming_table_delta; _ } ->
      let open Provider_backend.Reverse_naming_table_delta in
      let data = Pos (FileInfo.Const, FileInfo.get_pos_filename pos) in
      reverse_naming_table_delta.consts :=
        SMap.add !(reverse_naming_table_delta.consts) ~key:name ~data;
      reverse_naming_table_delta.consts_canon_key :=
        SMap.add
          !(reverse_naming_table_delta.consts_canon_key)
          ~key:(Naming_sqlite.to_canon_name_key name)
          ~data
    | Provider_backend.Decl_service _ as backend -> not_implemented backend

let remove_const_batch (backend : Provider_backend.t) (names : SSet.t) : unit =
  match backend with
  | Provider_backend.Shared_memory ->
    Naming_heap.Consts.remove_batch
      (Db_path_provider.get_naming_db_path backend)
      names
  | Provider_backend.Local_memory
      { Provider_backend.reverse_naming_table_delta; _ } ->
    let open Provider_backend.Reverse_naming_table_delta in
    reverse_naming_table_delta.consts :=
      SSet.fold
        names
        ~init:!(reverse_naming_table_delta.consts)
        ~f:(fun name acc -> SMap.add acc ~key:name ~data:Deleted);
    reverse_naming_table_delta.consts_canon_key :=
      SSet.fold
        names
        ~init:!(reverse_naming_table_delta.consts_canon_key)
        ~f:(fun name acc ->
          SMap.add acc ~key:(Naming_sqlite.to_canon_name_key name) ~data:Deleted)
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
      match Provider_context.get_backend ctx with
      | Provider_backend.Shared_memory ->
        Naming_heap.Funs.get_pos (db_path_of_ctx ctx) name
        >>| attach_name_type FileInfo.Fun
      | Provider_backend.Local_memory
          { Provider_backend.reverse_naming_table_delta; _ } ->
        let open Provider_backend.Reverse_naming_table_delta in
        get_and_cache
          ~ctx
          ~name
          ~cache:reverse_naming_table_delta.funs
          ~fallback:(fun db_path ->
            Naming_sqlite.get_fun_pos db_path ~case_insensitive:false name
            |> Option.map ~f:(fun path -> (FileInfo.Fun, path)))
        >>| attach_name_type_to_tuple
      | Provider_backend.Decl_service { decl; _ } ->
        Decl_service_client.rpc_get_fun_path decl name
        |> Option.map ~f:(fun path -> FileInfo.(File (Fun, path), Fun)))
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
    (match Provider_context.get_backend ctx with
    | Provider_backend.Shared_memory ->
      (* NB: as written, this code may return a canon name even when the
        given symbol has been deleted in a context entry. We're relying on
        the caller to have called `remove_fun_batch` on any deleted symbols
        before having called this function. `get_fun_canon_name` is only
        called in some functions in `Naming_global`, which expects the caller
        to have called `Naming_global.remove_decls` already. *)
      Naming_heap.Funs.get_canon_name ctx name
    | Provider_backend.Local_memory
        { Provider_backend.reverse_naming_table_delta; _ } ->
      let open Provider_backend.Reverse_naming_table_delta in
      get_and_cache
        ~ctx
        ~name:canon_name_key
        ~cache:reverse_naming_table_delta.funs_canon_key
        ~fallback:(fun db_path ->
          Naming_sqlite.get_fun_pos db_path ~case_insensitive:true name
          |> Option.map ~f:(fun path -> (FileInfo.Fun, path)))
      >>= fun (_name_type, path) ->
      (* If reverse_naming_table_delta thought the symbol was in ctx, but we definitively
      know that it isn't, then it isn't. *)
      if is_path_in_ctx ~ctx path then
        None
      else
        compute_symbol_canon_name path
    | Provider_backend.Decl_service _ ->
      (* FIXME: Not correct! We need to add a canon-name API to the decl service. *)
      if fun_exists ctx name then
        Some name
      else
        None)

let add_fun (backend : Provider_backend.t) (name : string) (pos : FileInfo.pos)
    : unit =
  if !writes_enabled then
    match backend with
    | Provider_backend.Shared_memory -> Naming_heap.Funs.add name pos
    | Provider_backend.Local_memory
        { Provider_backend.reverse_naming_table_delta; _ } ->
      let open Provider_backend.Reverse_naming_table_delta in
      let data = Pos (FileInfo.Fun, FileInfo.get_pos_filename pos) in
      reverse_naming_table_delta.funs :=
        SMap.add !(reverse_naming_table_delta.funs) ~key:name ~data;
      reverse_naming_table_delta.funs_canon_key :=
        SMap.add
          !(reverse_naming_table_delta.funs_canon_key)
          ~key:(Naming_sqlite.to_canon_name_key name)
          ~data
    | Provider_backend.Decl_service _ ->
      (* Do nothing. All naming table updates are expected to have happened
       already--we should have sent a control request to the decl service asking
       it to update in response to the list of changed files. *)
      ()

let remove_fun_batch (backend : Provider_backend.t) (names : SSet.t) : unit =
  match backend with
  | Provider_backend.Shared_memory ->
    Naming_heap.Funs.remove_batch
      (Db_path_provider.get_naming_db_path backend)
      names
  | Provider_backend.Local_memory
      { Provider_backend.reverse_naming_table_delta; _ } ->
    let open Provider_backend.Reverse_naming_table_delta in
    reverse_naming_table_delta.funs :=
      SSet.fold
        names
        ~init:!(reverse_naming_table_delta.funs)
        ~f:(fun name acc -> SMap.add acc ~key:name ~data:Deleted);
    reverse_naming_table_delta.funs_canon_key :=
      SSet.fold
        names
        ~init:!(reverse_naming_table_delta.funs_canon_key)
        ~f:(fun name acc ->
          SMap.add acc ~key:(Naming_sqlite.to_canon_name_key name) ~data:Deleted)
  | Provider_backend.Decl_service _ as backend ->
    (* Removing cache items is not the responsibility of hh_worker. *)
    not_implemented backend

let add_type
    (backend : Provider_backend.t)
    (name : string)
    (pos : FileInfo.pos)
    (kind : Naming_types.kind_of_type) : unit =
  if !writes_enabled then
    match backend with
    | Provider_backend.Shared_memory -> Naming_heap.Types.add name (pos, kind)
    | Provider_backend.Local_memory
        { Provider_backend.reverse_naming_table_delta; _ } ->
      let open Provider_backend.Reverse_naming_table_delta in
      let data = Pos (kind_to_name_type kind, FileInfo.get_pos_filename pos) in
      reverse_naming_table_delta.types :=
        SMap.add !(reverse_naming_table_delta.types) ~key:name ~data;
      reverse_naming_table_delta.types_canon_key :=
        SMap.add
          !(reverse_naming_table_delta.types_canon_key)
          ~key:(Naming_sqlite.to_canon_name_key name)
          ~data
    | Provider_backend.Decl_service _ ->
      (* Do nothing. Naming table updates should be done already. *)
      ()

let remove_type_batch (backend : Provider_backend.t) (names : SSet.t) : unit =
  match backend with
  | Provider_backend.Shared_memory ->
    Naming_heap.Types.remove_batch
      (Db_path_provider.get_naming_db_path backend)
      names
  | Provider_backend.Local_memory
      { Provider_backend.reverse_naming_table_delta; _ } ->
    let open Provider_backend.Reverse_naming_table_delta in
    reverse_naming_table_delta.types :=
      SSet.fold
        names
        ~init:!(reverse_naming_table_delta.types)
        ~f:(fun name acc -> SMap.add acc ~key:name ~data:Deleted);
    reverse_naming_table_delta.types_canon_key :=
      SSet.fold
        names
        ~init:!(reverse_naming_table_delta.types_canon_key)
        ~f:(fun name acc ->
          SMap.add acc ~key:(Naming_sqlite.to_canon_name_key name) ~data:Deleted)
  | Provider_backend.Decl_service _ as backend ->
    (* Removing cache items is not the responsibility of hh_worker. *)
    not_implemented backend

let get_entry_symbols_for_type { FileInfo.classes; typedefs; record_defs; _ } =
  let classes = List.map classes ~f:(attach_name_type FileInfo.Class) in
  let typedefs = List.map typedefs ~f:(attach_name_type FileInfo.Typedef) in
  let record_defs =
    List.map record_defs ~f:(attach_name_type FileInfo.RecordDef)
  in
  List.concat [classes; typedefs; record_defs]

let get_type_pos_and_kind (ctx : Provider_context.t) (name : string) :
    (FileInfo.pos * Naming_types.kind_of_type) option =
  let open Option.Monad_infix in
  find_symbol_in_context_with_suppression
    ~ctx
    ~get_entry_symbols:get_entry_symbols_for_type
    ~is_symbol:(String.equal name)
    ~fallback:(fun () ->
      match Provider_context.get_backend ctx with
      | Provider_backend.Shared_memory ->
        Naming_heap.Types.get_pos (db_path_of_ctx ctx) name
        >>| fun (pos, kind) -> (pos, kind_to_name_type kind)
      | Provider_backend.Local_memory
          { Provider_backend.reverse_naming_table_delta; _ } ->
        let open Provider_backend.Reverse_naming_table_delta in
        get_and_cache
          ~ctx
          ~name
          ~cache:reverse_naming_table_delta.types
          ~fallback:(fun db_path ->
            Naming_sqlite.get_type_pos db_path ~case_insensitive:false name
            |> Option.map ~f:(fun (path, kind) ->
                   (kind_to_name_type kind, path)))
        >>| fun (name_type, path) -> (FileInfo.File (name_type, path), name_type)
      | Provider_backend.Decl_service { decl; _ } ->
        Decl_service_client.rpc_get_type_path_and_kind decl name
        |> Option.map ~f:(fun (path, name_type) ->
               match name_type with
               | Naming_types.TClass -> FileInfo.(File (Class, path), Class)
               | Naming_types.TTypedef ->
                 FileInfo.(File (Typedef, path), Typedef)
               | Naming_types.TRecordDef ->
                 FileInfo.(File (RecordDef, path), RecordDef)))
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
    (match Provider_context.get_backend ctx with
    | Provider_backend.Shared_memory ->
      (* NB: as written, this code may return a canon name even when the
    given symbol has been deleted in a context entry. We're relying on
    the caller to have called `remove_fun_batch` on any deleted symbols
    before having called this function. `get_type_canon_name` is only
    called in some functions in `Naming_global`, which expects the caller
    to have called `Naming_global.remove_decls` already. *)
      Naming_heap.Types.get_canon_name ctx name
    | Provider_backend.Local_memory
        { Provider_backend.reverse_naming_table_delta; _ } ->
      let open Option.Monad_infix in
      let open Provider_backend.Reverse_naming_table_delta in
      get_and_cache
        ~ctx
        ~name:canon_name_key
        ~cache:reverse_naming_table_delta.types_canon_key
        ~fallback:(fun db_path ->
          Naming_sqlite.get_type_pos db_path ~case_insensitive:true name
          |> Option.map ~f:(fun (path, kind) -> (kind_to_name_type kind, path)))
      >>= fun (name_type, path) ->
      (* If reverse_naming_table_delta thought the symbol was in ctx, but we definitively
      know that it isn't, then it isn't. *)
      if is_path_in_ctx ~ctx path then
        None
      else
        compute_symbol_canon_name path (name_type_to_kind name_type)
    | Provider_backend.Decl_service _ ->
      (* FIXME: Not correct! We need to add a canon-name API to the decl service. *)
      if Option.is_some (get_type_kind ctx name) then
        Some name
      else
        None)

let get_class_path (ctx : Provider_context.t) (name : string) :
    Relative_path.t option =
  match get_type_path_and_kind ctx name with
  | Some (fn, Naming_types.TClass) -> Some fn
  | Some (_, (Naming_types.TRecordDef | Naming_types.TTypedef))
  | None ->
    None

let add_class
    (backend : Provider_backend.t) (name : string) (pos : FileInfo.pos) : unit =
  add_type backend name pos Naming_types.TClass

let get_record_def_path (ctx : Provider_context.t) (name : string) :
    Relative_path.t option =
  match get_type_path_and_kind ctx name with
  | Some (fn, Naming_types.TRecordDef) -> Some fn
  | Some (_, (Naming_types.TClass | Naming_types.TTypedef))
  | None ->
    None

let add_record_def
    (backend : Provider_backend.t) (name : string) (pos : FileInfo.pos) : unit =
  add_type backend name pos Naming_types.TRecordDef

let get_typedef_path (ctx : Provider_context.t) (name : string) :
    Relative_path.t option =
  match get_type_path_and_kind ctx name with
  | Some (fn, Naming_types.TTypedef) -> Some fn
  | Some (_, (Naming_types.TClass | Naming_types.TRecordDef))
  | None ->
    None

let add_typedef
    (backend : Provider_backend.t) (name : string) (pos : FileInfo.pos) : unit =
  add_type backend name pos Naming_types.TTypedef

let update
    ~(backend : Provider_backend.t)
    ~(path : Relative_path.t)
    ~(old_file_info : FileInfo.t option)
    ~(new_file_info : FileInfo.t option) : unit =
  ignore path;
  let open FileInfo in
  let strip_positions symbols =
    List.fold symbols ~init:SSet.empty ~f:(fun acc (_, x) -> SSet.add acc x)
  in
  (* Remove old entries *)
  Option.iter old_file_info ~f:(fun old_file_info ->
      remove_type_batch backend (strip_positions old_file_info.classes);
      remove_type_batch backend (strip_positions old_file_info.typedefs);
      remove_type_batch backend (strip_positions old_file_info.record_defs);
      remove_fun_batch backend (strip_positions old_file_info.funs);
      remove_const_batch backend (strip_positions old_file_info.consts));
  (* Add new entries.
  TODO: this doesn't handle name collisions. For instance, if you create
  duplicate names and then delete one of them, it will fail to think the
  remaining one is present.
  NOTE: We don't use [Naming_global.ndecl_file_fast] here because it
  attempts to look up the symbol by doing a file parse, but the file may not
  exist on disk anymore. We also don't need to do the file parse in this
  case anyways, since we just did one and know for a fact where the symbol
  is. *)
  Option.iter new_file_info ~f:(fun new_file_info ->
      List.iter new_file_info.funs ~f:(fun (pos, name) ->
          add_fun backend name pos);
      List.iter new_file_info.classes ~f:(fun (pos, name) ->
          add_class backend name pos);
      List.iter new_file_info.record_defs ~f:(fun (pos, name) ->
          add_record_def backend name pos);
      List.iter new_file_info.typedefs ~f:(fun (pos, name) ->
          add_typedef backend name pos);
      List.iter new_file_info.consts ~f:(fun (pos, name) ->
          add_const backend name pos));
  ()

let local_changes_push_sharedmem_stack () : unit =
  Naming_heap.push_local_changes ()

let local_changes_pop_sharedmem_stack () : unit =
  Naming_heap.pop_local_changes ()

let with_quarantined_writes ~(f : unit -> 'a) : 'a =
  let old_writes_enabled = !writes_enabled in
  writes_enabled := false;
  Utils.try_finally ~f ~finally:(fun () -> writes_enabled := old_writes_enabled)
