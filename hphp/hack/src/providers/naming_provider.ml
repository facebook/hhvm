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

let attach_name_type (name_type : FileInfo.name_type) (x : 'a) :
    'a * FileInfo.name_type =
  (x, name_type)

let remove_name_type (x : 'a * FileInfo.name_type) : 'a = fst x

let find_symbol_in_context
    ~(ctx : Provider_context.t)
    ~(get_entry_symbols : FileInfo.t -> (FileInfo.id * FileInfo.name_type) list)
    ~(is_symbol : string -> bool) :
    (Relative_path.t * (FileInfo.pos * FileInfo.name_type)) option =
  Provider_context.get_entries ctx
  |> Relative_path.Map.filter_map ~f:(fun entry ->
         let file_info = Ast_provider.compute_file_info ctx entry in
         let symbols = get_entry_symbols file_info in
         List.find_map symbols ~f:(fun ((pos, name), kind) ->
             if is_symbol name then
               Some (pos, kind)
             else
               None))
  |> Relative_path.Map.choose_opt

let is_pos_in_ctx ~(ctx : Provider_context.t) (pos : FileInfo.pos) : bool =
  let path = FileInfo.get_pos_filename pos in
  Relative_path.Map.mem (Provider_context.get_entries ctx) path

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
      (* If fallback said it thought the symbol was in ctx, but we definitively
      know that it isn't, then the answer is None. *)
      if is_pos_in_ctx ~ctx pos then
        None
      else
        Some (pos, kind)
    | None -> None)

let get_and_cache
    ~(name : 'name)
    ~(make_pos_for_kind : 'sqlite_result -> 'pos)
    ~(get_delta_for_kind :
       unit ->
       'pos Provider_backend.Reverse_naming_table_delta.pos_or_deleted SMap.t)
    ~(set_delta_for_kind :
       'pos Provider_backend.Reverse_naming_table_delta.pos_or_deleted SMap.t ->
       unit)
    ~(get_from_sqlite : unit -> 'sqlite_result option) =
  let open Provider_backend.Reverse_naming_table_delta in
  let delta = get_delta_for_kind () in
  match SMap.find_opt delta name with
  | Some Deleted -> None
  | Some (Pos pos) -> Some pos
  | None when Naming_sqlite.is_connected () ->
    (match get_from_sqlite () with
    | None -> None
    | Some path ->
      let pos = make_pos_for_kind path in
      let new_delta = SMap.add delta ~key:name ~data:(Pos pos) in
      set_delta_for_kind new_delta;
      Some pos)
  | None -> None

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
        Naming_heap.Consts.get_pos name >>| attach_name_type FileInfo.Const
      | Provider_backend.Local_memory
          { Provider_backend.reverse_naming_table_delta; _ } ->
        let open Provider_backend.Reverse_naming_table_delta in
        get_and_cache
          ~name
          ~make_pos_for_kind:(fun path -> FileInfo.File (FileInfo.Const, path))
          ~get_delta_for_kind:(fun () -> reverse_naming_table_delta.consts)
          ~set_delta_for_kind:(fun consts ->
            reverse_naming_table_delta.consts <- consts)
          ~get_from_sqlite:(fun () -> Naming_sqlite.get_const_pos name)
        >>| attach_name_type FileInfo.Const
      | Provider_backend.Decl_service _ as backend -> not_implemented backend)
  >>| remove_name_type

let const_exists (ctx : Provider_context.t) (name : string) : bool =
  get_const_pos ctx name |> Option.is_some

let get_const_path (ctx : Provider_context.t) (name : string) :
    Relative_path.t option =
  get_const_pos ctx name |> Option.map ~f:FileInfo.get_pos_filename

let add_const
    (backend : Provider_backend.t) (name : string) (pos : FileInfo.pos) : unit =
  match backend with
  | Provider_backend.Shared_memory -> Naming_heap.Consts.add name pos
  | Provider_backend.Local_memory
      { Provider_backend.reverse_naming_table_delta; _ } ->
    let open Provider_backend.Reverse_naming_table_delta in
    let data = Pos pos in
    reverse_naming_table_delta.consts <-
      SMap.add reverse_naming_table_delta.consts ~key:name ~data;
    reverse_naming_table_delta.consts_canon_key <-
      SMap.add
        reverse_naming_table_delta.consts_canon_key
        ~key:(Naming_sqlite.to_canon_name_key name)
        ~data
  | Provider_backend.Decl_service _ as backend -> not_implemented backend

let remove_const_batch (backend : Provider_backend.t) (names : SSet.t) : unit =
  match backend with
  | Provider_backend.Shared_memory -> Naming_heap.Consts.remove_batch names
  | Provider_backend.Local_memory
      { Provider_backend.reverse_naming_table_delta; _ } ->
    let open Provider_backend.Reverse_naming_table_delta in
    reverse_naming_table_delta.consts <-
      SSet.fold
        names
        ~init:reverse_naming_table_delta.consts
        ~f:(fun name acc -> SMap.add acc ~key:name ~data:Deleted);
    reverse_naming_table_delta.consts_canon_key <-
      SSet.fold
        names
        ~init:reverse_naming_table_delta.consts_canon_key
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
        Naming_heap.Funs.get_pos name >>| attach_name_type FileInfo.Fun
      | Provider_backend.Local_memory
          { Provider_backend.reverse_naming_table_delta; _ } ->
        let open Provider_backend.Reverse_naming_table_delta in
        get_and_cache
          ~name
          ~make_pos_for_kind:(fun path -> FileInfo.File (FileInfo.Fun, path))
          ~get_delta_for_kind:(fun () -> reverse_naming_table_delta.funs)
          ~set_delta_for_kind:(fun funs ->
            reverse_naming_table_delta.funs <- funs)
          ~get_from_sqlite:(fun () ->
            Naming_sqlite.get_fun_pos ~case_insensitive:false name)
        >>| attach_name_type FileInfo.Fun
      | Provider_backend.Decl_service { decl; _ } ->
        Decl_service_client.rpc_get_fun decl name
        |> Option.map ~f:(fun fun_elt ->
               FileInfo.Full fun_elt.Typing_defs.fe_pos
               |> attach_name_type FileInfo.Fun))
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
        ~name:canon_name_key
        ~make_pos_for_kind:(fun path -> FileInfo.File (FileInfo.Fun, path))
        ~get_delta_for_kind:(fun () ->
          reverse_naming_table_delta.funs_canon_key)
        ~set_delta_for_kind:(fun funs_lower ->
          reverse_naming_table_delta.funs_canon_key <- funs_lower)
        ~get_from_sqlite:(fun () ->
          Naming_sqlite.get_fun_pos ~case_insensitive:true name)
      >>= fun pos ->
      (* If reverse_naming_table_delta thought the symbol was in ctx, but we definitively
      know that it isn't, then it isn't. *)
      if is_pos_in_ctx ~ctx pos then
        None
      else
        Some pos >>= fun pos ->
        compute_symbol_canon_name (FileInfo.get_pos_filename pos)
    | Provider_backend.Decl_service _ ->
      (* FIXME: Not correct! We need to add a canon-name API to the decl service. *)
      if fun_exists ctx name then
        Some name
      else
        None)

let add_fun (backend : Provider_backend.t) (name : string) (pos : FileInfo.pos)
    : unit =
  match backend with
  | Provider_backend.Shared_memory -> Naming_heap.Funs.add name pos
  | Provider_backend.Local_memory
      { Provider_backend.reverse_naming_table_delta; _ } ->
    let open Provider_backend.Reverse_naming_table_delta in
    let data = Pos pos in
    reverse_naming_table_delta.funs <-
      SMap.add reverse_naming_table_delta.funs ~key:name ~data;
    reverse_naming_table_delta.funs_canon_key <-
      SMap.add
        reverse_naming_table_delta.funs_canon_key
        ~key:(Naming_sqlite.to_canon_name_key name)
        ~data
  | Provider_backend.Decl_service _ ->
    (* Do nothing. All naming table updates are expected to have happened
       already--we should have sent a control request to the decl service asking
       it to update in response to the list of changed files. *)
    ()

let remove_fun_batch (backend : Provider_backend.t) (names : SSet.t) : unit =
  match backend with
  | Provider_backend.Shared_memory -> Naming_heap.Funs.remove_batch names
  | Provider_backend.Local_memory
      { Provider_backend.reverse_naming_table_delta; _ } ->
    let open Provider_backend.Reverse_naming_table_delta in
    reverse_naming_table_delta.funs <-
      SSet.fold names ~init:reverse_naming_table_delta.funs ~f:(fun name acc ->
          SMap.add acc ~key:name ~data:Deleted);
    reverse_naming_table_delta.funs_canon_key <-
      SSet.fold
        names
        ~init:reverse_naming_table_delta.funs_canon_key
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
  match backend with
  | Provider_backend.Shared_memory -> Naming_heap.Types.add name (pos, kind)
  | Provider_backend.Local_memory
      { Provider_backend.reverse_naming_table_delta; _ } ->
    let open Provider_backend.Reverse_naming_table_delta in
    let data = Pos (pos, kind) in
    reverse_naming_table_delta.types <-
      SMap.add reverse_naming_table_delta.types ~key:name ~data;
    reverse_naming_table_delta.types_canon_key <-
      SMap.add
        reverse_naming_table_delta.types_canon_key
        ~key:(Naming_sqlite.to_canon_name_key name)
        ~data
  | Provider_backend.Decl_service _ ->
    (* Do nothing. Naming table updates should be done already. *)
    ()

let remove_type_batch (backend : Provider_backend.t) (names : SSet.t) : unit =
  match backend with
  | Provider_backend.Shared_memory -> Naming_heap.Types.remove_batch names
  | Provider_backend.Local_memory
      { Provider_backend.reverse_naming_table_delta; _ } ->
    let open Provider_backend.Reverse_naming_table_delta in
    reverse_naming_table_delta.types <-
      SSet.fold names ~init:reverse_naming_table_delta.types ~f:(fun name acc ->
          SMap.add acc ~key:name ~data:Deleted);
    reverse_naming_table_delta.types_canon_key <-
      SSet.fold
        names
        ~init:reverse_naming_table_delta.types_canon_key
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
      match Provider_context.get_backend ctx with
      | Provider_backend.Shared_memory ->
        Naming_heap.Types.get_pos name >>| fun (pos, kind) ->
        (pos, kind_to_name_type kind)
      | Provider_backend.Local_memory
          { Provider_backend.reverse_naming_table_delta; _ } ->
        let open Provider_backend.Reverse_naming_table_delta in
        get_and_cache
          ~name
          ~make_pos_for_kind:(fun (path, kind) ->
            let name_type = kind_to_name_type kind in
            let pos = FileInfo.File (name_type, path) in
            (pos, kind))
          ~get_delta_for_kind:(fun () -> reverse_naming_table_delta.types)
          ~set_delta_for_kind:(fun types ->
            reverse_naming_table_delta.types <- types)
          ~get_from_sqlite:(fun () ->
            Naming_sqlite.get_type_pos ~case_insensitive:false name)
        >>| fun (pos, kind) -> (pos, kind_to_name_type kind)
      | Provider_backend.Decl_service { decl; _ } ->
        (* TODO: We probably want to provide a decl service API for this. *)
        (match Decl_service_client.rpc_get_class decl name with
        | Some sc ->
          Some (FileInfo.Full (fst sc.Shallow_decl_defs.sc_name), FileInfo.Class)
        | None ->
          (match Decl_service_client.rpc_get_typedef decl name with
          | Some td ->
            Some (FileInfo.Full td.Typing_defs.td_pos, FileInfo.Typedef)
          | None ->
            (match Decl_service_client.rpc_get_record_def decl name with
            | Some rdt ->
              Some (FileInfo.Full rdt.Typing_defs.rdt_pos, FileInfo.RecordDef)
            | None -> None))))
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
        ~name:canon_name_key
        ~make_pos_for_kind:(fun (path, kind) ->
          let name_type =
            match kind with
            | Naming_types.TClass -> FileInfo.Class
            | Naming_types.TRecordDef -> FileInfo.RecordDef
            | Naming_types.TTypedef -> FileInfo.Typedef
          in
          let file_info = FileInfo.File (name_type, path) in
          (file_info, kind))
        ~get_delta_for_kind:(fun () ->
          reverse_naming_table_delta.types_canon_key)
        ~set_delta_for_kind:(fun types ->
          reverse_naming_table_delta.types_canon_key <- types)
        ~get_from_sqlite:(fun () ->
          Naming_sqlite.get_type_pos ~case_insensitive:true name)
      >>= fun (pos, kind) ->
      (* If reverse_naming_table_delta thought the symbol was in ctx, but we definitively
      know that it isn't, then it isn't. *)
      if is_pos_in_ctx ~ctx pos then
        None
      else
        Some (pos, kind) >>= fun (pos, kind) ->
        compute_symbol_canon_name (FileInfo.get_pos_filename pos) kind
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

let local_changes_push_sharedmem_stack () : unit =
  Naming_heap.push_local_changes ()

let local_changes_pop_sharedmem_stack () : unit =
  Naming_heap.pop_local_changes ()
