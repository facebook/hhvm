(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_core
open Ide_message
open Ide_parser_utils
open Ide_rpc_method_parser_utils
open Ide_rpc_protocol_parser_types
open Core_result.Monad_infix

let get_contents_field = get_string_field "contents"

let get_changes_field = get_array_field "changes"

let parse_autocomplete_params params =
  get_filename_field params >>= fun filename ->
  get_position_field params >>= fun position ->
  Ok { Ide_api_types.filename; position; }

let parse_coverage_levels_params params =
  get_filename_field params >>= fun filename ->
  Ok filename

let parse_did_open_file_params params =
  get_filename_field params >>= fun did_open_file_filename ->
  get_contents_field params >>= fun did_open_file_text ->
  Ok { did_open_file_filename; did_open_file_text; }

let parse_did_close_file_params params =
  get_filename_field params >>= fun did_close_file_filename ->
  Ok { did_close_file_filename; }

let parse_edit edit =
  get_text_field edit >>= fun text ->
  maybe_get_obj_field "range" edit >>= fun range_opt ->
  (match range_opt with
    | None -> Ok None
    | Some range ->
      (parse_range_field range >>= (fun r -> Ok (Some r)))
  ) >>= fun range ->
  Ok { Ide_api_types.text; range; }

let accumulate_edits edit acc =
  acc >>= fun acc ->
  parse_edit edit >>= fun edit ->
  Ok (edit::acc)

let parse_did_change_file_params params =
  get_filename_field params >>= fun did_change_file_filename ->
  get_changes_field params >>= List.fold_right
    ~f:accumulate_edits
    ~init:(Ok []) >>= fun changes ->
  Ok {
    Ide_message.did_change_file_filename;
    changes;
  }

let parse_autocomplete method_name params =
  assert_params_required method_name params >>=
  parse_autocomplete_params >>= fun params ->
  Ok (Autocomplete params)

let parse_coverage_levels method_name params =
  assert_params_required method_name params >>=
  parse_coverage_levels_params >>= fun params ->
  Ok (Coverage_levels params)

let parse_did_open_file method_name params =
  assert_params_required method_name params >>=
  parse_did_open_file_params >>= fun params ->
  Ok (Did_open_file params)

let parse_did_close_file method_name params =
  assert_params_required method_name params >>=
  parse_did_close_file_params >>= fun params ->
  Ok (Did_close_file params)

let parse_did_change_file method_name params =
  assert_params_required method_name params >>=
  parse_did_change_file_params >>= fun params ->
  Ok (Did_change_file params)

let parse_method method_name params = match method_name with
  | "getCompletions" -> parse_autocomplete method_name params
  | "coverageLevels" -> parse_coverage_levels method_name params
  | "didOpenFile" -> parse_did_open_file method_name params
  | "didCloseFile" -> parse_did_close_file method_name params
  | "didChangeFile" -> parse_did_change_file method_name params
  | "notifyDiagnostics" -> Ok Subscribe_diagnostics
  | "disconnect" -> Ok Disconnect
  | "sleep" -> Ok Sleep_for_test
  | _ -> Error (Method_not_found method_name)

let parse ~method_name ~params =
  match method_name with
  | Ide_rpc_protocol_parser_types.Unsubscribe_call ->
    Ok Ide_message.Unsubscribe_call
  | Method_name method_name -> parse_method method_name params
