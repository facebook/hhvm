(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

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
    let prev_ctx_telemetry = Provider_context.get_telemetry ctx in
    let prev_gc_telemetry = Telemetry.quick_gc_stat () in
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

    (* Logging... *)
    let ctx_telemetry =
      if Hh_logger.Level.passes_min_level Hh_logger.Level.Debug then
        Provider_context.get_telemetry ctx
        |> Telemetry.diff ~prev:prev_ctx_telemetry
      else
        Provider_context.get_telemetry ctx
    in
    let gc_telemetry =
      if Hh_logger.Level.passes_min_level Hh_logger.Level.Debug then
        Telemetry.quick_gc_stat () |> Telemetry.diff ~prev:prev_gc_telemetry
      else
        Telemetry.quick_gc_stat ()
    in
    let telemetry = Counters.get_counters () in
    Counters.restore_state prev_tally_state;
    let telemetry =
      telemetry
      |> Telemetry.object_ ~key:"ctx" ~value:ctx_telemetry
      |> Telemetry.object_ ~key:"gc" ~value:gc_telemetry
      |> Telemetry.int_ ~key:"errors.ast" ~value:(Errors.count ast_errors)
      |> Telemetry.int_ ~key:"errors.nast" ~value:(Errors.count nast_errors)
      |> Telemetry.int_ ~key:"errors.tast" ~value:(Errors.count tast_errors)
      |> Telemetry.float_
           ~key:"duration_decl_and_typecheck"
           ~value:(Unix.gettimeofday () -. t)
      |> Telemetry.int_
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
      let tast_errors = Errors.merge nast_errors tast_errors in
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
    | false -> Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f
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
    | false -> Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f
    | true -> f ())
