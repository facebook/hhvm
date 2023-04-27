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

open Hh_prelude
module Fact_id = Symbol_fact_id
module Predicate = Symbol_predicate
module Fact_acc = Symbol_predicate.Fact_acc
module XRefs = Symbol_xrefs
module Util = Symbol_json_util

val namespace_decl : string -> Fact_acc.t -> Fact_id.t * Fact_acc.t

val container_decl :
  Symbol_predicate.t -> string -> Fact_acc.t -> Fact_id.t * Fact_acc.t

val parent_decls :
  Provider_context.t ->
  Aast.hint list ->
  Symbol_predicate.t ->
  Fact_acc.t ->
  Hh_json.json list * Fact_acc.t

val container_defn :
  Provider_context.t ->
  Full_fidelity_source_text.t ->
  ('a, 'b) Aast.class_ ->
  Fact_id.t ->
  Hh_json.json list ->
  Fact_acc.t ->
  Fact_id.t * Fact_acc.t

val property_decl :
  string -> Fact_id.t -> string -> Fact_acc.t -> Fact_id.t * Fact_acc.t

val class_const_decl :
  string -> Fact_id.t -> string -> Fact_acc.t -> Fact_id.t * Fact_acc.t

val type_const_decl :
  string -> Fact_id.t -> string -> Fact_acc.t -> Fact_id.t * Fact_acc.t

val method_decl :
  string -> Fact_id.t -> string -> Fact_acc.t -> Fact_id.t * Fact_acc.t

val method_defn :
  Provider_context.t ->
  Full_fidelity_source_text.t ->
  ('a, 'b) Aast.method_ ->
  Fact_id.t ->
  Fact_acc.t ->
  Fact_id.t * Fact_acc.t

val method_overrides :
  string ->
  string ->
  string ->
  string ->
  string ->
  Fact_acc.t ->
  Fact_id.t * Fact_acc.t

val property_defn :
  Provider_context.t ->
  Full_fidelity_source_text.t ->
  ('a, 'b) Aast.class_var ->
  Fact_id.t ->
  Fact_acc.t ->
  Fact_id.t * Fact_acc.t

val class_const_defn :
  Provider_context.t ->
  Full_fidelity_source_text.t ->
  ('a, 'b) Aast.class_const ->
  Fact_id.t ->
  Fact_acc.t ->
  Fact_id.t * Fact_acc.t

val type_const_defn :
  Provider_context.t ->
  Full_fidelity_source_text.t ->
  ('a, 'b) Aast.class_typeconst_def ->
  Fact_id.t ->
  Fact_acc.t ->
  Fact_id.t * Fact_acc.t

val enum_decl : string -> Fact_acc.t -> Fact_id.t * Fact_acc.t

val enum_defn :
  Provider_context.t ->
  Full_fidelity_source_text.t ->
  ('a, 'b) Aast.class_ ->
  Fact_id.t ->
  Aast.enum_ ->
  Hh_json.json list ->
  Fact_acc.t ->
  Fact_id.t * Fact_acc.t

val enumerator : Fact_id.t -> string -> Fact_acc.t -> Fact_id.t * Fact_acc.t

val func_decl : string -> Fact_acc.t -> Fact_id.t * Fact_acc.t

val func_defn :
  Provider_context.t ->
  Full_fidelity_source_text.t ->
  ('a, 'b) Aast.fun_def ->
  Fact_id.t ->
  Fact_acc.t ->
  Fact_id.t * Fact_acc.t

val typedef_decl : string -> Fact_acc.t -> Fact_id.t * Fact_acc.t

val typedef_defn :
  Provider_context.t ->
  Full_fidelity_source_text.t ->
  ('a, 'b) Aast.typedef ->
  Fact_id.t ->
  Fact_acc.t ->
  Fact_id.t * Fact_acc.t

val module_decl : string -> Fact_acc.t -> Fact_id.t * Fact_acc.t

val module_defn :
  Provider_context.t ->
  Full_fidelity_source_text.t ->
  ('a, 'b) Aast.module_def ->
  Fact_id.t ->
  Fact_acc.t ->
  Fact_id.t * Fact_acc.t

val gconst_decl : string -> Fact_acc.t -> Fact_id.t * Fact_acc.t

val gconst_defn :
  Provider_context.t ->
  Full_fidelity_source_text.t ->
  ('a, 'b) Aast.gconst ->
  Fact_id.t ->
  Fact_acc.t ->
  Fact_id.t * Fact_acc.t

val decl_loc :
  path:string -> Pos.t -> Hh_json.json -> Fact_acc.t -> Fact_id.t * Fact_acc.t

val decl_comment :
  path:string -> Pos.t -> Hh_json.json -> Fact_acc.t -> Fact_id.t * Fact_acc.t

val decl_span :
  path:string -> Pos.t -> Hh_json.json -> Fact_acc.t -> Fact_id.t * Fact_acc.t

val file_lines :
  path:string ->
  Full_fidelity_source_text.t ->
  Fact_acc.t ->
  Fact_id.t * Fact_acc.t

val gen_code :
  path:string ->
  fully_generated:bool ->
  signature:string option ->
  source:string option ->
  command:string option ->
  class_:string option ->
  Fact_acc.t ->
  Fact_id.t * Fact_acc.t

val file_xrefs :
  path:string -> XRefs.fact_map -> Fact_acc.t -> Fact_id.t * Fact_acc.t

val file_decls :
  path:string -> Hh_json.json list -> Fact_acc.t -> Fact_id.t * Fact_acc.t

val file_call :
  path:string ->
  Pos.t ->
  callee_xref:Hh_json.json option ->
  call_args:Hh_json.json list ->
  dispatch_arg:Hh_json.json option ->
  receiver_type:Hh_json.json option ->
  Fact_acc.t ->
  Fact_id.t * Fact_acc.t

val global_namespace_alias :
  from:string -> to_:string -> Fact_acc.t -> Fact_id.t * Fact_acc.t

val method_occ :
  SymbolOccurrence.receiver_class ->
  string ->
  Fact_acc.t ->
  Fact_id.t * Fact_acc.t

val indexerInputsHash :
  string -> Md5.t list -> Fact_acc.t -> Fact_id.t * Fact_acc.t

val type_info :
  ty:string ->
  (Hh_json.json * Util.pos list) list ->
  Fact_acc.t ->
  Fact_id.t * Fact_acc.t
