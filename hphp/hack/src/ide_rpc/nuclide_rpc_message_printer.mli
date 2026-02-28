(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val identify_symbol_response_to_json :
  (string SymbolOccurrence.t * string SymbolDefinition.t option) list ->
  Hh_json.json

val print_json : Hh_json.json -> unit

val tast_holes_response_to_json :
  print_file:bool ->
  (string * string * string * string * Pos.t) list ->
  Hh_json.json

val outline_response_to_json : string SymbolDefinition.t list -> Hh_json.json

val highlight_references_response_to_json :
  Ide_api_types.range list -> Hh_json.json

val infer_type_error_response_to_json :
  string option * string option * string option * string option -> Hh_json.json

val infer_type_response_to_json : string option * string option -> Hh_json.json
