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
    ( mode,
      TypecheckerOptions.tast_under_dynamic (Provider_context.get_tcopt ctx),
      entry.Provider_context.tast,
      entry.Provider_context.all_errors )
  with
  | (Compute_tast_only, false, Provider_context.Entry_tast_no_dynamic tast, _)
    ->
    { Compute_tast.tast; telemetry = Telemetry.create () }
  | (Compute_tast_only, true, Provider_context.Entry_tast_under_dynamic tast, _)
    ->
    { Compute_tast.tast; telemetry = Telemetry.create () }
  | ( Compute_tast_and_errors,
      false,
      Provider_context.Entry_tast_no_dynamic tast,
      Some errors ) ->
    { Compute_tast_and_errors.tast; errors; telemetry = Telemetry.create () }
  | ( Compute_tast_and_errors,
      true,
      Provider_context.Entry_tast_under_dynamic tast,
      Some errors ) ->
    { Compute_tast_and_errors.tast; errors; telemetry = Telemetry.create () }
  | (mode, tast_under_dynamic, _, _) ->
    (* prepare logging *)
    Provider_context.reset_telemetry ctx;
    let prev_ctx_telemetry = Provider_context.get_telemetry ctx in
    let prev_gc_telemetry = Telemetry.quick_gc_stat () in
    Decl_counters.set_mode
      HackEventLogger.PerFileProfilingConfig.DeclingTopCounts;
    let prev_tally_state = Counters.reset () in
    let start_time = Unix.gettimeofday () in

    (* do the work *)
    let ({ Parser_return.ast; _ }, ast_errors) =
      Ast_provider.compute_parser_return_and_ast_errors
        ~popt:(Provider_context.get_popt ctx)
        ~entry
    in
    let (naming_errors, nast) =
      Errors.do_with_context entry.Provider_context.path (fun () ->
          Naming.program ctx ast)
    in
    let (typing_errors, tast) =
      let do_tast_checks =
        match mode with
        | Compute_tast_only -> false
        | Compute_tast_and_errors -> true
      in
      (* Note: duplicate name errors are normally caught during [nast_to_tast].
         But the way [Naming_provider.find_symbol_in_context] works is it always
         deems names in [entry] to be the winner; therefore, until Naming_provider
         is changed, we will be unable to detect duplicate names in [entry]. *)
      Errors.do_with_context entry.Provider_context.path (fun () ->
          Typing_toplevel.nast_to_tast ~do_tast_checks ctx nast)
    in

    (* Logging... *)
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
        |> Telemetry.diff ~all:true ~prev:prev_gc_telemetry
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
      |> Telemetry.int_ ~key:"errors.nast" ~value:(Errors.count naming_errors)
      |> Telemetry.int_ ~key:"errors.tast" ~value:(Errors.count typing_errors)
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

    let some_tast =
      if tast_under_dynamic then
        Provider_context.Entry_tast_under_dynamic tast
      else
        Provider_context.Entry_tast_no_dynamic tast
    in
    (match mode with
    | Compute_tast_and_errors ->
      let errors =
        naming_errors |> Errors.merge typing_errors |> Errors.merge ast_errors
      in
      entry.Provider_context.tast <- some_tast;
      entry.Provider_context.all_errors <- Some errors;
      { Compute_tast_and_errors.tast; errors; telemetry }
    | Compute_tast_only ->
      entry.Provider_context.tast <- some_tast;
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
  match
    ( TypecheckerOptions.tast_under_dynamic (Provider_context.get_tcopt ctx),
      entry.Provider_context.tast,
      entry.Provider_context.all_errors )
  with
  | (false, Provider_context.Entry_tast_no_dynamic tast, Some errors) ->
    { Compute_tast_and_errors.tast; errors; telemetry = Telemetry.create () }
  | (true, Provider_context.Entry_tast_under_dynamic tast, Some errors) ->
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
  match
    ( TypecheckerOptions.tast_under_dynamic (Provider_context.get_tcopt ctx),
      entry.Provider_context.tast )
  with
  | (false, Provider_context.Entry_tast_no_dynamic tast) ->
    { Compute_tast.tast; telemetry = Telemetry.create () }
  | (true, Provider_context.Entry_tast_under_dynamic tast) ->
    { Compute_tast.tast; telemetry = Telemetry.create () }
  (* Okay, we don't have memoized results, let's ensure we are quarantined before computing *)
  | (_, _) ->
    let f () =
      compute_tast_and_errors_unquarantined_internal
        ~ctx
        ~entry
        ~mode:Compute_tast_only
    in
    (match Provider_context.is_quarantined () with
    | false -> Provider_utils.respect_but_quarantine_unsaved_changes ~ctx ~f
    | true -> f ())
