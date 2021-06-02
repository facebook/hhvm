(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

(** Provides decls from the configured backend, e.g. shared memory, local memory, service, etc. *)

type fun_key = string

type type_key = string

type gconst_key = string

module Class : sig
  include module type of Typing_classes_heap.Api
end

type fun_decl = Typing_defs.fun_elt

type class_decl = Class.t

type record_def_decl = Typing_defs.record_def_type

type typedef_decl = Typing_defs.typedef_type

type gconst_decl = Typing_defs.const_decl

val prepare_for_typecheck :
  Provider_context.t -> Relative_path.t -> string -> unit

val get_fun :
  ?tracing_info:Decl_counters.tracing_info ->
  Provider_context.t ->
  fun_key ->
  fun_decl option

val get_class :
  ?tracing_info:Decl_counters.tracing_info ->
  Provider_context.t ->
  type_key ->
  class_decl option

val get_record_def :
  ?tracing_info:Decl_counters.tracing_info ->
  Provider_context.t ->
  type_key ->
  record_def_decl option

val get_typedef :
  ?tracing_info:Decl_counters.tracing_info ->
  Provider_context.t ->
  type_key ->
  typedef_decl option

val get_gconst :
  ?tracing_info:Decl_counters.tracing_info ->
  Provider_context.t ->
  gconst_key ->
  gconst_decl option

val local_changes_push_sharedmem_stack : unit -> unit

val local_changes_pop_sharedmem_stack : unit -> unit

val lookup_or_populate_class_cache :
  type_key ->
  (type_key -> Typing_classes_heap.class_t option) ->
  Typing_classes_heap.class_t option
