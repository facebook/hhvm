(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type fun_key = Decl_provider_ctx.fun_key

type class_key = Decl_provider_ctx.class_key

type record_def_key = Decl_provider_ctx.record_def_key

type typedef_key = Decl_provider_ctx.typedef_key

type gconst_key = Decl_provider_ctx.gconst_key

module Class : sig
  include module type of Decl_provider_ctx.Class
end

type fun_decl = Decl_provider_ctx.fun_decl

type class_decl = Decl_provider_ctx.class_decl

type record_def_decl = Decl_provider_ctx.record_def_decl

type typedef_decl = Decl_provider_ctx.typedef_decl

type gconst_decl = Decl_provider_ctx.gconst_decl

val get_fun : fun_key -> fun_decl option

val get_class : class_key -> class_decl option

val get_class_constructor : class_key -> fun_decl option

val get_class_method : class_key -> fun_key -> fun_decl option

val get_static_method : class_key -> fun_key -> fun_decl option

val get_record_def : record_def_key -> record_def_decl option

val get_typedef : typedef_key -> typedef_decl option

val get_gconst : gconst_key -> gconst_decl option

val invalidate_fun : fun_key -> unit

val invalidate_class : class_key -> unit

val invalidate_record_def : record_def_key -> unit

val invalidate_typedef : typedef_key -> unit

val invalidate_gconst : gconst_key -> unit

val local_changes_push_stack : unit -> unit

val local_changes_pop_stack : unit -> unit
