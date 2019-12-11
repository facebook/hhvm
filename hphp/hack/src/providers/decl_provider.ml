(*
 * Copyright (c) 2016, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

module Class = Decl_provider_ctx.Class

type fun_key = Decl_provider_ctx.fun_key

type class_key = Decl_provider_ctx.class_key

type record_def_key = Decl_provider_ctx.record_def_key

type typedef_key = Decl_provider_ctx.typedef_key

type gconst_key = Decl_provider_ctx.gconst_key

type fun_decl = Decl_provider_ctx.fun_decl

type class_decl = Decl_provider_ctx.class_decl

type record_def_decl = Decl_provider_ctx.record_def_decl

type typedef_decl = Decl_provider_ctx.typedef_decl

type gconst_decl = Decl_provider_ctx.gconst_decl

let get_fun name =
  Decl_provider_ctx.get_fun
    (Provider_context.get_global_context_or_empty_FOR_MIGRATION ())
    name

let get_class name =
  Decl_provider_ctx.get_class
    (Provider_context.get_global_context_or_empty_FOR_MIGRATION ())
    name

let get_class_constructor name =
  Decl_provider_ctx.get_class_constructor
    (Provider_context.get_global_context_or_empty_FOR_MIGRATION ())
    name

let get_class_method name =
  Decl_provider_ctx.get_class_method
    (Provider_context.get_global_context_or_empty_FOR_MIGRATION ())
    name

let get_static_method name =
  Decl_provider_ctx.get_static_method
    (Provider_context.get_global_context_or_empty_FOR_MIGRATION ())
    name

let get_typedef name =
  Decl_provider_ctx.get_typedef
    (Provider_context.get_global_context_or_empty_FOR_MIGRATION ())
    name

let get_record_def name =
  Decl_provider_ctx.get_record_def
    (Provider_context.get_global_context_or_empty_FOR_MIGRATION ())
    name

let get_gconst name =
  Decl_provider_ctx.get_gconst
    (Provider_context.get_global_context_or_empty_FOR_MIGRATION ())
    name

let invalidate_fun name =
  Decl_provider_ctx.invalidate_fun
    (Provider_context.get_global_context_or_empty_FOR_MIGRATION ())
    name

let invalidate_class name =
  Decl_provider_ctx.invalidate_class
    (Provider_context.get_global_context_or_empty_FOR_MIGRATION ())
    name

let invalidate_record_def name =
  Decl_provider_ctx.invalidate_record_def
    (Provider_context.get_global_context_or_empty_FOR_MIGRATION ())
    name

let invalidate_typedef name =
  Decl_provider_ctx.invalidate_typedef
    (Provider_context.get_global_context_or_empty_FOR_MIGRATION ())
    name

let invalidate_gconst name =
  Decl_provider_ctx.invalidate_gconst
    (Provider_context.get_global_context_or_empty_FOR_MIGRATION ())
    name

let local_changes_push_stack () =
  Decl_provider_ctx.local_changes_push_stack
    (Provider_context.get_global_context_or_empty_FOR_MIGRATION ())

let local_changes_pop_stack () =
  Decl_provider_ctx.local_changes_pop_stack
    (Provider_context.get_global_context_or_empty_FOR_MIGRATION ())
