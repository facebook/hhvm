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
open Ide_rpc_protocol_parser_types
open Hh_json

val assert_params_required :
  string -> 'a option -> ('a, error_t) result

val get_line_field :
  json -> (int, error_t) result

val get_column_field :
  json -> (int, error_t) result

val get_text_field :
  json -> (string, error_t) result

val get_filename_field :
  json -> (string, error_t) result

val get_position_field :
  json -> (position, error_t) result

val get_file_position_field :
  json -> (file_position, error_t) result

val parse_range_field :
  json -> (range, error_t) result

val get_file_range_field :
  json -> (file_range, error_t) result
