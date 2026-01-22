(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude

module Compute_tast = struct
  type t = {
    tast: Tast.program Tast_with_dynamic.t;
    telemetry: Telemetry.t;
  }
end

module Compute_tast_and_errors = struct
  type t = {
    tast: Tast.program Tast_with_dynamic.t;
    diagnostics: Diagnostics.t;
    telemetry: Telemetry.t;
  }
end

module ErrorFilter = struct
  type t = {
    error_filter: Filter_diagnostics.Filter.t;
    warnings_saved_state: Warnings_saved_state.t option;
  }

  let default =
    {
      error_filter =
        Filter_diagnostics.Filter.make ~default_all:true ~generated_files:[] [];
      warnings_saved_state = None;
    }

  let apply { error_filter; warnings_saved_state } errors =
    errors
    |> Diagnostics.filter_out_mergebase_warnings warnings_saved_state
    |> Filter_diagnostics.filter_rel error_filter
end

type _ compute_tast_mode =
  | Compute_tast_only : Compute_tast.t compute_tast_mode
  | Compute_tast_and_errors :
      ErrorFilter.t
      -> Compute_tast_and_errors.t compute_tast_mode

type seconds_since_epoch = float

type start_telemetry = {
  prev_ctx_telemetry: Telemetry.t;
  start_gc_telemetry: Telemetry.t;
  start_tally_state: Counters.t;
  start_time: seconds_since_epoch;
}

let prepare_logging ctx : start_telemetry =
  Provider_context.reset_telemetry ctx;
  let prev_ctx_telemetry = Provider_context.get_telemetry ctx in
  Decl_counters.set_mode HackEventLogger.PerFileProfilingConfig.DeclingTopCounts;
  {
    prev_ctx_telemetry;
    start_gc_telemetry = Telemetry.quick_gc_stat ();
    start_tally_state = Counters.reset ();
    start_time = Unix.gettimeofday ();
  }

let log_and_telemetry
    ctx
    { prev_ctx_telemetry; start_gc_telemetry; start_tally_state; start_time }
    entry
    ~ast_errors
    ~naming_errors
    ~typing_errors =
  let ctx_telemetry =
    if Hh_logger.Level.passes_min_level Hh_logger.Level.Debug then
      Provider_context.get_telemetry ctx
      |> Telemetry.diff ~all:true ~prev:prev_ctx_telemetry
    else
      Provider_context.get_telemetry ctx
  in
  let gc_telemetry =
    if Hh_logger.Level.passes_min_level Hh_logger.Level.Debug then
      Telemetry.quick_gc_stat ()
      |> Telemetry.diff ~all:true ~prev:start_gc_telemetry
    else
      Telemetry.quick_gc_stat ()
  in
  let telemetry = Counters.get_counters () in
  Counters.restore_state start_tally_state;
  let telemetry =
    telemetry
    |> Telemetry.object_ ~key:"ctx" ~value:ctx_telemetry
    |> Telemetry.object_ ~key:"gc" ~value:gc_telemetry
    |> Telemetry.int_ ~key:"errors.ast" ~value:(Diagnostics.count ast_errors)
    |> Telemetry.int_
         ~key:"errors.nast"
         ~value:(Diagnostics.count naming_errors)
    |> Telemetry.int_
         ~key:"errors.tast"
         ~value:(Diagnostics.count typing_errors)
    |> Telemetry.float_
         ~key:"duration_decl_and_typecheck"
         ~value:(Unix.gettimeofday () -. start_time)
    |> Telemetry.int_
         ~key:"filesize"
         ~value:
           (String.length
              (Provider_context.get_file_contents_if_present entry
              |> Option.value ~default:""))
  in

  Hh_logger.debug
    "compute_tast: %s\n%s"
    (Relative_path.suffix entry.Provider_context.path)
    (Telemetry.to_string telemetry);
  HackEventLogger.ProfileTypeCheck.compute_tast
    ~telemetry
    ~path:entry.Provider_context.path
    ~start_time;
  telemetry

let compute_tast_and_errors_unquarantined_internal
    (type a)
    ~(ctx : Provider_context.t)
    ~(entry : Provider_context.entry)
    ~(mode : a compute_tast_mode) : a =
  match
    (mode, entry.Provider_context.tast, entry.Provider_context.all_diagnostics)
  with
  | (Compute_tast_only, Some tast, _) ->
    { Compute_tast.tast; telemetry = Telemetry.create () }
  | (Compute_tast_and_errors _, Some tast, Some diagnostics) ->
    {
      Compute_tast_and_errors.tast;
      diagnostics;
      telemetry = Telemetry.create ();
    }
  | (_, _, _) ->
    let start_telemetry = prepare_logging ctx in

    let ({ Parser_return.ast; _ }, ast_errors) =
      Ast_provider.compute_parser_return_and_ast_errors
        ~popt:(Provider_context.get_popt ctx)
        ~entry
    in
    let (naming_errors, nast) =
      Diagnostics.do_with_context entry.Provider_context.path (fun () ->
          Naming.program ctx ast)
    in
    let (typing_errors, tast) =
      let do_tast_checks =
        match mode with
        | Compute_tast_only -> false
        | Compute_tast_and_errors _ -> true
      in
      Diagnostics.do_with_context entry.Provider_context.path (fun () ->
          Typing_toplevel.nast_to_tast ~do_tast_checks ctx nast)
    in

    let telemetry =
      log_and_telemetry
        ctx
        start_telemetry
        entry
        ~ast_errors
        ~naming_errors
        ~typing_errors
    in

    (match mode with
    | Compute_tast_and_errors error_filter ->
      let diagnostics =
        naming_errors
        |> Diagnostics.merge typing_errors
        |> Diagnostics.merge ast_errors
        |> ErrorFilter.apply error_filter
      in
      entry.Provider_context.tast <- Some tast;
      entry.Provider_context.all_diagnostics <- Some diagnostics;
      { Compute_tast_and_errors.tast; diagnostics; telemetry }
    | Compute_tast_only ->
      entry.Provider_context.tast <- Some tast;
      { Compute_tast.tast; telemetry })

let compute_tast_and_errors_unquarantined
    ~(ctx : Provider_context.t) ~(entry : Provider_context.entry) ~error_filter
    : Compute_tast_and_errors.t =
  compute_tast_and_errors_unquarantined_internal
    ~ctx
    ~entry
    ~mode:(Compute_tast_and_errors error_filter)

let compute_tast_unquarantined
    ~(ctx : Provider_context.t) ~(entry : Provider_context.entry) :
    Compute_tast.t =
  compute_tast_and_errors_unquarantined_internal
    ~ctx
    ~entry
    ~mode:Compute_tast_only

let compute_tast_and_errors_quarantined
    ~(ctx : Provider_context.t) ~(entry : Provider_context.entry) ~error_filter
    : Compute_tast_and_errors.t =
  (* If results have already been memoized, don't bother quarantining anything *)
  match
    (entry.Provider_context.tast, entry.Provider_context.all_diagnostics)
  with
  | (Some tast, Some diagnostics) ->
    {
      Compute_tast_and_errors.tast;
      diagnostics;
      telemetry = Telemetry.create ();
    }
  (* Okay, we don't have memoized results, let's ensure we are quarantined before computing *)
  | _ ->
    let f () =
      compute_tast_and_errors_unquarantined ~ctx ~entry ~error_filter
    in
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
