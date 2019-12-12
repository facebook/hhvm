(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Core_kernel

type compute_tast_result = {
  tast: Tast.program;
  errors: Errors.t;
  decl_cache_misses: int;
}

let respect_but_quarantine_unsaved_changes
    ~(ctx : Provider_context.t) ~(f : unit -> 'a) : 'a =
  let make_then_revert_local_changes f () =
    Utils.with_context
      ~enter:(fun () ->
        Provider_context.set_global_context_internal ctx;
        ServerIdeUtils.make_local_changes ())
      ~exit:(fun () ->
        ServerIdeUtils.revert_local_changes ();
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
  let (source_text, ast, comments) =
    Errors.ignore_ (fun () ->
        Ast_provider.parse_file_input ~full:true path file_input)
  in
  {
    Provider_context.path;
    file_input;
    source_text;
    ast;
    comments;
    cst = None;
    tast = None;
    errors = None;
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

let compute_tast_and_errors_unquarantined
    ~(ctx : Provider_context.t) ~(entry : Provider_context.entry) :
    compute_tast_result =
  match (entry.Provider_context.tast, entry.Provider_context.errors) with
  | (Some tast, Some errors) -> { tast; errors; decl_cache_misses = 0 }
  | _ ->
    (* prepare logging *)
    let prev_deferral_state =
      Deferred_decl.reset ~enable:true ~threshold_opt:None
    in
    begin
      match ctx.Provider_context.backend with
      | Provider_backend.Local_memory { decl_cache; _ } ->
        Memory_bounded_lru_cache.reset_telemetry decl_cache
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
      Errors.do_with_context
        entry.Provider_context.path
        Errors.Typing
        (fun () -> Typing.nast_to_tast ctx.Provider_context.tcopt nast)
    in
    let errors = Errors.merge nast_errors tast_errors in

    (* Logging... *)
    let decl_cache_misses = Deferred_decl.get_decl_cache_misses_counter () in
    let decl_cache_misses_time = Deferred_decl.get_decl_cache_misses_time () in
    let time_decl_and_typecheck = Unix.gettimeofday () -. t in
    Deferred_decl.restore_state prev_deferral_state;
    (* Sometimes we're called with a FileName that doesn't exist on disk. *)
    let filesize_opt =
      match entry.Provider_context.file_input with
      | ServerCommandTypes.FileName _ -> None
      | ServerCommandTypes.FileContent s -> Some (String.length s)
    in
    let (cache_overhead_time_opt, cache_peak_bytes_opt) =
      match ctx.Provider_context.backend with
      | Provider_backend.Local_memory { decl_cache; _ } ->
        let { Memory_bounded_lru_cache.time_spent; peak_size_in_words; _ } =
          Memory_bounded_lru_cache.get_telemetry decl_cache
        in
        let bytes_per_word = Sys.word_size / 8 in
        (Some time_spent, Some (peak_size_in_words * bytes_per_word))
      | _ -> (None, None)
    in

    let seconds_to_ms s = 1000. *. s |> int_of_float |> string_of_int in
    let bytes_to_k b = b / 1024 |> string_of_int in
    Hh_logger.debug
      "compute_tast: %s (%s ms / %s k), decl-fetch (%d / %s ms), cache (%s ms, %s k)"
      (Relative_path.suffix entry.Provider_context.path)
      (time_decl_and_typecheck |> seconds_to_ms)
      (Option.value_map filesize_opt ~default:"?" ~f:bytes_to_k)
      decl_cache_misses
      (decl_cache_misses_time |> seconds_to_ms)
      (Option.value_map cache_overhead_time_opt ~default:"?" ~f:seconds_to_ms)
      (Option.value_map cache_peak_bytes_opt ~default:"?" ~f:bytes_to_k);
    HackEventLogger.ProfileTypeCheck.compute_tast
      ~provider_backend:
        (ctx.Provider_context.backend |> Provider_backend.t_to_string)
      ~time_decl_and_typecheck
      ~decl_cache_misses
      ~decl_cache_misses_time
      ~cache_overhead_time_opt
      ~cache_peak_bytes_opt
      ~filesize_opt
      ~path:entry.Provider_context.path;

    (* Update the cache *)
    entry.Provider_context.tast <- Some tast;
    entry.Provider_context.errors <- Some errors;
    { tast; errors; decl_cache_misses }

let compute_tast_and_errors_quarantined
    ~(ctx : Provider_context.t) ~(entry : Provider_context.entry) :
    compute_tast_result =
  (* If results have already been memoized, don't bother quarantining anything *)
  match (entry.Provider_context.tast, entry.Provider_context.errors) with
  | (Some tast, Some errors) -> { tast; errors; decl_cache_misses = 0 }
  (* Okay, we don't have memoized results, let's ensure we are quarantined before computing *)
  | _ ->
    let f () = compute_tast_and_errors_unquarantined ~ctx ~entry in
    (* If global context is not set, set it and proceed *)
    (match Provider_context.get_global_context () with
    | None -> respect_but_quarantine_unsaved_changes ~ctx ~f
    | Some _ -> f ())

let compute_cst ~(ctx : Provider_context.t) ~(entry : Provider_context.entry) :
    Full_fidelity_ast.PositionedSyntaxTree.t =
  let _ = ctx in
  match entry.Provider_context.cst with
  | Some cst -> cst
  | None ->
    let parser_env = Full_fidelity_ast.make_env entry.Provider_context.path in
    let (cst, _) =
      Full_fidelity_ast.from_text_with_legacy_and_cst
        parser_env
        entry.Provider_context.source_text
    in
    entry.Provider_context.cst <- Some cst;
    cst
