(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)
type fun_key = string

type class_key = string

type typedef_key = string

type gconst_key = string

module Class : sig
  include module type of Typing_classes_heap.Api
end

type fun_decl = Typing_defs.decl_fun_type

type class_decl = Class.t

type typedef_decl = Typing_defs.typedef_type

type gconst_decl = Typing_defs.decl_ty * Errors.t

val get_fun : fun_key -> fun_decl option

val get_class : class_key -> class_decl option

val get_class_constructor : class_key -> fun_decl option

val get_class_method : class_key -> fun_key -> fun_decl option

val get_static_method : class_key -> fun_key -> fun_decl option

val get_typedef : typedef_key -> typedef_decl option

val get_gconst : gconst_key -> gconst_decl option

val invalidate_fun : fun_key -> unit

val invalidate_class : class_key -> unit

val invalidate_typedef : typedef_key -> unit

val invalidate_gconst : gconst_key -> unit

val local_changes_push_stack : unit -> unit

val local_changes_pop_stack : unit -> unit
