(*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*
 * These functions build up the JSON necessary and then add facts
 * to the running result.
 *)

val namespace_decl :
  Namespace_env.env ->
  Symbol_builder_types.result_progress ->
  Symbol_builder_types.result_progress

val container_decl :
  Symbol_builder_types.predicate ->
  string ->
  Symbol_builder_types.result_progress ->
  int * Symbol_builder_types.result_progress

val parent_decls :
  Provider_context.t ->
  Aast.hint list ->
  Symbol_builder_types.predicate ->
  Symbol_builder_types.result_progress ->
  Hh_json.json list * Symbol_builder_types.result_progress

val container_defn :
  Provider_context.t ->
  Full_fidelity_source_text.t SMap.t ->
  ('a, 'b) Aast.class_ ->
  int ->
  Hh_json.json list ->
  Symbol_builder_types.result_progress ->
  int * Symbol_builder_types.result_progress

val property_decl :
  string ->
  int ->
  string ->
  Symbol_builder_types.result_progress ->
  int * Symbol_builder_types.result_progress

val class_const_decl :
  string ->
  int ->
  string ->
  Symbol_builder_types.result_progress ->
  int * Symbol_builder_types.result_progress

val type_const_decl :
  string ->
  int ->
  string ->
  Symbol_builder_types.result_progress ->
  int * Symbol_builder_types.result_progress

val method_decl :
  string ->
  int ->
  string ->
  Symbol_builder_types.result_progress ->
  int * Symbol_builder_types.result_progress

val method_defn :
  Provider_context.t ->
  Full_fidelity_source_text.t SMap.t ->
  ('a, 'b) Aast.method_ ->
  int ->
  Symbol_builder_types.result_progress ->
  int * Symbol_builder_types.result_progress

val method_overrides :
  string ->
  string ->
  string ->
  string ->
  string ->
  Symbol_builder_types.result_progress ->
  int * Symbol_builder_types.result_progress

val property_defn :
  Provider_context.t ->
  Full_fidelity_source_text.t SMap.t ->
  ('a, 'b) Aast.class_var ->
  int ->
  Symbol_builder_types.result_progress ->
  int * Symbol_builder_types.result_progress

val class_const_defn :
  Provider_context.t ->
  Full_fidelity_source_text.t SMap.t ->
  ('a, 'b) Aast.class_const ->
  int ->
  Symbol_builder_types.result_progress ->
  int * Symbol_builder_types.result_progress

val type_const_defn :
  Provider_context.t ->
  Full_fidelity_source_text.t SMap.t ->
  ('a, 'b) Aast.class_typeconst_def ->
  int ->
  Symbol_builder_types.result_progress ->
  int * Symbol_builder_types.result_progress

val enum_decl :
  string ->
  Symbol_builder_types.result_progress ->
  int * Symbol_builder_types.result_progress

val enum_defn :
  Provider_context.t ->
  Full_fidelity_source_text.t SMap.t ->
  ('a, 'b) Aast.class_ ->
  int ->
  Aast.enum_ ->
  Hh_json.json list ->
  Symbol_builder_types.result_progress ->
  int * Symbol_builder_types.result_progress

val enumerator :
  int ->
  string ->
  Symbol_builder_types.result_progress ->
  int * Symbol_builder_types.result_progress

val func_decl :
  string ->
  Symbol_builder_types.result_progress ->
  int * Symbol_builder_types.result_progress

val func_defn :
  Provider_context.t ->
  Full_fidelity_source_text.t SMap.t ->
  ('a, 'b) Aast.fun_def ->
  int ->
  Symbol_builder_types.result_progress ->
  int * Symbol_builder_types.result_progress

val typedef_decl :
  string ->
  Symbol_builder_types.result_progress ->
  int * Symbol_builder_types.result_progress

val typedef_defn :
  Provider_context.t ->
  Full_fidelity_source_text.t SMap.t ->
  ('a, 'b) Aast.typedef ->
  int ->
  Symbol_builder_types.result_progress ->
  int * Symbol_builder_types.result_progress

val gconst_decl :
  string ->
  Symbol_builder_types.result_progress ->
  int * Symbol_builder_types.result_progress

val gconst_defn :
  Provider_context.t ->
  Full_fidelity_source_text.t SMap.t ->
  ('a, 'b) Aast.gconst ->
  int ->
  Symbol_builder_types.result_progress ->
  int * Symbol_builder_types.result_progress

val decl_loc :
  Relative_path.t Pos.pos ->
  Hh_json.json ->
  Symbol_builder_types.result_progress ->
  int * Symbol_builder_types.result_progress

val decl_comment :
  Relative_path.t Pos.pos ->
  Hh_json.json ->
  Symbol_builder_types.result_progress ->
  int * Symbol_builder_types.result_progress

val decl_span :
  Relative_path.t Pos.pos ->
  Hh_json.json ->
  Symbol_builder_types.result_progress ->
  int * Symbol_builder_types.result_progress

val file_lines :
  string ->
  Full_fidelity_source_text.t ->
  Symbol_builder_types.result_progress ->
  int * Symbol_builder_types.result_progress

val file_xrefs :
  string ->
  (Hh_json.json * Pos.t list) IMap.t ->
  Symbol_builder_types.result_progress ->
  int * Symbol_builder_types.result_progress

val file_decls :
  string ->
  Hh_json.json list ->
  Symbol_builder_types.result_progress ->
  int * Symbol_builder_types.result_progress

val method_occ :
  SymbolOccurrence.receiver_class ->
  string ->
  Symbol_builder_types.result_progress ->
  int * Symbol_builder_types.result_progress
