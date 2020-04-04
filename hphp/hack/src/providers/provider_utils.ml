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

let invalidate_tast_cache_for_all_ctx_entries ~(ctx : Provider_context.t) : unit
    =
  Relative_path.Map.iter
    (Provider_context.get_entries ctx)
    ~f:(fun _path entry ->
      entry.Provider_context.tast <- None;
      entry.Provider_context.tast_errors <- None)

let invalidate_local_decl_caches_for_ctx_entries (ctx : Provider_context.t) :
    unit =
  match Provider_context.get_backend ctx with
  | Provider_backend.Local_memory { Provider_backend.decl_cache; _ } ->
    ctx
    |> Provider_context.get_entries
    |> Relative_path.Map.iter ~f:(fun _path entry ->
           match entry.Provider_context.parser_return with
           | None -> () (* hasn't been parsed, hence nothing to invalidate *)
           | Some { Parser_return.ast; _ } ->
             let (funs, classes, record_defs, typedefs, gconsts) =
               Nast.get_defs ast
             in
             List.iter funs ~f:(fun (_, fun_name) ->
                 invalidate_fun decl_cache fun_name);
             List.iter classes ~f:(fun (_, class_name) ->
                 invalidate_class decl_cache class_name);
             List.iter record_defs ~f:(fun (_, record_name) ->
                 invalidate_record_def decl_cache record_name);
             List.iter typedefs ~f:(fun (_, typedef_name) ->
                 invalidate_typedef decl_cache typedef_name);
             List.iter gconsts ~f:(fun (_, gconst_name) ->
                 invalidate_gconst decl_cache gconst_name))
  | Provider_backend.Shared_memory ->
    (* Don't attempt to invalidate decls with shared memory, as we may not be
    running in the master process where that's allowed. *)
    ()
  | Provider_backend.Decl_service _ ->
    failwith
      "Decl_provider.invalidate_context_decls not yet impl. for decl memory provider"

let invalidate_local_decl_caches_for_file
    ~(ctx : Provider_context.t) (file_info : FileInfo.t) : unit =
  let open FileInfo in
  match Provider_context.get_backend ctx with
  | Provider_backend.Local_memory { Provider_backend.decl_cache; _ } ->
    let iter_names ids ~f = List.iter ids ~f:(fun (_, name) -> f name) in
    iter_names file_info.funs ~f:(invalidate_fun decl_cache);
    iter_names file_info.classes ~f:(invalidate_class decl_cache);
    iter_names
      file_info.classes
      ~f:(Shallow_classes_provider.invalidate_class ctx);
    iter_names file_info.record_defs ~f:(invalidate_record_def decl_cache);
    iter_names file_info.typedefs ~f:(invalidate_typedef decl_cache);
    iter_names file_info.consts ~f:(invalidate_gconst decl_cache);
    ()
  | _ -> failwith "only call invalidate for local memory backend"

let ctx_from_server_env (env : ServerEnv.env) : Provider_context.t =
  (* TODO: backend should be stored in [env]. *)
  Provider_context.empty_for_tool
    ~popt:env.ServerEnv.popt
    ~tcopt:env.ServerEnv.tcopt
    ~backend:(Provider_backend.get ())

(** This function will (1) enter quarantine, (2) respect ctx by updating
naming table for all files in ctx, (3) do the callback "f", (4) leave
quarantine. If an exception arises during step (1,2,3) then nevertheless
we guarantee that quarantine is safely left. If an exception arises during
step (4) then we'll raise an exception but the program state has become unstable...
*)
let respect_but_quarantine_unsaved_changes
    ~(ctx : Provider_context.t) ~(f : unit -> 'a) : 'a =
  let ast_pushed = ref false in
  let decl_pushed = ref false in
  let file_pushed = ref false in
  let fixme_pushed = ref false in
  let naming_pushed = ref false in
  let quarantine_set = ref false in
  let enter_quarantine_exn () =
    Ast_provider.local_changes_push_stack ();
    ast_pushed := true;
    Decl_provider.local_changes_push_stack ctx;
    invalidate_local_decl_caches_for_ctx_entries ctx;
    decl_pushed := true;
    File_provider.local_changes_push_stack ();
    file_pushed := true;
    Fixme_provider.local_changes_push_stack ();
    fixme_pushed := true;
    Naming_provider.push_local_changes ctx;
    naming_pushed := true;
    Ide_parser_cache.activate ();
    Errors.set_allow_errors_in_default_path true;
    SharedMem.allow_hashtable_writes_by_current_process false;
    Provider_context.set_is_quarantined_internal ();
    quarantine_set := true;
    ()
  in
  let leave_quarantine_exn () =
    if !quarantine_set then Provider_context.unset_is_quarantined_internal ();
    SharedMem.allow_hashtable_writes_by_current_process true;
    Errors.set_allow_errors_in_default_path false;
    Ide_parser_cache.deactivate ();
    if !naming_pushed then Naming_provider.pop_local_changes ctx;
    if !fixme_pushed then Fixme_provider.local_changes_pop_stack ();
    if !file_pushed then File_provider.local_changes_pop_stack ();
    if !decl_pushed then begin
      Decl_provider.local_changes_pop_stack ctx;
      invalidate_local_decl_caches_for_ctx_entries ctx
    end;
    if !ast_pushed then Ast_provider.local_changes_pop_stack ();
    SharedMem.invalidate_caches ();
    ()
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
