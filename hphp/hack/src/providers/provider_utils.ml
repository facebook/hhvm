(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel

module Compute_tast = struct
  type t = {
    tast: Tast.program;
    telemetry: Telemetry.t;
  }
end

module Compute_tast_and_errors = struct
  type t = {
    tast: Tast.program;
    errors: Errors.t;
    telemetry: Telemetry.t;
  }
end

let ctx_from_server_env (env : ServerEnv.env) : Provider_context.t =
  {
    Provider_context.popt = env.ServerEnv.popt;
    tcopt = env.ServerEnv.tcopt;
    (* TODO: backend should be stored in [env]. *)
    backend = Provider_backend.get ();
    entries = Relative_path.Map.empty;
  }

let find_entry ~(ctx : Provider_context.t) ~(path : Relative_path.t) :
    Provider_context.entry option =
  Relative_path.Map.find_opt ctx.Provider_context.entries path

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

type _ compute_tast_mode =
  | Compute_tast_only : Compute_tast.t compute_tast_mode
  | Compute_tast_and_errors : Compute_tast_and_errors.t compute_tast_mode

let compute_tast_and_errors_unquarantined_internal
    (type a)
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(mode : a compute_tast_mode) : a =
  match
    (mode, entry.Provider_context.tast, entry.Provider_context.tast_errors)
  with
  | (Compute_tast_only, Some tast, _) ->
    { Compute_tast.tast; telemetry = Telemetry.create () }
  | (Compute_tast_and_errors, Some tast, Some tast_errors) ->
    let (_parser_return, ast_errors) =
      Ast_provider.compute_parser_return_and_ast_errors ~ctx ~entry
    in
    let errors = Errors.merge ast_errors tast_errors in
    { Compute_tast_and_errors.tast; errors; telemetry = Telemetry.create () }
  | (mode, _, _) ->
    (* prepare logging *)
    Deferred_decl.reset ~enable:false ~threshold_opt:None;
    Provider_context.reset_telemetry ctx;
    let prev_telemetry =
      Telemetry.create () |> Provider_context.get_telemetry ctx
    in
    let prev_tally_state = Counters.reset ~enable:true in
    let t = Unix.gettimeofday () in

    (* do the work *)
    let ({ Parser_return.ast; _ }, ast_errors) =
      Ast_provider.compute_parser_return_and_ast_errors ~ctx ~entry
    in
    let (nast_errors, nast) =
      Errors.do_with_context
        entry.Provider_context.path
        Errors.Naming
        (fun () -> Naming.program ctx ast)
    in
    let (tast_errors, tast) =
      let do_tast_checks =
        match mode with
        | Compute_tast_only -> false
        | Compute_tast_and_errors -> true
      in
      Errors.do_with_context
        entry.Provider_context.path
        Errors.Typing
        (fun () -> Typing_toplevel.nast_to_tast ~do_tast_checks ctx nast)
    in
    let tast_errors = Errors.merge nast_errors tast_errors in

    (* Logging... *)
    let telemetry = Counters.get_counters () in
    Counters.restore_state prev_tally_state;
    let telemetry =
      telemetry
      |> Provider_context.get_telemetry ctx
      |> Telemetry.float_
           ~key:"duration_decl_and_typecheck"
           ~value:(Unix.gettimeofday () -. t)
      |> Telemetry.object_ ~key:"prev" ~value:prev_telemetry
    in
    (* File size. *)
    let telemetry =
      Telemetry.int_
        telemetry
        ~key:"filesize"
        ~value:(String.length entry.Provider_context.contents)
    in

    Hh_logger.debug
      "compute_tast: %s\n%s"
      (Relative_path.suffix entry.Provider_context.path)
      (Telemetry.to_string telemetry);
    HackEventLogger.ProfileTypeCheck.compute_tast
      ~telemetry
      ~path:entry.Provider_context.path;

    (match mode with
    | Compute_tast_and_errors ->
      entry.Provider_context.tast <- Some tast;
      entry.Provider_context.tast_errors <- Some tast_errors;
      let errors = Errors.merge ast_errors tast_errors in
      { Compute_tast_and_errors.tast; errors; telemetry }
    | Compute_tast_only ->
      entry.Provider_context.tast <- Some tast;
      { Compute_tast.tast; telemetry })

let compute_tast_and_errors_unquarantined
    ~(ctx : Provider_context.t) ~(entry : Provider_context.entry) :
    Compute_tast_and_errors.t =
  compute_tast_and_errors_unquarantined_internal
    ~ctx
    ~entry
    ~mode:Compute_tast_and_errors

let compute_tast_unquarantined
    ~(ctx : Provider_context.t) ~(entry : Provider_context.entry) :
    Compute_tast.t =
  compute_tast_and_errors_unquarantined_internal
    ~ctx
    ~entry
    ~mode:Compute_tast_only

let compute_tast_and_errors_quarantined
    ~(ctx : Provider_context.t) ~(entry : Provider_context.entry) :
    Compute_tast_and_errors.t =
  (* If results have already been memoized, don't bother quarantining anything *)
  match (entry.Provider_context.tast, entry.Provider_context.tast_errors) with
  | (Some tast, Some tast_errors) ->
    let (_parser_return, ast_errors) =
      Ast_provider.compute_parser_return_and_ast_errors ~ctx ~entry
    in
    let errors = Errors.merge ast_errors tast_errors in
    { Compute_tast_and_errors.tast; errors; telemetry = Telemetry.create () }
  (* Okay, we don't have memoized results, let's ensure we are quarantined before computing *)
  | _ ->
    let f () = compute_tast_and_errors_unquarantined ~ctx ~entry in
    (match Provider_context.is_quarantined () with
    | false -> respect_but_quarantine_unsaved_changes ~ctx ~f
    | true -> f ())

let compute_tast_quarantined
    ~(ctx : Provider_context.t) ~(entry : Provider_context.entry) :
    Compute_tast.t =
  (* If results have already been memoized, don't bother quarantining anything *)
  match entry.Provider_context.tast with
  | Some tast -> { Compute_tast.tast; telemetry = Telemetry.create () }
  (* Okay, we don't have memoized results, let's ensure we are quarantined before computing *)
  | None ->
    let f () =
      compute_tast_and_errors_unquarantined_internal
        ~ctx
        ~entry
        ~mode:Compute_tast_only
    in
    (match Provider_context.is_quarantined () with
    | false -> respect_but_quarantine_unsaved_changes ~ctx ~f
    | true -> f ())
