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
  Provider_context.empty ~tcopt:env.ServerEnv.tcopt

let respect_but_quarantine_unsaved_changes
    ~(ctx : Provider_context.t) ~(f : unit -> 'a) : 'a =
  let make_then_revert_local_changes f () =
    Utils.with_context
      ~enter:(fun () ->
        Provider_context.set_global_context_internal ctx;

        Errors.set_allow_errors_in_default_path true;
        SharedMem.allow_hashtable_writes_by_current_process false;

        Ast_provider.local_changes_push_stack ();
        Decl_provider.local_changes_push_stack ctx;
        File_provider.local_changes_push_stack ();
        Fixme_provider.local_changes_push_stack ();

        Ide_parser_cache.activate ();

        Naming_table.push_local_changes ())
      ~exit:(fun () ->
        Errors.set_allow_errors_in_default_path false;
        SharedMem.allow_hashtable_writes_by_current_process true;

        Ast_provider.local_changes_pop_stack ();
        Decl_provider.local_changes_pop_stack ctx;
        File_provider.local_changes_pop_stack ();
        Fixme_provider.local_changes_pop_stack ();

        Ide_parser_cache.deactivate ();

        Naming_table.pop_local_changes ();

        SharedMem.invalidate_caches ();

        Provider_context.unset_global_context_internal ())
      ~do_:f
  in
  let (_errors, result) =
    Errors.do_
    @@ make_then_revert_local_changes (fun () ->
           Relative_path.Map.iter
             ctx.Provider_context.entries
             ~f:(fun _path { Provider_context.ast; _ } ->
               let (funs, classes, record_defs, typedefs, consts) =
                 Nast.get_defs ast
               in
               (* Update the positions of the symbols present in the AST by redeclaring
        them. Note that this doesn't handle *removing* any entries from the
        naming table if they've disappeared since the last time we updated the
        naming table. *)
               let get_names ids = List.map ~f:snd ids |> SSet.of_list in
               Naming_global.remove_decls
                 ~funs:(get_names funs)
                 ~classes:(get_names classes)
                 ~record_defs:(get_names record_defs)
                 ~typedefs:(get_names typedefs)
                 ~consts:(get_names consts);
               Naming_global.make_env
                 ~funs
                 ~classes
                 ~record_defs
                 ~typedefs
                 ~consts);

           f ())
  in
  result

let internal_load_entry
    ~(path : Relative_path.t) ~(file_input : ServerCommandTypes.file_input) :
    Provider_context.entry =
  let (ast_errors, (source_text, ast, comments)) =
    Errors.do_with_context path Errors.Parsing (fun () ->
        Ast_provider.parse_file_input
          ~full:true
          ~keep_errors:true
          path
          file_input)
  in
  {
    Provider_context.path;
    file_input;
    source_text;
    ast;
    ast_errors;
    comments;
    cst = None;
    tast = None;
    tast_errors = None;
    symbols = None;
  }

let update_context
    ~(ctx : Provider_context.t)
    ~(path : Relative_path.t)
    ~(file_input : ServerCommandTypes.file_input) :
    Provider_context.t * Provider_context.entry =
  let entry = internal_load_entry ~path ~file_input in
  let ctx =
    {
      ctx with
      Provider_context.entries =
        Relative_path.Map.add ctx.Provider_context.entries path entry;
    }
  in
  (ctx, entry)

let find_entry ~(ctx : Provider_context.t) ~(path : Relative_path.t) :
    Provider_context.entry option =
  Relative_path.Map.find_opt ctx.Provider_context.entries path

let get_entry_VOLATILE ~(ctx : Provider_context.t) ~(path : Relative_path.t) :
    Provider_context.entry =
  match find_entry ~ctx ~path with
  | Some entry -> entry
  | None ->
    let file_input =
      ServerCommandTypes.FileName (Relative_path.to_absolute path)
    in
    internal_load_entry ~path ~file_input

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
    let errors = Errors.merge entry.Provider_context.ast_errors tast_errors in
    { Compute_tast_and_errors.tast; errors; telemetry = Telemetry.create () }
  | (mode, _, _) ->
    (* prepare logging *)
    Deferred_decl.reset ~enable:false ~threshold_opt:None;
    let prev_tally_state = Counters.reset ~enable:true in
    begin
      match ctx.Provider_context.backend with
      | Provider_backend.Local_memory { decl_cache; _ } ->
        Provider_backend.Decl_cache.reset_telemetry decl_cache
      | _ -> ()
    end;
    let t = Unix.gettimeofday () in

    (* do the work *)
    let (nast_errors, nast) =
      Errors.do_with_context
        entry.Provider_context.path
        Errors.Naming
        (fun () -> Naming.program entry.Provider_context.ast)
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
        (fun () ->
          Typing.nast_to_tast ~do_tast_checks ctx.Provider_context.tcopt nast)
    in
    let tast_errors = Errors.merge nast_errors tast_errors in

    (* Logging... *)
    let telemetry = Counters.get_counters () in
    Counters.restore_state prev_tally_state;
    let telemetry =
      telemetry
      |> Telemetry.float_
           ~key:"duration_decl_and_typecheck"
           ~value:(Unix.gettimeofday () -. t)
      |> Telemetry.string_
           ~key:"provider"
           ~value:(ctx.Provider_context.backend |> Provider_backend.t_to_string)
    in
    (* File size. *)
    let telemetry =
      match entry.Provider_context.file_input with
      | ServerCommandTypes.FileName _ -> telemetry
      | ServerCommandTypes.FileContent s ->
        Telemetry.int_ telemetry ~key:"filesize" ~value:(String.length s)
    in
    (* Decl-provider cache overhead *)
    let telemetry =
      match ctx.Provider_context.backend with
      | Provider_backend.Local_memory { decl_cache; _ } ->
        let { Provider_backend.Decl_cache.time_spent; peak_size; num_evictions }
            =
          Provider_backend.Decl_cache.get_telemetry decl_cache
        in
        let bytes_per_word = Sys.word_size / 8 in
        let local_cache_telemetry =
          Telemetry.create ()
          |> Telemetry.float_ ~key:"time" ~value:time_spent
          |> Telemetry.int_ ~key:"peak_bytes" ~value:(peak_size * bytes_per_word)
          |> Telemetry.int_ ~key:"num_evictions" ~value:num_evictions
          |> Telemetry.int_
               ~key:"num_entries"
               ~value:(Provider_backend.Decl_cache.length decl_cache)
        in
        Telemetry.object_
          telemetry
          ~key:"local_cache"
          ~value:local_cache_telemetry
      | _ -> telemetry
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
      let errors = Errors.merge entry.Provider_context.ast_errors tast_errors in
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
    let errors = Errors.merge entry.Provider_context.ast_errors tast_errors in
    { Compute_tast_and_errors.tast; errors; telemetry = Telemetry.create () }
  (* Okay, we don't have memoized results, let's ensure we are quarantined before computing *)
  | _ ->
    let f () = compute_tast_and_errors_unquarantined ~ctx ~entry in
    (* If global context is not set, set it and proceed *)
    (match Provider_context.get_global_context () with
    | None -> respect_but_quarantine_unsaved_changes ~ctx ~f
    | Some _ -> f ())

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
    (* If global context is not set, set it and proceed *)
    (match Provider_context.get_global_context () with
    | None -> respect_but_quarantine_unsaved_changes ~ctx ~f
    | Some _ -> f ())

let compute_cst ~(ctx : Provider_context.t) ~(entry : Provider_context.entry) :
    Provider_context.PositionedSyntaxTree.t =
  let _ = ctx in
  match entry.Provider_context.cst with
  | Some cst -> cst
  | None ->
    let cst =
      Provider_context.PositionedSyntaxTree.make
        entry.Provider_context.source_text
    in
    entry.Provider_context.cst <- Some cst;
    cst
