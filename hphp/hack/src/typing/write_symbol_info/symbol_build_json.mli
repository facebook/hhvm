(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*
 * JSON builder functions. These all return JSON objects, which
 * may be used to build up larger objects. The functions with suffix
 * _nested include the key field because they are used for writing
 * nested facts.
 *)

module Fact_id = Symbol_fact_id

val build_id_json : Fact_id.t -> Hh_json.json

val build_file_json_nested : string -> Hh_json.json

val build_name_json_nested : string -> Hh_json.json

val build_namespaceqname_json_nested : string -> Hh_json.json

val build_qname_json_nested : string -> Hh_json.json

val build_method_decl_nested : string -> string -> string -> Hh_json.json

val build_type_json_nested : string -> Hh_json.json

val build_attributes_json_nested :
  Full_fidelity_source_text.t ->
  ('a, 'b) Aast.user_attribute list ->
  Hh_json.json

val build_bytespan_json : 'a Pos.pos -> Hh_json.json

val build_decl_target_json : Hh_json.json -> Hh_json.json

val build_occ_target_json : Hh_json.json -> Hh_json.json

val build_file_lines_json : string -> int list -> bool -> bool -> Hh_json.json

val build_is_async_json : Ast_defs.fun_kind -> Hh_json.json

val build_signature_json :
  Provider_context.t ->
  Full_fidelity_source_text.t ->
  ('a, 'b) Aast.fun_param list ->
  Aast.contexts option ->
  'e Aast.type_hint ->
  Hh_json.json

val build_type_const_kind_json : Aast.class_typeconst -> Hh_json.json

val build_type_param_json :
  Provider_context.t ->
  Full_fidelity_source_text.t ->
  ('a, 'b) Aast.tparam ->
  Hh_json.json

val build_visibility_json : Aast.visibility -> Hh_json.json

val build_xrefs_json : (Hh_json.json * Pos.t list) Fact_id.Map.t -> Hh_json.json

val build_class_const_decl_json_ref : Fact_id.t -> Hh_json.json

val build_container_json_ref : string -> Fact_id.t -> Hh_json.json

val build_container_decl_json_ref : string -> Fact_id.t -> Hh_json.json

val build_enum_decl_json_ref : Fact_id.t -> Hh_json.json

val build_namespace_decl_json_ref : Fact_id.t -> Hh_json.json

val build_enumerator_decl_json_ref : Fact_id.t -> Hh_json.json

val build_func_decl_json_ref : Fact_id.t -> Hh_json.json

val build_gconst_decl_json_ref : Fact_id.t -> Hh_json.json

val build_method_decl_json_ref : Fact_id.t -> Hh_json.json

val build_property_decl_json_ref : Fact_id.t -> Hh_json.json

val build_type_const_decl_json_ref : Fact_id.t -> Hh_json.json

val build_typedef_decl_json_ref : Fact_id.t -> Hh_json.json

val build_method_occ_json_ref : Fact_id.t -> Hh_json.json
