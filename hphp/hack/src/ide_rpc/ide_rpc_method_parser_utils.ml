(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Ide_api_types
open Ide_parser_utils
open Ide_rpc_protocol_parser_types
open Core_result.Monad_infix

let assert_params_required method_name params =
  Core_result.of_option params
    ~error:(Invalid_params
      (Printf.sprintf "%s request requires params" method_name))

let get_line_field = get_int_field "line"

let get_column_field = get_int_field "column"

let get_text_field = get_string_field "text"

let get_filename_field = get_string_field "filename"

let parse_position position =
  get_line_field position >>= fun line ->
  get_column_field position >>= fun column ->
  Ok { Ide_api_types.line; column; }

let get_start_field obj = get_obj_field "start" obj >>= parse_position

let get_end_field obj = get_obj_field "end" obj >>= parse_position

let get_position_field obj = get_obj_field "position" obj >>= parse_position

let get_file_position_field obj =
  get_filename_field obj >>= fun filename ->
  get_position_field obj >>= fun position ->
  Ok { filename; position; }

let parse_range_field range =
  get_start_field range >>= fun st ->
  get_end_field range >>= fun ed ->
  Ok { Ide_api_types.st; ed; }

let get_range_field obj = get_obj_field "range" obj >>= parse_range_field

let get_file_range_field obj =
  get_filename_field obj >>= fun range_filename ->
  get_range_field obj >>= fun file_range ->
  Ok { range_filename; file_range; }
