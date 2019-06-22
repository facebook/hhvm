(**
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

type fun_decl = Typing_defs.decl Typing_defs.fun_type
type class_decl = Class.t
type typedef_decl = Typing_defs.typedef_type
type gconst_decl = Typing_defs.decl Typing_defs.ty * Errors.t

val get_fun : fun_key -> fun_decl option
val get_class : class_key -> class_decl option
val get_typedef : typedef_key -> typedef_decl option
val get_gconst : gconst_key -> gconst_decl option

val local_changes_push_stack : unit -> unit
val local_changes_pop_stack : unit -> unit
