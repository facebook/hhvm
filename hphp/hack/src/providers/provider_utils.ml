(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let invalidate_shallow_decls symbols local_memory =
  let FileInfo.{ n_classes; n_types; n_funs; n_consts; n_modules } = symbols in
  let {
    Provider_backend.decl_cache;
    shallow_decl_cache;
    folded_class_cache = _;
    _;
  } =
    local_memory
  in
  let open Provider_backend.Decl_cache_entry in
  SSet.iter
    (fun name ->
      Provider_backend.Decl_cache.remove decl_cache ~key:(Fun_decl name))
    n_funs;
  SSet.iter
    (fun name ->
      Provider_backend.Decl_cache.remove decl_cache ~key:(Gconst_decl name))
    n_consts;
  SSet.iter
    (fun name ->
      Provider_backend.Decl_cache.remove decl_cache ~key:(Typedef_decl name))
    n_types;
  SSet.iter
    (fun name ->
      Provider_backend.Decl_cache.remove decl_cache ~key:(Module_decl name))
    n_modules;
  let open Provider_backend.Shallow_decl_cache_entry in
  SSet.iter
    (fun name ->
      Provider_backend.Shallow_decl_cache.remove
        shallow_decl_cache
        ~key:(Shallow_class_decl name))
    n_classes;
  ()

let remove_folded_decl local_memory name =
  let open Provider_backend in
  Decl_cache.remove
    local_memory.decl_cache
    ~key:(Provider_backend.Decl_cache_entry.Class_decl name);
  Folded_class_cache.remove
    local_memory.folded_class_cache
    ~key:(Folded_class_cache_entry.Folded_class_decl name)

(** This is a leftover function, because we haven't yet migrated to
sticky decls and more correct invalidation. *)
let invalidate_shallow_and_some_folded_decls
    (local_memory : Provider_backend.local_memory) (ids : FileInfo.ids) : unit =
  invalidate_shallow_decls (FileInfo.ids_to_names ids) local_memory;
  let { FileInfo.classes; _ } = ids in
  List.iter classes ~f:(fun { FileInfo.name; _ } ->
      remove_folded_decl local_memory name);
  ()

(** This is a leftover function, because we haven't yet migrated to
sticky decls and more correct invalidation. *)
let invalidate_named_shallow_and_some_folded_decls_for_entry
    (local_memory : Provider_backend.local_memory)
    (entries : Provider_context.entries) : unit =
  Relative_path.Map.iter entries ~f:(fun _path entry ->
      match entry.Provider_context.parser_return with
      | None -> () (* hasn't been parsed, hence nothing to invalidate *)
      | Some { Parser_return.ast; _ } ->
        let ids = Nast.get_def_names ast in
        invalidate_shallow_and_some_folded_decls local_memory ids);
  ()

let combine_old_and_new_symbols (changes : FileInfo.change list) :
    FileInfo.names =
  (* Helper for merging [FileInfo.ids] list-of-ids into [FileInfo.names] set-of-names *)
  let merge_ids (acc : FileInfo.names) (change : FileInfo.ids) =
    let f set id = SSet.add id.FileInfo.name set in
    let open FileInfo in
    {
      n_funs = List.fold change.funs ~init:acc.n_funs ~f;
      n_classes = List.fold change.classes ~init:acc.n_classes ~f;
      n_types = List.fold change.typedefs ~init:acc.n_types ~f;
      n_consts = List.fold change.consts ~init:acc.n_consts ~f;
      n_modules = List.fold change.modules ~init:acc.n_modules ~f;
    }
  in
  List.fold
    changes
    ~init:FileInfo.empty_names
    ~f:(fun acc FileInfo.{ old_ids; new_ids; _ } ->
      let acc = Option.value_map old_ids ~default:acc ~f:(merge_ids acc) in
      let acc = Option.value_map new_ids ~default:acc ~f:(merge_ids acc) in
      acc)

let resolve_deps
    (dep_table : (Typing_deps.Dep.t, string) Stdlib.Hashtbl.t)
    (deps : Typing_deps.DepSet.t) : SSet.t =
  Typing_deps.DepSet.fold deps ~init:SSet.empty ~f:(fun dep set ->
      match Stdlib.Hashtbl.find_opt dep_table dep with
      | None -> set
      | Some name -> SSet.add name set)

(** Optimised folded decl invalidation using the dependency graph.
    This will flush the dependency edges before querying the graph. *)
let invalidate_folded_decls_flush_deps
    ctx
    ({ FileInfo.n_classes; _ } : FileInfo.names)
    ~(local_memory : Provider_backend.local_memory) =
  let deps_mode = Provider_context.get_deps_mode ctx in
  Typing_deps.flush_deps deps_mode;
  let classes_depset =
    SSet.fold
      (fun class_name set ->
        Typing_deps.DepSet.add
          set
          (Typing_deps.Dep.make (Typing_deps.Dep.Type class_name)))
      n_classes
      (Typing_deps.DepSet.make ())
  in
  let to_invalidate_depset =
    Typing_deps.add_extend_deps deps_mode classes_depset
  in
  let to_invalidate_class_names =
    resolve_deps local_memory.Provider_backend.dep_table to_invalidate_depset
  in
  SSet.iter (remove_folded_decl local_memory) to_invalidate_class_names;
  ()

let invalidate_upon_file_changes
    ~(ctx : Provider_context.t)
    ~(local_memory : Provider_backend.local_memory)
    ~(changes : FileInfo.change list)
    ~(entries : Provider_context.entries) : Telemetry.t =
  let start_time = Unix.gettimeofday () in

  let (kind, telemetry_opt, tasts_to_invalidate) =
    match
      (changes, !(local_memory.Provider_backend.decls_reflect_this_file))
    with
    | ([], _) -> ("no_changes", None, Relative_path.Map.empty)
    | ( [FileInfo.{ path; new_pfh_hash = Some pfh_hash; _ }],
        Some (path2, _, pfh_hash2) )
      when Relative_path.equal path path2 && Int64.equal pfh_hash pfh_hash2 ->
      let tasts_to_invalidate =
        Relative_path.Map.filter entries ~f:(fun path2 _entry ->
            not (Relative_path.equal path path2))
      in
      ("no_signature_changes", None, tasts_to_invalidate)
    | _ ->
      let symbols = combine_old_and_new_symbols changes in
      invalidate_shallow_decls symbols local_memory;
      invalidate_folded_decls_flush_deps ctx symbols ~local_memory;
      ("exhaustive", None, entries)
  in

  (* In addition to decls, we also have derived facts in the form of TASTs.
     We'll invalidate them all since we don't know which are affected.
     Only exception, calculated above, is that if [decls_reflect_this_file]
     matches one of the entries, then its TAST is already correct and
     needn't be invalidated. (This is the common case where you save
     a file that you'd previously been editing). *)
  Relative_path.Map.iter tasts_to_invalidate ~f:(fun _path entry ->
      entry.Provider_context.tast <- None;
      entry.Provider_context.all_diagnostics <- None);

  Option.value telemetry_opt ~default:(Telemetry.create ())
  |> Telemetry.duration ~key:"invalidate_upon_file_changes_ms" ~start_time
  |> Telemetry.string_ ~key:"kind" ~value:kind

let ctx_from_server_env (env : ServerEnv.env) : Provider_context.t =
  (* TODO: backend should be stored in [env]. *)
  Provider_context.empty_for_tool
    ~popt:env.ServerEnv.popt
    ~tcopt:env.ServerEnv.tcopt
    ~backend:(Provider_backend.get ())
    ~deps_mode:env.ServerEnv.deps_mode

let respect_but_quarantine_unsaved_changes
    ~(ctx : Provider_context.t) ~(f : unit -> 'a) : 'a =
  let backend_pushed = ref false in
  let quarantine_set = ref false in
  (* Explanation for the invariants+operation of this function are in the module-level
     comments in Provider_utils.mli. *)
  let enter_quarantine_exn () =
    begin
      match Provider_context.get_backend ctx with
      | Provider_backend.Shared_memory ->
        Ast_provider.local_changes_push_sharedmem_stack ();
        Decl_provider.local_changes_push_sharedmem_stack ();
        File_provider.local_changes_push_sharedmem_stack ();
        Fixme_provider.local_changes_push_sharedmem_stack ();
        Naming_provider.local_changes_push_sharedmem_stack ();
        SharedMem.set_allow_hashtable_writes_by_current_process false
      | Provider_backend.Rust_provider_backend backend ->
        Rust_provider_backend.push_local_changes backend;

        Ast_provider.local_changes_push_sharedmem_stack ();
        (* Shallow classes are stored in Rust when we're using
           Rust_provider_backend, but member filters are not, so we still need
           to push/pop the sharedmem stack for member filters. *)
        Decl_provider.local_changes_push_sharedmem_stack ();
        Fixme_provider.local_changes_push_sharedmem_stack ();
        SharedMem.set_allow_hashtable_writes_by_current_process false
      | Provider_backend.Local_memory local ->
        let start_time = Unix.gettimeofday () in
        let entries = Provider_context.get_entries ctx in
        if Relative_path.Map.cardinal entries <> 1 then
          HackEventLogger.invariant_violation_bug
            ~data_int:
              (Provider_context.get_entries ctx |> Relative_path.Map.cardinal)
            "Should only enter quarantine with exactly one entry";
        Relative_path.Map.iter entries ~f:(fun _path entry ->
            let (_ : Nast.program) =
              Ast_provider.compute_ast
                ~popt:(Provider_context.get_popt ctx)
                ~entry
            in
            ());
        let telemetry = Telemetry.create () in
        let telemetry =
          match !(local.Provider_backend.decls_reflect_this_file) with
          | Some _ -> telemetry
          | None ->
            let start_time = Unix.gettimeofday () in
            invalidate_named_shallow_and_some_folded_decls_for_entry
              local
              (Provider_context.get_entries ctx);
            telemetry
            |> Telemetry.duration ~key:"duration_nonsticky_ms" ~start_time
        in
        HackEventLogger.ProfileTypeCheck.quarantine
          ~count:(Relative_path.Map.cardinal entries)
          ~start_time
          ~path:(Relative_path.Map.choose_opt entries |> Option.map ~f:fst)
          telemetry;
        ()
      | _ -> ()
    end;
    backend_pushed := true;
    Ide_parser_cache.activate ();
    Diagnostics.set_allow_errors_in_default_path true;
    Provider_context.set_is_quarantined_internal ();
    quarantine_set := true;
    ()
  in
  let leave_quarantine_exn () =
    if !quarantine_set then Provider_context.unset_is_quarantined_internal ();
    Diagnostics.set_allow_errors_in_default_path false;
    Ide_parser_cache.deactivate ();
    if !backend_pushed then
      match Provider_context.get_backend ctx with
      | Provider_backend.Shared_memory ->
        Ast_provider.local_changes_pop_sharedmem_stack ();
        Decl_provider.local_changes_pop_sharedmem_stack ();
        File_provider.local_changes_pop_sharedmem_stack ();
        Fixme_provider.local_changes_pop_sharedmem_stack ();
        Naming_provider.local_changes_pop_sharedmem_stack ();
        SharedMem.set_allow_hashtable_writes_by_current_process true;
        SharedMem.invalidate_local_caches ()
      | Provider_backend.Rust_provider_backend backend ->
        Rust_provider_backend.pop_local_changes backend;

        Ast_provider.local_changes_pop_sharedmem_stack ();
        Decl_provider.local_changes_pop_sharedmem_stack ();
        Fixme_provider.local_changes_pop_sharedmem_stack ();
        SharedMem.set_allow_hashtable_writes_by_current_process true;
        SharedMem.invalidate_local_caches ()
      | Provider_backend.Local_memory local ->
        if Option.is_none !(local.Provider_backend.decls_reflect_this_file) then
          invalidate_named_shallow_and_some_folded_decls_for_entry
            local
            (Provider_context.get_entries ctx)
      | _ -> ()
  in
  let (_errors, result) =
    Diagnostics.do_ (fun () ->
        Utils.try_finally
          ~f:(fun () ->
            enter_quarantine_exn ();
            f ())
          ~finally:leave_quarantine_exn)
  in
  result
