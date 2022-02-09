(*
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *)

type class_entries = Decl_defs.decl_class_type * Decl_store.class_members option

type lazy_member_lookup_error =
  | LMLEShallowClassNotFound
  | LMLEMemberNotFound
[@@deriving show]

(** Check if the class is already in the heap, and if not,
    declare it, its members and its ancestors and add them to
    their respective shared heaps.
    Return what has been added in the multiple heaps, i.e. the class
    heap entry and the entries in the various member heaps. There is no
    guarantee that it returns all the member heap entries for that class
    as some might have already been added previously when decling the ancestors. *)
val class_decl_if_missing :
  sh:SharedMem.uses -> Provider_context.t -> string -> class_entries option

val class_decl :
  sh:SharedMem.uses ->
  Provider_context.t ->
  Shallow_decl_defs.shallow_class ->
  parents:Decl_store.class_entries SMap.t ->
  Decl_defs.decl_class_type * Decl_store.class_members

(** Extract the constructor signature from the shallow class.

Might return [Error] if the shallow class for [elt_origin] can't be found, or
if it has no constructor. *)
val constructor_decl_lazy :
  sh:SharedMem.uses ->
  Provider_context.t ->
  elt_origin:string ->
  (Typing_defs.fun_elt, lazy_member_lookup_error) result

(** Extract the property signature from the shallow class.

Might return [Error] if the shallow class for [elt_origin] can't be found, or
if it has no property with the given name. *)
val prop_decl_lazy :
  sh:SharedMem.uses ->
  Provider_context.t ->
  elt_origin:string ->
  sp_name:string ->
  (Typing_defs.decl_ty, lazy_member_lookup_error) result

(** Extract the static property signature from the shallow class.

Might return [Error] if the shallow class for [elt_origin] can't be found, or
if it has no static property with the given name. *)
val static_prop_decl_lazy :
  sh:SharedMem.uses ->
  Provider_context.t ->
  elt_origin:string ->
  sp_name:string ->
  (Typing_defs.decl_ty, lazy_member_lookup_error) result

(** Extract the method signature from the shallow class.

Might return [Error] if the shallow class for [elt_origin] can't be found, or
if it has no method with the given name. *)
val method_decl_lazy :
  sh:SharedMem.uses ->
  Provider_context.t ->
  is_static:bool ->
  elt_origin:string ->
  sm_name:string ->
  (Typing_defs.fun_elt, lazy_member_lookup_error) result
