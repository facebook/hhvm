(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

val source_at_span : Full_fidelity_source_text.t -> 'a Pos.pos -> string

(* True if source text ends in a newline *)
val ends_in_newline : Full_fidelity_source_text.t -> bool

val has_tabs_or_multibyte_codepoints : Full_fidelity_source_text.t -> bool

val get_type_from_hint : Provider_context.t -> Aast.hint -> string

(* Values pulled from source code may have quotation marks;
strip these when present, eg: "\"FOO\"" => "FOO" *)
val strip_nested_quotes : string -> string

(* Convert ContainerName<TParam> to ContainerName *)
val strip_tparams : string -> string

(* Split name or subnamespace from its parent namespace, and return
either Some (parent, name), or None if the name has no parent namespace.
The trailing slash is removed from the parent. *)
val split_name : string -> (string * string) option

(* Get the container name and predicate type for a given parent
container kind. *)
val parent_decl_predicate :
  Symbol_builder_types.parent_container_type ->
  string * Symbol_builder_types.predicate

val get_parent_kind :
  ('a, 'b) Aast.class_ -> Symbol_builder_types.parent_container_type

val init_progress : Symbol_builder_types.result_progress

val add_fact :
  Symbol_builder_types.predicate ->
  Hh_json.JMap.key ->
  Symbol_builder_types.result_progress ->
  int * Symbol_builder_types.result_progress

(* For building the map of cross-references *)
val add_xref :
  'a ->
  IMap.key ->
  Relative_path.t Pos.pos ->
  ('a * Relative_path.t Pos.pos list) IMap.t SMap.t ->
  ('a * Relative_path.t Pos.pos list) IMap.t SMap.t
