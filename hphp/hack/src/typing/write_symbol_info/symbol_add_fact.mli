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

type t

val init_progress : t

val progress_to_json : t -> Hh_json.json list

val add_fact : Symbol_predicate.t -> Hh_json.JMap.key -> t -> int * t

val namespace_decl : Namespace_env.env -> t -> t

val container_decl : Symbol_predicate.t -> string -> t -> int * t

val parent_decls :
  Provider_context.t ->
  Aast.hint list ->
  Symbol_predicate.t ->
  t ->
  Hh_json.json list * t

val container_defn :
  Provider_context.t ->
  Full_fidelity_source_text.t SMap.t ->
  ('a, 'b) Aast.class_ ->
  int ->
  Hh_json.json list ->
  t ->
  int * t

val property_decl : string -> int -> string -> t -> int * t

val class_const_decl : string -> int -> string -> t -> int * t

val type_const_decl : string -> int -> string -> t -> int * t

val method_decl : string -> int -> string -> t -> int * t

val method_defn :
  Provider_context.t ->
  Full_fidelity_source_text.t SMap.t ->
  ('a, 'b) Aast.method_ ->
  int ->
  t ->
  int * t

val method_overrides :
  string -> string -> string -> string -> string -> t -> int * t

val property_defn :
  Provider_context.t ->
  Full_fidelity_source_text.t SMap.t ->
  ('a, 'b) Aast.class_var ->
  int ->
  t ->
  int * t

val class_const_defn :
  Provider_context.t ->
  Full_fidelity_source_text.t SMap.t ->
  ('a, 'b) Aast.class_const ->
  int ->
  t ->
  int * t

val type_const_defn :
  Provider_context.t ->
  Full_fidelity_source_text.t SMap.t ->
  ('a, 'b) Aast.class_typeconst_def ->
  int ->
  t ->
  int * t

val enum_decl : string -> t -> int * t

val enum_defn :
  Provider_context.t ->
  Full_fidelity_source_text.t SMap.t ->
  ('a, 'b) Aast.class_ ->
  int ->
  Aast.enum_ ->
  Hh_json.json list ->
  t ->
  int * t

val enumerator : int -> string -> t -> int * t

val func_decl : string -> t -> int * t

val func_defn :
  Provider_context.t ->
  Full_fidelity_source_text.t SMap.t ->
  ('a, 'b) Aast.fun_def ->
  int ->
  t ->
  int * t

val typedef_decl : string -> t -> int * t

val typedef_defn :
  Provider_context.t ->
  Full_fidelity_source_text.t SMap.t ->
  ('a, 'b) Aast.typedef ->
  int ->
  t ->
  int * t

val gconst_decl : string -> t -> int * t

val gconst_defn :
  Provider_context.t ->
  Full_fidelity_source_text.t SMap.t ->
  ('a, 'b) Aast.gconst ->
  int ->
  t ->
  int * t

val decl_loc : Relative_path.t Pos.pos -> Hh_json.json -> t -> int * t

val decl_comment : Relative_path.t Pos.pos -> Hh_json.json -> t -> int * t

val decl_span : Relative_path.t Pos.pos -> Hh_json.json -> t -> int * t

val file_lines : string -> Full_fidelity_source_text.t -> t -> int * t

val file_xrefs : string -> (Hh_json.json * Pos.t list) IMap.t -> t -> int * t

val file_decls : string -> Hh_json.json list -> t -> int * t

val method_occ : SymbolOccurrence.receiver_class -> string -> t -> int * t
