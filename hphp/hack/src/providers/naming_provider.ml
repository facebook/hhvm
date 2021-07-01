(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Reordered_argument_collections

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
  | Some (Pos ((name_type, path), _rest)) -> Some (name_type, path)
  | None ->
    (match Option.bind (db_path_of_ctx ctx) ~f:fallback with
    | None -> None
    | Some (name_type, path) ->
      cache := SMap.add !cache ~key:name ~data:(Pos ((name_type, path), []));
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
      | Provider_backend.Analysis
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
      | Provider_backend.Decl_service { decl; _ } ->
        Decl_service_client.rpc_get_gconst_path decl name
        |> Option.map ~f:(fun path -> FileInfo.(File (Const, path), Const)))
  >>| remove_name_type

let const_exists (ctx : Provider_context.t) (name : string) : bool =
  get_const_pos ctx name |> Option.is_some

let get_const_path (ctx : Provider_context.t) (name : string) :
    Relative_path.t option =
  get_const_pos ctx name |> Option.map ~f:FileInfo.get_pos_filename

let add_const
    (backend : Provider_backend.t) (name : string) (pos : FileInfo.pos) : unit =
  match backend with
  | Provider_backend.Analysis -> failwith "invalid"
  | Provider_backend.Shared_memory -> Naming_heap.Consts.add name pos
  | Provider_backend.Local_memory
      { Provider_backend.reverse_naming_table_delta; _ } ->
    let open Provider_backend.Reverse_naming_table_delta in
    let data = Pos ((FileInfo.Const, FileInfo.get_pos_filename pos), []) in
    reverse_naming_table_delta.consts :=
      SMap.add !(reverse_naming_table_delta.consts) ~key:name ~data
  | Provider_backend.Decl_service _ as backend -> not_implemented backend

let remove_const_batch (backend : Provider_backend.t) (names : string list) :
    unit =
  match backend with
  | Provider_backend.Analysis -> failwith "invalid"
  | Provider_backend.Shared_memory ->
    Naming_heap.Consts.remove_batch
      (Db_path_provider.get_naming_db_path backend)
      names
  | Provider_backend.Local_memory
      { Provider_backend.reverse_naming_table_delta; _ } ->
    let open Provider_backend.Reverse_naming_table_delta in
    reverse_naming_table_delta.consts :=
      List.fold
        names
        ~init:!(reverse_naming_table_delta.consts)
        ~f:(fun acc name -> SMap.add acc ~key:name ~data:Deleted)
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
      | Provider_backend.Analysis
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
            Naming_sqlite.get_fun_pos db_path name
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
  let name = String.lowercase name in
  let symbol_opt =
    find_symbol_in_context
      ~ctx
      ~get_entry_symbols:(fun { FileInfo.funs; _ } ->
        List.map funs ~f:(attach_name_type FileInfo.Fun))
      ~is_symbol:(fun symbol_name ->
        String.equal (Naming_sqlite.to_canon_name_key symbol_name) name)
  in
  let canon_name fd = snd fd.Aast.fd_fun.Aast.f_name in
  let compute_symbol_canon_name path =
    Ast_provider.find_ifun_in_file ctx path name >>| canon_name
  in

  match symbol_opt with
  | Some (path, _pos) -> compute_symbol_canon_name path
  | None ->
    (match Provider_context.get_backend ctx with
    | Provider_backend.Analysis
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
        ~name
        ~cache:reverse_naming_table_delta.funs_canon_key
        ~fallback:(fun db_path ->
          Naming_sqlite.get_ifun_pos db_path name
          |> Option.map ~f:(fun path -> (FileInfo.Fun, path)))
      >>= fun (_name_type, path) ->
      (* If reverse_naming_table_delta thought the symbol was in ctx, but we definitively
      know that it isn't, then it isn't. *)
      if is_path_in_ctx ~ctx path then
        None
      else
        compute_symbol_canon_name path
    | Provider_backend.Decl_service { decl; _ } ->
      Decl_service_client.rpc_get_fun_canon_name decl name)

let add_fun (backend : Provider_backend.t) (name : string) (pos : FileInfo.pos)
    : unit =
  match backend with
  | Provider_backend.Analysis -> failwith "invalid"
  | Provider_backend.Shared_memory -> Naming_heap.Funs.add name pos
  | Provider_backend.Local_memory
      { Provider_backend.reverse_naming_table_delta; _ } ->
    let open Provider_backend.Reverse_naming_table_delta in
    let data = Pos ((FileInfo.Fun, FileInfo.get_pos_filename pos), []) in
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

let remove_fun_batch (backend : Provider_backend.t) (names : string list) : unit
    =
  match backend with
  | Provider_backend.Analysis
  | Provider_backend.Shared_memory ->
    Naming_heap.Funs.remove_batch
      (Db_path_provider.get_naming_db_path backend)
      names
  | Provider_backend.Local_memory
      { Provider_backend.reverse_naming_table_delta; _ } ->
    let open Provider_backend.Reverse_naming_table_delta in
    reverse_naming_table_delta.funs :=
      List.fold
        names
        ~init:!(reverse_naming_table_delta.funs)
        ~f:(fun acc name -> SMap.add acc ~key:name ~data:Deleted);
    reverse_naming_table_delta.funs_canon_key :=
      List.fold
        names
        ~init:!(reverse_naming_table_delta.funs_canon_key)
        ~f:(fun acc name ->
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
  | Provider_backend.Analysis -> failwith "invalid"
  | Provider_backend.Shared_memory -> Naming_heap.Types.add name (pos, kind)
  | Provider_backend.Local_memory
      { Provider_backend.reverse_naming_table_delta; _ } ->
    let open Provider_backend.Reverse_naming_table_delta in
    let data =
      Pos ((kind_to_name_type kind, FileInfo.get_pos_filename pos), [])
    in
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

let remove_type_batch (backend : Provider_backend.t) (names : string list) :
    unit =
  match backend with
  | Provider_backend.Analysis -> failwith "invalid"
  | Provider_backend.Shared_memory ->
    Naming_heap.Types.remove_batch
      (Db_path_provider.get_naming_db_path backend)
      names
  | Provider_backend.Local_memory
      { Provider_backend.reverse_naming_table_delta; _ } ->
    let open Provider_backend.Reverse_naming_table_delta in
    reverse_naming_table_delta.types :=
      List.fold
        names
        ~init:!(reverse_naming_table_delta.types)
        ~f:(fun acc name -> SMap.add acc ~key:name ~data:Deleted);
    reverse_naming_table_delta.types_canon_key :=
      List.fold
        names
        ~init:!(reverse_naming_table_delta.types_canon_key)
        ~f:(fun acc name ->
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
      | Provider_backend.Analysis
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
            Naming_sqlite.get_type_pos db_path name
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
  let name = String.lowercase name in
  let symbol_opt =
    find_symbol_in_context
      ~ctx
      ~get_entry_symbols:get_entry_symbols_for_type
      ~is_symbol:(fun symbol_name ->
        String.equal (Naming_sqlite.to_canon_name_key symbol_name) name)
  in
  let compute_symbol_canon_name path kind =
    match kind with
    | Naming_types.TClass ->
      Ast_provider.find_iclass_in_file ctx path name
      >>| fun { Aast.c_name = (_, canon_name); _ } -> canon_name
    | Naming_types.TTypedef ->
      Ast_provider.find_itypedef_in_file ctx path name
      >>| fun { Aast.t_name = (_, canon_name); _ } -> canon_name
    | Naming_types.TRecordDef ->
      Ast_provider.find_irecord_def_in_file ctx path name
      >>| fun { Aast.rd_name = (_, canon_name); _ } -> canon_name
  in

  match symbol_opt with
  | Some (path, (_pos, name_type)) ->
    compute_symbol_canon_name path (name_type_to_kind name_type)
  | None ->
    (match Provider_context.get_backend ctx with
    | Provider_backend.Analysis
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
        ~name
        ~cache:reverse_naming_table_delta.types_canon_key
        ~fallback:(fun db_path ->
          Naming_sqlite.get_itype_pos db_path name
          |> Option.map ~f:(fun (path, kind) -> (kind_to_name_type kind, path)))
      >>= fun (name_type, path) ->
      (* If reverse_naming_table_delta thought the symbol was in ctx, but we definitively
      know that it isn't, then it isn't. *)
      if is_path_in_ctx ~ctx path then
        None
      else
        compute_symbol_canon_name path (name_type_to_kind name_type)
    | Provider_backend.Decl_service { decl; _ } ->
      Decl_service_client.rpc_get_type_canon_name decl name)

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

let resolve_position : Provider_context.t -> Pos_or_decl.t -> Pos.t =
 fun ctx pos ->
  match Pos_or_decl.get_raw_pos_or_decl_reference pos with
  | `Raw pos -> pos
  | `Decl_ref decl ->
    let filename =
      (match decl with
      | Decl_reference.Function name -> get_fun_path ctx name
      | Decl_reference.Type name -> get_type_path ctx name
      | Decl_reference.GlobalConstant name -> get_const_path ctx name)
      |> Option.value ~default:Relative_path.default
      (* TODO: what to do if decl not found? *)
    in
    Pos_or_decl.fill_in_filename filename pos

let get_const_full_pos ctx (pos, name) =
  match pos with
  | FileInfo.Full p -> Some p
  | FileInfo.File (FileInfo.Const, fn) ->
    begin
      (* This should really be part of the decl_provider, but it's tangled in circular dependencies *)
      match Provider_context.get_backend ctx with
      | Provider_backend.Decl_service { decl = decl_service; _ } ->
        Decl_service_client.rpc_get_gconst decl_service name
        |> Option.map ~f:(fun decl ->
               decl.Typing_defs.cd_pos |> resolve_position ctx)
      | _ ->
        Ast_provider.find_gconst_in_file ctx fn name
        |> Option.map ~f:(fun ast -> fst ast.Aast.cst_name)
    end
  | _ -> None

let get_fun_full_pos ctx (pos, name) =
  match pos with
  | FileInfo.Full p -> Some p
  | FileInfo.File (FileInfo.Fun, fn) ->
    begin
      (* This should really be part of the decl_provider, but it's tangled in circular dependencies *)
      match Provider_context.get_backend ctx with
      | Provider_backend.Decl_service { decl = decl_service; _ } ->
        Decl_service_client.rpc_get_fun decl_service name
        |> Option.map ~f:(fun decl ->
               decl.Typing_defs.fe_pos |> resolve_position ctx)
      | _ ->
        Ast_provider.find_fun_in_file ctx fn name
        |> Option.map ~f:(fun fd -> fst fd.Aast.fd_fun.Aast.f_name)
    end
  | _ -> None

let get_type_full_pos ctx (pos, name) =
  match pos with
  | FileInfo.Full p -> Some p
  | FileInfo.File (name_type, fn) ->
    (* This should really be part of the decl_provider, but it's tangled in circular dependencies *)
    (match Provider_context.get_backend ctx with
    | Provider_backend.Decl_service { decl = decl_service; _ } ->
      begin
        match name_type with
        | FileInfo.Class ->
          Decl_service_client.rpc_get_class decl_service name
          |> Option.map ~f:(fun decl -> fst decl.Shallow_decl_defs.sc_name)
        | FileInfo.Typedef ->
          Decl_service_client.rpc_get_typedef decl_service name
          |> Option.map ~f:(fun decl -> decl.Typing_defs.td_pos)
        | FileInfo.RecordDef ->
          Decl_service_client.rpc_get_record_def decl_service name
          |> Option.map ~f:(fun decl -> decl.Typing_defs.rdt_pos)
        | _ -> None
      end
      |> Option.map ~f:(resolve_position ctx)
    | _ ->
      (match name_type with
      | FileInfo.Class ->
        Ast_provider.find_class_in_file ctx fn name
        |> Option.map ~f:(fun ast -> fst ast.Aast.c_name)
      | FileInfo.Typedef ->
        Ast_provider.find_typedef_in_file ctx fn name
        |> Option.map ~f:(fun ast -> fst ast.Aast.t_name)
      | FileInfo.RecordDef ->
        Ast_provider.find_record_def_in_file ctx fn name
        |> Option.map ~f:(fun ast -> fst ast.Aast.rd_name)
      | _ -> None))

(** This removes the name->path mapping from the naming table (i.e. the combination
of sqlite and delta). It is an error to call this method unless name->path exists.
We enforce this with exceptions in some cases where it's cheap enough to verify,
but not in others where enforcing it would involve a sqlite read.
Invariant: this never transitions an entry from Some to None. *)
let remove
    ~(case_insensitive : bool)
    (delta : Provider_backend.Reverse_naming_table_delta.pos_or_deleted SMap.t)
    (path : Relative_path.t)
    (name : string) :
    Provider_backend.Reverse_naming_table_delta.pos_or_deleted SMap.t =
  let open Provider_backend.Reverse_naming_table_delta in
  let name =
    if case_insensitive then
      Naming_sqlite.to_canon_name_key name
    else
      name
  in
  match SMap.find_opt delta name with
  | None ->
    (* We've never yet read/cached from sqlite. Presumably the caller is removing
    the name->path mapping that we assume is present in sqlite. We could read
    from sqlite right now solely to verify that the user-supplied path matches
    the one that's in sqlite, but that'd be costly and doesn't seem worth it. *)
    SMap.add delta ~key:name ~data:Deleted
  | Some Deleted -> failwith "removing symbol that's already removed"
  | Some (Pos ((_name_type, old_path), [])) ->
    if not (Relative_path.equal path old_path) then
      failwith
        (Printf.sprintf
           "Naming_provider invariant failed: symbol %s was in %s, but we're trying to remove %s"
           name
           (Relative_path.to_absolute old_path)
           (Relative_path.to_absolute path));
    SMap.add delta ~key:name ~data:Deleted
  | Some (Pos ((_name_type, old_path), rest_hd :: rest_tl)) ->
    if Relative_path.equal path old_path then
      SMap.add delta ~key:name ~data:(Pos (rest_hd, rest_tl))
    else
      let rest =
        List.filter (rest_hd :: rest_tl) ~f:(fun (_name_type, rest_path) ->
            not (Relative_path.equal path rest_path))
      in
      if List.length rest <> List.length rest_tl then
        failwith
          (Printf.sprintf
             "Naming_provider invariant failed: symbol %s was in several files, but we're trying to remove %s which isn't one of them"
             name
             (Relative_path.to_absolute path));
      SMap.add delta ~key:name ~data:(Pos ((_name_type, old_path), rest))

(** This adds name->path to the naming table (i.e. the combination of sqlite+delta).
Invariant: if this function causes the delta for this symbol to go from None->Some,
then the result will include the name->path mapping that was present in sqlite (if any),
in addition to the name->path mapping that we wish to add right now. *)
let add
    ~(case_insensitive : bool)
    (db_path : Naming_sqlite.db_path option)
    (delta : Provider_backend.Reverse_naming_table_delta.pos_or_deleted SMap.t)
    (pos : Provider_backend.Reverse_naming_table_delta.pos)
    (name : string) :
    Provider_backend.Reverse_naming_table_delta.pos_or_deleted SMap.t =
  let open Provider_backend.Reverse_naming_table_delta in
  let name =
    if case_insensitive then
      Naming_sqlite.to_canon_name_key name
    else
      name
  in
  match (SMap.find_opt delta name, db_path) with
  | (None, None) -> SMap.add delta ~key:name ~data:(Pos (pos, []))
  | (None, Some db_path) ->
    let (name_type, _) = pos in
    let sqlite_pos =
      match name_type with
      | FileInfo.Const ->
        Option.map
          (Naming_sqlite.get_const_pos db_path name)
          ~f:(fun sqlite_path -> (FileInfo.Const, sqlite_path))
      | FileInfo.Fun ->
        let pos =
          if case_insensitive then
            Naming_sqlite.get_ifun_pos db_path name
          else
            Naming_sqlite.get_fun_pos db_path name
        in
        Option.map pos ~f:(fun sqlite_path -> (FileInfo.Fun, sqlite_path))
      | FileInfo.RecordDef
      | FileInfo.Class
      | FileInfo.Typedef ->
        let pos =
          if case_insensitive then
            Naming_sqlite.get_itype_pos db_path name
          else
            Naming_sqlite.get_type_pos db_path name
        in
        Option.map pos ~f:(fun (sqlite_path, sqlite_kind) ->
            (kind_to_name_type sqlite_kind, sqlite_path))
    in
    let data =
      match sqlite_pos with
      | None -> Pos (pos, [])
      | Some sqlite_pos -> Pos (sqlite_pos, [pos])
    in
    SMap.add delta ~key:name ~data
  | (Some Deleted, _) -> SMap.add delta ~key:name ~data:(Pos (pos, []))
  | (Some (Pos (old_pos, rest)), _) ->
    SMap.add delta ~key:name ~data:(Pos (old_pos, pos :: rest))

let update
    ~(backend : Provider_backend.t)
    ~(path : Relative_path.t)
    ~(old_file_info : FileInfo.t option)
    ~(new_file_info : FileInfo.t option) : unit =
  let open FileInfo in
  let strip_positions symbols = List.map symbols ~f:snd in
  match backend with
  | Provider_backend.Decl_service _ -> not_implemented backend
  | Provider_backend.Analysis -> failwith "invalid"
  | Provider_backend.Shared_memory ->
    (* Remove old entries *)
    Option.iter old_file_info ~f:(fun old_file_info ->
        remove_type_batch backend (strip_positions old_file_info.classes);
        remove_type_batch backend (strip_positions old_file_info.typedefs);
        remove_type_batch backend (strip_positions old_file_info.record_defs);
        remove_fun_batch backend (strip_positions old_file_info.funs);
        remove_const_batch backend (strip_positions old_file_info.consts));
    (* Add new entries. Note: the caller is expected to have a solution
    for duplicate names. Note: can't use [Naming_global.ndecl_file_skip_if_already_bound]
    because it attempts to look up the symbol by doing a file parse, but
    we have to use the file_info we're given to avoid races. *)
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
  | Provider_backend.Local_memory
      {
        Provider_backend.reverse_naming_table_delta = deltas;
        naming_db_path_ref;
        _;
      } ->
    let open Provider_backend.Reverse_naming_table_delta in
    (* helper*)
    let update ?(case_insensitive = false) olds news delta name_type =
      (* The following code has a bug.
      Given "olds/news", it calculates "added/removed" based on case-sensitive comparison.
      That's straightforwardly correct for our case-sensitives maps.
      But how does it work for our case-insensitive maps? e.g. olds={Aa,aA}, news={aa}.
      Therefore added={aa}, removed={Aa,aA} because we calculated these case-sensitively.
      (1) it removes the lowercase version of "Aa"
      (2) it removes the lowercase version of "aA"  <-- failwith
      (3) it adds the lowercase version of "aa"
      Correctness requires that removal is idempotent, and that we do adds
      after removes. Unfortunately removal currently isn't idempotent;
      if fails if you try to remove the same thing twice. *)
      let olds = strip_positions olds in
      let news = strip_positions news in
      let olds_s = SSet.of_list olds in
      let news_s = SSet.of_list news in
      let removed = SSet.diff olds_s news_s in
      let added = SSet.diff news_s olds_s in
      SSet.iter removed ~f:(fun name ->
          delta := remove ~case_insensitive !delta path name);
      SSet.iter added ~f:(fun name ->
          delta :=
            add
              !naming_db_path_ref
              ~case_insensitive
              !delta
              (name_type, path)
              name);
      ()
    in
    (* do the update *)
    let oldfi = Option.value old_file_info ~default:FileInfo.empty_t in
    let newfi = Option.value new_file_info ~default:FileInfo.empty_t in
    update oldfi.funs newfi.funs deltas.funs FileInfo.Fun;
    update oldfi.consts newfi.consts deltas.consts FileInfo.Const;
    update oldfi.classes newfi.classes deltas.types FileInfo.Class;
    update oldfi.typedefs newfi.typedefs deltas.types FileInfo.Typedef;
    update oldfi.record_defs newfi.record_defs deltas.types FileInfo.RecordDef;
    (* update canon names too *)
    let updatei = update ~case_insensitive:true in
    updatei oldfi.funs newfi.funs deltas.funs_canon_key FileInfo.Fun;
    updatei oldfi.classes newfi.classes deltas.types_canon_key FileInfo.Class;
    updatei
      oldfi.typedefs
      newfi.typedefs
      deltas.types_canon_key
      FileInfo.Typedef;
    updatei
      oldfi.record_defs
      newfi.record_defs
      deltas.types_canon_key
      FileInfo.RecordDef;
    ()

let local_changes_push_sharedmem_stack () : unit =
  Naming_heap.push_local_changes ()

let local_changes_pop_sharedmem_stack () : unit =
  Naming_heap.pop_local_changes ()
