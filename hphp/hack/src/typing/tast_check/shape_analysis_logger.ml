(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
module A = Aast
module SA = Shape_analysis
module Env = Tast_env

let log_result_locally typing_env id results : unit =
  let log_shape result =
    Format.sprintf "%s: %s\n" id (SA.show_shape_result typing_env result)
    |> Out_channel.output_string !Typing_log.out_channel;
    Out_channel.flush !Typing_log.out_channel
  in
  List.iter results ~f:log_shape

let log_result typing_env id results : unit =
  let shape_analysis_log_level =
    Typing_env.get_tcopt typing_env
    |> TypecheckerOptions.log_levels
    |> SMap.find_opt "shape_analysis"
  in
  match shape_analysis_log_level with
  | Some 1 -> log_result_locally typing_env id results
  | Some 2 ->
    (* TODO: Scuba logging *)
    ()
  | _ -> ()

let compute_result tast_env params body =
  let typing_env = Tast_env.tast_env_as_typing_env tast_env in
  SA.callable tast_env params body |> SA.simplify typing_env

let should_not_skip tast_env =
  let typing_env = Tast_env.tast_env_as_typing_env tast_env in
  not @@ Typing_env.is_hhi typing_env

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_class_ tast_env A.{ c_methods; c_name = (_, class_name); _ } =
      let typing_env = Tast_env.tast_env_as_typing_env tast_env in
      let handle_method A.{ m_body; m_name = (_, method_name); m_params; _ } =
        let id = class_name ^ "::" ^ method_name in
        compute_result tast_env m_params m_body |> log_result typing_env id
      in
      if should_not_skip tast_env then List.iter ~f:handle_method c_methods

    method! at_fun_def tast_env fd =
      let A.{ f_body; f_name = (_, id); f_params; _ } = fd.A.fd_fun in
      if should_not_skip tast_env then
        let typing_env = Tast_env.tast_env_as_typing_env tast_env in
        compute_result tast_env f_params f_body |> log_result typing_env id
  end
