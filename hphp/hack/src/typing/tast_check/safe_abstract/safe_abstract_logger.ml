(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
open Hh_prelude

let log_prefix = "safe_abstract_logger:"

let infos_to_summarized_log_line
    (infos : Safe_abstract_internal.class_use_info list) : string option =
  let (news, calls, const_accesses) : bool list * bool list * bool list =
    List.partition3_map infos ~f:(fun (kind, _pos, warnings) ->
        let is_safe = List.is_empty warnings in
        match kind with
        | Safe_abstract_internal.New -> `Fst is_safe
        | Safe_abstract_internal.Static_method_call -> `Snd is_safe
        | Safe_abstract_internal.Const_access -> `Trd is_safe)
  in
  let sum f items = items |> List.filter ~f |> List.length in
  let safe_news = sum Fn.id news in
  let unsafe_news = sum not news in
  let safe_calls = sum Fn.id calls in
  let unsafe_calls = sum not calls in
  let safe_const_accesses = sum Fn.id const_accesses in
  let unsafe_const_accesses = sum not const_accesses in
  let is_worth_reporting =
    safe_news + unsafe_news + safe_calls + unsafe_calls + unsafe_const_accesses
    > 0
  in
  if is_worth_reporting then
    let filename =
      match List.hd infos with
      | Some (_, pos, _) -> Pos.filename (Pos.to_absolute pos)
      | None -> "unknown"
      (* unreachable, since we ensured length > 0 in is_worth_reporting calculation above *)
    in
    let json =
      `Assoc
        [
          ("filename", `String filename);
          ("safe_news", `Int safe_news);
          ("unsafe_news", `Int unsafe_news);
          ("safe_calls", `Int safe_calls);
          ("unsafe_calls", `Int unsafe_calls);
          ("safe_const_accesses", `Int safe_const_accesses);
          ("unsafe_const_accesses", `Int unsafe_const_accesses);
        ]
    in
    Some (log_prefix ^ Yojson.Safe.to_string json)
  else
    None

let infos_to_verbose_log_lines
    (infos : Safe_abstract_internal.class_use_info list) : string list =
  List.map infos ~f:(fun (kind, pos, warnings) ->
      let kind_str =
        match kind with
        | Safe_abstract_internal.New -> "new"
        | Safe_abstract_internal.Static_method_call -> "call"
        | Safe_abstract_internal.Const_access -> "const_access"
      in
      let pos_str =
        let (line, column) = Pos.line_column pos in
        let file = Relative_path.show (Pos.filename pos) in
        Printf.sprintf "%s %d:%d" file line column
      in
      let is_safe_json = List.is_empty warnings in
      let json =
        `Assoc
          [
            ("pos", `String pos_str);
            ("use_kind", `String kind_str);
            ("is_safe", `Bool is_safe_json);
          ]
      in
      log_prefix ^ Yojson.Safe.to_string json)

let infos_to_log_lines env (infos : Safe_abstract_internal.class_use_info list)
    : string list =
  let log_level =
    Tast_env.tast_env_as_typing_env env
    |> Typing_env.get_tcopt
    |> TypecheckerOptions.log_levels
    |> SMap.find_opt "safe_abstract"
    |> Option.value ~default:0
  in
  match log_level with
  | 0 -> []
  | 1 -> infos_to_summarized_log_line infos |> Option.to_list
  | 2 -> infos_to_verbose_log_lines infos
  | n ->
    failwith (Printf.sprintf "Unexpected log level for safe_abstract: %d" n)

let log_infos env (infos : Safe_abstract_internal.class_use_info list) : unit =
  let needs_flush = ref false in
  let output_line line =
    Out_channel.output_string !Typing_log.out_channel (line ^ "\n");
    needs_flush := true
  in
  infos |> infos_to_log_lines env |> List.iter ~f:output_line;
  if !needs_flush then Out_channel.flush !Typing_log.out_channel

let collect_infos ~current_method =
  object
    inherit [Safe_abstract_internal.class_use_info list] Tast_visitor.reduce

    method zero = []

    method plus = ( @ )

    method! on_expr env expr =
      Safe_abstract_internal.calc_warnings env expr ~current_method
      |> Option.to_list
  end

let handler =
  object
    inherit Tast_visitor.handler_base

    method! at_fun_def env f =
      let infos = (collect_infos ~current_method:None)#on_fun_def env f in
      log_infos env infos

    method! at_method_ env m =
      let infos = (collect_infos ~current_method:(Some m))#on_method_ env m in
      log_infos env infos
  end

let create_handler ctx =
  let safe_abstract_is_enabled =
    let tcopt = Provider_context.get_tcopt ctx in
    TypecheckerOptions.safe_abstract tcopt
  in
  if safe_abstract_is_enabled then
    handler
  else
    failwith
      {|Safe abstract logger requires safe_abstract to be enabled.
  Add `safe_abstract=true` to the .hhconfig or pass `--config safe_abstract=true`
      |}
