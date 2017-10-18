(**
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 *)

open Hh_json
open Ide_rpc_protocol_parser_types

val get_string_field :
  string -> json -> (string, error_t) result

val maybe_get_number_field :
  string -> json -> (string option, error_t) result

val get_int_field :
  string -> json -> (int, error_t) result

val maybe_get_int_field :
  string -> json -> (int option, error_t) result

val maybe_get_obj_field :
  string ->  json -> (json option, error_t) result

val get_obj_field :
  string -> json -> (json, error_t) result

val get_number_field :
  string -> json -> (string, error_t) result

val get_array_field :
  string -> json -> (json list, error_t) result

(* Map optional value v to [("label", f v)] or [] *)
val opt_field:
  v_opt:'a option ->
  label:string ->
  f:('a -> Hh_json.json) ->
  (string * Hh_json.json) list

val not_implemented :
  ('a, error_t) result
