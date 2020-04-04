(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type fun_key = string

type class_key = string

type record_def_key = string

type typedef_key = string

type gconst_key = string

module Class : sig
  include module type of Typing_classes_heap.Api
end

type fun_decl = Typing_defs.fun_elt

type class_decl = Class.t

type record_def_decl = Typing_defs.record_def_type

type typedef_decl = Typing_defs.typedef_type

type gconst_decl = Typing_defs.decl_ty * Errors.t

val get_fun : Provider_context.t -> fun_key -> fun_decl option

val get_class : Provider_context.t -> class_key -> class_decl option

val get_class_constructor : Provider_context.t -> class_key -> fun_decl option

val get_class_method :
  Provider_context.t -> class_key -> fun_key -> fun_decl option

val get_static_method :
  Provider_context.t -> class_key -> fun_key -> fun_decl option

val get_record_def :
  Provider_context.t -> record_def_key -> record_def_decl option

val get_typedef : Provider_context.t -> typedef_key -> typedef_decl option

val get_gconst : Provider_context.t -> gconst_key -> gconst_decl option

val local_changes_push_sharedmem_stack : unit -> unit

val local_changes_pop_sharedmem_stack : unit -> unit
