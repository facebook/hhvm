(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val build_decls_json :
  Provider_context.t ->
  Symbol_file_info.t list ->
  ownership:bool ->
  Hh_json.json list

val build_xrefs_json :
  Provider_context.t ->
  Symbol_file_info.t list ->
  ownership:bool ->
  Hh_json.json list

val build_json :
  Provider_context.t ->
  Symbol_file_info.t list ->
  ownership:bool ->
  Hh_json.json list
