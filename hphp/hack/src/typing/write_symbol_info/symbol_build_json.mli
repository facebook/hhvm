(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(*
 * Helper fact builder functions
 *)

open Symbol_glean_schema.Hack
module Fact_id = Symbol_fact_id
module Util = Symbol_json_util
module Predicate = Symbol_predicate

val build_call_arguments :
  (Argument.t option * Pos.t) list -> CallArgument.t list

val build_method_decl :
  string -> string -> Predicate.parent_container_type -> MethodDeclaration.t

val build_attributes :
  Full_fidelity_source_text.t ->
  ('a, 'b) Aast.user_attribute list ->
  UserAttribute.t list

(* params come with the string representation for their type
   and an option type_xref fact *)
val build_signature :
  Provider_context.t ->
  Full_fidelity_source_text.t ->
  (('a, 'b) Aast.fun_param * Fact_id.t option * string option) list ->
  Aast.contexts option ->
  ret_ty:string option ->
  return_info:Fact_id.t option ->
  Signature.t

val build_type_param :
  Provider_context.t ->
  Full_fidelity_source_text.t ->
  ('a, 'b) Aast.tparam ->
  TypeParameter.t

val build_xrefs : (XRefTarget.t * Pos.t list) Fact_id.Map.t -> XRef.t list

val build_hint_xrefs : (XRefTarget.t * Util.pos list) list -> XRef.t list

val build_module_membership : Fact_id.t -> internal:bool -> ModuleMembership.t
