(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

open Hh_prelude
open Bad_type_logger_types
open Bad_type_logger_common

(** Flattens the structure information about Tanys and writes it out to the logging channel. *)
let log_info_to_file_internal (info : info) : unit =
  let tag tag rest = Format.sprintf "%s[%s]" rest tag in
  let iftag tag cond rest =
    if cond then
      Format.sprintf "%s[%s]" rest tag
    else
      rest
  in
  let keyedtag tag value rest = Format.sprintf "%s[%s:%s]" rest tag value in
  let opttag tag opt rest =
    Option.value_map ~default:rest ~f:(fun value -> keyedtag tag value rest) opt
  in
  let exp_info_tags exp_info rest =
    rest
    |> iftag "decl_use" exp_info.declaration_usage
    |> opttag "producer" exp_info.producer
  in
  let param_info_tags param_info rest =
    rest
    |> iftag "inout" param_info.is_inout
    |> iftag "variadic" param_info.is_variadic
  in
  let position_tags position rest =
    match position with
    | Parameter param_info -> rest |> tag "param" |> param_info_tags param_info
    | Return -> tag "return" rest
  in
  let decl_info_tags decl_info rest =
    rest |> position_tags decl_info.position
  in
  let context_tags context rest =
    match context with
    | Expression exp_info -> rest |> tag "exp" |> exp_info_tags exp_info
    | Declaration decl_info -> rest |> tag "decl" |> decl_info_tags decl_info
  in
  let indicator_tags indicator rest =
    rest |> iftag "Tany" indicator.has_tany |> iftag "Terr" indicator.has_terr
  in
  let common_info_tags common_info rest =
    rest
    |> indicator_tags common_info.indicator
    |> iftag "generated" common_info.is_generated
    |> iftag "test" common_info.is_test
    |> keyedtag "context" (string_of_context_id common_info.context_id)
    |> keyedtag "position" (string_of_pos common_info.pos)
  in
  ""
  |> common_info_tags info.common_info
  |> context_tags info.context
  |> Format.sprintf "%s\n"
  |> Out_channel.output_string !Typing_log.out_channel;
  Out_channel.flush !Typing_log.out_channel

let log_info_to_file env (info : info) : unit =
  let log_level_opt =
    Tast_env.tast_env_as_typing_env env
    |> Typing_env.get_tcopt
    |> TypecheckerOptions.log_levels
    |> SMap.find_opt "tany"
  in
  match log_level_opt with
  | Some level when should_log level File -> log_info_to_file_internal info
  | _ -> ()
