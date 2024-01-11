(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

(** This is a leftover function, because we haven't yet migrated to
sticky decls and more correct invalidation. *)
let invalidate_shallow_and_some_folded_decls
    (local_memory : Provider_backend.local_memory) (ids : FileInfo.ids) : unit =
  let open FileInfo in
  let open Provider_backend in
  let open Provider_backend.Decl_cache_entry in
  let { funs; consts; classes; typedefs; modules } = ids in
  List.iter consts ~f:(fun (_, name, _) ->
      Decl_cache.remove local_memory.decl_cache ~key:(Gconst_decl name));
  List.iter funs ~f:(fun (_, name, _) ->
      Decl_cache.remove local_memory.decl_cache ~key:(Fun_decl name));
  List.iter typedefs ~f:(fun (_, name, _) ->
      Decl_cache.remove local_memory.decl_cache ~key:(Typedef_decl name));
  List.iter modules ~f:(fun (_, name, _) ->
      Decl_cache.remove local_memory.decl_cache ~key:(Module_decl name));
  List.iter classes ~f:(fun (_, name, _) ->
      Shallow_decl_cache.remove
        local_memory.shallow_decl_cache
        ~key:(Shallow_decl_cache_entry.Shallow_class_decl name);
      Decl_cache.remove local_memory.decl_cache ~key:(Class_decl name);
      Folded_class_cache.remove
        local_memory.folded_class_cache
        ~key:(Folded_class_cache_entry.Folded_class_decl name));
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
    let f set (_pos, name, _hash) = SSet.add name set in
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

(** This invalidates (evicts) every single folded class decl *)
let invalidate_all_folded_decls ~local_memory =
  let open Provider_backend in
  Decl_cache.clear local_memory.decl_cache;
  Folded_class_cache.clear local_memory.folded_class_cache;
  ()

(** This invalidates (evicts) every folded decl in cache that depends upon
the shallow class decl [changed_class_name], i.e. the folded decl for
[changed_class_name] itself, plus every folded decl which has [changed_class_name]
as one of its direct or indirect ancestors.

It does this by iterating over every single folded decl in cache. For each one,
it checks whether [changed_class_name] is in that folded decl's ancestor list.
This might be costly if there are thousands of folded decls in cache,
each with hundreds of ancestors. *)
let invalidate_folded_decls_by_checking_each_ones_ancestor_list
    ~local_memory changed_class_name =
  let open Provider_backend in
  let { decl_cache; folded_class_cache; _ } = local_memory in
  let matches_ty (ty : Typing_defs.decl_ty) =
    match Typing_defs.get_node ty with
    | Typing_defs.Tapply ((_pos, ty_name), _args) ->
      String.equal changed_class_name ty_name
    | _ -> false
  in
  let matches_dc (dc : Decl_defs.decl_class_type) =
    let open Decl_defs in
    SSet.mem changed_class_name dc.dc_extends
    || SSet.mem changed_class_name dc.dc_req_ancestors_extends
    || SSet.mem changed_class_name dc.dc_xhp_attr_deps
    || SMap.mem changed_class_name dc.dc_ancestors
    || List.exists dc.dc_req_ancestors ~f:(fun (_, ty) -> matches_ty ty)
    || List.exists dc.dc_req_class_ancestors ~f:(fun (_, ty) -> matches_ty ty)
  in

  let (to_remove, to_keep) =
    Folded_class_cache.fold
      folded_class_cache
      ~init:([changed_class_name], SSet.empty)
      ~f:(fun
           (Folded_class_cache.Element
             (Folded_class_cache_entry.Folded_class_decl name, dc))
           (to_remove, to_keep)
         ->
        if matches_dc dc then
          (name :: to_remove, to_keep)
        else
          (to_remove, SSet.add name to_keep))
  in
  List.iter to_remove ~f:(fun name ->
      Folded_class_cache.remove
        folded_class_cache
        ~key:(Folded_class_cache_entry.Folded_class_decl name);
      Decl_cache.remove decl_cache ~key:(Decl_cache_entry.Class_decl name));
  let to_remove =
    Decl_cache.fold
      decl_cache
      ~init:[]
      ~f:(fun (Decl_cache.Element (key, value)) to_remove ->
        match (key, value) with
        | (Decl_cache_entry.Class_decl name, (dc, _)) ->
          if SSet.mem name to_keep then
            to_remove
          else if matches_dc dc then
            name :: to_remove
          else
            to_remove
        | _ -> to_remove)
  in
  List.iter to_remove ~f:(fun name ->
      Decl_cache.remove decl_cache ~key:(Decl_cache_entry.Class_decl name));
  ()

(** A signature might change because you're editing the signature in an unsaved file.
Or because some files on disk changed, maybe due to a rebase or some codegen script.
We have to invalidate (evict from cache) any affected decls. For some changes like Fun,
all we'll have to do is remove the affected symbols from [decl_cache]. But for classlikes,
we'll have to not only remove them from [shallow_decl_cache] but also remove all folded
things that involve them from [decl_cache] and [folded_class_cache].

There's a tradeoff to be had. What if it takes too much computational time
right now to precisely identify which symbols have been changed, or to precisely evict
only things relating to these, more than the time we save by not having more-than-necessary
symbols evicted? It's a hard tradeoff to measure because the time we save in future is
spread across multiple future typechecks, but the time we spend happens now. *)
let invalidate_decls_upon_change
    ~(ctx : Provider_context.t)
    ~(local_memory : Provider_backend.local_memory)
    ~(new_decls_to_diff : Direct_decl_parser.parsed_file_with_hashes option)
    (symbols : FileInfo.names) : Telemetry.t =
  let open Provider_backend in
  let FileInfo.{ n_classes; n_types; n_funs; n_consts; n_modules } = symbols in
  let { decl_cache; shallow_decl_cache; folded_class_cache; _ } =
    local_memory
  in
  let start_decl_count = Decl_cache.length decl_cache in
  let start_shallow_count = Shallow_decl_cache.length shallow_decl_cache in
  let start_folded_count = Folded_class_cache.length folded_class_cache in
  let start_time = Unix.gettimeofday () in

  (* Are any of [n_classes] present in [new_decls]? If they are, and if we
     can prove that they're unchanged with respect to what they are currently
     in cache, then we don't need to invalidate them. This will help in the common
     case that you've done a keystroke in a class signature in a file that contains
     one class and one enum -- it'll notice that the enum was unchanged, and so
     there's only one single class change, and the rest of this method is
     optimized for the case of only one single class change. *)
  let n_classes =
    match new_decls_to_diff with
    | None -> n_classes
    | Some Direct_decl_parser.{ pfh_decls; _ } ->
      let package_info = Provider_context.get_package_info ctx in
      n_classes
      |> SSet.filter (fun name ->
             let prev_sc =
               Shallow_decl_cache.find
                 shallow_decl_cache
                 ~key:(Shallow_decl_cache_entry.Shallow_class_decl name)
             in
             let new_sc =
               List.find_map pfh_decls ~f:(function
                   | (decl_name, Shallow_decl_defs.Class decl, _hash)
                     when String.equal name decl_name ->
                     Some decl
                   | _ -> None)
             in
             match (prev_sc, new_sc) with
             | (Some prev_sc, Some new_sc) ->
               Shallow_class_diff.diff_class ctx package_info prev_sc new_sc
               |> Option.is_some
             | _ -> true)
  in

  (* Invalidate all folded decls. This is a performance-sensitive piece of code.
     We're going to use heuristics (guesses) and telemetry-driven experiments,
     and iterate from there if anything looks promising.
     1. Let's optimize for the common case where the user is editing in a file
     and they type a character inside a class signature. In other words, only
     one class has been changed.
     2. I'm not sure what strategy would be best.
     One idea (controlled by flag [lsp_invalidation] is to iterate over all folded decls
     currently in cache, and check the ancestor-list of each to see if they're affected,
     cost O(n_changes * n_cached_decls * n_ancestor_length_per_decl) for invalidation.
     Another strategy is just to invalidate every single folded decl,
     cost O(1) for invalidation but then more cost down the line to re-fold everything.
     3. I haven't yet put work into the case where there are multiple classes
     changed. I suspect this one is rare (only happens due to rebase or codegen where
     multiple changes happen at once). I'm scared of doing the iteration since that
     will have n_changes > 1, bringing big-O cost to O(n^3), which is a scary large exponential.
     So I'll leave it with the O(1) invalidate-every-decl strategy. *)
  let strategy =
    if SSet.cardinal n_classes = 1 then begin
      let change = SSet.choose n_classes in
      let lsp_invalidation =
        Provider_context.get_tcopt ctx
        |> TypecheckerOptions.tco_lsp_invalidation
      in
      if lsp_invalidation then begin
        invalidate_folded_decls_by_checking_each_ones_ancestor_list
          ~local_memory
          change;
        "check_each_folded_ancestors"
      end else begin
        invalidate_all_folded_decls ~local_memory;
        "invalidate_all"
      end
    end else begin
      invalidate_all_folded_decls ~local_memory;
      "multiple_class_changes_so_invalidate_all"
    end
  in

  (* Invalidate all shallow decls *)
  let t_shallow = Unix.gettimeofday () in
  let open Provider_backend.Decl_cache_entry in
  let open Provider_backend.Shallow_decl_cache_entry in
  SSet.iter
    (fun name -> Decl_cache.remove decl_cache ~key:(Fun_decl name))
    n_funs;
  SSet.iter
    (fun name -> Decl_cache.remove decl_cache ~key:(Gconst_decl name))
    n_consts;
  SSet.iter
    (fun name -> Decl_cache.remove decl_cache ~key:(Typedef_decl name))
    n_types;
  SSet.iter
    (fun name -> Decl_cache.remove decl_cache ~key:(Module_decl name))
    n_modules;
  SSet.iter
    (fun name ->
      Shallow_decl_cache.remove
        shallow_decl_cache
        ~key:(Shallow_class_decl name))
    n_classes;

  (* telemetry *)
  let end_decl_count = Decl_cache.length decl_cache in
  let end_shallow_count = Shallow_decl_cache.length shallow_decl_cache in
  let end_folded_count = Folded_class_cache.length folded_class_cache in
  Telemetry.create ()
  |> Telemetry.duration
       ~key:"duration_folded_ms"
       ~start_time
       ~end_time:t_shallow
  |> Telemetry.duration ~key:"duration_shallow_ms" ~start_time:t_shallow
  |> Telemetry.string_ ~key:"folded_strategy" ~value:strategy
  |> Telemetry.int_ ~key:"n_classes" ~value:(SSet.cardinal n_classes)
  |> Telemetry.int_ ~key:"n_funs" ~value:(SSet.cardinal n_funs)
  |> Telemetry.int_ ~key:"n_consts" ~value:(SSet.cardinal n_consts)
  |> Telemetry.int_ ~key:"n_types" ~value:(SSet.cardinal n_types)
  |> Telemetry.int_ ~key:"n_modules" ~value:(SSet.cardinal n_modules)
  |> Telemetry.int_ ~key:"start_decl_count" ~value:start_decl_count
  |> Telemetry.int_ ~key:"start_shallow_count" ~value:start_shallow_count
  |> Telemetry.int_ ~key:"start_folded_count" ~value:start_folded_count
  |> Telemetry.int_ ~key:"end_decl_count" ~value:end_decl_count
  |> Telemetry.int_ ~key:"end_shallow_count" ~value:end_shallow_count
  |> Telemetry.int_ ~key:"end_folded_count" ~value:end_folded_count

let invalidate_upon_file_changes
    ~(ctx : Provider_context.t)
    ~(local_memory : Provider_backend.local_memory)
    ~(changes : FileInfo.change list)
    ~(entries : Provider_context.entries) : Telemetry.t =
  let start_time = Unix.gettimeofday () in
  let sticky_quarantine =
    Provider_context.get_tcopt ctx |> TypecheckerOptions.tco_sticky_quarantine
  in

  (* The purpose+invariants of this function are documented in the module-level comment
     in Provider_utils.mli. *)

  (* In the common scenario where a user has been working on a file and saves it,
     then [decls_reflect_this_file] will have the exact same pfh_hash
     as what we just read from disk and we can skip any invalidation! *)
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
      (* Otherwise, given the disk change, we have to restore the invariant
         that "local_memory decls if present reflect truth as of decls_reflect_this_file
         and all other files as they are on disk". Note that invalidating decls
         will always work towards restoring that invariant!

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
        (* You might think that only the "old symbols" in a changed file ever need be evicted.
           But even "new symbols" might imply eviction too. For instance if there was an
           existing class Foo, but the change creates a new declaration for Foo that's the
           "winner" (i.e. in an alphabetically prior file), then the old declaration for Foo
           will have to be evicted. *)
        let symbols = combine_old_and_new_symbols changes in
        let telemetry =
          invalidate_decls_upon_change
            ~ctx
            ~local_memory
            ~new_decls_to_diff:None
            symbols
        in
        ("precise", Some telemetry, entries)
      end else begin
        List.iter changes ~f:(fun { FileInfo.old_ids; _ } ->
            Option.iter old_ids ~f:(fun ids ->
                invalidate_shallow_and_some_folded_decls local_memory ids));
        ("imprecise", None, entries)
      end
  in

  (* In addition to decls, we also have derived facts in the form of TASTs.
     We'll invalidate them all since we don't know which are affected.
     Only exception, calculated above, is that if [decls_reflect_this_file]
     matches one of the entries, then its TAST is already correct and
     needn't be invalidated. (This is the common case where you save
     a file that you'd previously been editing). *)
  Relative_path.Map.iter tasts_to_invalidate ~f:(fun _path entry ->
      entry.Provider_context.tast <- None;
      entry.Provider_context.all_errors <- None);

  Option.value telemetry_opt ~default:(Telemetry.create ())
  |> Telemetry.duration ~key:"invalidate_upon_file_changes_ms" ~start_time
  |> Telemetry.string_ ~key:"kind" ~value:kind

(** This function will leave with either [local.decls_reflect_this_file] reflecting
the current contents of [ctx.entries] if there's a single entry in there, or None.
It will also preserve the invariant that all decls in [local] reflect the truth of
[local.decls_reflect_this_file] if present, and disk-content otherwise.

That's a precise way of saying that this function will
1. if there's a single entry in [Provider_context.entries] and agrees with [decls_reflect_this_file] then it's a no-op
2. otherwise, it invalidates any decls associated with the old [decls_reflect_this_file],
sets [decls_reflect_this_file] to the new entry, and invaldiates any decls associated with the new.
*)
let update_sticky_quarantine ctx local_memory : Telemetry.t =
  let open Provider_backend in
  (* helper to reset [decls_reflect_this_file] *)
  let reset new_decls_to_diff : Telemetry.t option =
    match !(local_memory.decls_reflect_this_file) with
    | None -> None
    | Some (_path, { FileInfo.ids; _ }, _pfh_hash) ->
      local_memory.decls_reflect_this_file := None;
      let symbols = FileInfo.ids_to_names ids in
      Some
        (invalidate_decls_upon_change
           ~ctx
           ~local_memory
           ~new_decls_to_diff
           symbols)
  in

  let start_time = Unix.gettimeofday () in
  let entries = Provider_context.get_entries ctx in
  let (kind, telemetry_opt) =
    if Relative_path.Map.cardinal entries = 0 then begin
      ("zero_entries", reset None)
    end else if Relative_path.Map.cardinal entries <> 1 then begin
      (* if there isn't a single entry in [ctx.entries] then all we can do is reset [decls_reflect_this_file] *)
      ("multiple_entries", reset None)
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
        ("reuse_sticky_quarantine", None)
      | (None, _) ->
        (* if the new file is absent *)
        ("new_file_absent", None)
      | (Some parsed_file, _) ->
        (* if we're going to replace [decls_reflect_this_file] with the new entry *)
        let telemetry_opt = reset (Some parsed_file) in
        (* This sticks all shallow decls from the file into local_memory cache.
           Might as well; we have them in hand already. *)
        Direct_decl_utils.cache_decls
          ctx
          path
          parsed_file.Direct_decl_parser.pfh_decls;
        let file_info = Direct_decl_parser.decls_to_fileinfo path parsed_file in
        local_memory.decls_reflect_this_file :=
          Some (path, file_info, parsed_file.Direct_decl_parser.pfh_hash);
        ("sticky_quarantine_changed", telemetry_opt)
    end
  in
  Option.value telemetry_opt ~default:(Telemetry.create ())
  |> Telemetry.duration ~key:"update_sticky_quarantine_ms" ~start_time
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
  let sticky_quarantine =
    Provider_context.get_tcopt ctx |> TypecheckerOptions.tco_sticky_quarantine
  in
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
        Relative_path.Map.iter entries ~f:(fun _path entry ->
            let (_ : Nast.program) =
              Ast_provider.compute_ast
                ~popt:(Provider_context.get_popt ctx)
                ~entry
            in
            ());
        let telemetry =
          if sticky_quarantine then
            update_sticky_quarantine ctx local
          else
            Telemetry.create ()
        in
        (* The method [update_sticky_quarantine] is guaranteed to leave [decls_reflect_this_file]
           to be Some only if it is identical to ctx.entries; in this case we satisfy the
           invariant already. But in case of None, we'll ensure the invariant here
           by removing affected decls -- remember the invariant is that "all decls present in the cache
           reflect truth ..." hence if we remove a decl then it trivially satisfies the invariant! *)
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
    Errors.do_ (fun () ->
        Utils.try_finally
          ~f:(fun () ->
            enter_quarantine_exn ();
            f ())
          ~finally:leave_quarantine_exn)
  in
  result
