(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let invalidate_tast_cache_of_entry (entry : Provider_context.entry) : unit =
  entry.Provider_context.tast <- None;
  entry.Provider_context.all_errors <- None;
  ()

let invalidate_local_decl_caches_for_file
    (local_memory : Provider_backend.local_memory) (file_info : FileInfo.t) :
    unit =
  let {
    Provider_backend.shallow_decl_cache;
    folded_class_cache;
    decl_cache;
    reverse_naming_table_delta = _;
    fixmes = _;
    naming_db_path_ref = _;
  } =
    local_memory
  in
  let open FileInfo in
  let open Provider_backend in
  (* Consideration: would it have been better to decl-diff, detect
     when the shallow-decls are unchanged if so then avoid invalidating all
     the folded-decls and TASTs? Maybe. That would be better in the case of
     one file change, but worse in the case of 5000 file changes. *)

  (* Shallow decl cache: we only need clear the ones affected *)
  List.iter file_info.classes ~f:(fun (_, name, _) ->
      Shallow_decl_cache.remove
        shallow_decl_cache
        ~key:(Shallow_decl_cache_entry.Shallow_class_decl name));

  (* Decl cache: we don't track fine-grained dependencies, and therefore we
     should be evicting everything.

     It might be possible to do decl-diffing on shallow-decls and if they're
     unchanged, then avoid invalidating the folded decls. That would be better
     in the case of just one disk file change notification, but worse in the
     case of 5000 since it'd require getting shallow-decls on all of them just
     to compare, even if they weren't actually needed.

     I tried evicting everything but it was far too slow. That will need to be
     fixed. But for now, let's settle for evicting decls which we know are
     affected. This way at least the user has a fallback of opening relevant
     files in the IDE to get their relevant decls correct. *)
  let open Provider_backend.Decl_cache_entry in
  List.iter file_info.consts ~f:(fun (_, name, _) ->
      Decl_cache.remove decl_cache ~key:(Gconst_decl name));
  List.iter file_info.funs ~f:(fun (_, name, _) ->
      Decl_cache.remove decl_cache ~key:(Fun_decl name));
  List.iter file_info.typedefs ~f:(fun (_, name, _) ->
      Decl_cache.remove decl_cache ~key:(Typedef_decl name));
  List.iter file_info.classes ~f:(fun (_, name, _) ->
      Decl_cache.remove decl_cache ~key:(Class_decl name);
      Folded_class_cache.remove
        folded_class_cache
        ~key:(Folded_class_cache_entry.Folded_class_decl name));
  ()

let invalidate_local_decl_caches_for_entries
    (local_memory : Provider_backend.local_memory)
    (entries : Provider_context.entries) : unit =
  let invalidate_for_entry _path entry =
    match entry.Provider_context.parser_return with
    | None -> () (* hasn't been parsed, hence nothing to invalidate *)
    | Some { Parser_return.ast; _ } ->
      let file_info = Nast.get_def_names ast in
      invalidate_local_decl_caches_for_file local_memory file_info
  in
  Relative_path.Map.iter entries ~f:invalidate_for_entry

let invalidate_upon_change
    ~(ctx : Provider_context.t)
    ~(local_memory : Provider_backend.local_memory)
    ~(changes : FileInfo.change list)
    ~(entries : Provider_context.entries) : unit =
  ignore ctx;
  List.iter changes ~f:(fun { FileInfo.old_file_info; _ } ->
      Option.iter
        old_file_info
        ~f:(invalidate_local_decl_caches_for_file local_memory));
  Relative_path.Map.iter entries ~f:(fun _path entry ->
      invalidate_tast_cache_of_entry entry);
  ()

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
        Shallow_classes_provider.local_changes_push_sharedmem_stack ();
        Fixme_provider.local_changes_push_sharedmem_stack ();
        SharedMem.set_allow_hashtable_writes_by_current_process false
      | Provider_backend.Local_memory local ->
        Relative_path.Map.iter
          (Provider_context.get_entries ctx)
          ~f:(fun _path entry ->
            let (_ : Nast.program) =
              Ast_provider.compute_ast
                ~popt:(Provider_context.get_popt ctx)
                ~entry
            in
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
        File_provider.local_changes_pop_sharedmem_stack ();
        Fixme_provider.local_changes_pop_sharedmem_stack ();
        Naming_provider.local_changes_pop_sharedmem_stack ();
        SharedMem.set_allow_hashtable_writes_by_current_process true;
        SharedMem.invalidate_local_caches ()
      | Provider_backend.Rust_provider_backend backend ->
        Rust_provider_backend.pop_local_changes backend;

        Ast_provider.local_changes_pop_sharedmem_stack ();
        Shallow_classes_provider.local_changes_pop_sharedmem_stack ();
        Fixme_provider.local_changes_pop_sharedmem_stack ();
        SharedMem.set_allow_hashtable_writes_by_current_process true;
        SharedMem.invalidate_local_caches ()
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
