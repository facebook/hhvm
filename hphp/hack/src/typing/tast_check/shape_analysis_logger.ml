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
module Env = Tast_env

let log_events_locally typing_env : log_event list -> unit =
  let log_shape = function
    | Result { id; shape_result } ->
      Format.sprintf
        "[RESULT] %s: %s\n"
        id
        (SA.show_shape_result typing_env shape_result)
      |> Out_channel.output_string !Typing_log.out_channel;
      Out_channel.flush !Typing_log.out_channel
    | Failure { id; error_message } ->
      Format.sprintf "[FAILURE] %s: %s\n" id error_message
      |> Out_channel.output_string !Typing_log.out_channel;
      Out_channel.flush !Typing_log.out_channel
  in
  List.iter ~f:log_shape

let log_events typing_env : log_event list -> unit =
  let shape_analysis_log_level =
    Typing_env.get_tcopt typing_env
    |> TypecheckerOptions.log_levels
    |> SMap.find_opt "shape_analysis"
  in
  match shape_analysis_log_level with
  | Some 1 -> log_events_locally typing_env
  | Some 2 -> Shape_analysis_scuba.log_events_remotely typing_env
  | _ -> Fn.const ()

let compute_results tast_env id params return body =
  let strip_decorations { constraint_; _ } = constraint_ in
  let typing_env = Tast_env.tast_env_as_typing_env tast_env in
  try
    fst (SA.callable id tast_env params ~return body)
    |> List.map ~f:strip_decorations
    |> SA.simplify typing_env
    |> List.filter ~f:SA.is_shape_like_dict
    |> List.map ~f:(fun shape_result -> Result { id; shape_result })
  with
  | SA.Shape_analysis_exn error_message ->
    (* Logging failures is expensive because there are so many of them right
       now, to see all the shape results in a timely manner, simply don't log
       failure events. *)
    [Failure { id; error_message }]

let should_not_skip tast_env =
  let typing_env = Tast_env.tast_env_as_typing_env tast_env in
  not @@ Typing_env.is_hhi typing_env

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_class_ tast_env A.{ c_methods; c_name = (_, class_name); _ } =
      let typing_env = Tast_env.tast_env_as_typing_env tast_env in
      let collect_method_events
          A.{ m_body; m_name = (_, method_name); m_params; m_ret; _ } =
        let id = class_name ^ "::" ^ method_name in
        compute_results tast_env id m_params m_ret m_body
      in
      if should_not_skip tast_env then
        List.concat_map ~f:collect_method_events c_methods
        |> log_events typing_env

    method! at_fun_def tast_env fd =
      let A.{ f_body; f_name = (_, id); f_params; f_ret; _ } = fd.A.fd_fun in
      if should_not_skip tast_env then
        let typing_env = Tast_env.tast_env_as_typing_env tast_env in
        compute_results tast_env id f_params f_ret f_body
        |> log_events typing_env
  end
