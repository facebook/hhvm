(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel

let ctx_from_server_env (env : ServerEnv.env) : Provider_context.t =
  {
    Provider_context.popt = env.ServerEnv.popt;
    tcopt = env.ServerEnv.tcopt;
    (* TODO: backend should be stored in [env]. *)
    backend = Provider_backend.get ();
    entries = Relative_path.Map.empty;
  }

let respect_but_quarantine_unsaved_changes
    ~(ctx : Provider_context.t) ~(f : unit -> 'a) : 'a =
  let make_then_revert_local_changes f () =
    Utils.with_context
      ~enter:(fun () ->
        Provider_context.set_is_quarantined_internal ();

        Errors.set_allow_errors_in_default_path true;
        SharedMem.allow_hashtable_writes_by_current_process false;

        Ast_provider.local_changes_push_stack ();
        Decl_provider.local_changes_push_stack ctx;
        File_provider.local_changes_push_stack ();
        Fixme_provider.local_changes_push_stack ();

        Ide_parser_cache.activate ();

        Naming_provider.push_local_changes ctx)
      ~exit:(fun () ->
        Errors.set_allow_errors_in_default_path false;
        SharedMem.allow_hashtable_writes_by_current_process true;

        Ast_provider.local_changes_pop_stack ();
        Decl_provider.local_changes_pop_stack ctx;
        File_provider.local_changes_pop_stack ();
        Fixme_provider.local_changes_pop_stack ();

        Ide_parser_cache.deactivate ();

        Naming_provider.pop_local_changes ctx;

        SharedMem.invalidate_caches ();

        Provider_context.unset_is_quarantined_internal ())
      ~do_:f
  in
  let (_errors, result) =
    Errors.do_
    @@ make_then_revert_local_changes (fun () ->
           Relative_path.Map.iter
             ctx.Provider_context.entries
             ~f:(fun _path entry ->
               let ast = Ast_provider.compute_ast ctx entry in
               let (funs, classes, record_defs, typedefs, consts) =
                 Nast.get_defs ast
               in
               (* Update the positions of the symbols present in the AST by redeclaring
               them. Note that this doesn't handle *removing* any entries from the
               naming table if they've disappeared since the last time we updated the
               naming table. *)
               let get_names ids = List.map ~f:snd ids |> SSet.of_list in
               Naming_global.remove_decls
                 ~ctx
                 ~funs:(get_names funs)
                 ~classes:(get_names classes)
                 ~record_defs:(get_names record_defs)
                 ~typedefs:(get_names typedefs)
                 ~consts:(get_names consts);
               Naming_global.make_env
                 ctx
                 ~funs
                 ~classes
                 ~record_defs
                 ~typedefs
                 ~consts);

           f ())
  in
  result
