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
    if !decl_pushed then Decl_provider.local_changes_pop_stack ctx;
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
