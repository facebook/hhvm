(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** variance global environment *)
type vgenv = {
  ctx: Provider_context.t;
  tracing_info: Decl_counters.tracing_info;
}

val make_vgenv : Provider_context.t -> Relative_path.t -> vgenv

(** Check a class definition for correct usage of variant generic parameters *)
val class_def :
  vgenv -> string -> Decl_provider.Class.t -> Typing_defs.decl_ty list -> unit

(** Check a type definition for correct usage of variant generic parameters *)
val typedef : vgenv -> string -> unit

type variance

val make_tparam_variance : Typing_defs.decl_tparam -> variance

(** Get (positive,negative) occurrences of generic parameters to a method or function
 * in the function's type.
 *
 * The [variance SMap.t] parameter contains the variance information for generic parameters
 * from the enclosing class, if any.
 *)
val get_typarams :
  vgenv ->
  Typing_deps.Dep.dependent Typing_deps.Dep.variant * 'a ->
  variance SMap.t ->
  Typing_defs.decl_ty ->
  Typing_reason.t list SMap.t * Typing_reason.t list SMap.t
