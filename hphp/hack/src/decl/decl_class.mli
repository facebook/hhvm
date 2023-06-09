(*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

exception Decl_heap_elems_bug of string

(** Lookup the type signature of a class property.

    [Decl_defs.element] only points to where the type signature of an element
    can be found (i.e. in which class). This function looks up the actual type
    signature and returns it, together with the original metadata.

    The type signature is retrieved from the member heaps. If it is not present
    and a [Provider_context.t] is passed, the member will be extracting from
    disk. If no [Provider_context.t] is passed or it cannot be found on disk
    a [Decl_heap_elems_bug] exception is raised.

    Note that the lookup is lazy. The type signature is wrapped in a lazy value.
    The lookup will only happen when you try to force the lazy value. (See the
    definition of [Typing_defs.class_elt].) *)
val lookup_property_type_lazy :
  Provider_context.t option ->
  Decl_defs.decl_class_type ->
  string ->
  Decl_defs.element ->
  Typing_defs.class_elt

(** Lookup the type of a class static property.

    See [lookup_property_type_lazy] for more information. *)
val lookup_static_property_type_lazy :
  Provider_context.t option ->
  Decl_defs.decl_class_type ->
  string ->
  Decl_defs.element ->
  Typing_defs.class_elt

(** Lookup the type of a class method.

    See [lookup_property_type_lazy] for more information. *)
val lookup_method_type_lazy :
  Provider_context.t option ->
  Decl_defs.decl_class_type ->
  string ->
  Decl_defs.element ->
  Typing_defs.class_elt

(** Lookup the type of a class static method.

    See [lookup_property_type_lazy] for more information. *)
val lookup_static_method_type_lazy :
  Provider_context.t option ->
  Decl_defs.decl_class_type ->
  string ->
  Decl_defs.element ->
  Typing_defs.class_elt

(** Lookup the type of a class constructor.

    See [lookup_property_type_lazy] for more information. *)
val lookup_constructor_lazy :
  Provider_context.t option ->
  child_class_name:string ->
  Decl_defs.subst_context SMap.t ->
  Decl_defs.element option * Typing_defs.consistent_kind ->
  Typing_defs.class_elt option * Typing_defs.consistent_kind
