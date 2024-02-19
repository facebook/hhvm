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

open Hack

val call_arguments : (Argument.t option * Pos.t) list -> CallArgument.t list

val method_decl :
  string -> string -> Predicate.parent_container_type -> MethodDeclaration.t

val attributes :
  Full_fidelity_source_text.t ->
  ('a, 'b) Aast.user_attribute list ->
  UserAttribute.t list

(* params come with the string representation for their type
   and an option type_xref fact *)
val signature :
  Provider_context.t ->
  Full_fidelity_source_text.t ->
  (('a, 'b) Aast.fun_param * Fact_id.t option * string option) list ->
  Aast.contexts option ->
  ret_ty:string option ->
  return_info:Fact_id.t option ->
  Signature.t

val type_param :
  Provider_context.t ->
  Full_fidelity_source_text.t ->
  ('a, 'b) Aast.tparam ->
  TypeParameter.t

val xrefs : (XRefTarget.t * Pos.t list) Fact_id.Map.t -> XRef.t list

val hint_xrefs : (XRefTarget.t * Util.pos list) list -> XRef.t list

val module_membership : Fact_id.t -> internal:bool -> ModuleMembership.t
