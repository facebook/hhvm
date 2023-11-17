(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let invalidate_shallow_and_some_folded_decls
    (local_memory : Provider_backend.local_memory) (file_info : FileInfo.t) :
    unit =
  let open FileInfo in
  let open Provider_backend in
  let open Provider_backend.Decl_cache_entry in
  List.iter file_info.consts ~f:(fun (_, name, _) ->
      Decl_cache.remove local_memory.decl_cache ~key:(Gconst_decl name));
  List.iter file_info.funs ~f:(fun (_, name, _) ->
      Decl_cache.remove local_memory.decl_cache ~key:(Fun_decl name));
  List.iter file_info.typedefs ~f:(fun (_, name, _) ->
      Decl_cache.remove local_memory.decl_cache ~key:(Typedef_decl name));
  List.iter file_info.modules ~f:(fun (_, name, _) ->
      Decl_cache.remove local_memory.decl_cache ~key:(Module_decl name));
  List.iter file_info.classes ~f:(fun (_, name, _) ->
      Shallow_decl_cache.remove
        local_memory.shallow_decl_cache
        ~key:(Shallow_decl_cache_entry.Shallow_class_decl name);
      Decl_cache.remove local_memory.decl_cache ~key:(Class_decl name);
      Folded_class_cache.remove
        local_memory.folded_class_cache
        ~key:(Folded_class_cache_entry.Folded_class_decl name));
  ()

let invalidate_named_shallow_and_some_folded_decls_for_entry
    (local_memory : Provider_backend.local_memory)
    (entries : Provider_context.entries) : unit =
  Relative_path.Map.iter entries ~f:(fun _path entry ->
      match entry.Provider_context.parser_return with
      | None -> () (* hasn't been parsed, hence nothing to invalidate *)
      | Some { Parser_return.ast; _ } ->
        let file_info = Nast.get_def_names ast in
        invalidate_shallow_and_some_folded_decls local_memory file_info);
  ()

let invalidate_upon_change
    ~(ctx : Provider_context.t)
    ~(local_memory : Provider_backend.local_memory)
    ~(changes : FileInfo.change list)
    ~(entries : Provider_context.entries) : unit =
  let open Provider_backend in
  let sticky_quarantine =
    Provider_context.get_tcopt ctx |> TypecheckerOptions.tco_sticky_quarantine
  in
  (* Invalidate all TASTs, since we don't know which are valid *)
  Relative_path.Map.iter entries ~f:(fun _path entry ->
      entry.Provider_context.tast <- None;
      entry.Provider_context.all_errors <- None);

  (* We also have to invalidate shallow and folded decls.
     I'm rolling out a new feature called "sticky_quarantine"
     which makes some scenarios faster, but at the cost of being
     more exposed to flaws in this [invalidate_upon_change] method.

     Specifically: prior to sticky_quarantine, we'd redecl a file's
     shallow and folded decls every single time we entered quarantine,
     and this masked flaws in the invalidation we do here and now
     upon file change. But with sticky_quarantine, we only redecl
     a file's decls if the pfh_hash has changed, so it no longer
     masks the flaws in [invalidate_upon_change].

     Therefore: if sticky_quarantine, then we'll do the right thing
     upon file change, trusting that the cost of doing the right thing
     is compensated by the increased speed of the common case.
     But if not sticky_quarantine, then we'll stick with the existing
     flawed-but-papered-over behavior. *)
  if sticky_quarantine then begin
    (* Invalidate all folded decls; we could do something more targeted to
       only invalidate the ones affected by [changed_symbols] but that'd take work. *)
    Decl_cache.clear local_memory.decl_cache;
    Folded_class_cache.clear local_memory.folded_class_cache;
    (* Invalidate only the shallow decls listed *)
    let changes =
      List.fold changes ~init:[] ~f:(fun acc change ->
          Option.to_list change.FileInfo.old_file_info
          @ Option.to_list change.FileInfo.new_file_info
          @ acc)
    in
    List.iter changes ~f:(fun file_info ->
        let FileInfo.
              {
                classes;
                consts;
                funs;
                typedefs;
                modules;
                hash = _;
                file_mode = _;
                comments = _;
              } =
          file_info
        in
        let open Provider_backend.Shallow_decl_cache_entry in
        let open Provider_backend.Decl_cache_entry in
        List.iter classes ~f:(fun (_, name, _) ->
            Shallow_decl_cache.remove
              local_memory.shallow_decl_cache
              ~key:(Shallow_class_decl name));
        List.iter consts ~f:(fun (_, name, _) ->
            Decl_cache.remove local_memory.decl_cache ~key:(Gconst_decl name));
        List.iter funs ~f:(fun (_, name, _) ->
            Decl_cache.remove local_memory.decl_cache ~key:(Fun_decl name));
        List.iter typedefs ~f:(fun (_, name, _) ->
            Decl_cache.remove local_memory.decl_cache ~key:(Typedef_decl name));
        List.iter modules ~f:(fun (_, name, _) ->
            Decl_cache.remove local_memory.decl_cache ~key:(Module_decl name));
        ())
  end else begin
    List.iter changes ~f:(fun { FileInfo.old_file_info; _ } ->
        Option.iter
          old_file_info
          ~f:(invalidate_shallow_and_some_folded_decls local_memory));
    ()
  end

(** This function will leave with either [local.decls_reflect_this_file] reflecting
the current contents of [ctx.entries] if there's a single entry in there, or None.
It will also preserve the invariant that all decls in [local] reflect the truth of
[local.decls_reflect_this_file] if present, and disk-content otherwise.

That's a precise way of saying that this function will
1. if there's a single entry in [Provider_context.entries] and agrees with [decls_reflect_this_file] then it's a no-op
2. otherwise, it invalidates any decls associated with the old [decls_reflect_this_file],
sets [decls_reflect_this_file] to the new entry, and invaldiates any decls associated with the new.
*)
let update_sticky_quarantine ctx local_memory =
  let open Provider_backend in
  (* helper to reset [decls_reflect_this_file] *)
  let reset () =
    match !(local_memory.decls_reflect_this_file) with
    | None -> ()
    | Some (_path, file_info, _pfh_hash) ->
      invalidate_shallow_and_some_folded_decls local_memory file_info;
      local_memory.decls_reflect_this_file := None;
      ()
  in

  let entries = Provider_context.get_entries ctx in
  if Relative_path.Map.cardinal entries <> 1 then begin
    (* if there isn't a single entry in [ctx.entries] then all we can do is reset [decls_reflect_this_file] *)
    reset ();
    ()
  end else begin
    let (path, entry) = Relative_path.Map.choose entries in
    match
      ( Direct_decl_utils.direct_decl_parse ctx entry.Provider_context.path,
        !(local_memory.decls_reflect_this_file) )
    with
    | ( Some { Direct_decl_parser.pfh_hash; _ },
        Some (path2, _file_info2, pfh_hash2) )
      when Relative_path.equal path path2 && Int64.equal pfh_hash pfh_hash2 ->
      (* hurrah! we can re-use the existing [decls_reflect_this_file]! *)
      ()
    | (None, _) ->
      (* if the new file is absent *)
      reset ();
      ()
    | (Some parsed_file, _) ->
      (* if we're going to replace [decls_reflect_this_file] with the new entry *)
      reset ();
      (* This sticks all shallow decls from the file into local_memory cache.
         Might as well; we have them in hand already. *)
      Direct_decl_utils.cache_decls
        ctx
        path
        parsed_file.Direct_decl_parser.pfh_decls;
      let file_info = Direct_decl_parser.decls_to_fileinfo path parsed_file in
      local_memory.decls_reflect_this_file :=
        Some (path, file_info, parsed_file.Direct_decl_parser.pfh_hash);
      ()
  end

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
  let sticky_quarantine =
    Provider_context.get_tcopt ctx |> TypecheckerOptions.tco_sticky_quarantine
  in
  (* Normally we satisfy the invariant that "every shallow+folded decl present in the
     provider-backend reflects truth as it is on disk". The definition of quarantine is
     that "during quarantine, every decl present reflects truth as it is in ctx.entries for those
     files that have entries, and the previous truth for all others."

     The feature "sticky_quarantine" is used only for the local backend. It uses a different
     invariant: "every decl present in the local backend reflects truth as it is in [decls_reflect_this_file]
     for the file mentioned there if any, and reflects truth as it is on disk for all other files".
     And during quarantine, again truth from ctx.entries overrides that previous truth.
     (Note that if ctx.entries agrees with [decls_reflect_this_file], then entering and leaving
     quarantine is a no-op!)

     This function will (1) enter quarantine, (2) do the callback "f",
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
        if sticky_quarantine then update_sticky_quarantine ctx local;
        (* The method [update_sticky_quarantine] is guaranteed to leave [decls_reflect_this_file]
           to be Some only if it is identical to ctx.entries; in this case we satisfy the
           invariant already. But in case of None, we'll ensure the invariant here
           by removing affected decls -- remember the invariant is that "all decls present in the cache
           reflect truth ..." hence if we remove a decl then it trivially satisfies the invariant! *)
        if Option.is_none !(local.Provider_backend.decls_reflect_this_file) then
          invalidate_named_shallow_and_some_folded_decls_for_entry
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
        if Option.is_none !(local.Provider_backend.decls_reflect_this_file) then
          invalidate_named_shallow_and_some_folded_decls_for_entry
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
