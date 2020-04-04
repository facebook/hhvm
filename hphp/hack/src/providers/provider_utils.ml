(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel

let invalidate_fun decl_cache fun_name =
  Provider_backend.Decl_cache.remove
    decl_cache
    (Provider_backend.Decl_cache_entry.Fun_decl fun_name)

let invalidate_class decl_cache class_name =
  Provider_backend.Decl_cache.remove
    decl_cache
    (Provider_backend.Decl_cache_entry.Class_decl class_name)

let invalidate_shallow_class shallow_decl_cache class_name =
  Provider_backend.Shallow_decl_cache.remove
    shallow_decl_cache
    (Provider_backend.Shallow_decl_cache_entry.Shallow_class_decl class_name)

let invalidate_record_def decl_cache record_name =
  Provider_backend.Decl_cache.remove
    decl_cache
    (Provider_backend.Decl_cache_entry.Record_decl record_name)

let invalidate_typedef decl_cache typedef_name =
  Provider_backend.Decl_cache.remove
    decl_cache
    (Provider_backend.Decl_cache_entry.Typedef_decl typedef_name)

let invalidate_gconst decl_cache gconst_name =
  Provider_backend.Decl_cache.remove
    decl_cache
    (Provider_backend.Decl_cache_entry.Gconst_decl gconst_name)

let invalidate_tast_cache_of_entries (entries : Provider_context.entries) : unit
    =
  Relative_path.Map.iter entries ~f:(fun _path entry ->
      entry.Provider_context.tast <- None;
      entry.Provider_context.tast_errors <- None)

let invalidate_local_decl_caches_for_file
    (local_memory : Provider_backend.local_memory) (file_info : FileInfo.t) :
    unit =
  let open FileInfo in
  let decl_cache = local_memory.Provider_backend.decl_cache in
  let shallow_decl_cache = local_memory.Provider_backend.shallow_decl_cache in
  let iter_names ids ~f = List.iter ids ~f:(fun (_, name) -> f name) in
  iter_names file_info.funs ~f:(invalidate_fun decl_cache);
  iter_names file_info.classes ~f:(invalidate_class decl_cache);
  iter_names file_info.classes ~f:(invalidate_shallow_class shallow_decl_cache);
  iter_names file_info.record_defs ~f:(invalidate_record_def decl_cache);
  iter_names file_info.typedefs ~f:(invalidate_typedef decl_cache);
  iter_names file_info.consts ~f:(invalidate_gconst decl_cache);
  ()

let invalidate_local_decl_caches_for_entries
    (local_memory : Provider_backend.local_memory)
    (entries : Provider_context.entries) : unit =
  let invalidate_for_entry _path entry =
    match entry.Provider_context.parser_return with
    | None -> () (* hasn't been parsed, hence nothing to invalidate *)
    | Some { Parser_return.ast; _ } ->
      let (funs, classes, record_defs, typedefs, consts) = Nast.get_defs ast in
      invalidate_local_decl_caches_for_file
        local_memory
        {
          FileInfo.funs;
          classes;
          record_defs;
          typedefs;
          consts;
          hash = None;
          comments = None;
          file_mode = None;
        }
  in
  Relative_path.Map.iter entries ~f:invalidate_for_entry

let ctx_from_server_env (env : ServerEnv.env) : Provider_context.t =
  (* TODO: backend should be stored in [env]. *)
  Provider_context.empty_for_tool
    ~popt:env.ServerEnv.popt
    ~tcopt:env.ServerEnv.tcopt
    ~backend:(Provider_backend.get ())

let respect_but_quarantine_unsaved_changes
    ~(ctx : Provider_context.t) ~(f : unit -> 'a) : 'a =
  let backend_pushed = ref false in
  let quarantine_set = ref false in
  (* This function will (1) enter quarantine, (2) do the callback "f",
  (3) leave quarantine. If an exception arises during step (1,2) then nevertheless
  we guarantee that quarantine is safely left. If an exception arises during
  step (3) then we'll raise an exception but the program state has become unstable... *)
  let enter_quarantine_exn () =
    begin
      match Provider_context.get_backend ctx with
      | Provider_backend.Shared_memory ->
        Ast_provider.local_changes_push_sharedmem_stack ();
        Decl_provider.local_changes_push_sharedmem_stack ();
        Shallow_classes_provider.local_changes_push_sharedmem_stack ();
        Linearization_provider.local_changes_push_sharedmem_stack ();
        File_provider.local_changes_push_sharedmem_stack ();
        Fixme_provider.local_changes_push_sharedmem_stack ();
        Naming_provider.local_changes_push_sharedmem_stack ();
        SharedMem.allow_hashtable_writes_by_current_process false
      | Provider_backend.Local_memory local ->
        Relative_path.Map.iter
          (Provider_context.get_entries ctx)
          ~f:(fun _path entry ->
            let (_ : Nast.program) = Ast_provider.compute_ast ~ctx ~entry in
            ());
        invalidate_local_decl_caches_for_entries
          local
          (Provider_context.get_entries ctx)
      | _ -> ()
    end;
    backend_pushed := true;
    Ide_parser_cache.activate ();
    Errors.set_allow_errors_in_default_path true;
    Provider_context.set_is_quarantined_internal ();
    quarantine_set := true;
    ()
  in
  let leave_quarantine_exn () =
    if !quarantine_set then Provider_context.unset_is_quarantined_internal ();
    Errors.set_allow_errors_in_default_path false;
    Ide_parser_cache.deactivate ();
    if !backend_pushed then
      match Provider_context.get_backend ctx with
      | Provider_backend.Shared_memory ->
        Ast_provider.local_changes_pop_sharedmem_stack ();
        Decl_provider.local_changes_pop_sharedmem_stack ();
        Shallow_classes_provider.local_changes_pop_sharedmem_stack ();
        Linearization_provider.local_changes_pop_sharedmem_stack ();
        File_provider.local_changes_pop_sharedmem_stack ();
        Fixme_provider.local_changes_pop_sharedmem_stack ();
        Naming_provider.local_changes_pop_sharedmem_stack ();
        SharedMem.allow_hashtable_writes_by_current_process true;
        SharedMem.invalidate_caches ()
      | Provider_backend.Local_memory local ->
        invalidate_local_decl_caches_for_entries
          local
          (Provider_context.get_entries ctx)
      | _ -> ()
  in
  let (_errors, result) =
    Errors.do_ (fun () ->
        Utils.try_finally
          ~f:(fun () ->
            enter_quarantine_exn ();
            f ())
          ~finally:leave_quarantine_exn)
  in
  result
