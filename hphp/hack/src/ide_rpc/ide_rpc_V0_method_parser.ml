(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Ide_message
open Ide_parser_utils
open Ide_rpc_method_parser_utils
open Ide_rpc_protocol_parser_types
open Core_result.Monad_infix

let delegate_to_previous_version method_name params =
  Nuclide_rpc_method_parser.parse
    ~method_name:(Method_name method_name)
    ~params

let get_client_name_field = get_string_field "client_name"

let get_client_api_version_field = get_int_field "client_api_version"

let parse_init_params params =
  get_client_name_field params >>= fun client_name ->
  get_client_api_version_field params >>= fun client_api_version ->
  Ok { client_name; client_api_version; }

let parse_init method_name params =
  assert_params_required method_name params >>=
  parse_init_params >>= fun params ->
  Ok (Init params)

let parse_did_open_file_params params =
  get_filename_field params >>= fun did_open_file_filename ->
  get_text_field params >>= fun did_open_file_text ->
  Ok { did_open_file_filename; did_open_file_text; }

let parse_did_open_file method_name params =
  assert_params_required method_name params >>=
  parse_did_open_file_params >>= fun params ->
  Ok (Did_open_file params)

let parse_file_position method_name params =
  assert_params_required method_name params >>=
  get_file_position_field

let parse_file_range method_name params =
  assert_params_required method_name params >>=
  get_file_range_field

let parse_infer_type method_name params =
  parse_file_position method_name params >>= fun file_position ->
  Ok (Infer_type file_position)

let parse_identify_symbol method_name params =
  parse_file_position method_name params >>= fun file_position ->
  Ok (Identify_symbol file_position)

let parse_outline method_name params =
  assert_params_required method_name params >>=
  get_filename_field >>= fun filename ->
  Ok (Outline filename)

let parse_find_references method_name params =
  parse_file_position method_name params >>= fun file_position ->
  Ok (Find_references file_position)

let parse_highlight_references method_name params =
  parse_file_position method_name params >>= fun file_position ->
  Ok (Highlight_references file_position)

let parse_format method_name params =
  parse_file_range method_name params >>= fun file_range ->
  Ok (Format file_range)

let parse_coverage_levels method_name params =
  assert_params_required method_name params >>=
  get_filename_field >>= fun filename ->
  Ok (Coverage_levels filename)

let parse_method method_name params = match method_name with
  | "init" -> parse_init method_name params
  | "didOpenFile" -> parse_did_open_file method_name params
  | "autocomplete" -> delegate_to_previous_version "getCompletions" params
  | "inferType" -> parse_infer_type method_name params
  | "identifySymbol" -> parse_identify_symbol method_name params
  | "outline" -> parse_outline method_name params
  | "findReferences" -> parse_find_references method_name params
  | "highlightReferences" -> parse_highlight_references method_name params
  | "format" -> parse_format method_name params
  | "coverageLevels" -> parse_coverage_levels method_name params
  | _ -> delegate_to_previous_version method_name params

let parse ~method_name ~params =
  match method_name with
  | Ide_rpc_protocol_parser_types.Unsubscribe_call -> Error
    (* Should be impossible to get here *)
    (Internal_error "Unsubscribe_call is not part of jsonrpc protocol")
  | Method_name method_name -> parse_method method_name params
