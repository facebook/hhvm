(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Shape_analysis_types
module A = Aast
module SA = Shape_analysis
module SAC = Shape_analysis_codemod
module Env = Tast_env

let constraints_dir = "/tmp/shape_analysis_constraints"

let constraints_dir_created = ref false

type logger_mode =
  | LogLocally
  | LogScuba
  | LogCodemod of {
      atomic: bool;
          (** 'atomic' is described in D40008464. TODO(T138659101) remove option. *)
    }
  | LogConstraints of { mode: mode }

let mode_of_logger_mode = function
  | LogConstraints { mode } -> mode
  | LogCodemod { atomic = _ } -> Local
  | LogLocally
  | LogScuba ->
    Local

let parse_logger_mode_exn typing_env =
  let level_int =
    Typing_env.get_tcopt typing_env
    |> TypecheckerOptions.log_levels
    |> SMap.find_opt "shape_analysis"
  in
  match Option.value_exn level_int with
  | 1 -> LogLocally
  | 2 -> LogScuba
  | 3 -> LogCodemod { atomic = false }
  | 4 -> LogCodemod { atomic = true }
  | 5 -> LogConstraints { mode = Local }
  | 6 -> LogConstraints { mode = Global }
  | n -> failwith @@ Printf.sprintf "Unrecognized logger mode %d" n

let channel_opt = ref None

let get_channel () =
  match !channel_opt with
  | None ->
    let worker_id = Option.value ~default:0 !Typing_deps.worker_id in
    let filename =
      Format.asprintf "/tmp/shape-like-dict-codemod-%d.json" worker_id
    in
    let channel =
      Caml.Out_channel.(open_gen [Open_wronly; Open_creat] 0o644 filename)
    in
    channel_opt := Some channel;
    channel
  | Some channel -> channel

let log_events_locally typing_env : log list -> unit =
  let log_result id error_count result : unit =
    Format.sprintf
      "[RESULT] %s: (# of errors: %d) %s\n"
      id
      error_count
      (SA.show_shape_result typing_env result)
    |> Out_channel.output_string !Typing_log.out_channel;
    Out_channel.flush !Typing_log.out_channel
  in
  let log_results id result : unit =
    List.iter ~f:(log_result id result.error_count) result.results
  in
  let log_error id error_message =
    Format.sprintf "[FAILURE] %s: %s\n" id (Error.show error_message)
    |> Out_channel.output_string !Typing_log.out_channel;
    Out_channel.flush !Typing_log.out_channel
  in
  List.iter ~f:(fun result ->
      Either.iter
        ~first:(log_results result.location)
        ~second:(log_error result.location)
        result.result)

let log_events_as_codemod typing_env : log list -> unit =
  let log_success { results; error_count } =
    let channel = get_channel () in
    let len = Out_channel.length channel in
    let result_json = SAC.group_of_results typing_env results ~error_count in
    let result_str =
      if Int64.(len = of_int 0) then
        Format.asprintf "[\n%a\n]" Hh_json.pp_json result_json
      else begin
        Out_channel.seek channel Int64.(len - of_int 2);
        Format.asprintf ",\n%a\n]" Hh_json.pp_json result_json
      end
    in
    Out_channel.output_string channel result_str;
    Out_channel.flush channel
  in
  List.iter ~f:(fun result ->
      Either.iter ~first:log_success ~second:Fn.ignore result.result)

let log_events tast_env =
  let typing_env = Tast_env.tast_env_as_typing_env tast_env in
  match parse_logger_mode_exn typing_env with
  | LogLocally -> log_events_locally typing_env
  | LogScuba -> Shape_analysis_scuba.log_events_remotely typing_env
  | LogCodemod _ -> log_events_as_codemod typing_env
  | LogConstraints _ -> ignore

let compute_results source_file tast_env id params return body =
  let strip_decorations { constraint_; _ } = constraint_ in
  let typing_env = Tast_env.tast_env_as_typing_env tast_env in
  let logger_mode = parse_logger_mode_exn typing_env in
  let mode = mode_of_logger_mode logger_mode in
  try
    let (decorated_constraints, errors) =
      SA.callable mode id tast_env params ~return body
    in
    match logger_mode with
    | LogConstraints { mode = _ } ->
      let () =
        if not !constraints_dir_created then begin
          Sys_utils.mkdir_p constraints_dir;
          constraints_dir_created := true
        end;
        let constraints =
          SA.any_constraints_of_decorated_constraints decorated_constraints
        in
        let worker = Option.value ~default:0 !Typing_deps.worker_id in
        Shape_analysis_files.write_constraints
          ~constraints_dir
          ~worker
          ConstraintEntry.
            { source_file; id; constraints; error_count = List.length errors }
      in
      []
    | LogLocally
    | LogScuba
    | LogCodemod _ ->
      let error_count = List.length errors in
      let successes =
        fst decorated_constraints
        |> DecoratedConstraintSet.elements
        |> List.map ~f:strip_decorations
        |> SA.simplify typing_env
        |> List.filter ~f:SA.is_shape_like_dict
      in
      let successes =
        match logger_mode with
        | LogCodemod { atomic = true } ->
          if List.is_empty successes then
            []
          else
            [
              {
                location = id;
                result = Either.First { results = successes; error_count };
              };
            ]
        | _ ->
          List.map
            ~f:(fun success ->
              {
                location = id;
                result = Either.First { results = [success]; error_count };
              })
            successes
      in
      let failures =
        List.map
          ~f:(fun err -> { location = id; result = Either.Second err })
          errors
      in
      successes @ failures
  with
  | SA.Shape_analysis_exn error_message ->
    (* Logging failures is expensive because there are so many of them right
       now, to see all the shape results in a timely manner, simply don't log
       failure events. *)
    [{ location = id; result = Either.Second error_message }]

let should_not_skip tast_env =
  let typing_env = Tast_env.tast_env_as_typing_env tast_env in
  not @@ Typing_env.is_hhi typing_env

let create_handler _ctx =
  object
    inherit Tast_visitor.handler_base

    method! at_class_ tast_env A.{ c_methods; c_name = (_, class_name); _ } =
      let collect_method_events
          A.{ m_body; m_name = (_, method_name); m_params; m_ret; _ } =
        let id = class_name ^ "::" ^ method_name in
        let source_file = Tast_env.get_file tast_env in
        compute_results source_file tast_env id m_params m_ret m_body
      in
      if should_not_skip tast_env then
        List.concat_map ~f:collect_method_events c_methods
        |> log_events tast_env

    method! at_fun_def tast_env fd =
      let (_, id) = fd.A.fd_name in
      let A.{ f_body; f_params; f_ret; _ } = fd.A.fd_fun in
      let source_file = Tast_env.get_file tast_env in
      if should_not_skip tast_env then
        compute_results source_file tast_env id f_params f_ret f_body
        |> log_events tast_env
  end
